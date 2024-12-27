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

#ifndef __NITRATE_IR_CLASSES_H__
#define __NITRATE_IR_CLASSES_H__

#include <nitrate-core/Logger.hh>
#include <nitrate-ir/IR.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <optional>
#include <string>

#include "nitrate-core/FlowPtr.hh"

namespace ncc::ir {
  class SymbolEncoding final {
  public:
    SymbolEncoding() = default;

    std::optional<std::string> mangle_name(const Expr *symbol,
                                           AbiTag abi) const;

    std::optional<std::string> demangle_name(std::string_view symbol) const;
  };

  std::optional<std::string> MangleTypeName(const Type *type, AbiTag abi);

  std::optional<std::string> GetMangledSymbolName(const Expr *symbol,
                                                  AbiTag abi);

  std::optional<std::string> ExpandSymbolName(std::string_view mangled_name);
  NullableFlowPtr<Expr> GetSymbolFromMangledName(std::string_view mangled_name);
  NullableFlowPtr<Type> GetTypeFromMangledName(std::string_view mangled_name);
}  // namespace ncc::ir

#endif  // __NITRATE_IR_CLASSES_H__
