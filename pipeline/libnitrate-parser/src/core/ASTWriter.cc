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
#include <google/protobuf/util/json_util.h>

#include <functional>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <variant>

using namespace ncc::parse;
using namespace google;
using namespace nitrate::parser::SyntaxTree;

class ASTWriter::PImpl {
public:
  google::protobuf::Arena *m_pool;
  std::ostream &m_os;
  OptionalSourceProvider m_rd;
  Format m_format;

  PImpl(std::ostream &os, Format format, OptionalSourceProvider rd) : m_os(os), m_rd(rd), m_format(format) {
    m_pool = new google::protobuf::Arena();
  }

  ~PImpl() { delete m_pool; }
};

using Pool = google::protobuf::Arena;

static bool IsCompressable(const ncc::FlowPtr<ncc::parse::Type> &in) {
  return in->Is(AST_tINFER) && in->GetWidth() == nullptr && in->GetRangeBegin() == nullptr &&
         in->GetRangeEnd() == nullptr;
}

static SyntaxTree::Operator FromOperator(ncc::lex::Operator op) {
  using LexOp = ncc::lex::Operator;

  switch (op) {
    case LexOp::OpPlus:
      return SyntaxTree::Op_Plus;

    case LexOp::OpMinus:
      return SyntaxTree::Op_Minus;

    case LexOp::OpTimes:
      return SyntaxTree::Op_Times;

    case LexOp::OpSlash:
      return SyntaxTree::Op_Slash;

    case LexOp::OpPercent:
      return SyntaxTree::Op_Percent;

    case LexOp::OpBitAnd:
      return SyntaxTree::Op_BitAnd;

    case LexOp::OpBitOr:
      return SyntaxTree::Op_BitOr;

    case LexOp::OpBitXor:
      return SyntaxTree::Op_BitXor;

    case LexOp::OpBitNot:
      return SyntaxTree::Op_BitNot;

    case LexOp::OpLShift:
      return SyntaxTree::Op_LShift;

    case LexOp::OpRShift:
      return SyntaxTree::Op_RShift;

    case LexOp::OpROTL:
      return SyntaxTree::Op_ROTL;

    case LexOp::OpROTR:
      return SyntaxTree::Op_ROTR;

    case LexOp::OpLogicAnd:
      return SyntaxTree::Op_LogicAnd;

    case LexOp::OpLogicOr:
      return SyntaxTree::Op_LogicOr;

    case LexOp::OpLogicXor:
      return SyntaxTree::Op_LogicXor;

    case LexOp::OpLogicNot:
      return SyntaxTree::Op_LogicNot;

    case LexOp::OpLT:
      return SyntaxTree::Op_LT;

    case LexOp::OpGT:
      return SyntaxTree::Op_GT;

    case LexOp::OpLE:
      return SyntaxTree::Op_LE;

    case LexOp::OpGE:
      return SyntaxTree::Op_GE;

    case LexOp::OpEq:
      return SyntaxTree::Op_Eq;

    case LexOp::OpNE:
      return SyntaxTree::Op_NE;

    case LexOp::OpSet:
      return SyntaxTree::Op_Set;

    case LexOp::OpPlusSet:
      return SyntaxTree::Op_PlusSet;

    case LexOp::OpMinusSet:
      return SyntaxTree::Op_MinusSet;

    case LexOp::OpTimesSet:
      return SyntaxTree::Op_TimesSet;

    case LexOp::OpSlashSet:
      return SyntaxTree::Op_SlashSet;

    case LexOp::OpPercentSet:
      return SyntaxTree::Op_PercentSet;

    case LexOp::OpBitAndSet:
      return SyntaxTree::Op_BitAndSet;

    case LexOp::OpBitOrSet:
      return SyntaxTree::Op_BitOrSet;

    case LexOp::OpBitXorSet:
      return SyntaxTree::Op_BitXorSet;

    case LexOp::OpLogicAndSet:
      return SyntaxTree::Op_LogicAndSet;

    case LexOp::OpLogicOrSet:
      return SyntaxTree::Op_LogicOrSet;

    case LexOp::OpLogicXorSet:
      return SyntaxTree::Op_LogicXorSet;

    case LexOp::OpLShiftSet:
      return SyntaxTree::Op_LShiftSet;

    case LexOp::OpRShiftSet:
      return SyntaxTree::Op_RShiftSet;

    case LexOp::OpROTLSet:
      return SyntaxTree::Op_ROTLSet;

    case LexOp::OpROTRSet:
      return SyntaxTree::Op_ROTRSet;

    case LexOp::OpInc:
      return SyntaxTree::Op_Inc;

    case LexOp::OpDec:
      return SyntaxTree::Op_Dec;

    case LexOp::OpAs:
      return SyntaxTree::Op_As;

    case LexOp::OpBitcastAs:
      return SyntaxTree::Op_BitcastAs;

    case LexOp::OpIn:
      return SyntaxTree::Op_In;

    case LexOp::OpOut:
      return SyntaxTree::Op_Out;

    case LexOp::OpSizeof:
      return SyntaxTree::Op_Sizeof;

    case LexOp::OpBitsizeof:
      return SyntaxTree::Op_Bitsizeof;

    case LexOp::OpAlignof:
      return SyntaxTree::Op_Alignof;

    case LexOp::OpTypeof:
      return SyntaxTree::Op_Typeof;

    case LexOp::OpComptime:
      return SyntaxTree::Op_Comptime;

    case LexOp::OpDot:
      return SyntaxTree::Op_Dot;

    case LexOp::OpRange:
      return SyntaxTree::Op_Range;

    case LexOp::OpEllipsis:
      return SyntaxTree::Op_Ellipsis;

    case LexOp::OpArrow:
      return SyntaxTree::Op_Arrow;

    case LexOp::OpTernary:
      return SyntaxTree::Op_Question;
  }
}

static SyntaxTree::Vis FromVisibility(ncc::parse::Vis vis) {
  switch (vis) {
    case ncc::parse::Vis::Pub:
      return SyntaxTree::Vis_Public;

    case ncc::parse::Vis::Pro:
      return SyntaxTree::Vis_Protected;

    case ncc::parse::Vis::Sec:
      return SyntaxTree::Vis_Private;
  }
}

