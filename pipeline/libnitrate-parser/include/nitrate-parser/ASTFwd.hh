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

#ifndef __NITRATE_AST_NODES_FWD_H__
#define __NITRATE_AST_NODES_FWD_H__

namespace ncc::parse {
  class Expr;
  class Type;
  class Binary;
  class Unary;
  class Integer;
  class Float;
  class String;
  class Character;
  class Boolean;
  class Null;
  class Call;
  class List;
  class Assoc;
  class Index;
  class Slice;
  class FString;
  class Identifier;
  class Sequence;
  class TemplateCall;
  class Import;
  class RefTy;
  class PtrTy;
  class OpaqueTy;
  class ArrayTy;
  class TupleTy;
  class FuncTy;
  class NamedTy;
  class InferTy;
  class TemplateType;
  class Typedef;
  class Struct;
  class Enum;
  class Function;
  class Scope;
  class Export;
  class Block;
  class Variable;
  class Assembly;
  class Return;
  class Break;
  class Continue;
  class If;
  class While;
  class For;
  class Foreach;
  class Case;
  class Switch;
}  // namespace ncc::parse

#endif
