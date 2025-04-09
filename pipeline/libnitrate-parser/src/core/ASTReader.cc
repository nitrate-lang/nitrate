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
#include <google/protobuf/any.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver.h>
#include <google/protobuf/util/type_resolver_util.h>

#include <boost/multiprecision/cpp_int.hpp>
#include <memory>
#include <nitrate-core/Assert.hh>
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

class ASTReader::PImpl {
public:
  ReaderSourceManager m_rd;
  Result<Expr> m_root;

  PImpl(ReaderSourceManager source_manager) : m_rd(source_manager) {}
};

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

void ASTReader::UnmarshalLocationLocation(const SyntaxTree::SourceLocationRange &in, FlowPtr<Expr> out) {
  auto &m_rd = m_impl->m_rd;

  if (!m_rd) {
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

void ASTReader::UnmarshalCodeComment(
    const ::google::protobuf::RepeatedPtrField<::nitrate::parser::SyntaxTree::UserComment> &in, FlowPtr<Expr> out) {
  std::vector<string> comments;
  comments.reserve(in.size());

  for (const auto &comment : in) {
    comments.emplace_back(comment.comment());
  }

  out->SetComments(comments);
}

auto ASTReader::Unmarshal(const SyntaxTree::Expr &in) -> Result<Expr> {
  switch (in.node_case()) {
    case SyntaxTree::Expr::kDiscarded: {
      auto to_discard = CreateNull();
      to_discard->Discard();
      return to_discard;
    }

    case SyntaxTree::Expr::kUnary: {
      return Unmarshal(in.unary());
    }

    case SyntaxTree::Expr::kBinary: {
      return Unmarshal(in.binary());
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

auto ASTReader::Unmarshal(const SyntaxTree::Type &in) -> Result<Type> {
  switch (in.node_case()) {
    case SyntaxTree::Type::kDiscarded: {
      auto to_discard = CreateInferredType();
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

auto ASTReader::Unmarshal(const SyntaxTree::Type &in, bool is_set) -> Result<Type> {
  if (is_set) {
    return Unmarshal(in);
  }

  return CreateInferredType();
}

auto ASTReader::Unmarshal(const SyntaxTree::NamedTy &in) -> Result<NamedTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateNamed(in.name(), bit_width, minimum, maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto ASTReader::Unmarshal(const SyntaxTree::InferTy &in) -> Result<InferTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateInferredType(bit_width, minimum, maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto ASTReader::Unmarshal(const SyntaxTree::TemplateType &in) -> Result<TemplateType> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum) [[unlikely]] {
    return std::nullopt;
  }

  auto base = Unmarshal(in.base());
  if (!base) {
    return std::nullopt;
  }

  std::vector<CallArg> args;
  args.reserve(in.arguments_size());

  for (const auto &arg : in.arguments()) {
    auto argument = Unmarshal(arg.value());
    if (!argument) [[unlikely]] {
      return std::nullopt;
    }

    args.emplace_back(arg.name(), argument.Unwrap());
  }

  auto type = CreateTemplateType(args, base.Unwrap());
  type->SetWidth(bit_width);
  type->SetRangeBegin(minimum);
  type->SetRangeEnd(maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto ASTReader::Unmarshal(const SyntaxTree::PtrTy &in) -> Result<PtrTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum) [[unlikely]] {
    return std::nullopt;
  }

  auto pointee = Unmarshal(in.pointee());
  if (!pointee) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreatePointer(pointee.Unwrap(), in.volatile_(), bit_width, minimum, maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto ASTReader::Unmarshal(const SyntaxTree::OpaqueTy &in) -> Result<OpaqueTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateOpaque(in.name(), bit_width, minimum, maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto ASTReader::Unmarshal(const SyntaxTree::TupleTy &in) -> Result<TupleTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<FlowPtr<Type>> items;
  items.reserve(in.elements_size());

  for (const auto &element : in.elements()) {
    auto item = Unmarshal(element);
    if (!item) [[unlikely]] {
      return std::nullopt;
    }

    items.push_back(item.Unwrap());
  }

  auto type = CreateTuple(items, bit_width, minimum, maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto ASTReader::Unmarshal(const SyntaxTree::ArrayTy &in) -> Result<ArrayTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum) [[unlikely]] {
    return std::nullopt;
  }

  auto element_type = Unmarshal(in.element_type());
  if (!element_type) [[unlikely]] {
    return std::nullopt;
  }

  auto element_count = Unmarshal(in.element_count());
  if (!element_count) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateArray(element_type.Unwrap(), element_count.Unwrap(), bit_width, minimum, maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto ASTReader::Unmarshal(const SyntaxTree::RefTy &in) -> Result<RefTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum) [[unlikely]] {
    return std::nullopt;
  }

  auto pointee = Unmarshal(in.pointee());
  if (!pointee) [[unlikely]] {
    return std::nullopt;
  }

  auto type = CreateReference(pointee.Unwrap(), in.volatile_(), bit_width, minimum, maximum);

  UnmarshalLocationLocation(in.location(), type);
  UnmarshalCodeComment(in.comments(), type);

  return type;
}

auto ASTReader::Unmarshal(const SyntaxTree::FuncTy &in) -> Result<FuncTy> {
  auto bit_width = Unmarshal(in.bit_width());
  if (in.has_bit_width() && !bit_width) [[unlikely]] {
    return std::nullopt;
  }

  auto minimum = Unmarshal(in.minimum());
  if (in.has_minimum() && !minimum) [[unlikely]] {
    return std::nullopt;
  }

  auto maximum = Unmarshal(in.maximum());
  if (in.has_maximum() && !maximum) [[unlikely]] {
    return std::nullopt;
  }

  auto return_type = Unmarshal(in.return_type(), in.has_return_type());
  if (!return_type) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<FuncParam> parameters;
  parameters.reserve(in.parameters_size());

  for (const auto &param : in.parameters()) {
    auto type = Unmarshal(param.type(), param.has_type());
    if (!type) [[unlikely]] {
      return std::nullopt;
    }

    auto default_value = Unmarshal(param.default_value());
    if (param.has_default_value() && !default_value) [[unlikely]] {
      return std::nullopt;
    }

    parameters.emplace_back(param.name(), type.Unwrap(), default_value);
  }

  std::vector<FlowPtr<Expr>> attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.Unwrap());
  }

  auto type =
      CreateFunctionType(return_type.Unwrap(), parameters, in.variadic(), attributes, bit_width, minimum, maximum);
  if (!type) [[unlikely]] {
    return std::nullopt;
  }

  UnmarshalLocationLocation(in.location(), type.value());
  UnmarshalCodeComment(in.comments(), type.value());

  return type;
}

auto ASTReader::Unmarshal(const SyntaxTree::Unary &in) -> Result<Unary> {
  auto operand = Unmarshal(in.operand());
  if (!operand) [[unlikely]] {
    return std::nullopt;
  }

  auto op = FromOperator(in.operator_());
  if (!op) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateUnary(op.value(), operand.Unwrap(), in.is_postfix());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Binary &in) -> Result<Binary> {
  auto lhs = Unmarshal(in.left());
  if (!lhs) [[unlikely]] {
    return std::nullopt;
  }

  auto rhs = Unmarshal(in.right());
  if (!rhs) [[unlikely]] {
    return std::nullopt;
  }

  auto op = FromOperator(in.operator_());
  if (!op) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateBinary(lhs.Unwrap(), op.value(), rhs.Unwrap());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Integer &in) -> Result<Integer> {
  auto object = CreateInteger(in.number());
  if (!object) [[unlikely]] {
    return std::nullopt;
  }

  UnmarshalLocationLocation(in.location(), object.value());
  UnmarshalCodeComment(in.comments(), object.value());

  return object.value();
}

auto ASTReader::Unmarshal(const SyntaxTree::Float &in) -> Result<Float> {
  auto object = CreateFloat(in.number());
  if (!object) [[unlikely]] {
    return std::nullopt;
  }

  UnmarshalLocationLocation(in.location(), object.value());
  UnmarshalCodeComment(in.comments(), object.value());

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Boolean &in) -> Result<Boolean> {
  auto object = CreateBoolean(in.value());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::String &in) -> Result<String> {
  auto object = CreateString(in.text());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Character &in) -> Result<Character> {
  auto value = in.char_();
  if (value < 0 || value > 255) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateCharacter(value);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Null &in) -> Result<Null> {
  auto object = CreateNull();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Call &in) -> Result<Call> {
  auto callee = Unmarshal(in.callee());
  if (!callee) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<std::pair<string, FlowPtr<Expr>>> arguments;
  arguments.reserve(in.arguments_size());

  for (const auto &arg : in.arguments()) {
    auto value = Unmarshal(arg.value());
    if (!value) [[unlikely]] {
      return std::nullopt;
    }

    arguments.emplace_back(arg.name(), value.Unwrap());
  }

  auto object = CreateCall(arguments, callee.Unwrap());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::TemplateCall &in) -> Result<TemplateCall> {
  auto callee = Unmarshal(in.callee());
  if (!callee) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<CallArg> arguments;
  arguments.reserve(in.arguments_size());

  for (const auto &arg : in.arguments()) {
    auto value = Unmarshal(arg.value());
    if (!value) [[unlikely]] {
      return std::nullopt;
    }

    arguments.emplace_back(arg.name(), value.Unwrap());
  }

  std::vector<CallArg> parameters;
  parameters.reserve(in.template_arguments_size());

  for (const auto &param : in.template_arguments()) {
    auto value = Unmarshal(param.value());
    if (!value) [[unlikely]] {
      return std::nullopt;
    }

    parameters.emplace_back(param.name(), value.Unwrap());
  }

  auto object = CreateTemplateCall(arguments, parameters, callee.Unwrap());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Import &in) -> Result<Import> {
  auto subtree = Unmarshal(in.subtree());
  if (!subtree) [[unlikely]] {
    return std::nullopt;
  }

  auto mode = FromImportMode(in.mode());
  if (!mode) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateImport(in.name(), mode.value(), subtree.Unwrap());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::List &in) -> Result<List> {
  std::vector<FlowPtr<Expr>> items;
  items.reserve(in.elements_size());

  for (const auto &expr : in.elements()) {
    auto expression = Unmarshal(expr);
    if (!expression) [[unlikely]] {
      return std::nullopt;
    }

    items.push_back(expression.Unwrap());
  }

  auto object = CreateList(items);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Assoc &in) -> Result<Assoc> {
  auto key = Unmarshal(in.key());
  if (!key) [[unlikely]] {
    return std::nullopt;
  }

  auto value = Unmarshal(in.value());
  if (!value) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateAssociation(key.Unwrap(), value.Unwrap());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Index &in) -> Result<Index> {
  auto base = Unmarshal(in.base());
  if (!base) [[unlikely]] {
    return std::nullopt;
  }

  auto index = Unmarshal(in.index());
  if (!index) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateIndex(base.Unwrap(), index.Unwrap());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Slice &in) -> Result<Slice> {
  auto base = Unmarshal(in.base());
  if (!base) [[unlikely]] {
    return std::nullopt;
  }

  auto start = Unmarshal(in.start());
  if (!start) [[unlikely]] {
    return std::nullopt;
  }

  auto end = Unmarshal(in.end());
  if (!end) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateSlice(base.Unwrap(), start.Unwrap(), end.Unwrap());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::FString &in) -> Result<FString> {
  std::vector<std::variant<string, FlowPtr<Expr>>> items;
  items.reserve(in.elements_size());

  for (const auto &expr : in.elements()) {
    switch (expr.part_case()) {
      case SyntaxTree::FString::FStringTerm::kExpr: {
        auto expression = Unmarshal(expr.expr());
        if (!expression) [[unlikely]] {
          return std::nullopt;
        }

        items.emplace_back(expression.Unwrap());
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

  auto object = CreateFormatString(items);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Identifier &in) -> Result<Identifier> {
  auto object = CreateIdentifier(in.name());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Block &in) -> Result<Block> {
  std::vector<FlowPtr<Expr>> items;
  items.reserve(in.statements_size());

  for (const auto &stmt : in.statements()) {
    auto statement = Unmarshal(stmt);
    if (!statement) [[unlikely]] {
      return std::nullopt;
    }

    items.push_back(statement.Unwrap());
  }

  auto mode = in.has_safety() ? FromBlockMode(in.safety()) : BlockMode::Unknown;
  if (!mode) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateBlock(items, mode.value());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Variable &in) -> Result<Variable> {
  auto type = Unmarshal(in.type(), in.has_type());
  if (!type) [[unlikely]] {
    return std::nullopt;
  }

  auto value = Unmarshal(in.initial_value());
  if (in.has_initial_value() && !value) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<FlowPtr<Expr>> attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.Unwrap());
  }

  auto varkind = FromVariableKind(in.kind());
  if (!varkind) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateVariable(varkind.value(), in.name(), attributes, type.Unwrap(), value);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Assembly &in) -> Result<Assembly> {
  std::vector<FlowPtr<Expr>> arguments;
  arguments.reserve(in.arguments_size());

  for (const auto &arg : in.arguments()) {
    auto value = Unmarshal(arg);
    if (!value) [[unlikely]] {
      return std::nullopt;
    }

    arguments.push_back(value.Unwrap());
  }

  auto object = CreateAssembly(in.code());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::If &in) -> Result<If> {
  auto condition = Unmarshal(in.condition());
  if (!condition) [[unlikely]] {
    return std::nullopt;
  }

  auto then_block = Unmarshal(in.true_branch());
  if (!then_block) [[unlikely]] {
    return std::nullopt;
  }

  auto else_block = Unmarshal(in.false_branch());
  if (in.has_false_branch() && !else_block) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateIf(condition.Unwrap(), then_block.Unwrap(), else_block);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::While &in) -> Result<While> {
  auto condition = Unmarshal(in.condition());
  if (!condition) [[unlikely]] {
    return std::nullopt;
  }

  auto block = Unmarshal(in.body());
  if (!block) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateWhile(condition.Unwrap(), block.Unwrap());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::For &in) -> Result<For> {
  auto init = Unmarshal(in.init());
  if (in.has_init() && !init) [[unlikely]] {
    return std::nullopt;
  }

  auto condition = Unmarshal(in.condition());
  if (in.has_condition() && !condition) [[unlikely]] {
    return std::nullopt;
  }

  auto update = Unmarshal(in.step());
  if (in.has_step() && !update) [[unlikely]] {
    return std::nullopt;
  }

  auto block = Unmarshal(in.body());
  if (!block) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateFor(init, condition, update, block.Unwrap());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Foreach &in) -> Result<Foreach> {
  auto expression = Unmarshal(in.expression());
  if (!expression) [[unlikely]] {
    return std::nullopt;
  }

  auto block = Unmarshal(in.body());
  if (!block) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateForeach(in.index_name(), in.value_name(), expression.Unwrap(), block.Unwrap());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Break &in) -> Result<Break> {
  auto object = CreateBreak();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Continue &in) -> Result<Continue> {
  auto object = CreateContinue();
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Return &in) -> Result<Return> {
  auto value = Unmarshal(in.value());
  if (in.has_value() && !value) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateReturn(value);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Case &in) -> Result<Case> {
  auto condition = Unmarshal(in.condition());
  if (!condition) [[unlikely]] {
    return std::nullopt;
  }

  auto block = Unmarshal(in.body());
  if (!block) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateCase(condition.Unwrap(), block.Unwrap());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Switch &in) -> Result<Switch> {
  auto condition = Unmarshal(in.condition());
  if (!condition) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<FlowPtr<Case>> cases;
  cases.reserve(in.cases_size());

  for (const auto &c : in.cases()) {
    auto case_statement = Unmarshal(c);
    if (!case_statement) [[unlikely]] {
      return std::nullopt;
    }

    cases.push_back(case_statement.Unwrap());
  }

  auto default_case = Unmarshal(in.default_());
  if (in.has_default_() && !default_case) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateSwitch(condition.Unwrap(), default_case, cases);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Typedef &in) -> Result<Typedef> {
  auto type = Unmarshal(in.type());
  if (!type) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateTypedef(in.name(), type.Unwrap());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Function &in) -> Result<Function> {
  std::vector<FlowPtr<Expr>> attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.Unwrap());
  }

  std::optional<std::vector<TemplateParameter>> template_parameters;
  if (in.has_template_parameters()) {
    template_parameters = std::vector<TemplateParameter>();

    for (const auto &param : in.template_parameters().parameters()) {
      auto type = Unmarshal(param.type(), param.has_type());
      if (!type) [[unlikely]] {
        return std::nullopt;
      }

      auto default_value = Unmarshal(param.default_value());
      if (param.has_default_value() && !default_value) [[unlikely]] {
        return std::nullopt;
      }

      template_parameters->emplace_back(param.name(), type.Unwrap(), default_value);
    }
  }

  std::vector<ASTFactory::FactoryFunctionParameter> parameters;
  parameters.reserve(in.parameters_size());

  for (const auto &param : in.parameters()) {
    auto type = Unmarshal(param.type(), param.has_type());
    if (!type) [[unlikely]] {
      return std::nullopt;
    }

    auto default_value = Unmarshal(param.default_value());
    if (param.has_default_value() && !default_value) [[unlikely]] {
      return std::nullopt;
    }

    parameters.emplace_back(param.name(), type.Unwrap(), default_value);
  }

  auto block = Unmarshal(in.body());
  if (in.has_body() && !block) [[unlikely]] {
    return std::nullopt;
  }

  auto return_type = Unmarshal(in.return_type(), in.has_return_type());
  if (!return_type) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateFunction(in.name(), return_type.Unwrap(), parameters, in.variadic(), block, attributes,
                               template_parameters);
  if (!object) [[unlikely]] {
    return std::nullopt;
  }

  UnmarshalLocationLocation(in.location(), object.value());
  UnmarshalCodeComment(in.comments(), object.value());

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Struct &in) -> Result<Struct> {
  std::vector<FlowPtr<Expr>> attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.Unwrap());
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
    if (!type) [[unlikely]] {
      return std::nullopt;
    }

    auto value = Unmarshal(field.default_value());
    if (field.has_default_value() && !value) [[unlikely]] {
      return std::nullopt;
    }

    fields.emplace_back(FromVisibility(field.visibility()), is_static, field.name(), type.Unwrap(), value);
  }

  std::vector<StructFunction> methods;
  methods.reserve(in.methods_size());

  for (const auto &method : in.methods()) {
    auto func = Unmarshal(method.func());
    if (!func) [[unlikely]] {
      return std::nullopt;
    }

    methods.emplace_back(FromVisibility(method.visibility()), func.Unwrap());
  }

  std::optional<std::vector<TemplateParameter>> template_parameters;
  if (in.has_template_parameters()) {
    template_parameters = std::vector<TemplateParameter>();

    for (const auto &param : in.template_parameters().parameters()) {
      auto type = Unmarshal(param.type(), param.has_type());
      if (!type) [[unlikely]] {
        return std::nullopt;
      }

      auto default_value = Unmarshal(param.default_value());
      if (param.has_default_value() && !default_value) [[unlikely]] {
        return std::nullopt;
      }

      template_parameters->emplace_back(param.name(), type.Unwrap(), default_value);
    }
  }

  auto comptype = FromCompType(in.kind());
  if (!comptype) [[unlikely]] {
    return std::nullopt;
  }

  auto object = CreateStruct(comptype.value(), in.name(), template_parameters, fields, methods, names, attributes);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Enum &in) -> Result<Enum> {
  auto base_type = Unmarshal(in.base_type());
  if (in.has_base_type() && !base_type) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<std::pair<string, NullableFlowPtr<Expr>>> items;
  items.reserve(in.items_size());

  for (const auto &item : in.items()) {
    auto value = Unmarshal(item.value());
    if (item.has_value() && !value) [[unlikely]] {
      return std::nullopt;
    }

    items.emplace_back(item.name(), value);
  }

  auto object = CreateEnum(in.name(), items, base_type);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Scope &in) -> Result<Scope> {
  auto block = Unmarshal(in.body());
  if (!block) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<string> dependencies;
  dependencies.reserve(in.dependencies_size());
  for (const auto &dep : in.dependencies()) {
    dependencies.emplace_back(dep);
  }

  auto object = CreateScope(in.name(), block.Unwrap(), dependencies);
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

auto ASTReader::Unmarshal(const SyntaxTree::Export &in) -> Result<Export> {
  auto block = Unmarshal(in.body());
  if (!block) [[unlikely]] {
    return std::nullopt;
  }

  std::vector<FlowPtr<Expr>> attributes;
  attributes.reserve(in.attributes_size());

  for (const auto &attr : in.attributes()) {
    auto attribute = Unmarshal(attr);
    if (!attribute) [[unlikely]] {
      return std::nullopt;
    }

    attributes.push_back(attribute.Unwrap());
  }

  auto object = CreateExport(block.Unwrap(), attributes, FromVisibility(in.visibility()), in.abi_name());
  UnmarshalLocationLocation(in.location(), object);
  UnmarshalCodeComment(in.comments(), object);

  return object;
}

ASTReader::ASTReader(std::string_view buf, Format format, std::pmr::memory_resource &pool,
                     ReaderSourceManager source_manager)
    : ASTFactory(pool), m_impl(std::make_unique<PImpl>(source_manager)) {
  SyntaxTree::Expr root;

  switch (format) {
    case Format::PROTO: {
      google::protobuf::io::CodedInputStream input((const uint8_t *)buf.data(), buf.size());
      input.SetRecursionLimit(kRecursionLimit);
      if (!root.ParseFromCodedStream(&input)) [[unlikely]] {
        return;
      }

      break;
    }

    case Format::JSON: {
      std::string binary_data;

      {
        auto resolver = std::unique_ptr<google::protobuf::util::TypeResolver>(
            google::protobuf::util::NewTypeResolverForDescriptorPool(
                "type.googleapis.com", google::protobuf::DescriptorPool::generated_pool()));

        auto json_input_stream = google::protobuf::io::ArrayInputStream((const uint8_t *)buf.data(), buf.size());
        auto coded_output_stream = google::protobuf::io::StringOutputStream(&binary_data);
        google::protobuf::util::JsonParseOptions options;

        const auto type_url = "type.googleapis.com/" + SyntaxTree::Expr::GetDescriptor()->full_name();
        auto status = google::protobuf::util::JsonToBinaryStream(resolver.get(), type_url, &json_input_stream,
                                                                 &coded_output_stream, options);
        if (!status.ok()) [[unlikely]] {
          return;
        }
      }

      google::protobuf::io::CodedInputStream input((const uint8_t *)binary_data.data(), binary_data.size());
      input.SetRecursionLimit(kRecursionLimit);
      if (!root.ParseFromCodedStream(&input)) [[unlikely]] {
        return;
      }

      break;
    }
  }

  root.CheckInitialized();

  m_impl->m_root = Unmarshal(root);
}

ASTReader::~ASTReader() = default;

auto ASTReader::Get() -> NullableFlowPtr<Expr> {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_root;
}
