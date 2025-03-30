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
#include <google/protobuf/io/coded_stream.h>

#include <boost/multiprecision/cpp_int.hpp>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTReader.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>

static constexpr int kRecursionLimit = INT_MAX;

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;
using namespace nitrate::parser;

static NCC_FORCE_INLINE std::optional<parse::BlockMode> FromBlockMode(SyntaxTree::Block_Safety mode) noexcept {
  switch (mode) {
    case SyntaxTree::Block_Safety_Unspecified: {
      return std::nullopt;
    }

    case SyntaxTree::Block_Safety_Safe: {
      return parse::BlockMode::Safe;
    }

    case SyntaxTree::Block_Safety_Unsafe: {
      return parse::BlockMode::Unsafe;
    }
  }
}

static NCC_FORCE_INLINE std::optional<parse::VariableType> FromVariableKind(
    SyntaxTree::Variable::VariableKind type) noexcept {
  switch (type) {
    case SyntaxTree::Variable_VariableKind_Unspecified: {
      return std::nullopt;
    }

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

static NCC_FORCE_INLINE std::optional<Operator> FromOperator(SyntaxTree::Operator op) {
  switch (op) {
    case SyntaxTree::Op_Unspecified: {
      return std::nullopt;
    }

    case SyntaxTree::Op_Plus: {
      return OpPlus;
    }

    case SyntaxTree::Op_Minus: {
      return OpMinus;
    }

    case SyntaxTree::Op_Times: {
      return OpTimes;
    }

    case SyntaxTree::Op_Slash: {
      return OpSlash;
    }

    case SyntaxTree::Op_Percent: {
      return OpPercent;
    }

    case SyntaxTree::Op_BitAnd: {
      return OpBitAnd;
    }

    case SyntaxTree::Op_BitOr: {
      return OpBitOr;
    }

    case SyntaxTree::Op_BitXor: {
      return OpBitXor;
    }

    case SyntaxTree::Op_BitNot: {
      return OpBitNot;
    }

    case SyntaxTree::Op_LShift: {
      return OpLShift;
    }

    case SyntaxTree::Op_RShift: {
      return OpRShift;
    }

    case SyntaxTree::Op_ROTL: {
      return OpROTL;
    }

    case SyntaxTree::Op_ROTR: {
      return OpROTR;
    }

    case SyntaxTree::Op_LogicAnd: {
      return OpLogicAnd;
    }

    case SyntaxTree::Op_LogicOr: {
      return OpLogicOr;
    }

    case SyntaxTree::Op_LogicXor: {
      return OpLogicXor;
    }

    case SyntaxTree::Op_LogicNot: {
      return OpLogicNot;
    }

    case SyntaxTree::Op_LT: {
      return OpLT;
    }

    case SyntaxTree::Op_GT: {
      return OpGT;
    }

    case SyntaxTree::Op_LE: {
      return OpLE;
    }

    case SyntaxTree::Op_GE: {
      return OpGE;
    }

    case SyntaxTree::Op_Eq: {
      return OpEq;
    }

    case SyntaxTree::Op_NE: {
      return OpNE;
    }

    case SyntaxTree::Op_Set: {
      return OpSet;
    }

    case SyntaxTree::Op_PlusSet: {
      return OpPlusSet;
    }

    case SyntaxTree::Op_MinusSet: {
      return OpMinusSet;
    }

    case SyntaxTree::Op_TimesSet: {
      return OpTimesSet;
    }

    case SyntaxTree::Op_SlashSet: {
      return OpSlashSet;
    }

    case SyntaxTree::Op_PercentSet: {
      return OpPercentSet;
    }

    case SyntaxTree::Op_BitAndSet: {
      return OpBitAndSet;
    }

    case SyntaxTree::Op_BitOrSet: {
      return OpBitOrSet;
    }

    case SyntaxTree::Op_BitXorSet: {
      return OpBitXorSet;
    }

    case SyntaxTree::Op_LogicAndSet: {
      return OpLogicAndSet;
    }

    case SyntaxTree::Op_LogicOrSet: {
      return OpLogicOrSet;
    }

    case SyntaxTree::Op_LogicXorSet: {
      return OpLogicXorSet;
    }

    case SyntaxTree::Op_LShiftSet: {
      return OpLShiftSet;
    }

    case SyntaxTree::Op_RShiftSet: {
      return OpRShiftSet;
    }

    case SyntaxTree::Op_ROTLSet: {
      return OpROTLSet;
    }

    case SyntaxTree::Op_ROTRSet: {
      return OpROTRSet;
    }

    case SyntaxTree::Op_Inc: {
      return OpInc;
    }

    case SyntaxTree::Op_Dec: {
      return OpDec;
    }

    case SyntaxTree::Op_As: {
      return OpAs;
    }

    case SyntaxTree::Op_BitcastAs: {
      return OpBitcastAs;
    }

    case SyntaxTree::Op_In: {
      return OpIn;
    }

    case SyntaxTree::Op_Out: {
      return OpOut;
    }

    case SyntaxTree::Op_Sizeof: {
      return OpSizeof;
    }

    case SyntaxTree::Op_Bitsizeof: {
      return OpBitsizeof;
    }

    case SyntaxTree::Op_Alignof: {
      return OpAlignof;
    }

    case SyntaxTree::Op_Typeof: {
      return OpTypeof;
    }

    case SyntaxTree::Op_Comptime: {
      return OpComptime;
    }

    case SyntaxTree::Op_Dot: {
      return OpDot;
    }

    case SyntaxTree::Op_Range: {
      return OpRange;
    }

    case SyntaxTree::Op_Ellipsis: {
      return OpEllipsis;
    }

    case SyntaxTree::Op_Arrow: {
      return OpArrow;
    }

    case SyntaxTree::Op_Question: {
      return OpTernary;
    }
  }
}

static NCC_FORCE_INLINE parse::Vis FromVisibility(SyntaxTree::Vis vis) {
  switch (vis) {
    case SyntaxTree::Vis_Public: {
      return Vis::Pub;
    }

    case SyntaxTree::Vis_Private: {
      return Vis::Sec;
    }

    case SyntaxTree::Vis_Protected: {
      return Vis::Pro;
    }
  }
}

static NCC_FORCE_INLINE std::optional<parse::CompositeType> FromCompType(SyntaxTree::Struct::AggregateKind kind) {
  switch (kind) {
    case SyntaxTree::Struct_AggregateKind_Unspecified: {
      return std::nullopt;
    }

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

static NCC_FORCE_INLINE std::optional<parse::ImportMode> FromImportMode(SyntaxTree::Import_Mode m) {
  switch (m) {
    case SyntaxTree::Import_Mode_Unspecified: {
      return std::nullopt;
    }

    case SyntaxTree::Import_Mode_Code: {
      return ImportMode::Code;
    }

    case SyntaxTree::Import_Mode_String: {
      return ImportMode::String;
    }
  }
}

void AstReader::UnmarshalLocationLocation(const SyntaxTree::SourceLocationRange &in, FlowPtr<Expr> out) {
  if (!m_rd.has_value()) {
    return;
  }

  LocationID start_loc;
  LocationID end_loc;

  if (in.has_start()) {
    auto line = in.start().line();
    auto column = in.start().column();
    auto offset = in.start().offset();
    auto filename = in.start().has_file() ? in.start().file() : "";

    start_loc = m_rd->get().InternLocation(Location(offset, line, column, filename));
  }

  if (in.has_end()) {
    auto line = in.end().line();
    auto column = in.end().column();
    auto offset = in.end().offset();
    auto filename = in.end().has_file() ? in.end().file() : "";

    end_loc = m_rd->get().InternLocation(Location(offset, line, column, filename));
  }

  out->SetSourcePosition(start_loc, end_loc);
}

void AstReader::UnmarshalCodeComment(
    const ::google::protobuf::RepeatedPtrField<::nitrate::parser::SyntaxTree::UserComment> &in, FlowPtr<Expr> out) {
  std::vector<string> comments;
  comments.reserve(in.size());

  for (const auto &comment : in) {
    comments.emplace_back(comment.comment());
  }

  out->SetComments(comments);
}

auto AstReader::Unmarshal(const SyntaxTree::Expr &in) -> Result<Expr> {
  switch (in.node_case()) {
    case SyntaxTree::Expr::kDiscarded: {
      auto to_discard = m_fac.CreateUndefined();
      to_discard->Discard();
      return to_discard;
    }

    case SyntaxTree::Expr::kUnary: {
      return Unmarshal(in.unary());
    }

    case SyntaxTree::Expr::kBinary: {
      return Unmarshal(in.binary());
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

    case SyntaxTree::Expr::kImport: {
      return Unmarshal(in.import());
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

    case SyntaxTree::Expr::kBlock: {
      return Unmarshal(in.block());
    }

    case SyntaxTree::Expr::kVariable: {
      return Unmarshal(in.variable());
    }

    case SyntaxTree::Expr::kAssembly: {
      return Unmarshal(in.assembly());
    }

    case SyntaxTree::Expr::kIf: {
      return Unmarshal(in.if_());
    }

    case SyntaxTree::Expr::kWhile: {
      return Unmarshal(in.while_());
    }

    case SyntaxTree::Expr::kFor: {
      return Unmarshal(in.for_());
    }

    case SyntaxTree::Expr::kForeach: {
      return Unmarshal(in.foreach ());
    }

    case SyntaxTree::Expr::kBreak: {
      return Unmarshal(in.break_());
    }

    case SyntaxTree::Expr::kContinue: {
      return Unmarshal(in.continue_());
    }

    case SyntaxTree::Expr::kReturn: {
      return Unmarshal(in.return_());
    }

    case SyntaxTree::Expr::kReturnIf: {
      return Unmarshal(in.return_if());
    }

    case SyntaxTree::Expr::kCase: {
      return Unmarshal(in.case_());
    }

    case SyntaxTree::Expr::kSwitch: {
      return Unmarshal(in.switch_());
    }

    case SyntaxTree::Expr::kExport: {
      return Unmarshal(in.export_());
    }

    case SyntaxTree::Expr::kScope: {
      return Unmarshal(in.scope());
    }

    case SyntaxTree::Expr::kTypedef: {
      return Unmarshal(in.typedef_());
    }

    case SyntaxTree::Expr::kEnum: {
      return Unmarshal(in.enum_());
    }

    case SyntaxTree::Expr::kFunction: {
      return Unmarshal(in.function());
    }

    case SyntaxTree::Expr::kStruct: {
      return Unmarshal(in.struct_());
    }

    case SyntaxTree::Expr::kNamed: {
      return Unmarshal(in.named());
    }

    case SyntaxTree::Expr::kInfer: {
      return Unmarshal(in.infer());
    }

    case SyntaxTree::Expr::kTemplate: {
      return Unmarshal(in.template_());
    }

    case SyntaxTree::Expr::kU1: {
      return Unmarshal(in.u1());
    }

    case SyntaxTree::Expr::kU8: {
      return Unmarshal(in.u8());
    }

    case SyntaxTree::Expr::kU16: {
      return Unmarshal(in.u16());
    }

    case SyntaxTree::Expr::kU32: {
      return Unmarshal(in.u32());
    }

    case SyntaxTree::Expr::kU64: {
      return Unmarshal(in.u64());
    }

    case SyntaxTree::Expr::kU128: {
      return Unmarshal(in.u128());
    }

    case SyntaxTree::Expr::kI8: {
      return Unmarshal(in.i8());
    }

    case SyntaxTree::Expr::kI16: {
      return Unmarshal(in.i16());
    }

    case SyntaxTree::Expr::kI32: {
      return Unmarshal(in.i32());
    }

    case SyntaxTree::Expr::kI64: {
      return Unmarshal(in.i64());
    }

    case SyntaxTree::Expr::kI128: {
      return Unmarshal(in.i128());
    }

    case SyntaxTree::Expr::kF16: {
      return Unmarshal(in.f16());
    }

    case SyntaxTree::Expr::kF32: {
      return Unmarshal(in.f32());
    }

    case SyntaxTree::Expr::kF64: {
      return Unmarshal(in.f64());
    }

    case SyntaxTree::Expr::kF128: {
      return Unmarshal(in.f128());
    }

    case SyntaxTree::Expr::kVoid: {
      return Unmarshal(in.void_());
    }

    case SyntaxTree::Expr::kPtr: {
      return Unmarshal(in.ptr());
    }

    case SyntaxTree::Expr::kOpaque: {
      return Unmarshal(in.opaque());
    }

    case SyntaxTree::Expr::kTuple: {
      return Unmarshal(in.tuple());
    }

    case SyntaxTree::Expr::kArray: {
      return Unmarshal(in.array());
    }

    case SyntaxTree::Expr::kRef: {
      return Unmarshal(in.ref());
    }

    case SyntaxTree::Expr::kFunc: {
      return Unmarshal(in.func());
    }

    case SyntaxTree::Expr::NODE_NOT_SET: {
      return std::nullopt;
    }
  }
}

auto AstReader::Unmarshal(const SyntaxTree::Type &in) -> Result<Type> {
  switch (in.node_case()) {
    case SyntaxTree::Type::kDiscarded: {
      auto to_discard = m_fac.CreateVoid();
      to_discard->Discard();
      return to_discard;
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

auto AstReader::Unmarshal(const SyntaxTree::Type &in, bool is_set) -> Result<Type> {
  if (is_set) {
    return Unmarshal(in);
  }

  return m_fac.CreateUnknownType();
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

  auto type = m_fac.CreateNamed(in.name(), bit_width, minimum, maximum);

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

  auto type = m_fac.CreateUnknownType(bit_width, minimum, maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::TemplateType &in) -> Result<TemplateType> {
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

  std::vector<CallArg> args;
  args.reserve(in.arguments_size());

  for (const auto &arg : in.arguments()) {
    auto argument = Unmarshal(arg.value());
    if (!argument.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    args.emplace_back(arg.name(), argument.value());
  }

  auto type = m_fac.CreateTemplateType(args, base.value());
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

  auto type = m_fac.CreateU1(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateU8(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateU16(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateU32(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateU64(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateU128(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateI8(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateI16(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateI32(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateI64(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateI128(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateF16(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateF32(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateF64(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateF128(bit_width, minimum, maximum);

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

  auto type = m_fac.CreateVoid(bit_width, minimum, maximum);

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

  auto type = m_fac.CreatePointer(pointee.value(), in.volatile_(), bit_width, minimum, maximum);

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

  auto type = m_fac.CreateOpaque(in.name(), bit_width, minimum, maximum);

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

  std::vector<FlowPtr<Type>> items;
  items.reserve(in.elements_size());

  for (const auto &element : in.elements()) {
    auto item = Unmarshal(element);
    if (!item.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    items.push_back(item.value());
  }

  auto type = m_fac.CreateTuple(items, bit_width, minimum, maximum);

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

  auto type = m_fac.CreateArray(element_type.value(), element_count.value(), bit_width, minimum, maximum);

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

  auto type = m_fac.CreateReference(pointee.value(), in.volatile_(), bit_width, minimum, maximum);

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

  auto return_type = Unmarshal(in.return_type(), in.has_return_type());
  if (!return_type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<FuncParam> parameters;
  parameters.reserve(in.parameters_size());

  for (const auto &param : in.parameters()) {
    auto type = Unmarshal(param.type(), param.has_type());
    if (!type.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    auto default_value = Unmarshal(param.default_value());
    if (param.has_default_value() && !default_value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    parameters.emplace_back(param.name(), type.value(), default_value);
  }

  std::vector<FlowPtr<Expr>> attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.value());
  }

  auto type =
      m_fac.CreateFunctionType(return_type.value(), parameters, in.variadic(), attributes, bit_width, minimum, maximum);
  if (!type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  UnmarshalLocationLocation(in.location(), type.value());
  UnmarshalCodeComment(in.comments(), type.value());

  return type;
}

auto AstReader::Unmarshal(const SyntaxTree::Unary &in) -> Result<Unary> {
  auto operand = Unmarshal(in.operand());
  if (!operand.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto op = FromOperator(in.operator_());
  if (!op.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = m_fac.CreateUnary(op.value(), operand.value(), in.is_postfix());
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

  auto op = FromOperator(in.operator_());
  if (!op.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = m_fac.CreateBinary(lhs.value(), op.value(), rhs.value());
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

  auto object = m_fac.CreateTernary(condition.value(), true_expr.value(), false_expr.value());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Integer &in) -> Result<Integer> {
  auto object = m_fac.CreateInteger(in.number());
  if (!object.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  UnmarshalLocationLocation(in.location(), object.value());
  UnmarshalCodeComment(in.comments(), object.value());

  return object.value();
}

auto AstReader::Unmarshal(const SyntaxTree::Float &in) -> Result<Float> {
  auto object = m_fac.CreateFloat(in.number());
  if (!object.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  UnmarshalLocationLocation(in.location(), object.value());
  UnmarshalCodeComment(in.comments(), object.value());

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Boolean &in) -> Result<Boolean> {
  auto object = m_fac.CreateBoolean(in.value());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::String &in) -> Result<String> {
  auto object = m_fac.CreateString(in.text());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Character &in) -> Result<Character> {
  auto value = in.char_();
  if (value < 0 || value > 255) [[unlikely]] {
    return std::nullopt;
  }

  auto object = m_fac.CreateCharacter(value);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Null &in) -> Result<Null> {
  auto object = m_fac.CreateNull();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Undefined &in) -> Result<Undefined> {
  auto object = m_fac.CreateUndefined();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Call &in) -> Result<Call> {
  auto callee = Unmarshal(in.callee());
  if (!callee.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<std::pair<string, FlowPtr<Expr>>> arguments;
  arguments.reserve(in.arguments_size());

  for (const auto &arg : in.arguments()) {
    auto value = Unmarshal(arg.value());
    if (!value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    arguments.emplace_back(arg.name(), value.value());
  }

  auto object = m_fac.CreateCall(arguments, callee.value());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::TemplateCall &in) -> Result<TemplateCall> {
  auto callee = Unmarshal(in.callee());
  if (!callee.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<CallArg> arguments;
  arguments.reserve(in.arguments_size());

  for (const auto &arg : in.arguments()) {
    auto value = Unmarshal(arg.value());
    if (!value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    arguments.emplace_back(arg.name(), value.value());
  }

  std::vector<CallArg> parameters;
  parameters.reserve(in.template_arguments_size());

  for (const auto &param : in.template_arguments()) {
    auto value = Unmarshal(param.value());
    if (!value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    parameters.emplace_back(param.name(), value.value());
  }

  auto object = m_fac.CreateTemplateCall(arguments, parameters, callee.value());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Import &in) -> Result<Import> {
  auto subtree = Unmarshal(in.subtree());
  if (!subtree.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto mode = FromImportMode(in.mode());
  if (!mode.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = m_fac.CreateImport(in.name(), mode.value(), subtree.value());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::List &in) -> Result<List> {
  std::vector<FlowPtr<Expr>> items;
  items.reserve(in.elements_size());

  for (const auto &expr : in.elements()) {
    auto expression = Unmarshal(expr);
    if (!expression.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    items.push_back(expression.value());
  }

  auto object = m_fac.CreateList(items);
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

  auto object = m_fac.CreateAssociation(key.value(), value.value());
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

  auto object = m_fac.CreateIndex(base.value(), index.value());
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

  auto object = m_fac.CreateSlice(base.value(), start.value(), end.value());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::FString &in) -> Result<FString> {
  std::vector<std::variant<string, FlowPtr<Expr>>> items;
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

  auto object = m_fac.CreateFormatString(items);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Identifier &in) -> Result<Identifier> {
  auto object = m_fac.CreateIdentifier(in.name());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Block &in) -> Result<Block> {
  std::vector<FlowPtr<Expr>> items;
  items.reserve(in.statements_size());

  for (const auto &stmt : in.statements()) {
    auto statement = Unmarshal(stmt);
    if (!statement.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    items.push_back(statement.value());
  }

  auto mode = in.has_safety() ? FromBlockMode(in.safety()) : BlockMode::Unknown;
  if (!mode.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = m_fac.CreateBlock(items, mode.value());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Variable &in) -> Result<Variable> {
  auto type = Unmarshal(in.type(), in.has_type());
  if (!type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto value = Unmarshal(in.initial_value());
  if (in.has_initial_value() && !value.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<FlowPtr<Expr>> attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.value());
  }

  auto varkind = FromVariableKind(in.kind());
  if (!varkind.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = m_fac.CreateVariable(varkind.value(), in.name(), attributes, type.value(), value);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Assembly &in) -> Result<Assembly> {
  std::vector<FlowPtr<Expr>> arguments;
  arguments.reserve(in.arguments_size());

  for (const auto &arg : in.arguments()) {
    auto value = Unmarshal(arg);
    if (!value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    arguments.push_back(value.value());
  }

  auto object = m_fac.CreateAssembly(in.code());
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

  auto object = m_fac.CreateIf(condition.value(), then_block.value(), else_block);
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

  auto object = m_fac.CreateWhile(condition.value(), block.value());
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

  auto object = m_fac.CreateFor(init, condition, update, block.value());
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

  auto object = m_fac.CreateForeach(in.index_name(), in.value_name(), expression.value(), block.value());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Break &in) -> Result<Break> {
  auto object = m_fac.CreateBreak();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Continue &in) -> Result<Continue> {
  auto object = m_fac.CreateContinue();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Return &in) -> Result<Return> {
  auto value = Unmarshal(in.value());
  if (in.has_value() && !value.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = m_fac.CreateReturn(value);
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
  if (in.has_value() && !value.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = m_fac.CreateReturnIf(condition.value(), value);
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

  auto object = m_fac.CreateCase(condition.value(), block.value());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Switch &in) -> Result<Switch> {
  auto condition = Unmarshal(in.condition());
  if (!condition.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<FlowPtr<Case>> cases;
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

  auto object = m_fac.CreateSwitch(condition.value(), default_case, cases);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Typedef &in) -> Result<Typedef> {
  auto type = Unmarshal(in.type());
  if (!type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = m_fac.CreateTypedef(in.name(), type.value());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Function &in) -> Result<Function> {
  std::vector<FlowPtr<Expr>> attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.value());
  }

  std::optional<std::vector<TemplateParameter>> template_parameters;
  if (in.has_template_parameters()) {
    template_parameters = std::vector<TemplateParameter>();

    for (const auto &param : in.template_parameters().parameters()) {
      auto type = Unmarshal(param.type(), param.has_type());
      if (!type.has_value()) [[unlikely]] {
        return std::nullopt;
      }

      auto default_value = Unmarshal(param.default_value());
      if (param.has_default_value() && !default_value.has_value()) [[unlikely]] {
        return std::nullopt;
      }

      template_parameters->emplace_back(param.name(), type.value(), default_value);
    }
  }

  std::vector<ASTFactory::FactoryFunctionParameter> parameters;
  parameters.reserve(in.parameters_size());

  for (const auto &param : in.parameters()) {
    auto type = Unmarshal(param.type(), param.has_type());
    if (!type.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    auto default_value = Unmarshal(param.default_value());
    if (param.has_default_value() && !default_value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    parameters.emplace_back(param.name(), type.value(), default_value);
  }

  auto block = Unmarshal(in.body());
  if (in.has_body() && !block.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto return_type = Unmarshal(in.return_type(), in.has_return_type());
  if (!return_type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object = m_fac.CreateFunction(in.name(), return_type.value(), parameters, in.variadic(), block, attributes,
                                     template_parameters);
  if (!object.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  UnmarshalLocationLocation(in.location(), object.value());
  UnmarshalCodeComment(in.comments(), object.value());

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Struct &in) -> Result<Struct> {
  std::vector<FlowPtr<Expr>> attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.value());
  }

  std::vector<string> names;
  names.reserve(in.names_size());
  for (const auto &name : in.names()) {
    names.emplace_back(name);
  }

  std::vector<StructField> fields;
  fields.reserve(in.fields_size());

  for (const auto &field : in.fields()) {
    auto is_static = field.is_static();

    auto type = Unmarshal(field.type(), field.has_type());
    if (!type.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    auto value = Unmarshal(field.default_value());
    if (field.has_default_value() && !value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    fields.emplace_back(FromVisibility(field.visibility()), is_static, field.name(), type.value(), value);
  }

  std::vector<StructFunction> methods;
  methods.reserve(in.methods_size());

  for (const auto &method : in.methods()) {
    auto func = Unmarshal(method.func());
    if (!func.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    methods.emplace_back(FromVisibility(method.visibility()), func.value());
  }

  std::optional<std::vector<TemplateParameter>> template_parameters;
  if (in.has_template_parameters()) {
    template_parameters = std::vector<TemplateParameter>();

    for (const auto &param : in.template_parameters().parameters()) {
      auto type = Unmarshal(param.type(), param.has_type());
      if (!type.has_value()) [[unlikely]] {
        return std::nullopt;
      }

      auto default_value = Unmarshal(param.default_value());
      if (param.has_default_value() && !default_value.has_value()) [[unlikely]] {
        return std::nullopt;
      }

      template_parameters->emplace_back(param.name(), type.value(), default_value);
    }
  }

  auto comptype = FromCompType(in.kind());
  if (!comptype.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  auto object =
      m_fac.CreateStruct(comptype.value(), in.name(), template_parameters, fields, methods, names, attributes);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Enum &in) -> Result<Enum> {
  auto base_type = Unmarshal(in.base_type());
  if (in.has_base_type() && !base_type.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<std::pair<string, NullableFlowPtr<Expr>>> items;
  items.reserve(in.items_size());

  for (const auto &item : in.items()) {
    auto value = Unmarshal(item.value());
    if (item.has_value() && !value.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    items.emplace_back(item.name(), value);
  }

  auto object = m_fac.CreateEnum(in.name(), items, base_type);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Scope &in) -> Result<Scope> {
  auto block = Unmarshal(in.body());
  if (!block.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<string> dependencies;
  dependencies.reserve(in.dependencies_size());
  for (const auto &dep : in.dependencies()) {
    dependencies.emplace_back(dep);
  }

  auto object = m_fac.CreateScope(in.name(), block.value(), dependencies);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto AstReader::Unmarshal(const SyntaxTree::Export &in) -> Result<Export> {
  auto block = Unmarshal(in.body());
  if (!block.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<FlowPtr<Expr>> attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute.has_value()) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.value());
  }

  auto object = m_fac.CreateExport(block.value(), attributes, FromVisibility(in.visibility()), in.abi_name());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

AstReader::AstReader(std::string_view protobuf_data, std::pmr::memory_resource &pool,
                     ReaderSourceManager source_manager)
    : m_rd(source_manager), m_fac(pool) {
  google::protobuf::io::CodedInputStream input((const uint8_t *)protobuf_data.data(), protobuf_data.size());
  input.SetRecursionLimit(kRecursionLimit);

  SyntaxTree::Expr root;
  if (!root.ParseFromCodedStream(&input)) [[unlikely]] {
    return;
  }

  m_root = Unmarshal(root);
}

auto AstReader::Get() -> NullableFlowPtr<Expr> {
  if (!m_root.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  return m_root.value();
}
