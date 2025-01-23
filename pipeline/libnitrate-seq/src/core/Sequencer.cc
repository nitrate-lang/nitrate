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

#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-seq/EC.hh>
#include <nitrate-seq/Sequencer.hh>

extern "C" {
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

using namespace ncc::lex;
using namespace ncc::seq;
using namespace ncc::seq::ec;

NCC_EXPORT auto ncc::seq::FileSystemFetchModule(std::string_view path)
    -> std::optional<std::string> {
  if (!path.starts_with("file:///package/")) {
    return std::nullopt;
  }

  path.remove_prefix(16);

  if (path.size() < 37) {
    return std::nullopt;
  }
  auto job_uuid = path.substr(0, 36);
  path.remove_prefix(37);

  Log << Debug << "Opening file '" << path << "' on behalf of job '" << job_uuid
      << "'";

  /// TODO: Get the base directory of the project

  std::fstream file(std::string(path), std::ios::in);
  if (!file.is_open()) {
    return std::nullopt;
  }

  return std::string((std::istreambuf_iterator<char>(file)),
                     (std::istreambuf_iterator<char>()));
}

class RecursiveLimitGuard {
  static constexpr auto kMaxRecursionDepth = 10000;

  size_t &m_depth;

public:
  RecursiveLimitGuard(size_t &depth) : m_depth(depth) { m_depth++; }
  ~RecursiveLimitGuard() { m_depth--; }

  [[nodiscard]] auto ShouldStop() const -> bool {
    return m_depth >= kMaxRecursionDepth;
  }
};

void Sequencer::BindMethod(const char *name, MethodType func) {
  m_shared->m_captures.push_back(func);
  MethodType &func_ref = m_shared->m_captures.back();

  lua_pushinteger(m_shared->m_L, (lua_Integer)(uintptr_t)this);
  lua_pushinteger(m_shared->m_L, (lua_Integer)(uintptr_t)&func_ref);

  lua_pushcclosure(
      m_shared->m_L,
      [](lua_State *lua) -> int {
        auto &self = *(Sequencer *)(lua_tointeger(lua, lua_upvalueindex(1)));
        auto &func = *(MethodType *)(lua_tointeger(lua, lua_upvalueindex(2)));

        return (self.*func)();
      },
      2);

  lua_setfield(m_shared->m_L, -2, name);
}

void Sequencer::BindLuaAPI() {
  lua_newtable(m_shared->m_L);

  BindMethod("next", &Sequencer::SysNext);
  BindMethod("peek", &Sequencer::SysPeek);
  BindMethod("emit", &Sequencer::SysEmit);

  BindMethod("debug", &Sequencer::SysDebug);
  BindMethod("info", &Sequencer::SysInfo);
  BindMethod("warn", &Sequencer::SysWarn);
  BindMethod("error", &Sequencer::SysError);
  BindMethod("abort", &Sequencer::SysAbort);
  BindMethod("fatal", &Sequencer::SysFatal);

  BindMethod("get", &Sequencer::SysGet);
  BindMethod("set", &Sequencer::SysSet);

  BindMethod("ctrl", &Sequencer::SysCtrl);
  BindMethod("fetch", &Sequencer::SysFetch);
  BindMethod("random", &Sequencer::SysRandom);
  // BindMethod("defer", &Sequencer::SysDefer);

  lua_setglobal(m_shared->m_L, "n");
}

void Sequencer::LoadLuaLibs() const {
  static constexpr std::array<luaL_Reg, 5> kTheLibs = {
      luaL_Reg{LUA_GNAME, luaopen_base},
      luaL_Reg{LUA_TABLIBNAME, luaopen_table},
      luaL_Reg{LUA_STRLIBNAME, luaopen_string},
      luaL_Reg{LUA_MATHLIBNAME, luaopen_math},
      luaL_Reg{LUA_UTF8LIBNAME, luaopen_utf8},
  };

  for (const auto &lib : kTheLibs) {
    luaL_requiref(m_shared->m_L, lib.name, lib.func, 1);
    lua_pop(m_shared->m_L, 1);
  }
}

auto Sequencer::ExecuteLua(const char *code) -> std::optional<std::string> {
  auto top = lua_gettop(m_shared->m_L);
  auto rc = luaL_dostring(m_shared->m_L, code);

  if (rc) {
    ncc::Log << ec::SeqError
             << "Lua error: " << lua_tostring(m_shared->m_L, -1);
    SetFailBit();

    return std::nullopt;
  }

  // If no value was returned, return an empty string
  if (lua_gettop(m_shared->m_L) == top) {
    return "";
  }

  return lua_tostring(m_shared->m_L, -1);
}

auto Sequencer::FetchModuleData(std::string_view raw_module_name)
    -> std::optional<std::string> {
  const auto get_fetch_uri = [](const std::string &module_name,
                                const std::string &jobid) -> std::string {
    return "file:///package/" + jobid + "/" + module_name;
  };

  const auto canonicalize_module_name =
      [](std::string module_name) -> std::string {
    size_t pos = 0;
    while ((pos = module_name.find("::", pos)) != std::string::npos) {
      module_name.replace(pos, 2, ".");
    }

    std::replace(module_name.begin(), module_name.end(), '/', '.');

    return module_name;
  };

  auto module_name = canonicalize_module_name(std::string(raw_module_name));

  /* Translate module names into their actual names */
  if (auto actual_name = m_env->Get("map." + module_name)) {
    module_name = actual_name.value();
  }

  auto jobid = std::string(m_env->Get("this.job").value());
  auto module_uri = get_fetch_uri(module_name, jobid);

  Log << SeqError << Debug << "Fetching module: '" << module_name << "'";

  qcore_assert(m_shared->m_fetch_module);

  if (auto module_content = m_shared->m_fetch_module(module_uri)) {
    return module_content;
  }

  Log << SeqError << "Import not found: '" << module_name << "'";

  return std::nullopt;
}

void Sequencer::SequenceSource(std::string_view code) {
  std::istringstream ss(std::string(code.data(), code.size()));

  Sequencer clone(ss, m_env, false);
  clone.m_shared = m_shared;
  clone.m_shared->m_depth = m_shared->m_depth + 1;

  std::queue<Token> stack;
  Token tok;

  while ((tok = (clone.Next())).GetKind() != EofF) {
    stack.push(tok);
  }

  if (!stack.empty()) {
    m_shared->m_emission.push(std::move(stack));
  }
}

auto Sequencer::GetNext() -> Token {
  Token tok;

  try {
    RecursiveLimitGuard guard(m_shared->m_depth);
    if (guard.ShouldStop()) {
      Log << SeqError << "Maximum macro recursion depth reached, aborting";
      throw StopException();
    }

    while (true) {
      if (!m_shared->m_emission.empty()) {
        tok = m_shared->m_emission.front().front();
        m_shared->m_emission.front().pop();

        if (m_shared->m_emission.front().empty()) {
          m_shared->m_emission.pop();
        }
      } else {
        tok = m_scanner.Next();
        SetFailBit(HasError() || m_scanner.HasError());

        switch (tok.GetKind()) {
          case EofF:
          case IntL:
          case Text:
          case Char:
          case NumL:
          case Oper:
          case Punc:
          case Name:
          case Note: {
            break;
          }

          case KeyW: {
            if (tok.GetKeyword() == Import) [[unlikely]] {
              auto import_name = m_scanner.Next().GetString();

              if (m_scanner.Next().is<PuncSemi>()) {
                if (auto content = FetchModuleData(import_name.Get())) {
                  SequenceSource(content.value());
                  std::cout << "Looping" << std::endl;
                  continue;
                } /* Failed to fetch module */
              } else [[unlikely]] { /* No semicolon after import name */
                Log << SeqError << "Expected a semicolon after import name";
              }

              tok = Token::EndOfFile();
              SetFailBit();
            }

            break;
          }

          case MacB: {
            if (auto result_data = ExecuteLua(tok.GetString().Get())) {
              SequenceSource(result_data.value());
              continue;
            } /* Failed to execute macro */

            tok = Token::EndOfFile();
            SetFailBit();

            break;
          }

          case Macr: {
            if (auto result_data =
                    ExecuteLua((std::string(tok.GetString()) + "()").c_str())) {
              SequenceSource(result_data.value());
              continue;
            } /* Failed to execute macro function call */

            tok = Token::EndOfFile();
            SetFailBit();

            break;
          }
        }
      }

      break;
    }
  } catch (StopException &) {
    tok = Token::EndOfFile();
  }

  return tok;
}

auto Sequencer::GetLocationFallback(ncc::lex::LocationID id)
    -> std::optional<ncc::lex::Location> {
  return m_scanner.GetLocation(id);
}

auto Sequencer::HasError() const -> bool {
  return IScanner::HasError() || m_scanner.HasError();
}

auto Sequencer::SetFailBit(bool fail) -> bool {
  auto old = HasError();

  IScanner::SetFailBit(fail);
  m_scanner.SetFailBit(fail);

  return old;
}

auto Sequencer::SetFetchFunc(FetchModuleFunc func) -> void {
  if (!func) {
    func = [](std::string_view) {
      Log << SeqError << Debug << "No module fetch function provided";
      return std::nullopt;
    };
  }

  m_shared->m_fetch_module = func;
}

auto Sequencer::GetSourceWindow(Point start, Point end, char fillchar)
    -> std::optional<std::vector<std::string>> {
  return m_scanner.GetSourceWindow(start, end, fillchar);
}

Sequencer::SharedState::SharedState()
    : m_random(0),
      m_fetch_module([](std::string_view) {
        Log << SeqError << Debug << "No module fetch function provided";
        return std::nullopt;
      }),
      m_captures({}),
      m_L(luaL_newstate()),
      m_depth(0) {
  if (m_L == nullptr) {
    Log << Emergency << SeqError << "Failed to create Lua state";
    qcore_panic("Failed to allocate Lua state");
  }
}

Sequencer::SharedState::~SharedState() { lua_close(m_L); }

Sequencer::Sequencer(std::istream &file, std::shared_ptr<ncc::Environment> env,
                     bool is_root)
    : ncc::lex::IScanner(std::move(env)),
      m_scanner(file, m_env),
      m_shared(is_root ? std::make_shared<SharedState>() : nullptr) {
  if (is_root) {
    LoadLuaLibs();
    BindLuaAPI();
    SequenceSource(CodePrefix);
  }
}

Sequencer::~Sequencer() = default;
