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

#include <nitrate-core/Error.h>

#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>

using namespace nr;

bool NRBuilder::check_function_calls(Seq *root, IReport *I) {
  bool failed = false;

  std::for_each<Call>(root, [&](const Call *x) {
    Expr *target = x->getTarget();

    if (auto target_type = target->getType()) {
      if (target_type.value()->is_function()) {
        FnTy *fn_ty = target_type.value()->as<FnTy>();

        const auto &arguments = x->getArgs();

        bool variadic_two_few =
            fn_ty->isVariadic() && arguments.size() < fn_ty->getParams().size();
        bool two_few = arguments.size() < fn_ty->getParams().size();
        bool two_many = arguments.size() > fn_ty->getParams().size();

        if (variadic_two_few) {
          I->report(VariadicNotEnoughArguments, IC::Error, target->getName(),
                    x->getLoc());
        } else if (two_few) {
          I->report(TwoFewArguments, IC::Error, target->getName(), x->getLoc());
        } else if (two_many) {
          I->report(TwoManyArguments, IC::Error, target->getName(),
                    x->getLoc());
        }

        if (!two_few && !two_many && !variadic_two_few) {
          for (size_t i = 0; i < fn_ty->getParams().size(); ++i) {
            auto param_type = fn_ty->getParams()[i]->getType();

            if (!param_type.has_value()) {
              I->report(TypeInference, IC::Error,
                        "Unable to deduce function parameter type");
              failed = true;
              continue;
            }

            auto arg_type = arguments[i]->getType();
            if (!arg_type.has_value()) {
              I->report(TypeInference, IC::Error,
                        "Unable to deduce function argument type");
              failed = true;
              continue;
            }

            if (!param_type.value()->isSame(arg_type.value())) {
              I->report(BadCast, IC::Error,
                        {"Bad call argument cast from '",
                         arg_type.value()->toString(), "' to '",
                         param_type.value()->toString(), "'"});
              failed = true;
              continue;
            }

            /// TODO: Handle implicit conversions

            // The argument is valid
          }

          return;
        }
      }
    }

    /* Fallthough to error flag */
    failed = true;
  });

  return !failed;
}
