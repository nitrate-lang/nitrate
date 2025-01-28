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

#include <boost/multiprecision/cpp_int.hpp>
#include <charconv>
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

static NCC_FORCE_INLINE parse::SafetyMode FromSafetyMode(
    SyntaxTree::Block_SafetyMode mode) noexcept {
  switch (mode) {
    case SyntaxTree::Block_SafetyMode_Safe: {
      return parse::SafetyMode::Safe;
    }

    case SyntaxTree::Block_SafetyMode_Unsafe: {
      return parse::SafetyMode::Unsafe;
    }

    case SyntaxTree::Block_SafetyMode_Unspecified: {
      return parse::SafetyMode::Unknown;
    }
  }
}

static NCC_FORCE_INLINE parse::VariableType FromVariableKind(
    SyntaxTree::Variable::VariableKind type) noexcept {
  switch (type) {
    case SyntaxTree::Variable_VariableKind_Let: {
      return parse::VariableType::Let;
    }

    case SyntaxTree::Variable_VariableKind_Var: {
      return parse::VariableType::Var;
    }

    case SyntaxTree::Variable_VariableKind_Const: {
      return parse::VariableType::Const;
    }
  }
}

static NCC_FORCE_INLINE lex::Operator FromOperator(SyntaxTree::Operator op) {
  switch (op) {
    case SyntaxTree::Plus: {
      return lex::OpPlus;
    }

    case SyntaxTree::Minus: {
      return lex::OpMinus;
    }

    case SyntaxTree::Times: {
      return lex::OpTimes;
    }

    case SyntaxTree::Slash: {
      return lex::OpSlash;
    }

    case SyntaxTree::Percent: {
      return lex::OpPercent;
    }

    case SyntaxTree::BitAnd: {
      return lex::OpBitAnd;
    }

    case SyntaxTree::BitOr: {
      return lex::OpBitOr;
    }

    case SyntaxTree::BitXor: {
      return lex::OpBitXor;
    }

    case SyntaxTree::BitNot: {
      return lex::OpBitNot;
    }

    case SyntaxTree::LShift: {
      return lex::OpLShift;
    }

    case SyntaxTree::RShift: {
      return lex::OpRShift;
    }

    case SyntaxTree::ROTL: {
      return lex::OpROTL;
    }

    case SyntaxTree::ROTR: {
      return lex::OpROTR;
    }

    case SyntaxTree::LogicAnd: {
      return lex::OpLogicAnd;
    }

    case SyntaxTree::LogicOr: {
      return lex::OpLogicOr;
    }

    case SyntaxTree::LogicXor: {
      return lex::OpLogicXor;
    }

    case SyntaxTree::LogicNot: {
      return lex::OpLogicNot;
    }

    case SyntaxTree::LT: {
      return lex::OpLT;
    }

    case SyntaxTree::GT: {
      return lex::OpGT;
    }

    case SyntaxTree::LE: {
      return lex::OpLE;
    }

    case SyntaxTree::GE: {
      return lex::OpGE;
    }

    case SyntaxTree::Eq: {
      return lex::OpEq;
    }

    case SyntaxTree::NE: {
      return lex::OpNE;
    }

    case SyntaxTree::Set: {
      return lex::OpSet;
    }

    case SyntaxTree::PlusSet: {
      return lex::OpPlusSet;
    }

    case SyntaxTree::MinusSet: {
      return lex::OpMinusSet;
    }

    case SyntaxTree::TimesSet: {
      return lex::OpTimesSet;
    }

    case SyntaxTree::SlashSet: {
      return lex::OpSlashSet;
    }

    case SyntaxTree::PercentSet: {
      return lex::OpPercentSet;
    }

    case SyntaxTree::BitAndSet: {
      return lex::OpBitAndSet;
    }

    case SyntaxTree::BitOrSet: {
      return lex::OpBitOrSet;
    }

    case SyntaxTree::BitXorSet: {
      return lex::OpBitXorSet;
    }

    case SyntaxTree::LogicAndSet: {
      return lex::OpLogicAndSet;
    }

    case SyntaxTree::LogicOrSet: {
      return lex::OpLogicOrSet;
    }

    case SyntaxTree::LogicXorSet: {
      return lex::OpLogicXorSet;
    }

    case SyntaxTree::LShiftSet: {
      return lex::OpLShiftSet;
    }

    case SyntaxTree::RShiftSet: {
      return lex::OpRShiftSet;
    }

    case SyntaxTree::ROTLSet: {
      return lex::OpROTLSet;
    }

    case SyntaxTree::ROTRSet: {
      return lex::OpROTRSet;
    }

    case SyntaxTree::Inc: {
      return lex::OpInc;
    }

    case SyntaxTree::Dec: {
      return lex::OpDec;
    }

    case SyntaxTree::As: {
      return lex::OpAs;
    }

    case SyntaxTree::BitcastAs: {
      return lex::OpBitcastAs;
    }

    case SyntaxTree::In: {
      return lex::OpIn;
    }

    case SyntaxTree::Out: {
      return lex::OpOut;
    }

    case SyntaxTree::Sizeof: {
      return lex::OpSizeof;
    }

    case SyntaxTree::Bitsizeof: {
      return lex::OpBitsizeof;
    }

    case SyntaxTree::Alignof: {
      return lex::OpAlignof;
    }

    case SyntaxTree::Typeof: {
      return lex::OpTypeof;
    }

    case SyntaxTree::Comptime: {
      return lex::OpComptime;
    }

    case SyntaxTree::Dot: {
      return lex::OpDot;
    }

    case SyntaxTree::Range: {
      return lex::OpRange;
    }

    case SyntaxTree::Ellipsis: {
      return lex::OpEllipsis;
    }

    case SyntaxTree::Arrow: {
      return lex::OpArrow;
    }

    case SyntaxTree::Question: {
      return lex::OpTernary;
    }
  }
}

