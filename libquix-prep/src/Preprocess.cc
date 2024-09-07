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
#include <quix-lexer/Token.h>

extern "C" {
#include <lua5.4/lauxlib.h>
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
}

#include <functional>
#include <new>
#include <quix-lexer/Base.hh>
#include <string_view>
#include <unordered_map>

#include "LibMacro.h"

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
  qlex_t *m_inner;
  std::unordered_map<std::string_view, std::string> m_defines;
  lua_State *m_L;
  std::vector<QSysCall> m_qsyscalls;

  std::vector<qlex_tok_t> run_lua_code(std::string_view s) {
    std::vector<qlex_tok_t> result;

    if (luaL_dostring(m_L, s.data()) != LUA_OK) {
      const char *err = lua_tostring(m_L, -1);
      qcore_panicf("run_lua_code: %s", err);
    }

    /// TODO: get output into token buffer queue
    return result;
  }

  virtual qlex_tok_t next_impl() override {
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

        if (m_defines.find(name) != m_defines.end()) {
          x.ty = qText;
          x.v.str_idx = put_string(m_defines[name]);
        }

        return x;
      }
      case qMacB: {
        /// TODO: Macro block
        std::string_view code = get_string(x.v.str_idx);

        run_lua_code(code);

        return next_impl();
      }
      case qMacr: {
        std::string_view name = get_string(x.v.str_idx);

        if (name.find('(') != std::string_view::npos) {
          /// TODO: Function-like macro
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
  }

  void install_lua_api() {
    setup_qsyscalls();

    lua_newtable(m_L);
    lua_newtable(m_L);

    for (auto qcall : m_qsyscalls) {
      std::string_view name = qcall.getName();

      lua_pushinteger(m_L, (lua_Integer)(uintptr_t)this);
      lua_pushinteger(m_L, qcall.getId());

      lua_pushcclosure(
          m_L,
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

            std::string result = engine->m_qsyscalls[call_num].call(engine, args);

            /* Return a single string result */
            lua_pushstring(L, result.c_str());

            return 1;
          },
          2);
      lua_setfield(m_L, -2, name.data());
    }

    lua_setfield(m_L, -2, "api");
    lua_setglobal(m_L, "quix");
  }

public:
  qprep_impl_t(FILE *file, const char *filename, bool is_owned)
      : qlex_t(nullptr, filename, is_owned) {
    /* Create the inner lexer */
    if ((m_inner = qlex_new(file, filename)) == nullptr) {
      throw std::bad_alloc();
    }

    /* Bind the inner string interner to the outer one
       so that external calls will work properly. */
    replace_interner(m_inner->m_strings);

    /* Create the Lua state */
    if ((m_L = luaL_newstate()) == nullptr) {
      throw std::bad_alloc();
    }

    /* Load the special selection of standard libraries */
    luaL_openlibs(m_L);

    /* Install the QUIX API */
    install_lua_api();
  }

  virtual ~qprep_impl_t() {
    lua_close(m_L);

    qlex_free(m_inner);
  }
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
