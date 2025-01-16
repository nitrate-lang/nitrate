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

#ifndef __NITRATE_IR_EC_H__
#define __NITRATE_IR_EC_H__

#include <nitrate-core/Logger.hh>

namespace ncc::ir::ec {
  auto Formatter(std::string_view msg, Sev sev) -> std::string;
#define EXPAND(path) "$NCC_CONF/ec/ir/" path
#define DEFINE(group, name) NCC_EC_EX(group, name, Formatter, EXPAND(#name));

  ///=============================================================================

  NCC_EC_GROUP(IRConvertEG);
  NCC_EC_GROUP(IRSemCheckEG);
  NCC_EC_GROUP(IRPerfEG);
  NCC_EC_GROUP(IRMisusageEG);
  NCC_EC_GROUP(IRDevEG);

  ///=============================================================================

  DEFINE(IRConvertEG, CyclicRef);
  DEFINE(IRConvertEG, IRNullRef);
  DEFINE(IRConvertEG, UndefinedType);
  DEFINE(IRConvertEG, UndefinedSymbol);
  DEFINE(IRConvertEG, ConflictingType);
  DEFINE(IRConvertEG, ConflictingSymbol);
  DEFINE(IRConvertEG, ExpectedArguments);
  DEFINE(IRConvertEG, BadInfer);
  DEFINE(IRConvertEG, UnexpectedUndefLiteral);
  DEFINE(IRConvertEG, ReturnTypeMismatch);
  DEFINE(IRConvertEG, MissingReturn);
  DEFINE(IRConvertEG, BadCast);

  DEFINE(IRSemCheckEG, AssignToConst);

  ///=============================================================================

#undef DEFINE
#undef EXPAND
}  // namespace ncc::ir::ec

#endif
