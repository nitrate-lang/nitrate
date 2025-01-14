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
    lua_State* L = nullptr;
    std::vector<DeferCallback> defer_callbacks;
    std::deque<ncc::lex::Token> buffer;
    std::mt19937 m_qsys_random_engine;
    bool m_do_expanse = true;
    size_t m_depth = 0;

    ~PImpl();
  };

  typedef int (*qsyscall_t)(lua_State* L);
  class QSysCall final {
    std::string_view m_name;
    uint32_t m_id;
    qsyscall_t m_func;

  public:
    QSysCall(std::string_view name = "", uint32_t id = 0,
             qsyscall_t func = nullptr)
        : m_name(name), m_id(id), m_func(func) {}

    std::string_view getName() const { return m_name; }
    uint32_t getId() const { return m_id; }
    qsyscall_t getFunc() const { return m_func; }
  };

  ///////////// BEGIN QCALL FUNCTIONS /////////////

  /* ==== Source processing ==== */
  int sys_next(lua_State* L);
  int sys_peek(lua_State* L);
  int sys_emit(lua_State* L);
  int sys_defer(lua_State* L);

  /* ===== Message Logging ===== */
  int sys_debug(lua_State* L);
  int sys_info(lua_State* L);
  int sys_warn(lua_State* L);
  int sys_error(lua_State* L);
  int sys_abort(lua_State* L);
  int sys_fatal(lua_State* L);

  /* ====== Global State ======= */
  int sys_get(lua_State* L);
  int sys_set(lua_State* L);

  /* ====== Data Feching ======= */
  int sys_fetch(lua_State* L);

  /* ===== Random Generator ===== */
  int sys_random(lua_State* L);

  /* ===== Implementation specific ===== */
  int sys_ctrl(lua_State* L);

  ////////////// END QCALL FUNCTIONS //////////////

  static inline const std::vector<QSysCall> SysFunctions = {
      {"next", 0x0010, sys_next},   /* Get the next token from the lexer */
      {"peek", 0x0011, sys_peek},   /* Peek at the next token from the lexer */
      {"emit", 0x0012, sys_emit},   /* Emit data  */
      {"defer", 0x0013, sys_defer}, /* Callback after every token is emitted */
      {"debug", 0x0050, sys_debug}, /* Print a debug message */
      {"info", 0x0051, sys_info},   /* Print an informational message */
      {"warn", 0x0052, sys_warn},   /* Print a warning message */
      {"error", 0x0053, sys_error}, /* Print an error message */
      {"abort", 0x0054, sys_abort}, /* Print an error and halt */
      {"fatal", 0x0055, sys_fatal}, /* Print a fatal error and halt */
      {"get", 0x0080, sys_get},     /* Get a value from the environment */
      {"set", 0x0081, sys_set},     /* Set a value in the environment */
      {"fetch", 0x0082, sys_fetch}, /* Import module */
      {"random", 0x00A0, sys_random}, /* Get a random number */
      {"ctrl", 0x00C0, sys_ctrl}      /* Implementation specific stuff */
  };

};  // namespace ncc::seq

#endif  // __QPREP_QCALL_LIST_HH__