static SyntaxTree::Struct_AggregateKind FromStructKind(ncc::parse::CompositeType type) {
  switch (type) {
    case ncc::parse::CompositeType::Struct:
      return SyntaxTree::Struct_AggregateKind_Struct_;

    case ncc::parse::CompositeType::Class:
      return SyntaxTree::Struct_AggregateKind_Class_;

    case ncc::parse::CompositeType::Group:
      return SyntaxTree::Struct_AggregateKind_Group_;

    case ncc::parse::CompositeType::Region:
      return SyntaxTree::Struct_AggregateKind_Region_;

    case ncc::parse::CompositeType::Union:
      return SyntaxTree::Struct_AggregateKind_Union_;
  }
}

static SyntaxTree::Import_Mode FromImportMode(ncc::parse::ImportMode mode) {
  switch (mode) {
    case ncc::parse::ImportMode::Code:
      return SyntaxTree::Import_Mode_Code;

    case ncc::parse::ImportMode::String:
      return SyntaxTree::Import_Mode_String;
  }
}

void ASTWriter::SetTypeMetadata(auto *message, const FlowPtr<Type> &in) {
  if (in->GetWidth()) [[unlikely]] {
    message->set_allocated_bit_width(From(in->GetWidth().Unwrap()));
  }

  if (in->GetRangeBegin()) [[unlikely]] {
    message->set_allocated_minimum(From(in->GetRangeBegin().Unwrap()));
  }

  if (in->GetRangeEnd()) [[unlikely]] {
    message->set_allocated_maximum(From(in->GetRangeEnd().Unwrap()));
  }
}

SyntaxTree::SourceLocationRange *ASTWriter::FromSource(FlowPtr<Expr> in) {
  auto &m_rd = m_impl->m_rd;
  auto &pool = m_impl->m_pool;
  if (!m_rd) {
    return nullptr;
  }

  const auto &pos = in->GetSourcePosition();
  auto start_pos = m_rd->get().GetLocation(pos.first);
  auto end_pos = m_rd->get().GetLocation(pos.second);

  if (start_pos == lex::Location::EndOfFile() && end_pos == lex::Location::EndOfFile()) {
    return nullptr;
  }

  auto *message = Pool::CreateMessage<SyntaxTree::SourceLocationRange>(pool);

  if (start_pos != lex::Location::EndOfFile()) {
    auto *start = Pool::CreateMessage<SyntaxTree::SourceLocationRange_SourceLocation>(pool);
    start->set_line(start_pos.GetRow() + 1);
    start->set_column(start_pos.GetCol() + 1);
    start->set_offset(start_pos.GetOffset());
    start->set_file(start_pos.GetFilename().Get());

    message->set_allocated_start(start);
  }

  if (end_pos != lex::Location::EndOfFile()) {
    auto *end = Pool::CreateMessage<SyntaxTree::SourceLocationRange_SourceLocation>(pool);
    end->set_line(end_pos.GetRow() + 1);
    end->set_column(end_pos.GetCol() + 1);
    end->set_offset(end_pos.GetOffset());
    end->set_file(end_pos.GetFilename().Get());

    message->set_allocated_end(end);
  }

  return message;
}