static NCC_FORCE_INLINE parse::Vis FromVisibility(SyntaxTree::Vis vis) {
  switch (vis) {
    case SyntaxTree::Vis::Public: {
      return Vis::Pub;
    }

    case SyntaxTree::Vis::Private: {
      return Vis::Sec;
    }

    case SyntaxTree::Vis::Protected: {
      return Vis::Pro;
    }
  }
}

static NCC_FORCE_INLINE parse::CompositeType FromCompType(
    SyntaxTree::Struct::AggregateKind kind) {
  switch (kind) {
    case SyntaxTree::Struct_AggregateKind_Struct_: {
      return CompositeType::Struct;
    }

    case SyntaxTree::Struct_AggregateKind_Union_: {
      return CompositeType::Union;
    }

    case SyntaxTree::Struct_AggregateKind_Class_: {
      return CompositeType::Class;
    }

    case SyntaxTree::Struct_AggregateKind_Group_: {
      return CompositeType::Group;
    }

    case SyntaxTree::Struct_AggregateKind_Region_: {
      return CompositeType::Region;
    }
  }
}

static NCC_FORCE_INLINE parse::Purity FromPurity(
    SyntaxTree::FunctionPurity purity) {
  switch (purity) {
    case SyntaxTree::FunctionPurity::Pure: {
      return Purity::Pure;
    }

    case SyntaxTree::FunctionPurity::Impure: {
      return Purity::Impure;
    }

    case SyntaxTree::FunctionPurity::Impure_TSafe: {
      return Purity::Impure_TSafe;
    }

    case SyntaxTree::FunctionPurity::Quasi: {
      return Purity::Quasi;
    }

    case SyntaxTree::FunctionPurity::Retro: {
      return Purity::Retro;
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
  std::vector<string> comments;
  comments.reserve(in.size());

  for (const auto &comment : in) {
    comments.emplace_back(comment.comment());
  }

  out->SetComments(comments);
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
      return CreateNode<Expr>(QAST_BASE)();
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
      return CreateNode<Stmt>(QAST_BASE)();
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
      return CreateNode<Type>(QAST_BASE)();
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
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<NamedTy>(in.name())();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::InferTy &in) -> Result<InferTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<InferTy>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::TemplateType &in)
    -> Result<TemplateType> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto base = Unmarshal(in.base());
  if (!base.has_value()) {
    return std::nullopt;
  }

  CallArgs args;
  args.reserve(in.arguments_size());

  for (const auto &arg : in.arguments()) {
    auto argument = Unmarshal(arg.value());
    if (!argument.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    args.emplace_back(arg.name(), argument.value());
  }

  auto type = CreateNode<TemplateType>(base.value(), args)();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::U1 &in) -> Result<U1> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<U1>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::U8 &in) -> Result<U8> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<U8>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::U16 &in) -> Result<U16> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<U16>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::U32 &in) -> Result<U32> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<U32>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::U64 &in) -> Result<U64> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<U64>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::U128 &in) -> Result<U128> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<U128>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::I8 &in) -> Result<I8> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<I8>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::I16 &in) -> Result<I16> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<I16>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::I32 &in) -> Result<I32> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<I32>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::I64 &in) -> Result<I64> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<I64>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::I128 &in) -> Result<I128> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<I128>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::F16 &in) -> Result<F16> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<F16>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::F32 &in) -> Result<F32> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<F32>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::F64 &in) -> Result<F64> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<F64>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::F128 &in) -> Result<F128> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<F128>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::VoidTy &in) -> Result<VoidTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<VoidTy>()();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::PtrTy &in) -> Result<PtrTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto pointee = Unmarshal(in.pointee());
  if (!pointee.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<PtrTy>(pointee.value(), in.volatile_())();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::OpaqueTy &in) -> Result<OpaqueTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<OpaqueTy>(in.name())();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::TupleTy &in) -> Result<TupleTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  TupleTyItems items;
  items.reserve(in.elements_size());

  for (const auto &element : in.elements()) {
    auto item = Unmarshal(element);
    if (!item.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    items.push_back(item.value());
  }

  auto type = CreateNode<TupleTy>(items)();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::ArrayTy &in) -> Result<ArrayTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto element_type = Unmarshal(in.element_type());
  if (!element_type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto element_count = Unmarshal(in.element_count());
  if (!element_count.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type =
      CreateNode<ArrayTy>(element_type.value(), element_count.value())();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::RefTy &in) -> Result<RefTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto pointee = Unmarshal(in.pointee());
  if (!pointee.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNode<RefTy>(pointee.value())();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::FuncTy &in) -> Result<FuncTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto return_type = Unmarshal(in.return_type());
  if (!return_type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  FuncParams parameters;
  parameters.reserve(in.parameters_size());

  for (const auto &param : in.parameters()) {
    auto type = Unmarshal(param.type());
    if (!type.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    auto default_value = Unmarshal(param.default_value());
    if (param.has_default_value() && !default_value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    parameters.emplace_back(param.name(), type.value(), default_value);
  }

  ExpressionList attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.value());
  }

  auto type = CreateNode<FuncTy>(return_type.value(), parameters, in.variadic(),
                                 FromPurity(in.purity()), attributes)();
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::Unary &in) -> Result<Unary> {
  auto operand = Unmarshal(in.operand());
  if (!operand.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object =
      CreateNode<Unary>(FromOperator(in.operator_()), operand.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Binary &in) -> Result<Binary> {
  auto lhs = Unmarshal(in.left());
  if (!lhs.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto rhs = Unmarshal(in.right());
  if (!rhs.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Binary>(lhs.value(), FromOperator(in.operator_()),
                                   rhs.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::PostUnary &in)
    -> Result<PostUnary> {
  auto operand = Unmarshal(in.operand());
  if (!operand.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object =
      CreateNode<PostUnary>(operand.value(), FromOperator(in.operator_()))();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Ternary &in) -> Result<Ternary> {
  auto condition = Unmarshal(in.condition());
  if (!condition.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto true_expr = Unmarshal(in.true_branch());
  if (!true_expr.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto false_expr = Unmarshal(in.false_branch());
  if (!false_expr.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Ternary>(condition.value(), true_expr.value(),
                                    false_expr.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Integer &in) -> Result<Integer> {
  bool lexically_valid = std::all_of(in.number().begin(), in.number().end(),
                                     [](char c) { return std::isdigit(c); });
  if (!lexically_valid) [[unlikely]] {
    return std::nullopt;
  }

  /* Do range checking */
  boost::multiprecision::cpp_int value(in.number());
  if (value < 0 || value > boost::multiprecision::cpp_int(
                               "340282366920938463463374607431768211455"))
      [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Integer>(in.number())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Float &in) -> Result<Float> {
  long double f = 0.0;

  /* Verify float format  */
  if (std::from_chars(in.number().data(),
                      in.number().data() + in.number().size(), f)
          .ec != std::errc()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Float>(in.number())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Boolean &in) -> Result<Boolean> {
  auto object = CreateNode<Boolean>(in.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::String &in) -> Result<String> {
  auto object = CreateNode<String>(in.text())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Character &in)
    -> Result<Character> {
  auto value = in.char_();
  if (value < 0 || value > 255) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Character>(value)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Null &in) -> Result<Null> {
  auto object = CreateNode<Null>()();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Undefined &in)
    -> Result<Undefined> {
  auto object = CreateNode<Undefined>()();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Call &in) -> Result<Call> {
  auto callee = Unmarshal(in.callee());
  if (!callee.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  CallArgs arguments;
  arguments.reserve(in.arguments_size());

  for (const auto &arg : in.arguments()) {
    auto value = Unmarshal(arg.value());
    if (!value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    arguments.emplace_back(arg.name(), value.value());
  }

  auto object = CreateNode<Call>(callee.value(), arguments)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::TemplateCall &in)
    -> Result<TemplateCall> {
  auto callee = Unmarshal(in.callee());
  if (!callee.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  CallArgs arguments;
  arguments.reserve(in.arguments_size());

  for (const auto &arg : in.arguments()) {
    auto value = Unmarshal(arg.value());
    if (!value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    arguments.emplace_back(arg.name(), value.value());
  }

  CallArgs parameters;
  parameters.reserve(in.template_arguments_size());

  for (const auto &param : in.template_arguments()) {
    auto value = Unmarshal(param.value());
    if (!value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    parameters.emplace_back(param.name(), value.value());
  }

  auto object =
      CreateNode<TemplateCall>(callee.value(), arguments, parameters)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::List &in) -> Result<List> {
  ExpressionList items;
  items.reserve(in.elements_size());

  for (const auto &expr : in.elements()) {
    auto expression = Unmarshal(expr);
    if (!expression.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    items.push_back(expression.value());
  }

  auto object = CreateNode<List>(items)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Assoc &in) -> Result<Assoc> {
  auto key = Unmarshal(in.key());
  if (!key.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto value = Unmarshal(in.value());
  if (!value.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Assoc>(key.value(), value.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Index &in) -> Result<Index> {
  auto base = Unmarshal(in.base());
  if (!base.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto index = Unmarshal(in.index());
  if (!index.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Index>(base.value(), index.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Slice &in) -> Result<Slice> {
  auto base = Unmarshal(in.base());
  if (!base.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto start = Unmarshal(in.start());
  if (!start.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto end = Unmarshal(in.end());
  if (!end.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Slice>(base.value(), start.value(), end.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::FString &in) -> Result<FString> {
  FStringItems items;
  items.reserve(in.elements_size());

  for (const auto &expr : in.elements()) {
    switch (expr.part_case()) {
      case SyntaxTree::FString::FStringTerm::kExpr: {
        auto expression = Unmarshal(expr.expr());
        if (!expression.has_value()) [[unlikely]] {
          return std::nullopt;
        }

        items.emplace_back(expression.value());
        break;
      }

      case SyntaxTree::FString::FStringTerm::kText: {
        items.emplace_back(expr.text());
        break;
      }

      case SyntaxTree::FString::FStringTerm::PART_NOT_SET: {
        return std::nullopt;
      }
    }
  }

  auto object = CreateNode<FString>(items)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Identifier &in)
    -> Result<Identifier> {
  auto object = CreateNode<Identifier>(in.name())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Sequence &in) -> Result<Sequence> {
  ExpressionList items;
  items.reserve(in.elements_size());

  for (const auto &expr : in.elements()) {
    auto expression = Unmarshal(expr);
    if (!expression.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    items.push_back(expression.value());
  }

  auto object = CreateNode<Sequence>(items)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
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
  auto type = Unmarshal(in.type());
  if (in.has_type() && !type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto value = Unmarshal(in.initial_value());
  if (in.has_initial_value() && !value.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  ExpressionList attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.value());
  }

  auto object = CreateNode<Variable>(in.name(), type, value,
                                     FromVariableKind(in.kind()), attributes)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Assembly &in) -> Result<Assembly> {
  ExpressionList arguments;
  arguments.reserve(in.arguments_size());

  for (const auto &arg : in.arguments()) {
    auto value = Unmarshal(arg);
    if (!value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    arguments.push_back(value.value());
  }

  auto object = CreateNode<Assembly>(in.code(), arguments)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
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
      CreateNode<If>(condition.value(), then_block.value(), else_block)();
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

  auto object = CreateNode<For>(init, condition, update, block.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Foreach &in) -> Result<Foreach> {
  auto expression = Unmarshal(in.expression());
  if (!expression.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto block = Unmarshal(in.body());
  if (!block.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Foreach>(in.index_name(), in.value_name(),
                                    expression.value(), block.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
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

  auto object = CreateNode<Return>(value)();
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
  auto condition = Unmarshal(in.condition());
  if (!condition.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto block = Unmarshal(in.body());
  if (!block.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Case>(condition.value(), block.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Switch &in) -> Result<Switch> {
  auto condition = Unmarshal(in.condition());
  if (!condition.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  SwitchCases cases;
  cases.reserve(in.cases_size());

  for (const auto &c : in.cases()) {
    auto case_statement = Unmarshal(c);
    if (!case_statement.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    cases.push_back(case_statement.value());
  }

  auto default_case = Unmarshal(in.default_());
  if (in.has_default_() && !default_case.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Switch>(condition.value(), cases, default_case)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Typedef &in) -> Result<Typedef> {
  auto type = Unmarshal(in.type());
  if (!type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Typedef>(in.name(), type.value())();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Function &in) -> Result<Function> {
  ExpressionList attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.value());
  }

  std::optional<TemplateParameters> template_parameters;
  if (in.has_template_parameters()) {
    template_parameters = TemplateParameters();

    for (const auto &param : in.template_parameters().parameters()) {
      auto type = Unmarshal(param.type());
      if (!type.has_value()) [[unlikely]] {
        return std::nullopt;
      }

      auto default_value = Unmarshal(param.default_value());
      if (param.has_default_value() && !default_value.has_value())
          [[unlikely]] {
        return std::nullopt;
      }

      template_parameters->emplace_back(param.name(), type.value(),
                                        default_value);
    }
  }

  FnCaptures captures;
  captures.reserve(in.captures_size());
  for (const auto &cap : in.captures()) {
    captures.emplace_back(cap.name(), cap.is_reference());
  }

  FuncParams parameters;
  parameters.reserve(in.parameters_size());

  for (const auto &param : in.parameters()) {
    auto type = Unmarshal(param.type());
    if (!type.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    auto default_value = Unmarshal(param.default_value());
    if (param.has_default_value() && !default_value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    parameters.emplace_back(param.name(), type.value(), default_value);
  }

  auto precondition = Unmarshal(in.precondition());
  if (in.has_precondition() && !precondition.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto postcondition = Unmarshal(in.postcondition());
  if (in.has_postcondition() && !postcondition.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto block = Unmarshal(in.body());
  if (in.has_body() && !block.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto return_type = Unmarshal(in.return_type());
  if (!return_type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateNode<Function>(
      attributes, FromPurity(in.purity()), captures, in.name(),
      template_parameters, parameters, in.variadic(), return_type.value(),
      precondition, postcondition, block)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Struct &in) -> Result<Struct> {
  ExpressionList attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.value());
  }

  StructNames names;
  names.reserve(in.names_size());
  for (const auto &name : in.names()) {
    names.emplace_back(name);
  }

  StructFields fields;
  fields.reserve(in.fields_size());

  for (const auto &field : in.fields()) {
    auto vis = FromVisibility(field.visibility());
    auto is_static = field.is_static();

    auto type = Unmarshal(field.type());
    if (!type.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    auto value = Unmarshal(field.default_value());
    if (field.has_default_value() && !value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    fields.emplace_back(vis, is_static, field.name(), type.value(), value);
  }

  StructMethods methods;
  methods.reserve(in.methods_size());

  for (const auto &method : in.methods()) {
    auto vis = FromVisibility(method.visibility());
    auto func = Unmarshal(method.func());
    if (!func.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    methods.emplace_back(vis, func.value());
  }

  StructMethods static_methods;
  static_methods.reserve(in.static_methods_size());

  for (const auto &method : in.static_methods()) {
    auto vis = FromVisibility(method.visibility());
    auto func = Unmarshal(method.func());
    if (!func.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    static_methods.emplace_back(vis, func.value());
  }

  std::optional<TemplateParameters> template_parameters;
  if (in.has_template_parameters()) {
    template_parameters = TemplateParameters();

    for (const auto &param : in.template_parameters().parameters()) {
      auto type = Unmarshal(param.type());
      if (!type.has_value()) [[unlikely]] {
        return std::nullopt;
      }

      auto default_value = Unmarshal(param.default_value());
      if (param.has_default_value() && !default_value.has_value())
          [[unlikely]] {
        return std::nullopt;
      }

      template_parameters->emplace_back(param.name(), type.value(),
                                        default_value);
    }
  }

  auto object = CreateNode<Struct>(FromCompType(in.kind()), attributes,
                                   in.name(), template_parameters, names,
                                   fields, methods, static_methods)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Enum &in) -> Result<Enum> {
  auto base_type = Unmarshal(in.base_type());
  if (in.has_base_type() && !base_type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  EnumItems items;
  items.reserve(in.items_size());

  for (const auto &item : in.items()) {
    auto value = Unmarshal(item.value());
    if (item.has_value() && !value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    items.emplace_back(item.name(), value);
  }

  auto object = CreateNode<Enum>(in.name(), base_type, items)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Scope &in) -> Result<Scope> {
  auto block = Unmarshal(in.body());
  if (!block.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  ScopeDeps dependencies;
  dependencies.reserve(in.dependencies_size());
  for (const auto &dep : in.dependencies()) {
    dependencies.emplace_back(dep);
  }

  auto object = CreateNode<Scope>(in.name(), block.value(), dependencies)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Export &in) -> Result<Export> {
  auto block = Unmarshal(in.body());
  if (!block.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  ExpressionList attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.value());
  }

  auto vis = FromVisibility(in.visibility());

  auto object =
      CreateNode<Export>(block.value(), in.abi_name(), vis, attributes)();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

AstReader::AstReader(std::istream &protobuf_data,
                     ReaderSourceManager source_manager)
    : m_rd(source_manager), m_mm(std::make_unique<DynamicArena>()) {
  SyntaxTree::Root root;
  if (!root.ParseFromIstream(&protobuf_data)) [[unlikely]] {
    return;
  }

  std::swap(MainAllocator, m_mm);
  m_root = Unmarshal(root);
  std::swap(MainAllocator, m_mm);
}

AstReader::AstReader(std::string_view protobuf_data,
                     ReaderSourceManager source_manager)
    : m_rd(source_manager), m_mm(std::make_unique<DynamicArena>()) {
  SyntaxTree::Root root;
  if (!root.ParseFromArray(protobuf_data.data(), protobuf_data.size()))
      [[unlikely]] {
    return;
  }

  std::swap(MainAllocator, m_mm);
  m_root = Unmarshal(root);
  std::swap(MainAllocator, m_mm);
}

auto AstReader::Get() -> std::optional<ASTRoot> {
  if (!m_root.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  return ASTRoot(m_root.value(), std::move(m_mm), false);
}
