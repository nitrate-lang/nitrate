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

#include <core/SyntaxTree.pb.h>

#include <iostream>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTCommon.hh>
#include <nitrate-parser/ASTReader.hh>

using namespace ncc;
using namespace ncc::parse;
using namespace nitrate::parser;

static parse::SafetyMode FromSafetyMode(SyntaxTree::SafetyMode mode) noexcept {
  switch (mode) {
    case SyntaxTree::SafetyMode::Safe: {
      return parse::SafetyMode::Safe;
    }

    case SyntaxTree::SafetyMode::Unsafe: {
      return parse::SafetyMode::Unsafe;
    }

    case SyntaxTree::SafetyMode::Unspecified: {
      return parse::SafetyMode::Unknown;
    }
  }
}

void AstReader::UnmarshalLocationLocation(
    const SyntaxTree::SourceLocationRange &in, const FlowPtr<Base> &out) {
  if (!m_rd.has_value()) {
    return;
  }

  lex::LocationID start_loc;
  lex::LocationID end_loc;

  if (in.has_start()) {
    auto line = in.start().line();
    auto column = in.start().column();
    auto offset = in.start().offset();
    auto filename = in.start().has_file() ? in.start().file() : "";

    start_loc = m_rd->get().InternLocation(
        lex::Location(offset, line, column, filename));
  }

  if (in.has_end()) {
    auto line = in.end().line();
    auto column = in.end().column();
    auto offset = in.end().offset();
    auto filename = in.end().has_file() ? in.end().file() : "";

    end_loc = m_rd->get().InternLocation(
        lex::Location(offset, line, column, filename));
  }

  out->SetLoc(start_loc, end_loc);
}

void AstReader::UnmarshalCodeComment(
    const ::google::protobuf::RepeatedPtrField<
        ::nitrate::parser::SyntaxTree::UserComment> &in,
    const FlowPtr<Base> &out) {
  return;

  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
  (void)out;
}

