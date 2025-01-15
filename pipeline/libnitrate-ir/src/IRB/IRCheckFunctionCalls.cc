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

using namespace ncc::ir;

bool NRBuilder::CheckFunctionCalls(FlowPtr<Seq> root, IReport *i) {
  bool failed = false;

  for_each<Call>(root, [&](auto x) {
    if (auto target_opt = x->getTarget()) {
      auto target = target_opt.value();

      if (auto target_type = target->getType()) {
        if (target_type.value()->is_function()) {
          FnTy *fn_ty = target_type.value()->template as<FnTy>();

          const auto &arguments = x->getArgs();

          bool variadic_two_few = fn_ty->IsVariadic() &&
                                  arguments.size() < fn_ty->GetParams().size();
          bool two_few = arguments.size() < fn_ty->GetParams().size();
          bool two_many = arguments.size() > fn_ty->GetParams().size();

          if (variadic_two_few) {
            i->Report(VariadicNotEnoughArguments, IC::Error, target->getName(),
                      x->getLoc());
          } else if (two_few) {
            i->Report(TwoFewArguments, IC::Error, target->getName(),
                      x->getLoc());
          } else if (two_many) {
            i->Report(TwoManyArguments, IC::Error, target->getName(),
                      x->getLoc());
          }

          if (!two_few && !two_many && !variadic_two_few) {
            for (size_t i = 0; i < fn_ty->GetParams().size(); ++i) {
              auto param_type = fn_ty->GetParams()[i]->GetType();

              if (!param_type.has_value()) {
                i->Report(TypeInference, IC::Error,
                          "Unable to deduce function parameter type");
                failed = true;
                continue;
              }

              auto arg_type = arguments[i]->getType();
              if (!arg_type.has_value()) {
                i->Report(TypeInference, IC::Error,
                          "Unable to deduce function argument type");
                failed = true;
                continue;
              }

              if (!param_type.value()->IsEq(arg_type.value().get())) {
                i->Report(BadCast, IC::Error,
                          {"Bad call argument cast from '",
                           arg_type.value()->toString(), "' to '",
                           param_type.value()->ToString(), "'"});
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
    }

    /* Fallthough to error flag */
    failed = true;
  });

  return !failed;
}
