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

#ifndef __QPREP_QCALL_LIST_HH__
#define __QPREP_QCALL_LIST_HH__

#include <cstdint>
#include <functional>
#include <list>
#include <nitrate-seq/EC.hh>
#include <nitrate-seq/Sequencer.hh>
#include <random>
#include <string_view>
#include <vector>

extern "C" {
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

namespace ncc::seq {
  class StopException {};

  enum DeferOp : uint8_t {
    EmitToken,
    SkipToken,
    UninstallHandler,
  };

  using DeferCallback =
      std::function<DeferOp(Sequencer* obj, ncc::lex::Token last)>;

  class Sequencer::PImpl final {
  public:
    std::mt19937 m_random;
    ncc::lex::Tokenizer m_scanner;
    std::deque<ncc::lex::Token> m_buffer;
    std::vector<DeferCallback> m_defer;
    std::shared_ptr<Environment> m_env;
    FetchModuleFunc m_fetch_module;
    lua_State* m_L;
    size_t m_depth;

  private:
    std::function<void()> m_on_error;

    void SetFailBit() { m_on_error(); }

    ///=========================================================================

    [[nodiscard]] auto SysNext() -> int32_t;
    [[nodiscard]] auto SysPeek() -> int32_t;
    [[nodiscard]] auto SysEmit() -> int32_t;
    [[nodiscard]] auto SysDebug() -> int32_t;
    [[nodiscard]] auto SysInfo() -> int32_t;
    [[nodiscard]] auto SysWarn() -> int32_t;
    [[nodiscard]] auto SysError() -> int32_t;
    [[nodiscard]] auto SysAbort() -> int32_t;
    [[nodiscard]] auto SysFatal() -> int32_t;
    [[nodiscard]] auto SysGet() -> int32_t;
    [[nodiscard]] auto SysSet() -> int32_t;
    [[nodiscard]] auto SysCtrl() -> int32_t;
    [[nodiscard]] auto SysFetch() -> int32_t;
    [[nodiscard]] auto SysRandom() -> int32_t;
    [[nodiscard]] auto SysDefer() -> int32_t;

    ///=========================================================================

    using MethodType = int (ncc::seq::Sequencer::PImpl::*)();

    std::list<MethodType> m_captures;

    void BindMethod(const char* name, MethodType func) {
      m_captures.push_back(func);
      MethodType& func_ref = m_captures.back();

      lua_pushinteger(m_L, (lua_Integer)(uintptr_t)this);
      lua_pushinteger(m_L, (lua_Integer)(uintptr_t)&func_ref);

      lua_pushcclosure(
          m_L,
          [](lua_State* lua) -> int {
            auto& self = *(PImpl*)(lua_tointeger(lua, lua_upvalueindex(1)));
            auto& func =
                *(MethodType*)(lua_tointeger(lua, lua_upvalueindex(2)));

            return (self.*func)();
          },
          2);

      lua_setfield(m_L, -2, name);
    }

    void BindLuaAPI() {
      lua_newtable(m_L);

      BindMethod("next", &PImpl::SysNext);
      BindMethod("peek", &PImpl::SysPeek);
      BindMethod("emit", &PImpl::SysEmit);

      BindMethod("debug", &PImpl::SysDebug);
      BindMethod("info", &PImpl::SysInfo);
      BindMethod("warn", &PImpl::SysWarn);
      BindMethod("error", &PImpl::SysError);
      BindMethod("abort", &PImpl::SysAbort);
      BindMethod("fatal", &PImpl::SysFatal);

      BindMethod("get", &PImpl::SysGet);
      BindMethod("set", &PImpl::SysSet);

      BindMethod("ctrl", &PImpl::SysCtrl);
      BindMethod("fetch", &PImpl::SysFetch);
      BindMethod("random", &PImpl::SysRandom);
      BindMethod("defer", &PImpl::SysDefer);

      lua_setglobal(m_L, "n");
    }

    void LoadLuaLibs() const {
      static constexpr std::array<luaL_Reg, 5> kTheLibs = {
          luaL_Reg{LUA_GNAME, luaopen_base},
          luaL_Reg{LUA_TABLIBNAME, luaopen_table},
          luaL_Reg{LUA_STRLIBNAME, luaopen_string},
          luaL_Reg{LUA_MATHLIBNAME, luaopen_math},
          luaL_Reg{LUA_UTF8LIBNAME, luaopen_utf8},
      };

      for (const auto& lib : kTheLibs) {
        luaL_requiref(m_L, lib.name, lib.func, 1);
        lua_pop(m_L, 1);
      }
    }

  public:
    PImpl(std::shared_ptr<Environment> env, std::istream& scanner,
          std::function<void()> on_error);
    PImpl(const PImpl&) = delete;
    PImpl(PImpl&&) = delete;
    ~PImpl();

    auto FetchModuleData(std::string_view module_name)
        -> std::optional<std::string>;

    auto ExecuteLua(const char* code) -> std::optional<std::string> {
      auto top = lua_gettop(m_L);
      auto rc = luaL_dostring(m_L, code);

      if (rc) {
        ncc::Log << ec::SeqError << "Lua error: " << lua_tostring(m_L, -1);
        SetFailBit();

        return std::nullopt;
      }

      if (lua_isstring(m_L, -1) != 0) {
        return lua_tostring(m_L, -1);
      }

      if (lua_isnumber(m_L, -1) != 0) {
        return std::to_string(lua_tonumber(m_L, -1));
      }

      if (lua_isboolean(m_L, -1)) {
        return (lua_toboolean(m_L, -1) != 0) ? "true" : "false";
      }

      if (lua_gettop(m_L) != top && !lua_isnil(m_L, -1)) {
        return std::nullopt;
      }

      return "";
    }
  };
};  // namespace ncc::seq

#endif  // __QPREP_QCALL_LIST_HH__
