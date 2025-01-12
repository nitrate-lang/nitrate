////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <nitrate-seq/Lib.h>

#include <cstddef>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-lexer/Token.hh>
#include <nitrate-seq/Preprocess.hh>
#include <optional>
#include <qcall/List.hh>
#include <sstream>
#include <string_view>

extern "C" {
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

using namespace ncc::lex;
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
  s.remove_suffix(
      std::min(s.size() - s.find_last_not_of(" \t\n\r") - 1, s.size()));
  return s;
}

bool qprep_impl_t::run_defer_callbacks(Token last) {
  /**
   * @brief We do it this way because the callback could potentially modify the
   * `defer_callbacks` vector, which would invalidate the iterator.
   * The callback is able to instruct us to keep it around for the next token or
   * to remove it.
   *
   * The callback is able to consume and emit token sequences, which may result
   * in this `run_defer_callbacks` function being called recursively.
   *
   * So like, if any of the callbacks says to emit a token, we should emit a
   * token. Otherwise, we should not emit a token.
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
    qcore_logf(QCORE_ERROR, "Lua error: %s\n", lua_tostring(m_core->L, -1));
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
  std::istringstream ss(std::string(code.data(), code.size()));

  {
    std::vector<Token> tokens;

    {
      qprep_impl_t clone(ss, m_env, false);
      clone.m_core = m_core;
      clone.m_core->m_depth = m_core->m_depth + 1;

      Token tok;
      while ((tok = (clone.Next())).get_type() != EofF) {
        tokens.push_back(tok);
      }
    }

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

CPP_EXPORT Token qprep_impl_t::GetNext() {
func_entry:  // do tail call optimization manually

  Token x;

  try {
    RecursiveGuard guard(m_core->m_depth);
    if (guard.should_stop()) {
      qcore_logf(QCORE_FATAL, "Maximum macro recursion depth reached\n");
      throw StopException();
    }

    if (!m_core->buffer.empty()) {
      x = m_core->buffer.front();
      m_core->buffer.pop_front();
    } else {
      x = m_scanner->Next();
    }

    if (m_core->m_do_expanse) {
      switch (x.get_type()) {
        case EofF:
          return x;
        case KeyW:
        case IntL:
        case Text:
        case Char:
        case NumL:
        case Oper:
        case Punc:
        case Note: {
          goto emit_token;
        }

        case Name: { /* Handle the expansion of defines */
          std::string key = "def." + std::string(x.as_string());

          if (let value = m_env->get(key.c_str())) {
            expand_raw(value.value());
            goto func_entry;
          } else {
            goto emit_token;
          }
        }

        case MacB: {
          std::string_view block = ltrim(x.as_string());
          if (!block.starts_with("fn ")) {
            if (!run_and_expand(std::string(block))) {
              qcore_logf(QCORE_ERROR, "Failed to expand macro block: %s\n",
                         block.data());
              x = Token::EndOfFile();
              SetFailBit();

              goto emit_token;
            }
          } else {
            block = ltrim(block.substr(3));
            size_t pos = block.find_first_of("(");
            if (pos == std::string_view::npos) {
              qcore_logf(QCORE_ERROR, "Invalid macro function definition: %s\n",
                         block.data());
              x = Token::EndOfFile();
              SetFailBit();

              goto emit_token;
            }

            std::string_view name = rtrim(block.substr(0, pos));
            std::string code = "function " + std::string(name) +
                               std::string(block.substr(pos));

            { /* Remove the opening brace */
              pos = code.find_first_of("{");
              if (pos == std::string::npos) {
                qcore_logf(QCORE_ERROR,
                           "Invalid macro function definition: %s\n",
                           block.data());
                x = Token::EndOfFile();
                SetFailBit();

                goto emit_token;
              }
              code.erase(pos, 1);
            }

            { /* Remove the closing brace */
              pos = code.find_last_of("}");
              if (pos == std::string::npos) {
                qcore_logf(QCORE_ERROR,
                           "Invalid macro function definition: %s\n",
                           block.data());
                x = Token::EndOfFile();
                SetFailBit();

                goto emit_token;
              }
              code.erase(pos, 1);
              code.insert(pos, "end");
            }

            if (!run_and_expand(code)) {
              qcore_logf(QCORE_ERROR, "Failed to expand macro function: %s\n",
                         name.data());
              x = Token::EndOfFile();
              SetFailBit();

              goto emit_token;
            }
          }

          goto func_entry;
        }

        case Macr: {
          std::string_view body = x.as_string();

          size_t pos = body.find_first_of("(");
          if (pos != std::string_view::npos) {
            if (!run_and_expand("return " + std::string(body))) {
              qcore_logf(QCORE_ERROR, "Failed to expand macro function: %s\n",
                         body.data());
              x = Token::EndOfFile();
              SetFailBit();

              goto emit_token;
            }

            goto func_entry;
          } else {
            if (!run_and_expand("return " + std::string(body) + "()")) {
              qcore_logf(QCORE_ERROR, "Failed to expand macro function: %s\n",
                         body.data());
              x = Token::EndOfFile();
              SetFailBit();

              goto emit_token;
            }

            goto func_entry;
          }
        }
      }
    }

  emit_token:
    if (!m_core->m_do_expanse || run_defer_callbacks(x)) { /* Emit the token */
      return x;
    } else { /* Skip the token */
      goto func_entry;
    }
  } catch (StopException &) {
    x = Token::EndOfFile();
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

  lua_setglobal(m_core->L, "n");
}

CPP_EXPORT qprep_impl_t::qprep_impl_t(std::istream &file,
                                      std::shared_ptr<ncc::Environment> env,
                                      bool is_root)
    : ncc::lex::IScanner(env) {
  m_core = std::make_shared<Core>();
  m_scanner = std::make_unique<Tokenizer>(file, env);

  if (is_root) {
    { /* Create the Lua state */
      m_core->L = luaL_newstate();

      { /* Load the special selection of standard libraries */
        static const luaL_Reg loadedlibs[] = {
            {LUA_GNAME, luaopen_base},
            // {LUA_LOADLIBNAME, luaopen_package},
            // {LUA_COLIBNAME, luaopen_coroutine},
            {LUA_TABLIBNAME, luaopen_table},
            {LUA_IOLIBNAME, luaopen_io},
            // {LUA_OSLIBNAME, luaopen_os},
            {LUA_STRLIBNAME, luaopen_string},
            {LUA_MATHLIBNAME, luaopen_math},
            {LUA_UTF8LIBNAME, luaopen_utf8},
            {LUA_DBLIBNAME, luaopen_debug},
            {NULL, NULL}};

        const luaL_Reg *lib;
        /* "require" functions from 'loadedlibs' and set results to global table
         */
        for (lib = loadedlibs; lib->func; lib++) {
          luaL_requiref(m_core->L, lib->name, lib->func, 1);
          lua_pop(m_core->L, 1); /* remove lib */
        }
      }

      /* Install the Nitrate API */
      install_lua_api();
    }

    // Run the standard language prefix
    expand_raw(nit_code_prefix);
  }
}

CPP_EXPORT qprep_impl_t::~qprep_impl_t() {}

CPP_EXPORT
std::optional<std::vector<std::string>> qprep_impl_t::GetSourceWindow(
    Point start, Point end, char fillchar) {
  return m_scanner->GetSourceWindow(start, end, fillchar);
}