SyntaxTree::Expr *ASTWriter::From(FlowPtr<Expr> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Expr>(pool);

  switch (in->GetKind()) {
    case AST_DISCARDED: {
      message->set_allocated_discarded(Pool::CreateMessage<SyntaxTree::Discarded>(pool));
      break;
    }

    case AST_eBIN: {
      message->set_allocated_binary(From(in.As<Binary>()));
      break;
    }

    case AST_eUNARY: {
      message->set_allocated_unary(From(in.As<Unary>()));
      break;
    }

    case AST_eINT: {
      message->set_allocated_integer(From(in.As<Integer>()));
      break;
    }

    case AST_eFLOAT: {
      message->set_allocated_float_(From(in.As<Float>()));
      break;
    }

    case AST_eSTRING: {
      message->set_allocated_string(From(in.As<String>()));
      break;
    }

    case AST_eCHAR: {
      message->set_allocated_character(From(in.As<Character>()));
      break;
    }

    case AST_eBOOL: {
      message->set_allocated_boolean(From(in.As<Boolean>()));
      break;
    }

    case AST_eNULL: {
      message->set_allocated_null(From(in.As<Null>()));
      break;
    }

    case AST_eCALL: {
      message->set_allocated_call(From(in.As<Call>()));
      break;
    }

    case AST_eLIST: {
      message->set_allocated_list(From(in.As<List>()));
      break;
    }

    case AST_ePAIR: {
      message->set_allocated_assoc(From(in.As<Assoc>()));
      break;
    }

    case AST_eINDEX: {
      message->set_allocated_index(From(in.As<Index>()));
      break;
    }

    case AST_eSLICE: {
      message->set_allocated_slice(From(in.As<Slice>()));
      break;
    }

    case AST_eFSTRING: {
      message->set_allocated_fstring(From(in.As<FString>()));
      break;
    }

    case AST_eIDENT: {
      message->set_allocated_identifier(From(in.As<Identifier>()));
      break;
    }

    case AST_sIF: {
      message->set_allocated_if_(From(in.As<If>()));
      break;
    }

    case AST_sSWITCH: {
      message->set_allocated_switch_(From(in.As<Switch>()));
      break;
    }

    case AST_sCASE: {
      message->set_allocated_case_(From(in.As<Case>()));
      break;
    }

    case AST_sRET: {
      message->set_allocated_return_(From(in.As<Return>()));
      break;
    }

    case AST_sBRK: {
      message->set_allocated_break_(From(in.As<Break>()));
      break;
    }

    case AST_sCONT: {
      message->set_allocated_continue_(From(in.As<Continue>()));
      break;
    }

    case AST_sWHILE: {
      message->set_allocated_while_(From(in.As<While>()));
      break;
    }

    case AST_sFOR: {
      message->set_allocated_for_(From(in.As<For>()));
      break;
    }

    case AST_sFOREACH: {
      message->set_allocated_foreach(From(in.As<Foreach>()));
      break;
    }

    case AST_sASM: {
      message->set_allocated_assembly(From(in.As<Assembly>()));
      break;
    }

    case AST_sTYPEDEF: {
      message->set_allocated_typedef_(From(in.As<Typedef>()));
      break;
    }

    case AST_sSTRUCT: {
      message->set_allocated_struct_(From(in.As<Struct>()));
      break;
    }

    case AST_sENUM: {
      message->set_allocated_enum_(From(in.As<Enum>()));
      break;
    }

    case AST_sSCOPE: {
      message->set_allocated_scope(From(in.As<Scope>()));
      break;
    }

    case AST_sBLOCK: {
      message->set_allocated_block(From(in.As<Block>()));
      break;
    }

    case AST_sEXPORT: {
      message->set_allocated_export_(From(in.As<Export>()));
      break;
    }

    case AST_sVAR: {
      message->set_allocated_variable(From(in.As<Variable>()));
      break;
    }

    case AST_sFUNCTION: {
      message->set_allocated_function(From(in.As<Function>()));
      break;
    }

    case AST_eTEMPLATE_CALL: {
      message->set_allocated_template_call(From(in.As<TemplateCall>()));
      break;
    }

    case AST_eIMPORT: {
      message->set_allocated_import(From(in.As<Import>()));
      break;
    }

    case AST_tU1: {
      message->set_allocated_u1(From(in.As<U1>()));
      break;
    }

    case AST_tU8: {
      message->set_allocated_u8(From(in.As<U8>()));
      break;
    }

    case AST_tU16: {
      message->set_allocated_u16(From(in.As<U16>()));
      break;
    }

    case AST_tU32: {
      message->set_allocated_u32(From(in.As<U32>()));
      break;
    }

    case AST_tU64: {
      message->set_allocated_u64(From(in.As<U64>()));
      break;
    }

    case AST_tU128: {
      message->set_allocated_u128(From(in.As<U128>()));
      break;
    }

    case AST_tI8: {
      message->set_allocated_i8(From(in.As<I8>()));
      break;
    }

    case AST_tI16: {
      message->set_allocated_i16(From(in.As<I16>()));
      break;
    }

    case AST_tI32: {
      message->set_allocated_i32(From(in.As<I32>()));
      break;
    }

    case AST_tI64: {
      message->set_allocated_i64(From(in.As<I64>()));
      break;
    }

    case AST_tI128: {
      message->set_allocated_i128(From(in.As<I128>()));
      break;
    }

    case AST_tF16: {
      message->set_allocated_f16(From(in.As<F16>()));
      break;
    }

    case AST_tF32: {
      message->set_allocated_f32(From(in.As<F32>()));
      break;
    }

    case AST_tF64: {
      message->set_allocated_f64(From(in.As<F64>()));
      break;
    }

    case AST_tF128: {
      message->set_allocated_f128(From(in.As<F128>()));
      break;
    }

    case AST_tVOID: {
      message->set_allocated_void_(From(in.As<VoidTy>()));
      break;
    }

    case AST_tINFER: {
      message->set_allocated_infer(From(in.As<InferTy>()));
      break;
    }

    case AST_tOPAQUE: {
      message->set_allocated_opaque(From(in.As<OpaqueTy>()));
      break;
    }

    case AST_tNAMED: {
      message->set_allocated_named(From(in.As<NamedTy>()));
      break;
    }

    case AST_tREF: {
      message->set_allocated_ref(From(in.As<RefTy>()));
      break;
    }

    case AST_tPTR: {
      message->set_allocated_ptr(From(in.As<PtrTy>()));
      break;
    }

    case AST_tARRAY: {
      message->set_allocated_array(From(in.As<ArrayTy>()));
      break;
    }

    case AST_tTUPLE: {
      message->set_allocated_tuple(From(in.As<TupleTy>()));
      break;
    }

    case AST_tTEMPLATE: {
      message->set_allocated_template_(From(in.As<TemplateType>()));
      break;
    }

    case AST_tFUNCTION: {
      message->set_allocated_func(From(in.As<FuncTy>()));
      break;
    }
  }

  return message;
}

SyntaxTree::Type *ASTWriter::From(FlowPtr<Type> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Type>(pool);

  switch (in->GetKind()) {
    case AST_tU1: {
      message->set_allocated_u1(From(in.As<U1>()));
      break;
    }

    case AST_tU8: {
      message->set_allocated_u8(From(in.As<U8>()));
      break;
    }

    case AST_tU16: {
      message->set_allocated_u16(From(in.As<U16>()));
      break;
    }

    case AST_tU32: {
      message->set_allocated_u32(From(in.As<U32>()));
      break;
    }

    case AST_tU64: {
      message->set_allocated_u64(From(in.As<U64>()));
      break;
    }

    case AST_tU128: {
      message->set_allocated_u128(From(in.As<U128>()));
      break;
    }

    case AST_tI8: {
      message->set_allocated_i8(From(in.As<I8>()));
      break;
    }

    case AST_tI16: {
      message->set_allocated_i16(From(in.As<I16>()));
      break;
    }

    case AST_tI32: {
      message->set_allocated_i32(From(in.As<I32>()));
      break;
    }

    case AST_tI64: {
      message->set_allocated_i64(From(in.As<I64>()));
      break;
    }

    case AST_tI128: {
      message->set_allocated_i128(From(in.As<I128>()));
      break;
    }

    case AST_tF16: {
      message->set_allocated_f16(From(in.As<F16>()));
      break;
    }

    case AST_tF32: {
      message->set_allocated_f32(From(in.As<F32>()));
      break;
    }

    case AST_tF64: {
      message->set_allocated_f64(From(in.As<F64>()));
      break;
    }

    case AST_tF128: {
      message->set_allocated_f128(From(in.As<F128>()));
      break;
    }

    case AST_tVOID: {
      message->set_allocated_void_(From(in.As<VoidTy>()));
      break;
    }

    case AST_tINFER: {
      message->set_allocated_infer(From(in.As<InferTy>()));
      break;
    }

    case AST_tOPAQUE: {
      message->set_allocated_opaque(From(in.As<OpaqueTy>()));
      break;
    }

    case AST_tNAMED: {
      message->set_allocated_named(From(in.As<NamedTy>()));
      break;
    }

    case AST_tREF: {
      message->set_allocated_ref(From(in.As<RefTy>()));
      break;
    }

    case AST_tPTR: {
      message->set_allocated_ptr(From(in.As<PtrTy>()));
      break;
    }

    case AST_tARRAY: {
      message->set_allocated_array(From(in.As<ArrayTy>()));
      break;
    }

    case AST_tTUPLE: {
      message->set_allocated_tuple(From(in.As<TupleTy>()));
      break;
    }

    case AST_tTEMPLATE: {
      message->set_allocated_template_(From(in.As<TemplateType>()));
      break;
    }

    case AST_tFUNCTION: {
      message->set_allocated_func(From(in.As<FuncTy>()));
      break;
    }

    default: {
      qcore_panicf("Unknown type kind %s", std::string(in->GetKindName()).c_str());
    }
  }

  return message;
}

