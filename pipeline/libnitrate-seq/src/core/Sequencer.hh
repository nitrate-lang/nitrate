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
    std::deque<ncc::lex::Token> m_buffer;
    std::vector<DeferCallback> m_defer;
    std::shared_ptr<Environment> m_env;
    std::unique_ptr<ncc::lex::Tokenizer> m_scanner;
    FetchModuleFunc m_fetch_module;
    lua_State* m_L = nullptr;
    size_t m_depth = 0;

  private:
    ///=========================================================================

    [[nodiscard]] auto SysNext() const -> int;
    [[nodiscard]] auto SysPeek() const -> int;
    [[nodiscard]] auto SysEmit() const -> int;
    [[nodiscard]] auto SysDebug() const -> int;
    [[nodiscard]] auto SysInfo() const -> int;
    [[nodiscard]] auto SysWarn() const -> int;
    [[nodiscard]] auto SysError() const -> int;
    [[nodiscard]] auto SysAbort() const -> int;
    [[nodiscard]] auto SysFatal() const -> int;
    [[nodiscard]] auto SysGet() const -> int;
    [[nodiscard]] auto SysSet() const -> int;
    [[nodiscard]] auto SysCtrl() const -> int;
    [[nodiscard]] auto SysFetch() -> int;
    [[nodiscard]] auto SysRandom() -> int;
    [[nodiscard]] auto SysDefer() -> int;

    ///=========================================================================

    void BindLuaAPI() const {
      lua_newtable(m_L);

      // for (auto func : SYS_FUNCTIONS) {
      //   lua_pushinteger(m_L, (lua_Integer)(uintptr_t)this);
      //   lua_pushcclosure(m_L, func.GetFunc(), 1);
      //   lua_setfield(m_L, -2, func.GetName().data());
      // }

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
    auto FetchModuleData(std::string_view module_name)
        -> std::optional<std::string>;

    auto ExecuteLua(const char* code) const -> std::optional<std::string> {
      auto top = lua_gettop(m_L);
      auto rc = luaL_dostring(m_L, code);

      if (rc) {
        ncc::Log << ec::SeqError << "Lua error: " << lua_tostring(m_L, -1);
        /// TODO: Set error bit
        // SetFailBit();

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

    PImpl(std::shared_ptr<Environment> env, std::istream& scanner);
    ~PImpl();
  };
};  // namespace ncc::seq

#endif  // __QPREP_QCALL_LIST_HH__
