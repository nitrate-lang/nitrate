////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <core/LibMacro.h>
#include <nitrate-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <quix-lexer/Lib.h>
#include <quix-lexer/Token.h>
#include <quix-prep/Lib.h>

#include <core/Preprocess.hh>
#include <cstddef>
#include <memory>
#include <new>
#include <optional>
#include <qcall/List.hh>
#include <quix-lexer/Base.hh>
#include <sstream>
#include <string_view>

#include "LibMacro.h"

extern "C" {
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

using namespace qcall;

#define MAX_RECURSION_DEPTH 10000

///=============================================================================

qprep_impl_t::Core::~Core() {
  if (L) {
    lua_close(L);
  }
}

static std::string_view ltrim(std::string_view s) {
  s.remove_prefix(std::min(s.find_first_not_of(" \t\n\r"), s.size()));
  return s;
}

static std::string_view rtrim(std::string_view s) {
  s.remove_suffix(std::min(s.size() - s.find_last_not_of(" \t\n\r") - 1, s.size()));
  return s;
}

bool qprep_impl_t::run_defer_callbacks(qlex_tok_t last) {
  /**
   * @brief We do it this way because the callback could potentially modify the
   * `defer_callbacks` vector, which would invalidate the iterator.
   * The callback is able to instruct us to keep it around for the next token or to
   * remove it.
   *
   * The callback is able to consume and emit token sequences, which may result in this
   * `run_defer_callbacks` function being called recursively.
   *
   * So like, if any of the callbacks says to emit a token, we should emit a token. Otherwise,
   * we should not emit a token.
   */

  std::vector<DeferCallback> saved;
  saved.reserve(m_core->defer_callbacks.size());
  bool emit_token = m_core->defer_callbacks.empty();

  while (!m_core->defer_callbacks.empty()) {
    DeferCallback cb = m_core->defer_callbacks.back();
    m_core->defer_callbacks.pop_back();

    DeferOp op = cb(this, last);
    if (op != DeferOp::UninstallHandler) {
      saved.push_back(cb);
    }
    if (op == DeferOp::EmitToken) {
      emit_token = true;
    }
  }

  m_core->defer_callbacks = saved;

  return emit_token;
}

std::optional<std::string> qprep_impl_t::run_lua_code(const std::string &s) {
  int before_size = lua_gettop(m_core->L);

  int error = luaL_dostring(m_core->L, std::string(s.data(), s.size()).c_str());
  if (error) {
    qcore_print(QCORE_ERROR, "Failed to run Lua code: %s\n", lua_tostring(m_core->L, -1));
    return std::nullopt;
  }

  if (lua_gettop(m_core->L) == before_size) {
    return "";
  } else if (lua_isnil(m_core->L, -1)) {
    return "";
  } else if (lua_isstring(m_core->L, -1)) {
    return lua_tostring(m_core->L, -1);
  } else if (lua_isnumber(m_core->L, -1)) {
    return std::to_string(lua_tonumber(m_core->L, -1));
  } else if (lua_isboolean(m_core->L, -1)) {
    return lua_toboolean(m_core->L, -1) ? "true" : "false";
  } else {
    return std::nullopt;
  }
}

void qprep_impl_t::expand_raw(std::string_view code) {
  auto ss = std::make_shared<std::istringstream>(std::string(code.data(), code.size()));

  {
    qlex_t *clone = weak_clone(ss, m_filename);

    qlex_tok_t tok;
    std::vector<qlex_tok_t> tokens;
    while ((tok = qlex_next(clone)).ty != qEofF) {
      tokens.push_back(tok);
    }

    qlex_free(clone);

    for (auto it = tokens.rbegin(); it != tokens.rend(); it++) {
      m_core->buffer.push_front(*it);
    }
  }
}

bool qprep_impl_t::run_and_expand(const std::string &code) {
  auto res = run_lua_code(code);
  if (!res.has_value()) {
    return false;
  }

  expand_raw(*res);

  return true;
}

class RecursiveGuard {
  size_t &m_depth;

public:
  RecursiveGuard(size_t &depth) : m_depth(depth) { m_depth++; }
  ~RecursiveGuard() { m_depth--; }

  bool should_stop() { return m_depth >= MAX_RECURSION_DEPTH; }
};

CPP_EXPORT qlex_tok_t qprep_impl_t::next_impl() {
func_entry:  // do tail call optimization manually

  qlex_tok_t x{};

  try {
    RecursiveGuard guard(m_depth);
    if (guard.should_stop()) {
      qcore_print(QCORE_FATAL, "Maximum macro recursion depth reached\n");
      throw StopException();
    }

    if (!m_core->buffer.empty()) {
      x = m_core->buffer.front();
      m_core->buffer.pop_front();
    } else {
      x = qlex_t::next_impl();
    }

    if (m_do_expanse) {
      switch (x.ty) {
        case qEofF:
          return x;
        case qErro:
        case qKeyW:
        case qIntL:
        case qText:
        case qChar:
        case qNumL:
        case qOper:
        case qPunc:
        case qNote: {
          goto emit_token;
        }

        case qName: { /* Handle the expansion of defines */
          std::string key = "def." + std::string(get_string(x.v.str_idx));

          const char *value = qcore_env_get(key.c_str());
          if (value == nullptr) {
            goto emit_token;
          }

          expand_raw(value);
          goto func_entry;
        }

        case qMacB: {
          std::string_view block = ltrim(get_string(x.v.str_idx));
          if (!block.starts_with("fn ")) {
            if (!run_and_expand(std::string(block))) {
              qcore_print(QCORE_ERROR, "Failed to expand macro block: %s\n", block.data());
              x.ty = qErro;
              goto emit_token;
            }
          } else {
            block = ltrim(block.substr(3));
            size_t pos = block.find_first_of("(");
            if (pos == std::string_view::npos) {
              qcore_print(QCORE_ERROR, "Invalid macro function definition: %s\n", block.data());
              x.ty = qErro;
              goto emit_token;
            }

            std::string_view name = rtrim(block.substr(0, pos));
            std::string code = "function " + std::string(name) + std::string(block.substr(pos));

            { /* Remove the opening brace */
              pos = code.find_first_of("{");
              if (pos == std::string::npos) {
                qcore_print(QCORE_ERROR, "Invalid macro function definition: %s\n", block.data());
                x.ty = qErro;
                goto emit_token;
              }
              code.erase(pos, 1);
            }

            { /* Remove the closing brace */
              pos = code.find_last_of("}");
              if (pos == std::string::npos) {
                qcore_print(QCORE_ERROR, "Invalid macro function definition: %s\n", block.data());
                x.ty = qErro;
                goto emit_token;
              }
              code.erase(pos, 1);
              code.insert(pos, "end");
            }

            if (!run_and_expand(code)) {
              qcore_print(QCORE_ERROR, "Failed to expand macro function: %s\n", name.data());
              x.ty = qErro;
              goto emit_token;
            }
          }

          goto func_entry;
        }

        case qMacr: {
          std::string_view body = get_string(x.v.str_idx);

          size_t pos = body.find_first_of("(");
          if (pos != std::string_view::npos) {
            if (!run_and_expand("return " + std::string(body))) {
              qcore_print(QCORE_ERROR, "Failed to expand macro function: %s\n", body.data());
              x.ty = qErro;
              goto emit_token;
            }

            goto func_entry;
          } else {
            if (!run_and_expand("return " + std::string(body) + "()")) {
              qcore_print(QCORE_ERROR, "Failed to expand macro function: %s\n", body.data());
              x.ty = qErro;
              goto emit_token;
            }

            goto func_entry;
          }
        }
      }
    }

  emit_token:
    if (!m_do_expanse || run_defer_callbacks(x)) { /* Emit the token */
      return x;
    } else { /* Skip the token */
      goto func_entry;
    }
  } catch (StopException &) {
    x.ty = qEofF;
    return x;
  }
}

void qprep_impl_t::install_lua_api() {
  lua_newtable(m_core->L);

  for (const auto &qcall : qsyscalls) {
    lua_pushinteger(m_core->L, (lua_Integer)(uintptr_t)this);
    lua_pushcclosure(m_core->L, qcall.getFunc(), 1);
    lua_setfield(m_core->L, -2, qcall.getName().data());
  }

  lua_setglobal(m_core->L, "quix");
}

qlex_t *qprep_impl_t::weak_clone(std::shared_ptr<std::istream> file, const char *filename) {
  qprep_impl_t *clone = new qprep_impl_t(file, filename, m_env);

  clone->m_core = m_core;
  clone->replace_interner(m_strings);
  clone->m_flags = m_flags;
  clone->m_depth = m_depth + 1;

  return clone;
}

qprep_impl_t::qprep_impl_t(std::shared_ptr<std::istream> file, const char *filename,
                           qcore_env_t env)
    : qlex_t(file, filename, env) {
  m_core = std::make_shared<Core>();
  m_do_expanse = true;
  m_depth = 0;
}

qprep_impl_t::qprep_impl_t(std::shared_ptr<std::istream> file, qcore_env_t env,
                           const char *filename)
    : qlex_t(file, filename, env) {
  m_core = std::make_shared<Core>();
  m_do_expanse = true;
  m_depth = 0;

  { /* Create the Lua state */
    m_core->L = luaL_newstate();

    /* Load the special selection of standard libraries */
    luaL_openlibs(m_core->L);

    /* Install the QUIX API */
    install_lua_api();
  }

  // Run the standard language prefix
  expand_raw(quix_code_prefix);
}

CPP_EXPORT qprep_impl_t::~qprep_impl_t() {}

///=============================================================================

CPP_EXPORT qlex_t *qprep_new(std::shared_ptr<std::istream> file, const char *filename,
                             qcore_env_t env) {
  try {
    return new qprep_impl_t(file, env, filename);
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    qcore_panic("qprep_new: failed to create lexer");
  }
}

LIB_EXPORT void qprep_set_fetch_module(qlex_t *ctx, qprep_fetch_module_t fetch_fn, uintptr_t any) {
  qprep_impl_t *obj = reinterpret_cast<qprep_impl_t *>(ctx);
  std::lock_guard<std::mutex> lock(obj->m_mutex);

  obj->m_fetch_module = {fetch_fn, any};
}