SyntaxTree::NamedTy *ASTWriter::From(FlowPtr<NamedTy> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::NamedTy>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::InferTy *ASTWriter::From(FlowPtr<InferTy> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::InferTy>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::TemplateType *ASTWriter::From(FlowPtr<TemplateType> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::TemplateType>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_base(From(in->GetTemplate()));
  SetTypeMetadata(message, in);

  { /* Add all arguments */
    const auto &args = in->GetArgs();
    auto *arg_list = message->mutable_arguments();
    arg_list->Reserve(args.size());

    for (const auto &arg : args) {
      auto *argument = Pool::CreateMessage<SyntaxTree::CallArgument>(pool);
      argument->set_name(arg.first.Get());
      argument->set_allocated_value(From(arg.second));
      arg_list->AddAllocated(argument);
    }
  }

  return message;
}

SyntaxTree::U1 *ASTWriter::From(FlowPtr<U1> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::U1>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::U8 *ASTWriter::From(FlowPtr<U8> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::U8>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::U16 *ASTWriter::From(FlowPtr<U16> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::U16>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::U32 *ASTWriter::From(FlowPtr<U32> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::U32>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::U64 *ASTWriter::From(FlowPtr<U64> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::U64>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::U128 *ASTWriter::From(FlowPtr<U128> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::U128>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::I8 *ASTWriter::From(FlowPtr<I8> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::I8>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::I16 *ASTWriter::From(FlowPtr<I16> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::I16>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::I32 *ASTWriter::From(FlowPtr<I32> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::I32>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::I64 *ASTWriter::From(FlowPtr<I64> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::I64>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::I128 *ASTWriter::From(FlowPtr<I128> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::I128>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::F16 *ASTWriter::From(FlowPtr<F16> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::F16>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::F32 *ASTWriter::From(FlowPtr<F32> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::F32>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::F64 *ASTWriter::From(FlowPtr<F64> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::F64>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::F128 *ASTWriter::From(FlowPtr<F128> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::F128>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::VoidTy *ASTWriter::From(FlowPtr<VoidTy> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::VoidTy>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::PtrTy *ASTWriter::From(FlowPtr<PtrTy> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::PtrTy>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_pointee(From(in->GetItem()));
  if (in->IsVolatile()) {
    message->set_volatile_(in->IsVolatile());
  }
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::OpaqueTy *ASTWriter::From(FlowPtr<OpaqueTy> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::OpaqueTy>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::TupleTy *ASTWriter::From(FlowPtr<TupleTy> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::TupleTy>(pool);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  { /* Add all elements */
    const auto &items = in->GetItems();

    message->mutable_elements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      if (item->IsDiscarded()) {
        return;
      }

      message->mutable_elements()->AddAllocated(From(item));
    });
  }

  return message;
}

SyntaxTree::ArrayTy *ASTWriter::From(FlowPtr<ArrayTy> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::ArrayTy>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_element_type(From(in->GetItem()));
  message->set_allocated_element_count(From(in->GetSize()));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::RefTy *ASTWriter::From(FlowPtr<RefTy> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::RefTy>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_pointee(From(in->GetItem()));
  if (in->IsVolatile()) {
    message->set_volatile_(in->IsVolatile());
  }
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::FuncTy *ASTWriter::From(FlowPtr<FuncTy> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::FuncTy>(pool);

  message->set_allocated_location(FromSource(in));
  if (!IsCompressable(in->GetReturn())) {
    message->set_allocated_return_type(From(in->GetReturn()));
  }
  if (in->IsVariadic()) {
    message->set_variadic(in->IsVariadic());
  }
  SetTypeMetadata(message, in);

  { /* Add all parameters */
    const auto &params = in->GetParams();
    auto *param_list = message->mutable_parameters();
    param_list->Reserve(params.size());

    for (const auto &param : params) {
      auto *parameter = Pool::CreateMessage<SyntaxTree::FunctionParameter>(pool);
      const auto &[name, type, default_] = param;
      parameter->set_name(name.Get());
      if (!IsCompressable(type)) {
        parameter->set_allocated_type(From(type));
      }
      if (default_) {
        parameter->set_allocated_default_value(From(default_.Unwrap()));
      }

      param_list->AddAllocated(parameter);
    }
  }

  { /* Add all attributes */
    const auto &attrs = in->GetAttributes();
    message->mutable_attributes()->Reserve(attrs.size());
    std::for_each(attrs.begin(), attrs.end(), [&](auto item) {
      if (item->IsDiscarded()) {
        return;
      }

      message->mutable_attributes()->AddAllocated(From(item));
    });
  }

  return message;
}

SyntaxTree::Unary *ASTWriter::From(FlowPtr<Unary> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Unary>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_operator_(FromOperator(in->GetOp()));
  message->set_allocated_operand(From(in->GetRHS()));
  if (in->IsPostfix()) {
    message->set_is_postfix(in->IsPostfix());
  }

  return message;
}

SyntaxTree::Binary *ASTWriter::From(FlowPtr<Binary> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Binary>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_operator_(FromOperator(in->GetOp()));
  message->set_allocated_left(From(in->GetLHS()));
  message->set_allocated_right(From(in->GetRHS()));

  return message;
}

SyntaxTree::Integer *ASTWriter::From(FlowPtr<Integer> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Integer>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_number(in->GetValue().Get());

  return message;
}

SyntaxTree::Float *ASTWriter::From(FlowPtr<Float> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Float>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_number(in->GetValue().Get());

  return message;
}

SyntaxTree::Boolean *ASTWriter::From(FlowPtr<Boolean> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Boolean>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_value(in->GetValue());

  return message;
}

SyntaxTree::String *ASTWriter::From(FlowPtr<String> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::String>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_text(in->GetValue().Get());

  return message;
}

SyntaxTree::Character *ASTWriter::From(FlowPtr<Character> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Character>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_char_(in->GetValue());

  return message;
}

SyntaxTree::Null *ASTWriter::From(FlowPtr<Null> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Null>(pool);

  message->set_allocated_location(FromSource(in));

  return message;
}

SyntaxTree::Call *ASTWriter::From(FlowPtr<Call> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Call>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_callee(From(in->GetFunc()));

  { /* Add all arguments */
    const auto &args = in->GetArgs();
    message->mutable_arguments()->Reserve(args.size());

    std::for_each(args.begin(), args.end(), [&](auto arg) {
      auto *argument = Pool::CreateMessage<SyntaxTree::CallArgument>(pool);
      argument->set_name(arg.first.Get());
      argument->set_allocated_value(From(arg.second));
      message->mutable_arguments()->AddAllocated(argument);
    });
  }

  return message;
}

SyntaxTree::TemplateCall *ASTWriter::From(FlowPtr<TemplateCall> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::TemplateCall>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_callee(From(in->GetFunc()));

  { /* Add all arguments */
    const auto &args = in->GetArgs();
    message->mutable_arguments()->Reserve(args.size());

    std::for_each(args.begin(), args.end(), [&](auto arg) {
      auto *argument = Pool::CreateMessage<SyntaxTree::CallArgument>(pool);
      argument->set_name(arg.first.Get());
      argument->set_allocated_value(From(arg.second));
      message->mutable_arguments()->AddAllocated(argument);
    });
  }

  { /* Add all template arguments */
    const auto &args = in->GetTemplateArgs();
    message->mutable_template_arguments()->Reserve(args.size());

    std::for_each(args.begin(), args.end(), [&](auto arg) {
      auto *argument = Pool::CreateMessage<SyntaxTree::CallArgument>(pool);
      argument->set_name(arg.first.Get());
      argument->set_allocated_value(From(arg.second));
      message->mutable_template_arguments()->AddAllocated(argument);
    });
  }

  return message;
}

SyntaxTree::Import *ASTWriter::From(FlowPtr<Import> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Import>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());
  message->set_allocated_subtree(From(in->GetSubtree()));

  if (in->GetMode() != ncc::parse::ImportMode::Code) {
    message->set_mode(FromImportMode(in->GetMode()));
  }

  return message;
}

SyntaxTree::List *ASTWriter::From(FlowPtr<List> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::List>(pool);

  message->set_allocated_location(FromSource(in));

  { /* Add all elements */
    const auto &items = in->GetItems();

    message->mutable_elements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      if (item->IsDiscarded()) {
        return;
      }

      message->mutable_elements()->AddAllocated(From(item));
    });
  }

  return message;
}

SyntaxTree::Assoc *ASTWriter::From(FlowPtr<Assoc> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Assoc>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_key(From(in->GetKey()));
  message->set_allocated_value(From(in->GetValue()));

  return message;
}

SyntaxTree::Index *ASTWriter::From(FlowPtr<Index> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Index>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_base(From(in->GetBase()));
  message->set_allocated_index(From(in->GetIndex()));

  return message;
}

SyntaxTree::Slice *ASTWriter::From(FlowPtr<Slice> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Slice>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_base(From(in->GetBase()));
  message->set_allocated_start(From(in->GetStart()));
  message->set_allocated_end(From(in->GetEnd()));

  return message;
}

SyntaxTree::FString *ASTWriter::From(FlowPtr<FString> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::FString>(pool);

  message->set_allocated_location(FromSource(in));

  { /* Add all elements */
    const auto &items = in->GetItems();
    message->mutable_elements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      SyntaxTree::FString_FStringTerm *element = nullptr;

      if (std::holds_alternative<FlowPtr<Expr>>(item)) {
        if (std::get<FlowPtr<Expr>>(item)->IsDiscarded()) {
          return;
        }

        element = Pool::CreateMessage<SyntaxTree::FString::FStringTerm>(pool);
        element->set_allocated_expr(From(std::get<FlowPtr<Expr>>(item)));
      } else {
        element = Pool::CreateMessage<SyntaxTree::FString::FStringTerm>(pool);
        element->set_text(std::get<string>(item).Get());
      }

      message->mutable_elements()->AddAllocated(element);
    });
  }

  return message;
}

SyntaxTree::Identifier *ASTWriter::From(FlowPtr<Identifier> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Identifier>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());

  return message;
}

SyntaxTree::Block *ASTWriter::From(FlowPtr<Block> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Block>(pool);

  message->set_allocated_location(FromSource(in));

  switch (in->GetSafety()) {
    case BlockMode::Unknown: {
      break;
    }

    case BlockMode::Safe: {
      message->set_safety(SyntaxTree::Block_Safety_Safe);
      break;
    }

    case BlockMode::Unsafe: {
      message->set_safety(SyntaxTree::Block_Safety_Unsafe);
      break;
    }
  }

  { /* Add all statements */
    const auto &items = in->GetStatements();

    message->mutable_statements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      if (item->IsDiscarded()) {
        return;
      }

      message->mutable_statements()->AddAllocated(From(item));
    });
  }

  return message;
}

SyntaxTree::Variable *ASTWriter::From(FlowPtr<Variable> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Variable>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());
  if (!IsCompressable(in->GetType())) {
    message->set_allocated_type(From(in->GetType()));
  }
  if (in->GetInitializer()) {
    message->set_allocated_initial_value(From(in->GetInitializer().Unwrap()));
  }

  switch (in->GetVariableKind()) {
    case VariableType::Var:
      message->set_kind(SyntaxTree::Variable_VariableKind_Var);
      break;

    case VariableType::Let:
      message->set_kind(SyntaxTree::Variable_VariableKind_Let);
      break;

    case VariableType::Const:
      message->set_kind(SyntaxTree::Variable_VariableKind_Const);
      break;
  }

  { /* Add all attributes */
    const auto &items = in->GetAttributes();

    message->mutable_attributes()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      if (item->IsDiscarded()) {
        return;
      }

      message->mutable_attributes()->AddAllocated(From(item));
    });
  }

  return message;
}

SyntaxTree::Assembly *ASTWriter::From(FlowPtr<Assembly> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Assembly>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_code(in->GetCode().Get());

  { /* Add all arguments */
    const auto &items = in->GetArguments();

    message->mutable_arguments()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      if (item->IsDiscarded()) {
        return;
      }

      message->mutable_arguments()->AddAllocated(From(item));
    });
  }

  return message;
}

SyntaxTree::If *ASTWriter::From(FlowPtr<If> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::If>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_condition(From(in->GetCond()));
  message->set_allocated_true_branch(From(in->GetThen()));

  if (in->GetElse()) {
    message->set_allocated_false_branch(From(in->GetElse().Unwrap()));
  }

  return message;
}

SyntaxTree::While *ASTWriter::From(FlowPtr<While> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::While>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_condition(From(in->GetCond()));
  message->set_allocated_body(From(in->GetBody()));

  return message;
}

SyntaxTree::For *ASTWriter::From(FlowPtr<For> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::For>(pool);

  message->set_allocated_location(FromSource(in));

  if (in->GetInit()) {
    message->set_allocated_init(From(in->GetInit().Unwrap()));
  }

  if (in->GetCond()) {
    message->set_allocated_condition(From(in->GetCond().Unwrap()));
  }

  if (in->GetStep()) {
    message->set_allocated_step(From(in->GetStep().Unwrap()));
  }

  message->set_allocated_body(From(in->GetBody()));

  return message;
}

SyntaxTree::Foreach *ASTWriter::From(FlowPtr<Foreach> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Foreach>(pool);

  message->set_allocated_location(FromSource(in));
  if (in->GetIndex()) {
    message->set_index_name(in->GetIndex().Get());
  }
  message->set_value_name(in->GetValue().Get());
  message->set_allocated_expression(From(in->GetExpr()));
  message->set_allocated_body(From(in->GetBody()));

  return message;
}

SyntaxTree::Break *ASTWriter::From(FlowPtr<Break> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Break>(pool);

  message->set_allocated_location(FromSource(in));

  return message;
}

SyntaxTree::Continue *ASTWriter::From(FlowPtr<Continue> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Continue>(pool);

  message->set_allocated_location(FromSource(in));

  return message;
}

SyntaxTree::Return *ASTWriter::From(FlowPtr<Return> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Return>(pool);

  message->set_allocated_location(FromSource(in));
  if (in->GetValue()) {
    message->set_allocated_value(From(in->GetValue().Unwrap()));
  }

  return message;
}

SyntaxTree::Case *ASTWriter::From(FlowPtr<Case> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Case>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_condition(From(in->GetCond()));
  message->set_allocated_body(From(in->GetBody()));

  return message;
}

SyntaxTree::Switch *ASTWriter::From(FlowPtr<Switch> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Switch>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_condition(From(in->GetCond()));

  { /* Add all cases */
    const auto &items = in->GetCases();

    message->mutable_cases()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      if (item->IsDiscarded()) {
        return;
      }

      message->mutable_cases()->AddAllocated(From(item));
    });
  }

  if (in->GetDefault()) {
    message->set_allocated_default_(From(in->GetDefault().Unwrap()));
  }

  return message;
}

SyntaxTree::Typedef *ASTWriter::From(FlowPtr<Typedef> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Typedef>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());
  message->set_allocated_type(From(in->GetType()));

  return message;
}

SyntaxTree::Function *ASTWriter::From(FlowPtr<Function> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Function>(pool);

  message->set_allocated_location(FromSource(in));
  if (!IsCompressable(in->GetReturn())) {
    message->set_allocated_return_type(From(in->GetReturn()));
  }
  message->set_name(in->GetName().Get());
  if (in->IsVariadic()) {
    message->set_variadic(in->IsVariadic());
  }

  if (in->GetBody()) {
    message->set_allocated_body(From(in->GetBody().Unwrap()));
  }

  { /* Add all attributes */
    const auto &items = in->GetAttributes();

    message->mutable_attributes()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      if (item->IsDiscarded()) {
        return;
      }

      message->mutable_attributes()->AddAllocated(From(item));
    });
  }

  /* Add all template parameters */
  if (in->GetTemplateParams()) {
    auto params = in->GetTemplateParams().value();
    auto param_list = message->mutable_template_parameters()->parameters();
    param_list.Reserve(params.size());

    for (const auto &param : params) {
      auto *parameter = Pool::CreateMessage<SyntaxTree::TemplateParameters::TemplateParameter>(pool);
      const auto &[name, type, default_] = param;
      parameter->set_name(name.Get());
      if (!IsCompressable(type)) {
        parameter->set_allocated_type(From(type));
      }
      if (default_) {
        parameter->set_allocated_default_value(From(default_.Unwrap()));
      }

      param_list.AddAllocated(parameter);
    }
  }

  { /* Add all parameters */
    const auto &params = in->GetParams();
    auto *param_list = message->mutable_parameters();
    param_list->Reserve(params.size());

    for (const auto &param : params) {
      auto *parameter = Pool::CreateMessage<SyntaxTree::FunctionParameter>(pool);
      const auto &[name, type, default_] = param;
      parameter->set_name(name.Get());
      if (!IsCompressable(type)) {
        parameter->set_allocated_type(From(type));
      }
      if (default_) {
        parameter->set_allocated_default_value(From(default_.Unwrap()));
      }

      param_list->AddAllocated(parameter);
    }
  }

  if (in->GetTemplateParams()) {
    auto items = in->GetTemplateParams().value();

    message->mutable_template_parameters()->mutable_parameters()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *parameter = Pool::CreateMessage<SyntaxTree::TemplateParameters_TemplateParameter>(pool);
      const auto &param_name = std::get<0>(item);
      const auto &param_type = std::get<1>(item);
      const auto &param_default = std::get<2>(item);

      parameter->set_name(param_name.Get());
      if (!IsCompressable(param_type)) {
        parameter->set_allocated_type(From(param_type));
      }
      if (param_default) {
        parameter->set_allocated_default_value(From(param_default.Unwrap()));
      }

      message->mutable_template_parameters()->mutable_parameters()->AddAllocated(parameter);
    });
  }

  return message;
}

