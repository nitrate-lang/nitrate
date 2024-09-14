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

#define __QUIX_IMPL__

#include <core/LibMacro.h>
#include <quix-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <quix-lexer/Lib.h>
#include <quix-lexer/Token.h>
#include <quix-prep/Lib.h>

#include <core/Preprocess.hh>
#include <cstddef>
#include <qcall/List.hh>

#include "quix-core/Env.h"

extern "C" {
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

#include <memory>
#include <new>
#include <optional>
#include <quix-lexer/Base.hh>
#include <string_view>

using namespace qcall;

#define MAX_RECURSION_DEPTH 256

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

std::optional<std::string> qprep_impl_t::run_lua_code(std::string_view s) {
  int error;

  error = luaL_dostring(m_core->L, s.data());
  if (error) {
    qcore_print(QCORE_ERROR, "Failed to run Lua code: %s\n", lua_tostring(m_core->L, -1));
    return std::nullopt;
  }

  if (lua_isnil(m_core->L, -1)) {
    return "";
  } else if (lua_isstring(m_core->L, -1)) {
    return lua_tostring(m_core->L, -1);
  } else if (lua_isnumber(m_core->L, -1)) {
    return std::to_string(lua_tonumber(m_core->L, -1));
  } else if (lua_isboolean(m_core->L, -1)) {
    return lua_toboolean(m_core->L, -1) ? "true" : "false";
  } else {
    qcore_print(QCORE_ERROR, "Invalid Lua return value: %s\n",
                lua_typename(m_core->L, lua_type(m_core->L, -1)));
    return std::nullopt;
  }
}

void qprep_impl_t::expand_raw(std::string_view code) {
  FILE *resbuf = fmemopen((void *)code.data(), code.size(), "r");
  if (resbuf == nullptr) {
    qcore_panic("qprep_impl_t::next_impl: failed to create a memory buffer");
  }

  {
    qlex_t *clone = weak_clone(resbuf, m_filename);

    qlex_tok_t tok;
    while ((tok = qlex_next(clone)).ty != qEofF) {
      qlex_insert(this, tok);
    }

    qlex_free(clone);
  }

  fclose(resbuf);
}

bool qprep_impl_t::run_and_expand(std::string_view code) {
  auto res = run_lua_code(code);
  if (!res.has_value()) {
    return false;
  }

  code = ltrim(code);

  if (code.starts_with("function ")) {
    size_t pos = code.find_first_of("(");
    if (pos != std::string_view::npos) {
      std::string_view name = code.substr(9, pos - 9);
      name = rtrim(name);
    }
  }

  if (res->empty()) {
    return true;
  }

  expand_raw(*res);

  return true;
}

void qprep_impl_t::eof_callback() { qlex_t::eof_callback(); }

class RecursiveGuard {
  size_t &m_depth;

public:
  RecursiveGuard(size_t &depth) : m_depth(depth) { m_depth++; }
  ~RecursiveGuard() { m_depth--; }

  bool should_stop() { return m_depth >= MAX_RECURSION_DEPTH; }
};

qlex_tok_t qprep_impl_t::next_impl() {
  qlex_tok_t x;

  try {
    RecursiveGuard guard(m_depth);

    if (guard.should_stop()) {
      qcore_print(QCORE_FATAL, "Maximum macro recursion depth reached\n");
      throw StopException();
    }

    x = qlex_t::next_impl();

    if (m_do_expanse) {
      switch (x.ty) {
        case qEofF:
        case qErro:
        case qKeyW:
        case qIntL:
        case qText:
        case qChar:
        case qNumL:
        case qOper:
        case qPunc:
        case qNote: {
          /// Just pass x through
          break;
        }
        case qName: {
          std::string key = "def." + std::string(get_string(x.v.str_idx));

          const char *value = qcore_env_get(key.c_str());
          if (value == nullptr) {
            break;
          }

          expand_raw(value);
          x = qlex_next(this);
          break;
        }
        case qMacB: {
          std::string_view block = get_string(x.v.str_idx);
          block = ltrim(block);
          if (!block.starts_with("fn ")) {
            if (!run_and_expand(block)) {
              qcore_print(QCORE_ERROR, "Failed to expand macro block: %s\n", block.data());
              break;
            }
          } else {
            block = block.substr(3);
            block = ltrim(block);
            size_t pos = block.find_first_of("(");
            if (pos == std::string_view::npos) {
              qcore_print(QCORE_ERROR, "Invalid macro function definition: %s\n", block.data());
              break;
            }

            std::string_view name = block.substr(0, pos);
            name = rtrim(name);

            std::string code = "function " + std::string(name) + std::string(block.substr(pos));

            { /* Remove the opening brace */
              pos = code.find_first_of("{");
              if (pos == std::string::npos) {
                qcore_print(QCORE_ERROR, "Invalid macro function definition: %s\n", block.data());
                break;
              }
              code.erase(pos, 1);
            }

            { /* Remove the closing brace */
              pos = code.find_last_of("}");
              if (pos == std::string::npos) {
                qcore_print(QCORE_ERROR, "Invalid macro function definition: %s\n", block.data());
                break;
              }
              code.erase(pos, 1);
              code.insert(pos, "end");
            }

            std::string_view sv = get_string(put_string(code));

            if (!run_and_expand(sv)) {
              qcore_print(QCORE_ERROR, "Failed to expand macro function: %s\n", name.data());
              break;
            }
          }

          x = qlex_next(this);
          break;
        }
        case qMacr: {
          std::string_view body = get_string(x.v.str_idx);

          size_t pos = body.find_first_of("(");
          if (pos != std::string_view::npos) {
            std::string code = "return " + std::string(body);
            if (!run_and_expand(code)) {
              qcore_print(QCORE_ERROR, "Failed to expand macro function: %s\n", body.data());
              break;
            }

            x = qlex_next(this);
            break;
          } else {
            std::string code = "return " + std::string(body) + "()";
            if (!run_and_expand(code)) {
              qcore_print(QCORE_ERROR, "Failed to expand macro function: %s\n", body.data());
              break;
            }

            x = qlex_next(this);
            break;
          }
        }
      }
    }
  } catch (StopException &) {
    x.ty = qEofF;
  }

  return x;
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

qlex_t *qprep_impl_t::weak_clone(FILE *file, const char *filename) {
  qprep_impl_t *clone = new qprep_impl_t(file, filename, m_env);

  clone->m_core = m_core;
  clone->replace_interner(m_strings);
  clone->m_flags = m_flags;
  clone->m_depth = m_depth + 1;

  return clone;
}

qprep_impl_t::qprep_impl_t(FILE *file, const char *filename, qcore_env_t env)
    : qlex_t(file, filename, false, env) {
  m_core = std::make_shared<Core>();
  m_do_expanse = true;
  m_depth = 0;
}

qprep_impl_t::qprep_impl_t(FILE *file, const char *filename, bool is_owned, qcore_env_t env)
    : qlex_t(file, filename, is_owned, env) {
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

qprep_impl_t::~qprep_impl_t() {}

///=============================================================================

LIB_EXPORT qlex_t *qprep_new(FILE *file, const char *filename, qcore_env_t env) {
  try {
    return new qprep_impl_t(file, filename, false, env);
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    qcore_panic("qprep_new: failed to create lexer");
  }
}