auto AstReader::Unmarshal(const SyntaxTree::Root &in) -> Result<Base> {
  switch (in.node_case()) {
    case SyntaxTree::Root::kBase: {
      return Unmarshal(in.base());
    }

    case SyntaxTree::Root::kStmtExpr: {
      return Unmarshal(in.stmt_expr());
    }

    case SyntaxTree::Root::kTypeExpr: {
      return Unmarshal(in.type_expr());
    }

    case SyntaxTree::Root::kUnary: {
      return Unmarshal(in.unary());
    }

    case SyntaxTree::Root::kBinary: {
      return Unmarshal(in.binary());
    }

    case SyntaxTree::Root::kPostUnary: {
      return Unmarshal(in.post_unary());
    }

    case SyntaxTree::Root::kTernary: {
      return Unmarshal(in.ternary());
    }

    case SyntaxTree::Root::kInteger: {
      return Unmarshal(in.integer());
    }

    case SyntaxTree::Root::kFloat: {
      return Unmarshal(in.float_());
    }

    case SyntaxTree::Root::kBoolean: {
      return Unmarshal(in.boolean());
    }

    case SyntaxTree::Root::kString: {
      return Unmarshal(in.string());
    }

    case SyntaxTree::Root::kCharacter: {
      return Unmarshal(in.character());
    }

    case SyntaxTree::Root::kNull: {
      return Unmarshal(in.null());
    }

    case SyntaxTree::Root::kUndefined: {
      return Unmarshal(in.undefined());
    }

    case SyntaxTree::Root::kCall: {
      return Unmarshal(in.call());
    }

    case SyntaxTree::Root::kTemplateCall: {
      return Unmarshal(in.template_call());
    }

    case SyntaxTree::Root::kList: {
      return Unmarshal(in.list());
    }

    case SyntaxTree::Root::kAssoc: {
      return Unmarshal(in.assoc());
    }

    case SyntaxTree::Root::kIndex: {
      return Unmarshal(in.index());
    }

    case SyntaxTree::Root::kSlice: {
      return Unmarshal(in.slice());
    }

    case SyntaxTree::Root::kFstring: {
      return Unmarshal(in.fstring());
    }

    case SyntaxTree::Root::kIdentifier: {
      return Unmarshal(in.identifier());
    }

    case SyntaxTree::Root::kSequence: {
      return Unmarshal(in.sequence());
    }

    case SyntaxTree::Root::kExpr: {
      return Unmarshal(in.expr());
    }

    case SyntaxTree::Root::kBlock: {
      return Unmarshal(in.block());
    }

    case SyntaxTree::Root::kVariable: {
      return Unmarshal(in.variable());
    }

    case SyntaxTree::Root::kAssembly: {
      return Unmarshal(in.assembly());
    }

    case SyntaxTree::Root::kIf: {
      return Unmarshal(in.if_());
    }

    case SyntaxTree::Root::kWhile: {
      return Unmarshal(in.while_());
    }

    case SyntaxTree::Root::kFor: {
      return Unmarshal(in.for_());
    }

    case SyntaxTree::Root::kForeach: {
      return Unmarshal(in.foreach ());
    }

    case SyntaxTree::Root::kBreak: {
      return Unmarshal(in.break_());
    }

    case SyntaxTree::Root::kContinue: {
      return Unmarshal(in.continue_());
    }

    case SyntaxTree::Root::kReturn: {
      return Unmarshal(in.return_());
    }

    case SyntaxTree::Root::kReturnIf: {
      return Unmarshal(in.return_if());
    }

    case SyntaxTree::Root::kCase: {
      return Unmarshal(in.case_());
    }

    case SyntaxTree::Root::kSwitch: {
      return Unmarshal(in.switch_());
    }

    case SyntaxTree::Root::kExport: {
      return Unmarshal(in.export_());
    }

    case SyntaxTree::Root::kScope: {
      return Unmarshal(in.scope());
    }

    case SyntaxTree::Root::kTypedef: {
      return Unmarshal(in.typedef_());
    }

    case SyntaxTree::Root::kEnum: {
      return Unmarshal(in.enum_());
    }

    case SyntaxTree::Root::kFunction: {
      return Unmarshal(in.function());
    }

    case SyntaxTree::Root::kStruct: {
      return Unmarshal(in.struct_());
    }

    case SyntaxTree::Root::kNamed: {
      return Unmarshal(in.named());
    }

    case SyntaxTree::Root::kInfer: {
      return Unmarshal(in.infer());
    }

    case SyntaxTree::Root::kTemplate: {
      return Unmarshal(in.template_());
    }

    case SyntaxTree::Root::kU1: {
      return Unmarshal(in.u1());
    }

    case SyntaxTree::Root::kU8: {
      return Unmarshal(in.u8());
    }

    case SyntaxTree::Root::kU16: {
      return Unmarshal(in.u16());
    }

    case SyntaxTree::Root::kU32: {
      return Unmarshal(in.u32());
    }

    case SyntaxTree::Root::kU64: {
      return Unmarshal(in.u64());
    }

    case SyntaxTree::Root::kU128: {
      return Unmarshal(in.u128());
    }

    case SyntaxTree::Root::kI8: {
      return Unmarshal(in.i8());
    }

    case SyntaxTree::Root::kI16: {
      return Unmarshal(in.i16());
    }

    case SyntaxTree::Root::kI32: {
      return Unmarshal(in.i32());
    }

    case SyntaxTree::Root::kI64: {
      return Unmarshal(in.i64());
    }

    case SyntaxTree::Root::kI128: {
      return Unmarshal(in.i128());
    }

    case SyntaxTree::Root::kF16: {
      return Unmarshal(in.f16());
    }

    case SyntaxTree::Root::kF32: {
      return Unmarshal(in.f32());
    }

    case SyntaxTree::Root::kF64: {
      return Unmarshal(in.f64());
    }

    case SyntaxTree::Root::kF128: {
      return Unmarshal(in.f128());
    }

    case SyntaxTree::Root::kVoid: {
      return Unmarshal(in.void_());
    }

    case SyntaxTree::Root::kPtr: {
      return Unmarshal(in.ptr());
    }

    case SyntaxTree::Root::kOpaque: {
      return Unmarshal(in.opaque());
    }

    case SyntaxTree::Root::kTuple: {
      return Unmarshal(in.tuple());
    }

    case SyntaxTree::Root::kArray: {
      return Unmarshal(in.array());
    }

    case SyntaxTree::Root::kRef: {
      return Unmarshal(in.ref());
    }

    case SyntaxTree::Root::kFunc: {
      return Unmarshal(in.func());
    }

    case SyntaxTree::Root::NODE_NOT_SET: {
      return std::nullopt;
    }
  }
}