SyntaxTree::Struct *ASTWriter::From(FlowPtr<Struct> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Struct>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());
  message->set_kind(FromStructKind(in->GetCompositeType()));

  if (in->GetTemplateParams()) {
    auto items = in->GetTemplateParams().value();

    message->mutable_template_parameters()->mutable_parameters()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *parameter = Pool::CreateMessage<SyntaxTree::TemplateParameters_TemplateParameter>(pool);
      const auto &param_name = std::get<0>(item);
      const auto &param_type = std::get<1>(item);
      const auto &param_default = std::get<2>(item);

      parameter->set_name(param_name.Get());
      if (!IsCompressable(param_type)) {
        parameter->set_allocated_type(From(param_type));
      }
      if (param_default) {
        parameter->set_allocated_default_value(From(param_default.Unwrap()));
      }

      message->mutable_template_parameters()->mutable_parameters()->AddAllocated(parameter);
    });
  }

  { /* Add names */
    const auto &items = in->GetNames();

    message->mutable_names()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) { message->mutable_names()->Add(item.Get().c_str()); });
  }

  { /* Add all attributes */
    const auto &items = in->GetAttributes();

    message->mutable_attributes()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      if (item->IsDiscarded()) {
        return;
      }

      message->mutable_attributes()->AddAllocated(From(item));
    });
  }

  { /* Add all fields */
    const auto &items = in->GetFields();

    message->mutable_fields()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *field = Pool::CreateMessage<SyntaxTree::Struct_Field>(pool);
      field->set_name(item.GetName().Get());
      if (!IsCompressable(item.GetType())) {
        field->set_allocated_type(From(item.GetType()));
      }
      if (item.GetVis() != Vis::Sec) {
        field->set_visibility(FromVisibility(item.GetVis()));
      }
      if (item.IsStatic()) {
        field->set_is_static(true);
      }
      if (item.GetValue()) {
        field->set_allocated_default_value(From(item.GetValue().Unwrap()));
      }

      message->mutable_fields()->AddAllocated(field);
    });
  }

  { /* Add all methods */
    const auto &items = in->GetMethods();

    message->mutable_methods()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *method = Pool::CreateMessage<SyntaxTree::Struct_Method>(pool);
      method->set_allocated_func(From(item.m_func));

      if (item.m_vis != Vis::Sec) {
        method->set_visibility(FromVisibility(item.m_vis));
      }

      message->mutable_methods()->AddAllocated(method);
    });
  }

  return message;
}

