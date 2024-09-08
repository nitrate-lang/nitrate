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
#include <qcall/List.hh>

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
#include <unordered_map>
#include <unordered_set>

using namespace qcall;

#define get_engine() (qprep_impl_t *)(uintptr_t)luaL_checkinteger(L, lua_upvalueindex(1))

///=============================================================================

qprep_impl_t::Core::~Core() {
  if (L) {
    lua_close(L);
  }
}

void qprep_impl_t::emit_message(Level level, std::string_view format, ...) {
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

qlex_tok_t qprep_impl_t::next_impl() {
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

        std::string code = "return " + std::string(body);
        if (!run_and_expand(code)) {
          return x;
        }

        return next_impl();
      } else {
        if (!m_core->macros_funcs.contains(body)) {
          emit_message(Level::Error, "Undefined macro function: %s\n", body.data());
          return x;
        }

        std::string code = "return " + std::string(body);
        if (!run_and_expand(code)) {
          return x;
        }

        return next_impl();
      }
    }
  }

  __builtin_unreachable();
}

void qprep_impl_t::install_lua_api() {
  lua_newtable(m_core->L);
  lua_newtable(m_core->L);

  for (const auto &qcall : qsyscalls) {
    lua_pushinteger(m_core->L, (lua_Integer)(uintptr_t)this);
    lua_pushcclosure(m_core->L, qcall.getFunc(), 1);
    lua_setfield(m_core->L, -2, qcall.getName().data());
  }

  lua_setfield(m_core->L, -2, "api");
  lua_setglobal(m_core->L, "quix");
}

qlex_t *qprep_impl_t::weak_clone(FILE *file, const char *filename) const {
  qprep_impl_t *clone = new qprep_impl_t(file, filename);

  clone->m_core = m_core;
  clone->replace_interner(m_strings);

  return clone;
}

void qprep_impl_t::replace_interner(StringInterner new_interner) {
  m_strings = new_interner;
  m_inner->replace_interner(new_interner);
}

qprep_impl_t::qprep_impl_t(FILE *file, const char *filename) : qlex_t(file, filename, false) {
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

qprep_impl_t::qprep_impl_t(FILE *file, const char *filename, bool is_owned)
    : qlex_t(file, filename, is_owned) {
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

qprep_impl_t::~qprep_impl_t() { qlex_free(m_inner); }

///=============================================================================

LIB_EXPORT qlex_t *qprep_new(FILE *file, const char *filename) {
  try {
    return new qprep_impl_t(file, filename, false);
  } catch (std::bad_alloc &) {
    return nullptr;
  } catch (...) {
    qcore_panic("qprep_new: failed to create lexer");
  }
}
