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

#ifndef __NITRATE_AST_PROTOBUF_FWD_H__
#define __NITRATE_AST_PROTOBUF_FWD_H__

namespace nitrate::parser::SyntaxTree {  // NOLINT
  class Base;
  class Root;
  class Binary;
  class Unary;
  class Ternary;
  class Integer;
  class Float;
  class String;
  class Character;
  class Boolean;
  class Null;
  class Undefined;
  class Call;
  class List;
  class Assoc;
  class Index;
  class Slice;
  class FString;
  class Identifier;
  class Sequence;
  class PostUnary;
  class LambdaExpr;
  class TemplateCall;
  class RefTy;
  class U1;
  class U8;
  class U16;
  class U32;
  class U64;
  class U128;
  class I8;
  class I16;
  class I32;
  class I64;
  class I128;
  class F16;
  class F32;
  class F64;
  class F128;
  class VoidTy;
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
  class ReturnIf;
  class Break;
  class Continue;
  class If;
  class While;
  class For;
  class Foreach;
  class Case;
  class Switch;
  class ExprStmt;
  class Expr;
  class Stmt;
  class Type;
  class SourceLocationRange;
  class UserComment;
}  // namespace nitrate::parser::SyntaxTree

namespace google::protobuf {
  class Arena;

  template <typename T>
  class RepeatedPtrField;
}  // namespace google::protobuf

#endif
