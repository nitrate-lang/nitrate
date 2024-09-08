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

#include <quix-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <quix-lexer/Lib.h>
#include <quix-lexer/Token.h>
#include <quix-prep/Lib.h>

#include <iostream>

extern "C" {
#include <lua5.4/lauxlib.h>
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
}

#include <functional>
#include <memory>
#include <new>
#include <optional>
#include <queue>
#include <quix-lexer/Base.hh>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/LibMacro.h"

class qprep_impl_t;

typedef std::function<std::string(qprep_impl_t *, std::vector<const char *>)> qsyscall_t;

class QSysCall final {
  std::string_view m_name;
  uint32_t m_id;
  qsyscall_t m_func;

public:
  QSysCall(std::string_view name = "", uint32_t id = 0, qsyscall_t func = nullptr)
      : m_name(name), m_id(id), m_func(func) {}

  std::string_view getName() const { return m_name; }
  uint32_t getId() const { return m_id; }

  std::string call(qprep_impl_t *engine, std::vector<const char *> args);
};

class qprep_impl_t : public qlex_t {
  struct Core {
    std::unordered_map<std::string_view, std::string> defines;
    std::unordered_set<std::string_view> macros_funcs;
    std::vector<QSysCall> qsyscalls;
    std::shared_ptr<lua_State *> L;
  };

  std::queue<qlex_tok_t> m_tok_queue;
  std::shared_ptr<Core> m_core;
  qlex_t *m_inner;

  enum class Level {
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
  };

  void emit_message(Level level, std::string_view format, ...) {
    /// TODO: Implement message emitter

    const std::unordered_map<Level, const char *> level_names = {
        {Level::Debug, "DEBUG"}, {Level::Info, "INFO"},   {Level::Warn, "WARN"},
        {Level::Error, "ERROR"}, {Level::Fatal, "FATAL"},
    };

    fprintf(stderr, "%s: ", level_names.at(level));

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format.data(), args);
    va_end(args);
  }

  std::optional<std::string> run_lua_code(std::string_view s) {
    int error;

    error = luaL_dostring(*m_core->L, s.data());
    if (error) {
      emit_message(Level::Error, "Failed to run Lua code: %s\n", lua_tostring(*m_core->L, -1));
      return std::nullopt;
    }

    if (lua_isnil(*m_core->L, -1)) {
      return "";
    } else if (lua_isstring(*m_core->L, -1)) {
      return lua_tostring(*m_core->L, -1);
    } else if (lua_isnumber(*m_core->L, -1)) {
      return std::to_string(lua_tonumber(*m_core->L, -1));
    } else if (lua_isboolean(*m_core->L, -1)) {
      return lua_toboolean(*m_core->L, -1) ? "true" : "false";
    } else {
      emit_message(Level::Error, "Invalid Lua return value: %s\n",
                   lua_typename(*m_core->L, lua_type(*m_core->L, -1)));
      return std::nullopt;
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

  bool run_and_expand(std::string_view code) {
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
        m_core->macros_funcs.insert(name);
      }
    }

    FILE *resbuf = fmemopen((void *)res->data(), res->size(), "r");
    if (resbuf == nullptr) {
      qcore_panic("qprep_impl_t::next_impl: failed to create a memory buffer");
    }

    {
      qlex_t *clone = weak_clone(resbuf, m_filename);

      qlex_tok_t tok;
      while ((tok = qlex_next(clone)).ty != qEofF) {
        m_tok_queue.push(tok);
      }

      qlex_free(clone);
    }

    fclose(resbuf);

    return true;
  }

  virtual qlex_tok_t next_impl() override {
    if (!m_tok_queue.empty()) {
      qlex_tok_t x = m_tok_queue.front();
      m_tok_queue.pop();
      return x;
    }

    qlex_tok_t x = m_inner->next_impl();

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
        /// Just pass through
        return x;
      }
      case qName: {
        std::string_view name = get_string(x.v.str_idx);

        if (m_core->defines.find(name) != m_core->defines.end()) {
          x.ty = qText;
          x.v.str_idx = put_string((m_core->defines)[name]);
        }

        return x;
      }
      case qMacB: {
        std::string_view block = get_string(x.v.str_idx);
        block = ltrim(block);
        if (!block.starts_with("fn ")) {
          if (!run_and_expand(block)) {
            return x;
          }
        } else {
          block = block.substr(3);
          block = ltrim(block);
          size_t pos = block.find_first_of("(");
          if (pos == std::string_view::npos) {
            emit_message(Level::Error, "Invalid macro function definition: %s\n", block.data());
            return x;
          }

          std::string_view name = block.substr(0, pos);
          name = rtrim(name);

          std::string code = "function " + std::string(name) + std::string(block.substr(pos));

          { /* Remove the opening brace */
            pos = code.find_first_of("{");
            if (pos == std::string::npos) {
              emit_message(Level::Error, "Invalid macro function definition: %s\n", block.data());
              return x;
            }
            code.erase(pos, 1);
          }

          { /* Remove the closing brace */
            pos = code.find_last_of("}");
            if (pos == std::string::npos) {
              emit_message(Level::Error, "Invalid macro function definition: %s\n", block.data());
              return x;
            }
            code.erase(pos, 1);
            code.insert(pos, "end");
          }

          std::string_view sv = get_string(put_string(code));

          if (!run_and_expand(sv)) {
            return x;
          }
        }

        return next_impl();
      }
      case qMacr: {
        std::string_view body = get_string(x.v.str_idx);

        size_t pos = body.find_first_of("(");
        if (pos != std::string_view::npos) {
          std::string_view name = body.substr(0, pos);

          if (!m_core->macros_funcs.contains(name)) {
            emit_message(Level::Error, "Undefined macro function: %s\n", name.data());
            return x;
          }

          std::string code = std::string(body);

          // Conv arguments to string type

          code = "return " + code;
          if (!run_and_expand(code)) {
            return x;
          }

          return next_impl();
        } else {
          /// TODO: Object-like macro
        }
        return next_impl();
      }
    }

    __builtin_unreachable();
  }

  void setup_qsyscalls() {
    /// TODO: Write qsyscalls

#define add_qsyscall(name, id, func) m_core->qsyscalls.emplace_back(name, id, func)

    add_qsyscall("get_lexer_version", 0x0000, [](qprep_impl_t *, std::vector<const char *>) {
      /**
        @brief Get the version of the lexer library.
       */

      return qlex_lib_version();
    });

    add_qsyscall("get_preprocessor_version", 0x0001, [](qprep_impl_t *, std::vector<const char *>) {
      /**
        @brief Get the version of the lexer library.
       */

      return qprep_lib_version();
    });
  }

  void install_lua_api() {
    setup_qsyscalls();

    lua_newtable(*m_core->L);
    lua_newtable(*m_core->L);

    for (auto qcall : m_core->qsyscalls) {
      std::string_view name = qcall.getName();

      lua_pushinteger(*m_core->L, (lua_Integer)(uintptr_t)this);
      lua_pushinteger(*m_core->L, qcall.getId());

      lua_pushcclosure(
          *m_core->L,
          [](lua_State *L) -> int {
            qprep_impl_t *engine;
            uint32_t call_num;

            { /* Get closure upvalues */
              int engine_idx = lua_upvalueindex(1);
              int call_num_idx = lua_upvalueindex(2);

              engine = (qprep_impl_t *)(uintptr_t)luaL_checkinteger(L, engine_idx);
              call_num = luaL_checkinteger(L, call_num_idx);
            }

            // Detect how many arguments are passed
            int nargs = lua_gettop(L);
            std::vector<const char *> args;
            for (int i = 1; i <= nargs; i++) {
              if (lua_isstring(L, i) || lua_isnumber(L, i)) {
                args.push_back(lua_tostring(L, i));
              } else if (lua_isboolean(L, i)) {
                args.push_back(lua_toboolean(L, i) ? "1" : "0");
              } else {
                return luaL_error(L, "Invalid argument #%d", i);
              }
            }

            std::string result = engine->m_core->qsyscalls.at(call_num).call(engine, args);

            /* Return a single string result */
            lua_pushstring(L, result.c_str());

            return 1;
          },
          2);
      lua_setfield(*m_core->L, -2, name.data());
    }

    lua_setfield(*m_core->L, -2, "api");
    lua_setglobal(*m_core->L, "quix");
  }

  void replace_interner(StringInterner new_interner) override {
    m_strings = new_interner;
    m_inner->replace_interner(new_interner);
  }

  qlex_t *weak_clone(FILE *file, const char *filename) const {
    qprep_impl_t *clone = new qprep_impl_t(file, filename);

    clone->m_core = m_core;
    clone->replace_interner(m_strings);

    return clone;
  }

