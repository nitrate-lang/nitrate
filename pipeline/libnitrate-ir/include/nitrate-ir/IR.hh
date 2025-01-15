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

#ifndef __NITRATE_IR_IR_H__
#define __NITRATE_IR_IR_H__

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <nitrate-ir/IR/Fwd.hh>
#include <nitrate-ir/IR/Nodes.hh>

namespace ncc::parse {
  class Base;
}

namespace ncc::ir {
  typedef enum nr_serial_t {
    IR_SERIAL_CODE = 0, /* Human readable ASCII text */
  } nr_serial_t;

  void nr_write(IRModule *mod, NullableFlowPtr<Expr> _node, std::ostream &out);

  std::unique_ptr<IRModule> nr_lower(ncc::parse::Base *base, const char *name,
                                     bool diagnostics);

  typedef void (*nr_node_cb)(Expr *cur, uintptr_t userdata);

  typedef enum {
    IR_LEVEL_DEBUG = 0,
    IR_LEVEL_INFO = 1,
    IR_LEVEL_WARN = 2,
    IR_LEVEL_ERROR = 3,
    IR_LEVEL_MAX = 5,
    IR_LEVEL_DEFAULT = IR_LEVEL_WARN,
  } nr_level_t;
}  // namespace ncc::ir

#endif  // __NITRATE_IR_IR_H__
