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

#include <quix-core/Error.h>
#include <quix-lexer/Lexer.h>
#include <quix-lexer/Lib.h>
#include <quix-lexer/Token.h>
#include <quix-prep/Lib.h>

extern "C" {
#include <lua5.4/lauxlib.h>
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
}

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

typedef int (*qsyscall_t)(lua_State *L);

class QSysCall final {
  std::string_view m_name;
  uint32_t m_id;
  qsyscall_t m_func;

public:
  QSysCall(std::string_view name = "", uint32_t id = 0, qsyscall_t func = nullptr)
      : m_name(name), m_id(id), m_func(func) {}

  std::string_view getName() const { return m_name; }
  uint32_t getId() const { return m_id; }
  qsyscall_t getFunc() const { return m_func; }
};

struct qprep_impl_t final : public qlex_t {
private:
  struct Core {
    std::unordered_map<std::string_view, std::string> defines;
    std::unordered_set<std::string_view> macros_funcs;
    std::vector<QSysCall> qsyscalls;
    lua_State *L;

    ~Core() {
      if (L) {
        lua_close(L);
      }
    }
  };

  std::shared_ptr<Core> m_core;
  qlex_t *m_inner;
  bool m_do_expanse;

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

    error = luaL_dostring(m_core->L, s.data());
    if (error) {
      emit_message(Level::Error, "Failed to run Lua code: %s\n", lua_tostring(m_core->L, -1));
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
      emit_message(Level::Error, "Invalid Lua return value: %s\n",
                   lua_typename(m_core->L, lua_type(m_core->L, -1)));
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

    if (res->empty()) {
      return true;
    }

    FILE *resbuf = fmemopen((void *)res->data(), res->size(), "r");
    if (resbuf == nullptr) {
      qcore_panic("qprep_impl_t::next_impl: failed to create a memory buffer");
    }

    {
      qlex_t *clone = weak_clone(resbuf, m_filename);

      qlex_tok_t tok;
      while ((tok = qlex_next(clone)).ty != qEofF) {
        qlex_push(m_inner, tok);
      }

      qlex_free(clone);
    }

    fclose(resbuf);

    return true;
  }

  virtual qlex_tok_t next_impl() override {
    qlex_tok_t x = m_inner->next_impl();

    if (!m_do_expanse) {
      return x;
    }

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
#define add_qsyscall(__name, __id, __func) m_core->qsyscalls.emplace_back(__name, __id, __func)
#define get_engine() (qprep_impl_t *)(uintptr_t)luaL_checkinteger(L, lua_upvalueindex(1))

    static const std::unordered_map<std::string_view, std::string_view (*)(void)> comp_versions;

    add_qsyscall("verof", 0x0000, [](lua_State *L) {
      /**
        @brief Get the version of a component.
       */

      int nargs = lua_gettop(L);
      if (nargs != 1) {
        return luaL_error(L, "Invalid number of arguments: expected 1, got %d", nargs);
      }

      if (!lua_isstring(L, 1)) {
        return luaL_error(L, "Invalid argument #1: expected string, got %s",
                          lua_typename(L, lua_type(L, 1)));
      }

      std::string_view name = lua_tostring(L, 1);

      if (comp_versions.contains(name)) {
        lua_pushstring(L, comp_versions.at(name)().data());
        return 1;
      }

      luaL_error(L, "Unknown component: %s", name.data());
      return 0;
    });

    add_qsyscall("next", 0x0001, [](lua_State *L) {
      /**
        @brief Next token.
       */

      qprep_impl_t *obj = get_engine();

      bool old = obj->m_do_expanse;
      obj->m_do_expanse = false;
      qlex_tok_t tok = qlex_next(obj);
      obj->m_do_expanse = old;

      lua_newtable(L);

      lua_pushstring(L, "ty");
      lua_pushstring(L, qlex_ty_str(tok.ty));
      lua_settable(L, -3);

      lua_pushstring(L, "v");
      switch (tok.ty) {
        case qEofF:
        case qErro: {
          lua_pushnil(L);
          break;
        }
        case qKeyW: {
          lua_pushstring(L, qlex_kwstr(tok.v.key));
          break;
        }
        case qOper: {
          lua_pushstring(L, qlex_opstr(tok.v.op));
          break;
        }
        case qPunc: {
          lua_pushstring(L, qlex_punctstr(tok.v.punc));
          break;
        }
        case qIntL:
        case qNumL:
        case qText:
        case qChar:
        case qName:
        case qMacB:
        case qMacr:
        case qNote: {
          lua_pushstring(L, obj->get_string(tok.v.str_idx).data());
          break;
        }
      }

      lua_settable(L, -3);

      return 1;
    });

    add_qsyscall("peek", 0x0002, [](lua_State *L) {
      /**
        @brief Peek token.
       */

      qprep_impl_t *obj = get_engine();

      bool old = obj->m_do_expanse;
      obj->m_do_expanse = false;
      qlex_tok_t tok = qlex_peek(obj);
      obj->m_do_expanse = old;

      lua_newtable(L);

      lua_pushstring(L, "ty");
      lua_pushstring(L, qlex_ty_str(tok.ty));
      lua_settable(L, -3);

      lua_pushstring(L, "v");
      switch (tok.ty) {
        case qEofF:
        case qErro: {
          lua_pushnil(L);
          break;
        }
        case qKeyW: {
          lua_pushstring(L, qlex_kwstr(tok.v.key));
          break;
        }
        case qOper: {
          lua_pushstring(L, qlex_opstr(tok.v.op));
          break;
        }
        case qPunc: {
          lua_pushstring(L, qlex_punctstr(tok.v.punc));
          break;
        }
        case qIntL:
        case qNumL:
        case qText:
        case qChar:
        case qName:
        case qMacB:
        case qMacr:
        case qNote: {
          lua_pushstring(L, obj->get_string(tok.v.str_idx).data());
          break;
        }
      }

      lua_settable(L, -3);

      return 1;
    });
  }

  void install_lua_api() {
    setup_qsyscalls();

    lua_newtable(m_core->L);
    lua_newtable(m_core->L);

    for (const auto &qcall : m_core->qsyscalls) {
      lua_pushinteger(m_core->L, (lua_Integer)(uintptr_t)this);
      lua_pushcclosure(m_core->L, qcall.getFunc(), 1);
      lua_setfield(m_core->L, -2, qcall.getName().data());
    }

    lua_setfield(m_core->L, -2, "api");
    lua_setglobal(m_core->L, "quix");
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
    m_do_expanse = true;

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
    m_do_expanse = true;

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
      m_core->L = luaL_newstate();

      /* Load the special selection of standard libraries */
      luaL_openlibs(m_core->L);

      /* Install the QUIX API */
      install_lua_api();
    }
  }

  virtual ~qprep_impl_t() override { qlex_free(m_inner); }
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