public:
  qprep_impl_t(FILE *file, const char *filename) : qlex_t(file, filename, false) {
    m_inner = nullptr;
    m_core = std::make_shared<Core>();

    { /* Create the inner lexer */
      qlex_t *inner = nullptr;
      if ((inner = qlex_new(file, filename)) == nullptr) {
        throw std::bad_alloc();
      }

      m_inner = inner;
    }

    /* Bind the inner string interner to the outer one
       so that external calls will work properly. */
    replace_interner(m_inner->m_strings);
  }

  qprep_impl_t(FILE *file, const char *filename, bool is_owned) : qlex_t(file, filename, is_owned) {
    m_core = std::make_shared<Core>();
    m_inner = nullptr;

    { /* Create the inner lexer */
      qlex_t *inner = nullptr;
      if ((inner = qlex_new(file, filename)) == nullptr) {
        throw std::bad_alloc();
      }

      m_inner = inner;
    }

    /* Bind the inner string interner to the outer one
       so that external calls will work properly. */
    replace_interner(m_inner->m_strings);

    { /* Create the Lua state */
      lua_State **L = new lua_State *(luaL_newstate());
      if ((m_core->L = std::shared_ptr<lua_State *>(L, [](lua_State **p) {
             lua_close(*p);
             delete p;
           })) == nullptr) {
        throw std::bad_alloc();
      }

      /* Load the special selection of standard libraries */
      luaL_openlibs(*m_core->L);

      /* Install the QUIX API */
      install_lua_api();
    }
  }

  virtual ~qprep_impl_t() { qlex_free(m_inner); }
};

LIB_EXPORT qlex_t *qprep_new(FILE *file, const char *filename) {
  try {
    return new qprep_impl_t(file, filename, false);
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    qcore_panic("qprep_new: failed to create lexer");
  }
}

std::string QSysCall::call(qprep_impl_t *engine, std::vector<const char *> args) {
  qcore_assert(engine != nullptr && m_func != nullptr);

  return m_func(engine, args);
}