auto AstReader::Unmarshal(const SyntaxTree::Expr &in) -> Result<Expr> {
  switch (in.node_case()) {
    case SyntaxTree::Expr::kBase: {
      return Unmarshal(in.base());
    }

    case SyntaxTree::Expr::kStmtExpr: {
      return Unmarshal(in.stmt_expr());
    }

    case SyntaxTree::Expr::kTypeExpr: {
      return Unmarshal(in.type_expr());
    }

    case SyntaxTree::Expr::kUnary: {
      return Unmarshal(in.unary());
    }

    case SyntaxTree::Expr::kBinary: {
      return Unmarshal(in.binary());
    }

    case SyntaxTree::Expr::kPostUnary: {
      return Unmarshal(in.post_unary());
    }

    case SyntaxTree::Expr::kTernary: {
      return Unmarshal(in.ternary());
    }

    case SyntaxTree::Expr::kInteger: {
      return Unmarshal(in.integer());
    }

    case SyntaxTree::Expr::kFloat: {
      return Unmarshal(in.float_());
    }

    case SyntaxTree::Expr::kBoolean: {
      return Unmarshal(in.boolean());
    }

    case SyntaxTree::Expr::kString: {
      return Unmarshal(in.string());
    }

    case SyntaxTree::Expr::kCharacter: {
      return Unmarshal(in.character());
    }

    case SyntaxTree::Expr::kNull: {
      return Unmarshal(in.null());
    }

    case SyntaxTree::Expr::kUndefined: {
      return Unmarshal(in.undefined());
    }

    case SyntaxTree::Expr::kCall: {
      return Unmarshal(in.call());
    }

    case SyntaxTree::Expr::kTemplateCall: {
      return Unmarshal(in.template_call());
    }

    case SyntaxTree::Expr::kList: {
      return Unmarshal(in.list());
    }

    case SyntaxTree::Expr::kAssoc: {
      return Unmarshal(in.assoc());
    }

    case SyntaxTree::Expr::kIndex: {
      return Unmarshal(in.index());
    }

    case SyntaxTree::Expr::kSlice: {
      return Unmarshal(in.slice());
    }

    case SyntaxTree::Expr::kFstring: {
      return Unmarshal(in.fstring());
    }

    case SyntaxTree::Expr::kIdentifier: {
      return Unmarshal(in.identifier());
    }

    case SyntaxTree::Expr::kSequence: {
      return Unmarshal(in.sequence());
    }

    case SyntaxTree::Expr::NODE_NOT_SET: {
      return std::nullopt;
    }
  }
}

auto AstReader::Unmarshal(const SyntaxTree::Stmt &in) -> Result<Stmt> {
  switch (in.node_case()) {
    case SyntaxTree::Stmt::kExprStmt: {
      return Unmarshal(in.expr_stmt());
    }

    case SyntaxTree::Stmt::kBase: {
      return Unmarshal(in.base());
    }

    case SyntaxTree::Stmt::kBlock: {
      return Unmarshal(in.block());
    }

    case SyntaxTree::Stmt::kVariable: {
      return Unmarshal(in.variable());
    }

    case SyntaxTree::Stmt::kAssembly: {
      return Unmarshal(in.assembly());
    }

    case SyntaxTree::Stmt::kIf: {
      return Unmarshal(in.if_());
    }

    case SyntaxTree::Stmt::kWhile: {
      return Unmarshal(in.while_());
    }

    case SyntaxTree::Stmt::kFor: {
      return Unmarshal(in.for_());
    }

    case SyntaxTree::Stmt::kForeach: {
      return Unmarshal(in.foreach ());
    }

    case SyntaxTree::Stmt::kBreak: {
      return Unmarshal(in.break_());
    }

    case SyntaxTree::Stmt::kContinue: {
      return Unmarshal(in.continue_());
    }

    case SyntaxTree::Stmt::kReturn: {
      return Unmarshal(in.return_());
    }

    case SyntaxTree::Stmt::kReturnIf: {
      return Unmarshal(in.return_if());
    }

    case SyntaxTree::Stmt::kCase: {
      return Unmarshal(in.case_());
    }

    case SyntaxTree::Stmt::kSwitch: {
      return Unmarshal(in.switch_());
    }

    case SyntaxTree::Stmt::kExport: {
      return Unmarshal(in.export_());
    }

    case SyntaxTree::Stmt::kScope: {
      return Unmarshal(in.scope());
    }

    case SyntaxTree::Stmt::kTypedef: {
      return Unmarshal(in.typedef_());
    }

    case SyntaxTree::Stmt::kEnum: {
      return Unmarshal(in.enum_());
    }

    case SyntaxTree::Stmt::kFunction: {
      return Unmarshal(in.function());
    }

    case SyntaxTree::Stmt::kStruct: {
      return Unmarshal(in.struct_());
    }

    case SyntaxTree::Stmt::NODE_NOT_SET: {
      return std::nullopt;
    }
  }
}

