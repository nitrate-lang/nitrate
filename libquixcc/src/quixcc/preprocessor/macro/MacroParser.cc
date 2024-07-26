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

#define QUIXCC_INTERNAL

#include "LibMacro.h"
#include "parser/Parse.h"
#include <quixcc/core/Logger.h>
#include <quixcc/preprocessor/Preprocessor.h>

extern "C" {
#include <lua5.4/lauxlib.h>
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
}

#include <map>
#include <stdexcept>

/// TODO: Macro call type checking

using namespace libquixcc;

class LuaEngine {
  lua_State *m_L;
  quixcc_cc_job_t *m_job;
  quixcc_engine_t *m_engine;

  void bind_function(const char *name, lua_CFunction func) {}

  void bind_qsys_api() {
    lua_newtable(m_L);
    lua_newtable(m_L);

    for (auto call_num : m_job->m_qsyscalls.GetRegistered()) {
      auto name = m_job->m_qsyscalls.GetName(call_num);

      lua_pushinteger(m_L, (lua_Integer)(uintptr_t)&m_job);
      lua_pushinteger(m_L, (lua_Integer)(uintptr_t)&m_engine);
      lua_pushinteger(m_L, call_num);

      lua_pushcclosure(
          m_L,
          [](lua_State *L) -> int {
            quixcc_cc_job_t **job;
            quixcc_engine_t **engine;
            uint32_t call_num;

            { /* Get closure upvalues */
              int job_idx = lua_upvalueindex(1);
              int engine_idx = lua_upvalueindex(2);
              int call_num_idx = lua_upvalueindex(3);

              job = (quixcc_cc_job_t **)(uintptr_t)luaL_checkinteger(L, job_idx);
              engine = (quixcc_engine_t **)(uintptr_t)luaL_checkinteger(L, engine_idx);
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

            auto result = (*job)->m_qsyscalls.Call(*engine, call_num, args);

            /* Return a single string result */
            lua_pushstring(L, result.c_str());

            return 1;
          },
          3);
      lua_setfield(m_L, -2, name.data());
    }

    lua_setfield(m_L, -2, "api");
    lua_setglobal(m_L, "quix");
  }

  public:
  LuaEngine(quixcc_cc_job_t *job, quixcc_engine_t *engine) : m_job(job), m_engine(engine) {
    assert(job && engine);

    m_L = luaL_newstate();
    if (!m_L) {
      throw std::runtime_error("LuaEngine: luaL_newstate failed");
    }
    luaL_openlibs(m_L);

    bind_qsys_api(); /* add binding for compiler specific API */
  }

  ~LuaEngine() { lua_close(m_L); }

  void fix_dangling_pointers(quixcc_cc_job_t *job, quixcc_engine_t *engine) {
    m_job = job;
    m_engine = engine;
  }

  bool run(const char *code, std::string &output) {
    using namespace libquixcc;

    int error;

    error = luaL_loadstring(m_L, code) || lua_pcall(m_L, 0, 0, 0);
    if (error) {
      LOG(ERROR) << "Failed to load Lua code: {}" << lua_tostring(m_L, -1) << std::endl;
      return false;
    }

    error = lua_getglobal(m_L, "m");
    if (error != LUA_TFUNCTION) {
      LOG(ERROR) << "Failed to get Lua function: {}" << lua_tostring(m_L, -1) << std::endl;
      return false;
    }

    error = lua_pcall(m_L, 0, 1, 0);
    if (error) {
      LOG(ERROR) << "Failed to run Lua code: {}" << lua_tostring(m_L, -1) << std::endl;
      return false;
    }

    if (lua_isnil(m_L, -1)) {
      return true;
    } else if (lua_isstring(m_L, -1)) {
      output = lua_tostring(m_L, -1);
    } else if (lua_isnumber(m_L, -1)) {
      output = std::to_string(lua_tonumber(m_L, -1));
    } else if (lua_isboolean(m_L, -1)) {
      output = lua_toboolean(m_L, -1) ? "true" : "false";
    } else {
      LOG(ERROR) << "Invalid Lua return value: {}" << lua_typename(m_L, lua_type(m_L, -1))
                 << std::endl;
      return false;
    }

    return true;
  }
};

bool libquixcc::PrepEngine::expand_user_macro(
    const libquixcc::MacroData &fn_body,
    const std::vector<std::pair<std::string, std::string>> &user_args) {
  /* Keep the interpreter alive for the compilation unit */
  static thread_local LuaEngine engine(job, (quixcc_engine_t *)this);

  /* Ensure we always have a pointer to the correct contexts */
  engine.fix_dangling_pointers(job, (quixcc_engine_t *)this);

  std::string lua_code = "function m() \n";

  { /* Prepare arguments */
    std::unordered_map<std::string, std::string> the_args;

    std::set<std::string> params;
    for (const auto &item : fn_body.args) {
      params.insert(std::get<0>(item));
    }

    for (const auto &[key, value] : user_args) {
      if (key.starts_with("__")) {
        size_t pos = std::stoul(key.substr(2));

        if (pos >= fn_body.args.size()) {
          LOG(ERROR) << "Too many arguments for macro: {}" << key << std::endl;
          return false;
        }

        the_args[std::get<0>(fn_body.args[pos])] = value;
      } else {
        if (!params.contains(key)) {
          LOG(ERROR) << "Unknown argument: {}" << key << std::endl;
          return false;
        }

        if (the_args.contains(key)) {
          LOG(ERROR) << "Duplicate argument: {}" << key << std::endl;
          return false;
        }

        the_args[key] = value;
      }
    }

    for (const auto &[name, type, defval] : fn_body.args) {
      if (!the_args.contains(name)) {
        if (!defval) {
          LOG(ERROR) << "Missing required argument: {}" << name << std::endl;
          return false;
        }

        the_args[name] = *defval;
      }
    }

    for (const auto &[name, value] : the_args) {
      lua_code += "local " + name + " = " + value + "\n";
    }
  }

  lua_code += fn_body.luacode;

  lua_code += "\nend";

  LOG(DEBUG) << "Running Lua: '" << lua_code << "'" << std::endl;
  std::string output;
  if (!engine.run(lua_code.c_str(), output)) {
    return false;
  }

  if (output.empty()) {
    return true;
  }

  auto prep = clone();
  prep->set_source(output, "macro_expansion");
  Token t;
  while ((t = prep->next()).type() != tEofF) {
    emit(t);
  }

  return true;
}

static std::string trim(const std::string &str) {
  return str.substr(str.find_first_not_of(" \t\n\r\f\v"), str.find_last_not_of(" \t\n\r\f\v") + 1);
}

bool libquixcc::PrepEngine::parse_macro(const libquixcc::Token &macro) {
  typedef bool (libquixcc::PrepEngine::*Routine)(const libquixcc::Token &, const std::string &,
                                                 const std::string &);

  const static std::map<std::string, Routine> routines = {
      {"define", &PrepEngine::ParseDefine},
      {"pragma", &PrepEngine::ParsePragma},
      {"encoding", &PrepEngine::ParseEncoding},
      {"language", &PrepEngine::ParseLang},
      {"copyright", &PrepEngine::ParseAuthor},
      {"license", &PrepEngine::ParseLicense},
      {"use", &PrepEngine::ParseUse},
      {"description", &PrepEngine::ParseDescription},
      {"fn", &PrepEngine::ParseFn},
  };

  std::string content = trim(macro.as<std::string>());

  if (macro.type() == tMacr) {
    std::string directive, parameter;
    size_t start = content.find('('), end = 0;

    if (start != std::string::npos) {
      /*==================== PARSE MACRO DIRECTIVE ====================*/
      end = content.find_last_of(')');
      if (end == std::string::npos) {
        LOG(ERROR) << "Invalid macro directive: {}" << content << macro << std::endl;
        return false;
      }

      directive = trim(content.substr(0, start));
      if (start + 1 < content.size()) {
        parameter = content.substr(start + 1, end - start - 1);
      }

      /*==================== EXECUTE BUILTIN MACRO DIRECTIVE ====================*/
      if (routines.contains(directive)) {
        if (!(this->*routines.at(directive))(macro, directive, parameter)) {
          LOG(ERROR) << "Failed to process macro directive: {}" << directive << macro << std::endl;
        }
        return true;
      }

      /*==================== EXECUTE USER DEFINED MACRO DIRECTIVE ====================*/
      if (m_macros->contains(directive)) {
        const MacroData &fn_body = m_macros->at(directive);

        /*==================== PARSE MACRO ARGUMENTS ====================*/
        auto prep = clone();
        prep->set_source("m(" + parameter + ");\n", "macro_argument");

        qast::Expr *expr = nullptr;
        if (!qast::parser::parse_expr(*job, prep.get(), {Token(tPunc, Punctor::Semicolon)}, &expr,
                                      0)) {
          LOG(ERROR) << "Failed to parse macro arguments: {}" << parameter << std::endl;
          return false;
        }

        if (!expr->is<qast::Call>() || !expr->verify(std::cerr)) {
          LOG(ERROR) << "Invalid macro arguments: {}" << parameter << std::endl;
          return false;
        }
        qast::Call *call = expr->as<qast::Call>();

        std::vector<std::pair<std::string, std::string>> args;
        for (auto &[name, val] : call->get_args()) {
          std::stringstream ss;
          val->print(ss); /* Hopefuly, a temporary solution */
          args.emplace_back(name, ss.str());
        }

        /*====================== CALL MACRO ======================*/
        if (!expand_user_macro(fn_body, args)) {
          LOG(ERROR) << "Failed to expand user-defined macro: {}" << directive << macro
                     << std::endl;
          return false;
        }

        return true;
      }

      LOG(WARN) << "Ignoring unknown macro directive: {}" << content << macro << std::endl;

      return true;
    } else {
      for (auto &routine : routines) {
        if (content.starts_with(routine.first)) {
          if (!(this->*routine.second)(macro, routine.first,
                                       content.substr(routine.first.size()))) {
            LOG(ERROR) << "Failed to process macro directive: {}" << routine.first << macro
                       << std::endl;
          }

          return true;
        }
      }

      LOG(WARN) << "Ignoring unknown macro directive: {}" << content << macro << std::endl;
      return true;
    }
  } else if (macro.type() == tMacB) {
    std::string directive;
    std::string body;

    size_t start = content.find(' ');
    if (start == std::string::npos) {
      directive = content;
    } else {
      directive = content.substr(0, start);
      body = content.substr(start + 1);
    }

    if (!routines.contains(directive)) {
      LOG(WARN) << "Ignoring unknown macro directive: {}" << content << macro << std::endl;
      return true;
    }

    if (!(this->*routines.at(directive))(macro, directive, body)) {
      LOG(ERROR) << "Failed to process macro directive: {}" << directive << macro << std::endl;
    }

    return true;
  } else {
    throw std::runtime_error("Invalid macro type");
  }

  return false;
}