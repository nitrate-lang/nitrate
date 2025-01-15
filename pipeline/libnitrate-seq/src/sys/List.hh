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
#include <nitrate-seq/Sequencer.hh>
#include <random>
#include <string_view>
#include <vector>

#define get_engine() \
  ((Sequencer*)(uintptr_t)luaL_checkinteger(L, lua_upvalueindex(1)))

namespace ncc::seq {
  class Sequencer::PImpl final {
  public:
    std::mt19937 m_random;
    std::deque<ncc::lex::Token> m_buffer;
    std::vector<DeferCallback> m_defer;
    std::shared_ptr<Environment> m_env;
    FetchModuleFunc m_fetch_module;
    lua_State* m_L = nullptr;
    size_t m_depth = 0;

    std::optional<std::string> FetchModuleData(std::string_view module_name);

    PImpl(std::shared_ptr<Environment> env);
    ~PImpl();
  };

  typedef int (*QsyscallT)(lua_State* l);
  class QSysCall final {
    std::string_view m_name;
    uint32_t m_id;
    QsyscallT m_func;

  public:
    QSysCall(std::string_view name = "", uint32_t id = 0,
             QsyscallT func = nullptr)
        : m_name(name), m_id(id), m_func(func) {}

    std::string_view GetName() const { return m_name; }
    uint32_t GetId() const { return m_id; }
    QsyscallT GetFunc() const { return m_func; }
  };

  ///////////// BEGIN QCALL FUNCTIONS /////////////

  /* ==== Source processing ==== */
  int SysNext(lua_State* l);
  int SysPeek(lua_State* l);
  int SysEmit(lua_State* l);
  int SysDefer(lua_State* l);

  /* ===== Message Logging ===== */
  int SysDebug(lua_State* l);
  int SysInfo(lua_State* l);
  int SysWarn(lua_State* l);
  int SysError(lua_State* l);
  int SysAbort(lua_State* l);
  int SysFatal(lua_State* l);

  /* ====== Global State ======= */
  int SysGet(lua_State* l);
  int SysSet(lua_State* l);

  /* ====== Data Feching ======= */
  int SysFetch(lua_State* l);

  /* ===== Random Generator ===== */
  int SysRandom(lua_State* l);

  /* ===== Implementation specific ===== */
  int SysCtrl(lua_State* l);

  ////////////// END QCALL FUNCTIONS //////////////

  static inline const std::vector<QSysCall> SYS_FUNCTIONS = {
      {"next", 0x0010, SysNext},   /* Get the next token from the lexer */
      {"peek", 0x0011, SysPeek},   /* Peek at the next token from the lexer */
      {"emit", 0x0012, SysEmit},   /* Emit data  */
      {"defer", 0x0013, SysDefer}, /* Callback after every token is emitted */
      {"debug", 0x0050, SysDebug}, /* Print a debug message */
      {"info", 0x0051, SysInfo},   /* Print an informational message */
      {"warn", 0x0052, SysWarn},   /* Print a warning message */
      {"error", 0x0053, SysError}, /* Print an error message */
      {"abort", 0x0054, SysAbort}, /* Print an error and halt */
      {"fatal", 0x0055, SysFatal}, /* Print a fatal error and halt */
      {"get", 0x0080, SysGet},     /* Get a value from the environment */
      {"set", 0x0081, SysSet},     /* Set a value in the environment */
      {"fetch", 0x0082, SysFetch}, /* Import module */
      {"random", 0x00A0, SysRandom}, /* Get a random number */
      {"ctrl", 0x00C0, SysCtrl}      /* Implementation specific stuff */
  };

};  // namespace ncc::seq

#endif  // __QPREP_QCALL_LIST_HH__