auto AstReader::Unmarshal(const SyntaxTree::Type &in) -> Result<Type> {
  switch (in.node_case()) {
    case SyntaxTree::Type::kBase: {
      return Unmarshal(in.base());
    }

    case SyntaxTree::Type::kNamed: {
      return Unmarshal(in.named());
    }

    case SyntaxTree::Type::kInfer: {
      return Unmarshal(in.infer());
    }

    case SyntaxTree::Type::kTemplate: {
      return Unmarshal(in.template_());
    }

    case SyntaxTree::Type::kU1: {
      return Unmarshal(in.u1());
    }

    case SyntaxTree::Type::kU8: {
      return Unmarshal(in.u8());
    }

    case SyntaxTree::Type::kU16: {
      return Unmarshal(in.u16());
    }

    case SyntaxTree::Type::kU32: {
      return Unmarshal(in.u32());
    }

    case SyntaxTree::Type::kU64: {
      return Unmarshal(in.u64());
    }

    case SyntaxTree::Type::kU128: {
      return Unmarshal(in.u128());
    }

    case SyntaxTree::Type::kI8: {
      return Unmarshal(in.i8());
    }

    case SyntaxTree::Type::kI16: {
      return Unmarshal(in.i16());
    }

    case SyntaxTree::Type::kI32: {
      return Unmarshal(in.i32());
    }

    case SyntaxTree::Type::kI64: {
      return Unmarshal(in.i64());
    }

    case SyntaxTree::Type::kI128: {
      return Unmarshal(in.i128());
    }

    case SyntaxTree::Type::kF16: {
      return Unmarshal(in.f16());
    }

    case SyntaxTree::Type::kF32: {
      return Unmarshal(in.f32());
    }

    case SyntaxTree::Type::kF64: {
      return Unmarshal(in.f64());
    }

    case SyntaxTree::Type::kF128: {
      return Unmarshal(in.f128());
    }

    case SyntaxTree::Type::kVoid: {
      return Unmarshal(in.void_());
    }

    case SyntaxTree::Type::kPtr: {
      return Unmarshal(in.ptr());
    }

    case SyntaxTree::Type::kOpaque: {
      return Unmarshal(in.opaque());
    }

    case SyntaxTree::Type::kTuple: {
      return Unmarshal(in.tuple());
    }

    case SyntaxTree::Type::kArray: {
      return Unmarshal(in.array());
    }

    case SyntaxTree::Type::kRef: {
      return Unmarshal(in.ref());
    }

    case SyntaxTree::Type::kFunc: {
      return Unmarshal(in.func());
    }

    case SyntaxTree::Type::NODE_NOT_SET: {
      return std::nullopt;
    }
  }
}