SyntaxTree::Enum *ASTWriter::From(FlowPtr<Enum> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Enum>(pool);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());

  if (in->GetType()) {
    message->set_allocated_base_type(From(in->GetType().Unwrap()));
  }

  { /* Add all elements */
    const auto &items = in->GetFields();

    message->mutable_items()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *element = Pool::CreateMessage<SyntaxTree::Enum_Field>(pool);
      element->set_name(item.first.Get());
      if (item.second) {
        element->set_allocated_value(From(item.second.Unwrap()));
      }
      message->mutable_items()->AddAllocated(element);
    });
  }

  return message;
}

SyntaxTree::Scope *ASTWriter::From(FlowPtr<Scope> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Scope>(pool);

  message->set_allocated_location(FromSource(in));

  { /* Add all dependencies */
    const auto &items = in->GetDeps();
    message->mutable_dependencies()->Assign(items.begin(), items.end());
  }

  message->set_name(in->GetName().Get());
  message->set_allocated_body(From(in->GetBody()));

  return message;
}

SyntaxTree::Export *ASTWriter::From(FlowPtr<Export> in) {
  auto &pool = m_impl->m_pool;
  auto *message = Pool::CreateMessage<SyntaxTree::Export>(pool);

  message->set_allocated_location(FromSource(in));
  if (in->GetAbiName()) {
    message->set_abi_name(in->GetAbiName().Get());
  }
  if (in->GetVis() != Vis::Pub) {
    message->set_visibility(FromVisibility(in->GetVis()));
  }

  message->set_allocated_body(From(in->GetBody()));

  { /* Add all attributes */
    const auto &items = in->GetAttributes();

    message->mutable_attributes()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      if (item->IsDiscarded()) {
        return;
      }

      message->mutable_attributes()->AddAllocated(From(item));
    });
  }

  return message;
}

