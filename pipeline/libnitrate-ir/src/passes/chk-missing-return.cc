#////////////////////////////////////////////////////////////////////////////////
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

#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <passes/PassList.hh>

using namespace nr;

bool nr::pass::chk_missing_return(qmodule_t* M, IReport* log) {
  /**
   * Check that functions that require a return statement actually have one.
   * Declarations and non-applicable functions are ignored.
   */

  for (auto& [key, val] : M->getFunctions()) {
    Fn* fn = val.second;

    if (!fn->getBody().has_value()) {
      continue;
    }

    FnTy* fnty = val.first;

    /* Skip the function declarations and all functions that have a void return
     * type. */
    if (fn->getBody().value()->getKind() == NR_NODE_IGN ||
        fnty->getReturn()->getKind() == NR_NODE_VOID_TY) {
      continue;
    }

    /* If the function has a non-void return type, then we need to check if
     * there is a return statement. */
    Seq* body = fn->getBody().value()->as<Seq>();
    bool any_ret =
        std::any_of(body->getItems().begin(), body->getItems().end(),
                    [](auto item) { return item->getKind() == NR_NODE_RET; });

    if (!any_ret) { /* If no return statement is found, this is an error. */
      log->report(MissingReturn, IC::Error, key, fn->getLoc());
    }
  }

  return true;
}