auto AstReader::Unmarshal(const SyntaxTree::Base &in) -> Result<Base> {
  auto object = CreateNode<Base>(QAST_BASE)();

  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::ExprStmt &in) -> Result<ExprStmt> {
  auto expression = Unmarshal(in.expression());
  if (!expression.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<ExprStmt>(expression.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::StmtExpr &in) -> Result<StmtExpr> {
  auto statement = Unmarshal(in.statement());
  if (!statement.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<StmtExpr>(statement.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::TypeExpr &in) -> Result<TypeExpr> {
  auto type = Unmarshal(in.type());
  if (!type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<TypeExpr>(type.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::NamedTy &in) -> Result<NamedTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::InferTy &in) -> Result<InferTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::TemplateType &in)
    -> Result<TemplateType> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::U1 &in) -> Result<U1> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::U8 &in) -> Result<U8> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::U16 &in) -> Result<U16> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::U32 &in) -> Result<U32> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::U64 &in) -> Result<U64> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::U128 &in) -> Result<U128> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::I8 &in) -> Result<I8> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::I16 &in) -> Result<I16> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::I32 &in) -> Result<I32> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::I64 &in) -> Result<I64> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::I128 &in) -> Result<I128> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::F16 &in) -> Result<F16> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::F32 &in) -> Result<F32> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::F64 &in) -> Result<F64> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::F128 &in) -> Result<F128> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::VoidTy &in) -> Result<VoidTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::PtrTy &in) -> Result<PtrTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::OpaqueTy &in) -> Result<OpaqueTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::TupleTy &in) -> Result<TupleTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::ArrayTy &in) -> Result<ArrayTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::RefTy &in) -> Result<RefTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::FuncTy &in) -> Result<FuncTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Unary &in) -> Result<Unary> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Binary &in) -> Result<Binary> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::PostUnary &in)
    -> Result<PostUnary> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Ternary &in) -> Result<Ternary> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Integer &in) -> Result<Integer> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Float &in) -> Result<Float> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Boolean &in) -> Result<Boolean> {
  auto object = CreateNode<Boolean>(in.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::String &in) -> Result<String> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Character &in)
    -> Result<Character> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Null &in) -> Result<Null> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Undefined &in)
    -> Result<Undefined> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Call &in) -> Result<Call> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::TemplateCall &in)
    -> Result<TemplateCall> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::List &in) -> Result<List> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Assoc &in) -> Result<Assoc> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Index &in) -> Result<Index> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Slice &in) -> Result<Slice> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::FString &in) -> Result<FString> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Identifier &in)
    -> Result<Identifier> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Sequence &in) -> Result<Sequence> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Block &in) -> Result<Block> {
  BlockItems items;
  items.reserve(in.statements_size());

  for (const auto &stmt : in.statements()) {
    auto statement = Unmarshal(stmt);
    if (!statement.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    items.push_back(statement.value());
  }

  auto object = CreateNode<Block>(items, FromSafetyMode(in.guarantor()))();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Variable &in) -> Result<Variable> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Assembly &in) -> Result<Assembly> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::If &in) -> Result<If> {
  auto condition = Unmarshal(in.condition());
  if (!condition.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto then_block = Unmarshal(in.true_branch());
  if (!then_block.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto else_block = Unmarshal(in.false_branch());
  if (in.has_false_branch() && !else_block.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object =
      CreateNode<If>(condition.value(), then_block.value(), else_block.get())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::While &in) -> Result<While> {
  auto condition = Unmarshal(in.condition());
  if (!condition.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto block = Unmarshal(in.body());
  if (!block.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<While>(condition.value(), block.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::For &in) -> Result<For> {
  auto init = Unmarshal(in.init());
  if (in.has_init() && !init.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto condition = Unmarshal(in.condition());
  if (in.has_condition() && !condition.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto update = Unmarshal(in.step());
  if (in.has_step() && !update.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto block = Unmarshal(in.body());
  if (!block.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<For>(init.value(), condition.value(), update.value(),
                                block.value())();

  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Foreach &in) -> Result<Foreach> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Break &in) -> Result<Break> {
  auto object = CreateNode<Break>()();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Continue &in) -> Result<Continue> {
  auto object = CreateNode<Continue>()();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Return &in) -> Result<Return> {
  auto value = Unmarshal(in.value());
  if (in.has_value() && !value.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Return>(value.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::ReturnIf &in) -> Result<ReturnIf> {
  auto condition = Unmarshal(in.condition());
  if (!condition.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto value = Unmarshal(in.value());
  if (!value.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<ReturnIf>(condition.value(), value.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Case &in) -> Result<Case> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Switch &in) -> Result<Switch> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Typedef &in) -> Result<Typedef> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Function &in) -> Result<Function> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Struct &in) -> Result<Struct> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Enum &in) -> Result<Enum> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Scope &in) -> Result<Scope> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Export &in) -> Result<Export> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

AstReader::AstReader(std::istream &protobuf_data,
                     ReaderSourceManager source_manager)
    : m_rd(source_manager), m_mm(std::make_unique<DynamicArena>()) {
  SyntaxTree::Root root;
  if (!root.ParseFromIstream(&protobuf_data)) [[unlikely]] {
    return;
  }

  std::swap(NparAllocator, m_mm);
  m_root = Unmarshal(root);
  std::swap(NparAllocator, m_mm);
}

AstReader::AstReader(std::string_view protobuf_data,
                     ReaderSourceManager source_manager)
    : m_rd(source_manager), m_mm(std::make_unique<DynamicArena>()) {
  SyntaxTree::Root root;
  if (!root.ParseFromArray(protobuf_data.data(), protobuf_data.size()))
      [[unlikely]] {
    return;
  }

  std::swap(NparAllocator, m_mm);
  m_root = Unmarshal(root);
  std::swap(NparAllocator, m_mm);
}

auto AstReader::Get() -> std::optional<ASTRoot> {
  if (!m_root.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  return ASTRoot(m_root.value(), std::move(m_mm), false);
}