////////////////////////////////////////////////////////////////////////////////

#define SEND(__message, __node_name)                                          \
  {                                                                           \
    auto *message = From(n);                                                  \
    message->CheckInitialized();                                              \
    auto *root = Pool::CreateMessage<SyntaxTree::Expr>(m_impl->m_pool);       \
    root->set_allocated_##__node_name(message);                               \
    root->CheckInitialized();                                                 \
    switch (m_impl->m_format) {                                               \
      case Format::PROTO: {                                                   \
        if (!root->SerializeToOstream(&m_impl->m_os)) [[unlikely]] {          \
          qcore_panic("Failed to serialize protobuf message");                \
        }                                                                     \
        break;                                                                \
      }                                                                       \
                                                                              \
      case Format::JSON: {                                                    \
        google::protobuf::util::JsonPrintOptions options;                     \
        std::string m_json;                                                   \
        google::protobuf::util::MessageToJsonString(*root, &m_json, options); \
        m_impl->m_os << m_json;                                               \
        break;                                                                \
      }                                                                       \
    }                                                                         \
  }

void ASTWriter::Visit(FlowPtr<NamedTy> n) { SEND(From(n), named); }
void ASTWriter::Visit(FlowPtr<InferTy> n) { SEND(From(n), infer); }
void ASTWriter::Visit(FlowPtr<TemplateType> n) { SEND(From(n), template_); }
void ASTWriter::Visit(FlowPtr<U1> n) { SEND(From(n), u1); }
void ASTWriter::Visit(FlowPtr<U8> n) { SEND(From(n), u8); }
void ASTWriter::Visit(FlowPtr<U16> n) { SEND(From(n), u16); }
void ASTWriter::Visit(FlowPtr<U32> n) { SEND(From(n), u32); }
void ASTWriter::Visit(FlowPtr<U64> n) { SEND(From(n), u64); }
void ASTWriter::Visit(FlowPtr<U128> n) { SEND(From(n), u128); }
void ASTWriter::Visit(FlowPtr<I8> n) { SEND(From(n), i8); }
void ASTWriter::Visit(FlowPtr<I16> n) { SEND(From(n), i16); }
void ASTWriter::Visit(FlowPtr<I32> n) { SEND(From(n), i32); }
void ASTWriter::Visit(FlowPtr<I64> n) { SEND(From(n), i64); }
void ASTWriter::Visit(FlowPtr<I128> n) { SEND(From(n), i128); }
void ASTWriter::Visit(FlowPtr<F16> n) { SEND(From(n), f16); }
void ASTWriter::Visit(FlowPtr<F32> n) { SEND(From(n), f32); }
void ASTWriter::Visit(FlowPtr<F64> n) { SEND(From(n), f64); }
void ASTWriter::Visit(FlowPtr<F128> n) { SEND(From(n), f128); }
void ASTWriter::Visit(FlowPtr<VoidTy> n) { SEND(From(n), void_); }
void ASTWriter::Visit(FlowPtr<PtrTy> n) { SEND(From(n), ptr); }
void ASTWriter::Visit(FlowPtr<OpaqueTy> n) { SEND(From(n), opaque); }
void ASTWriter::Visit(FlowPtr<TupleTy> n) { SEND(From(n), tuple); }
void ASTWriter::Visit(FlowPtr<ArrayTy> n) SEND(From(n), array);
void ASTWriter::Visit(FlowPtr<RefTy> n) { SEND(From(n), ref); }
void ASTWriter::Visit(FlowPtr<FuncTy> n) { SEND(From(n), func); }
void ASTWriter::Visit(FlowPtr<Unary> n) { SEND(From(n), unary); }
void ASTWriter::Visit(FlowPtr<Binary> n) { SEND(From(n), binary); }
void ASTWriter::Visit(FlowPtr<Integer> n) { SEND(From(n), integer); }
void ASTWriter::Visit(FlowPtr<Float> n) { SEND(From(n), float_); }
void ASTWriter::Visit(FlowPtr<Boolean> n) { SEND(From(n), boolean); }
void ASTWriter::Visit(FlowPtr<String> n) { SEND(From(n), string); }
void ASTWriter::Visit(FlowPtr<Character> n) { SEND(From(n), character); }
void ASTWriter::Visit(FlowPtr<Null> n) { SEND(From(n), null); }
void ASTWriter::Visit(FlowPtr<Call> n) { SEND(From(n), call); }
void ASTWriter::Visit(FlowPtr<TemplateCall> n) { SEND(From(n), template_call); }
void ASTWriter::Visit(FlowPtr<Import> n) { SEND(From(n), import); }
void ASTWriter::Visit(FlowPtr<List> n) { SEND(From(n), list); }
void ASTWriter::Visit(FlowPtr<Assoc> n) { SEND(From(n), assoc); }
void ASTWriter::Visit(FlowPtr<Index> n) { SEND(From(n), index); }
void ASTWriter::Visit(FlowPtr<Slice> n) { SEND(From(n), slice); }
void ASTWriter::Visit(FlowPtr<FString> n) { SEND(From(n), fstring); }
void ASTWriter::Visit(FlowPtr<Identifier> n) { SEND(From(n), identifier); }
void ASTWriter::Visit(FlowPtr<Block> n) { SEND(From(n), block); }
void ASTWriter::Visit(FlowPtr<Variable> n) { SEND(From(n), variable); }
void ASTWriter::Visit(FlowPtr<Assembly> n) { SEND(From(n), assembly); }
void ASTWriter::Visit(FlowPtr<If> n) { SEND(From(n), if_); }
void ASTWriter::Visit(FlowPtr<While> n) { SEND(From(n), while_); }
void ASTWriter::Visit(FlowPtr<For> n) { SEND(From(n), for_); }
void ASTWriter::Visit(FlowPtr<Foreach> n) { SEND(From(n), foreach); }
void ASTWriter::Visit(FlowPtr<Break> n) { SEND(From(n), break_); }
void ASTWriter::Visit(FlowPtr<Continue> n) { SEND(From(n), continue_); }
void ASTWriter::Visit(FlowPtr<Return> n) { SEND(From(n), return_); }
void ASTWriter::Visit(FlowPtr<Case> n) { SEND(From(n), case_); }
void ASTWriter::Visit(FlowPtr<Switch> n) { SEND(From(n), switch_); }
void ASTWriter::Visit(FlowPtr<Typedef> n) { SEND(From(n), typedef_); }
void ASTWriter::Visit(FlowPtr<Function> n) { SEND(From(n), function); }
void ASTWriter::Visit(FlowPtr<Struct> n) { SEND(From(n), struct_); }
void ASTWriter::Visit(FlowPtr<Enum> n) { SEND(From(n), enum_); }
void ASTWriter::Visit(FlowPtr<Scope> n) { SEND(From(n), scope); }
void ASTWriter::Visit(FlowPtr<Export> n) { SEND(From(n), export_); }

ASTWriter::ASTWriter(std::ostream &os, Format format, OptionalSourceProvider rd)
    : m_impl(std::make_unique<PImpl>(os, format, rd)) {}

ASTWriter::~ASTWriter() = default;
