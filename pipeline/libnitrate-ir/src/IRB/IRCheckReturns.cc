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

#define IRBUILDER_IMPL

#include <nitrate-core/Logger.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/IRB/Builder.hh>
#include <nitrate-ir/diagnostic/Report.hh>

using namespace ncc::ir;

bool NRBuilder::CheckReturns(FlowPtr<Seq> root, IReport *d) {
  bool failed = false;

  for_each<Function>(root, [&](auto x) {
    /* Skip function declarations */
    if (!x->GetBody()) {
      return;
    }

    auto fn_ty_opt = x->GetType();
    if (!fn_ty_opt) {
      d->Report(TypeInference, IC::Error, "Failed to deduce function type",
                x->GetLoc());
      failed = true;

      return;
    }

    const auto fn_ty = fn_ty_opt.value()->template as<FnTy>();
    const auto return_ty = fn_ty->GetReturn();

    bool found_ret = false;

    for_each<Ret>(x->GetBody().value(), [&](auto y) {
      found_ret = true;

      auto ret_expr_ty_opt = y->GetExpr()->GetType();
      if (!ret_expr_ty_opt) {
        d->Report(TypeInference, IC::Error,
                  "Failed to deduce return expression type", y->GetLoc());
        failed = true;

        return;
      }

      /// TODO: Implement return type coercion

      if (!return_ty->IsEq(ret_expr_ty_opt.value().get())) {
        d->Report(ReturnTypeMismatch, IC::Error,
                  {"Return value type '", ret_expr_ty_opt.value()->ToString(),
                   "' does not match function return type '",
                   return_ty->ToString(), "'"},
                  y->GetLoc());
        failed = true;

        return;
      }
    });

    if (!found_ret) {
      d->Report(MissingReturn, IC::Error, x->GetName(), x->GetLoc());
      failed = true;
    }
  });

  return !failed;
}
