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

#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <variant>

using namespace ncc::parse;
using namespace google;
using namespace nitrate::parser::SyntaxTree;

using Pool = google::protobuf::Arena;

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

static SyntaxTree::FunctionPurity FromPurity(ncc::parse::Purity purity) {
  switch (purity) {
    case ncc::parse::Purity::Impure:
      return SyntaxTree::Purity_Impure;

    case ncc::parse::Purity::Impure_TSafe:
      return SyntaxTree::Purity_Impure_TSafe;

    case ncc::parse::Purity::Pure:
      return SyntaxTree::Purity_Pure;

    case ncc::parse::Purity::Quasi:
      return SyntaxTree::Purity_Quasi;

    case ncc::parse::Purity::Retro:
      return SyntaxTree::Purity_Retro;
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

void AstWriter::SetTypeMetadata(auto *message, const FlowPtr<Type> &in) {
  if (in->GetWidth().has_value()) [[unlikely]] {
    message->set_allocated_bit_width(From(in->GetWidth().value()));
  }

  if (in->GetRangeBegin().has_value()) [[unlikely]] {
    message->set_allocated_minimum(From(in->GetRangeBegin().value()));
  }

  if (in->GetRangeEnd().has_value()) [[unlikely]] {
    message->set_allocated_maximum(From(in->GetRangeEnd().value()));
  }
}

SyntaxTree::SourceLocationRange *AstWriter::FromSource(const FlowPtr<Base> &in) {
  if (!m_rd.has_value()) {
    return nullptr;
  }

  const auto &pos = in->GetPos();
  auto start_pos = m_rd->get().GetLocation(pos.first);
  auto end_pos = m_rd->get().GetLocation(pos.second);

  if (start_pos == lex::Location::EndOfFile() && end_pos == lex::Location::EndOfFile()) {
    return nullptr;
  }

  auto *message = Pool::CreateMessage<SyntaxTree::SourceLocationRange>(m_arena);

  if (start_pos != lex::Location::EndOfFile()) {
    auto *start = Pool::CreateMessage<SyntaxTree::SourceLocationRange_SourceLocation>(m_arena);
    start->set_line(start_pos.GetRow() + 1);
    start->set_column(start_pos.GetCol() + 1);
    start->set_offset(start_pos.GetOffset());
    start->set_file(start_pos.GetFilename().Get());

    message->set_allocated_start(start);
  }

  if (end_pos != lex::Location::EndOfFile()) {
    auto *end = Pool::CreateMessage<SyntaxTree::SourceLocationRange_SourceLocation>(m_arena);
    end->set_line(end_pos.GetRow() + 1);
    end->set_column(end_pos.GetCol() + 1);
    end->set_offset(end_pos.GetOffset());
    end->set_file(end_pos.GetFilename().Get());

    message->set_allocated_end(end);
  }

  return message;
}

SyntaxTree::Expr *AstWriter::From(const FlowPtr<Expr> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Expr>(m_arena);

  switch (in->GetKind()) {
    case QAST_BASE: {
      message->set_allocated_base(From(in.As<Base>()));
      break;
    }

    case QAST_BINEXPR: {
      message->set_allocated_binary(From(in.As<Binary>()));
      break;
    }

    case QAST_UNEXPR: {
      message->set_allocated_unary(From(in.As<Unary>()));
      break;
    }

    case QAST_POST_UNEXPR: {
      message->set_allocated_post_unary(From(in.As<PostUnary>()));
      break;
    }

    case QAST_TEREXPR: {
      message->set_allocated_ternary(From(in.As<Ternary>()));
      break;
    }

    case QAST_INT: {
      message->set_allocated_integer(From(in.As<Integer>()));
      break;
    }

    case QAST_FLOAT: {
      message->set_allocated_float_(From(in.As<Float>()));
      break;
    }

    case QAST_STRING: {
      message->set_allocated_string(From(in.As<String>()));
      break;
    }

    case QAST_CHAR: {
      message->set_allocated_character(From(in.As<Character>()));
      break;
    }

    case QAST_BOOL: {
      message->set_allocated_boolean(From(in.As<Boolean>()));
      break;
    }

    case QAST_NULL: {
      message->set_allocated_null(From(in.As<Null>()));
      break;
    }

    case QAST_UNDEF: {
      message->set_allocated_undefined(From(in.As<Undefined>()));
      break;
    }

    case QAST_CALL: {
      message->set_allocated_call(From(in.As<Call>()));
      break;
    }

    case QAST_LIST: {
      message->set_allocated_list(From(in.As<List>()));
      break;
    }

    case QAST_ASSOC: {
      message->set_allocated_assoc(From(in.As<Assoc>()));
      break;
    }

    case QAST_INDEX: {
      message->set_allocated_index(From(in.As<Index>()));
      break;
    }

    case QAST_SLICE: {
      message->set_allocated_slice(From(in.As<Slice>()));
      break;
    }

    case QAST_FSTRING: {
      message->set_allocated_fstring(From(in.As<FString>()));
      break;
    }

    case QAST_IDENT: {
      message->set_allocated_identifier(From(in.As<Identifier>()));
      break;
    }

    case QAST_LAMBDA: {
      message->set_allocated_lambda_expr(From(in.As<LambdaExpr>()));
      break;
    }

    case QAST_TEMPL_CALL: {
      message->set_allocated_template_call(From(in.As<TemplateCall>()));
      break;
    }

    case QAST_U1: {
      message->set_allocated_u1(From(in.As<U1>()));
      break;
    }

    case QAST_U8: {
      message->set_allocated_u8(From(in.As<U8>()));
      break;
    }

    case QAST_U16: {
      message->set_allocated_u16(From(in.As<U16>()));
      break;
    }

    case QAST_U32: {
      message->set_allocated_u32(From(in.As<U32>()));
      break;
    }

    case QAST_U64: {
      message->set_allocated_u64(From(in.As<U64>()));
      break;
    }

    case QAST_U128: {
      message->set_allocated_u128(From(in.As<U128>()));
      break;
    }

    case QAST_I8: {
      message->set_allocated_i8(From(in.As<I8>()));
      break;
    }

    case QAST_I16: {
      message->set_allocated_i16(From(in.As<I16>()));
      break;
    }

    case QAST_I32: {
      message->set_allocated_i32(From(in.As<I32>()));
      break;
    }

    case QAST_I64: {
      message->set_allocated_i64(From(in.As<I64>()));
      break;
    }

    case QAST_I128: {
      message->set_allocated_i128(From(in.As<I128>()));
      break;
    }

    case QAST_F16: {
      message->set_allocated_f16(From(in.As<F16>()));
      break;
    }

    case QAST_F32: {
      message->set_allocated_f32(From(in.As<F32>()));
      break;
    }

    case QAST_F64: {
      message->set_allocated_f64(From(in.As<F64>()));
      break;
    }

    case QAST_F128: {
      message->set_allocated_f128(From(in.As<F128>()));
      break;
    }

    case QAST_VOID: {
      message->set_allocated_void_(From(in.As<VoidTy>()));
      break;
    }

    case QAST_INFER: {
      message->set_allocated_infer(From(in.As<InferTy>()));
      break;
    }

    case QAST_OPAQUE: {
      message->set_allocated_opaque(From(in.As<OpaqueTy>()));
      break;
    }

    case QAST_NAMED: {
      message->set_allocated_named(From(in.As<NamedTy>()));
      break;
    }

    case QAST_REF: {
      message->set_allocated_ref(From(in.As<RefTy>()));
      break;
    }

    case QAST_PTR: {
      message->set_allocated_ptr(From(in.As<PtrTy>()));
      break;
    }

    case QAST_ARRAY: {
      message->set_allocated_array(From(in.As<ArrayTy>()));
      break;
    }

    case QAST_TUPLE: {
      message->set_allocated_tuple(From(in.As<TupleTy>()));
      break;
    }

    case QAST_TEMPLATE: {
      message->set_allocated_template_(From(in.As<TemplateType>()));
      break;
    }

    case QAST_FUNCTOR: {
      message->set_allocated_func(From(in.As<FuncTy>()));
      break;
    }

    default: {
      qcore_panic("Unknown expression kind");
    }
  }

  return message;
}

SyntaxTree::Stmt *AstWriter::From(const FlowPtr<Stmt> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Stmt>(m_arena);

  switch (in->GetKind()) {
    case QAST_BASE: {
      message->set_allocated_base(From(in.As<Base>()));
      break;
    }

    case QAST_IF: {
      message->set_allocated_if_(From(in.As<If>()));
      break;
    }

    case QAST_RETIF: {
      message->set_allocated_return_if(From(in.As<ReturnIf>()));
      break;
    }

    case QAST_SWITCH: {
      message->set_allocated_switch_(From(in.As<Switch>()));
      break;
    }

    case QAST_CASE: {
      message->set_allocated_case_(From(in.As<Case>()));
      break;
    }

    case QAST_RETURN: {
      message->set_allocated_return_(From(in.As<Return>()));
      break;
    }

    case QAST_BREAK: {
      message->set_allocated_break_(From(in.As<Break>()));
      break;
    }

    case QAST_CONTINUE: {
      message->set_allocated_continue_(From(in.As<Continue>()));
      break;
    }

    case QAST_WHILE: {
      message->set_allocated_while_(From(in.As<While>()));
      break;
    }

    case QAST_FOR: {
      message->set_allocated_for_(From(in.As<For>()));
      break;
    }

    case QAST_FOREACH: {
      message->set_allocated_foreach(From(in.As<Foreach>()));
      break;
    }

    case QAST_INLINE_ASM: {
      message->set_allocated_assembly(From(in.As<Assembly>()));
      break;
    }

    case QAST_ESTMT: {
      message->set_allocated_expr_stmt(From(in.As<ExprStmt>()));
      break;
    }

    case QAST_TYPEDEF: {
      message->set_allocated_typedef_(From(in.As<Typedef>()));
      break;
    }

    case QAST_STRUCT: {
      message->set_allocated_struct_(From(in.As<Struct>()));
      break;
    }

    case QAST_ENUM: {
      message->set_allocated_enum_(From(in.As<Enum>()));
      break;
    }

    case QAST_SCOPE: {
      message->set_allocated_scope(From(in.As<Scope>()));
      break;
    }

    case QAST_BLOCK: {
      message->set_allocated_block(From(in.As<Block>()));
      break;
    }

    case QAST_EXPORT: {
      message->set_allocated_export_(From(in.As<Export>()));
      break;
    }

    case QAST_VAR: {
      message->set_allocated_variable(From(in.As<Variable>()));
      break;
    }

    case QAST_FUNCTION: {
      message->set_allocated_function(From(in.As<Function>()));
      break;
    }

    default: {
      qcore_panic("Unknown statement kind");
    }
  }

  return message;
}

SyntaxTree::Type *AstWriter::From(const FlowPtr<Type> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Type>(m_arena);

  switch (in->GetKind()) {
    case QAST_BASE: {
      message->set_allocated_base(From(in.As<Base>()));
      break;
    }

    case QAST_U1: {
      message->set_allocated_u1(From(in.As<U1>()));
      break;
    }

    case QAST_U8: {
      message->set_allocated_u8(From(in.As<U8>()));
      break;
    }

    case QAST_U16: {
      message->set_allocated_u16(From(in.As<U16>()));
      break;
    }

    case QAST_U32: {
      message->set_allocated_u32(From(in.As<U32>()));
      break;
    }

    case QAST_U64: {
      message->set_allocated_u64(From(in.As<U64>()));
      break;
    }

    case QAST_U128: {
      message->set_allocated_u128(From(in.As<U128>()));
      break;
    }

    case QAST_I8: {
      message->set_allocated_i8(From(in.As<I8>()));
      break;
    }

    case QAST_I16: {
      message->set_allocated_i16(From(in.As<I16>()));
      break;
    }

    case QAST_I32: {
      message->set_allocated_i32(From(in.As<I32>()));
      break;
    }

    case QAST_I64: {
      message->set_allocated_i64(From(in.As<I64>()));
      break;
    }

    case QAST_I128: {
      message->set_allocated_i128(From(in.As<I128>()));
      break;
    }

    case QAST_F16: {
      message->set_allocated_f16(From(in.As<F16>()));
      break;
    }

    case QAST_F32: {
      message->set_allocated_f32(From(in.As<F32>()));
      break;
    }

    case QAST_F64: {
      message->set_allocated_f64(From(in.As<F64>()));
      break;
    }

    case QAST_F128: {
      message->set_allocated_f128(From(in.As<F128>()));
      break;
    }

    case QAST_VOID: {
      message->set_allocated_void_(From(in.As<VoidTy>()));
      break;
    }

    case QAST_INFER: {
      message->set_allocated_infer(From(in.As<InferTy>()));
      break;
    }

    case QAST_OPAQUE: {
      message->set_allocated_opaque(From(in.As<OpaqueTy>()));
      break;
    }

    case QAST_NAMED: {
      message->set_allocated_named(From(in.As<NamedTy>()));
      break;
    }

    case QAST_REF: {
      message->set_allocated_ref(From(in.As<RefTy>()));
      break;
    }

    case QAST_PTR: {
      message->set_allocated_ptr(From(in.As<PtrTy>()));
      break;
    }

    case QAST_ARRAY: {
      message->set_allocated_array(From(in.As<ArrayTy>()));
      break;
    }

    case QAST_TUPLE: {
      message->set_allocated_tuple(From(in.As<TupleTy>()));
      break;
    }

    case QAST_TEMPLATE: {
      message->set_allocated_template_(From(in.As<TemplateType>()));
      break;
    }

    case QAST_FUNCTOR: {
      message->set_allocated_func(From(in.As<FuncTy>()));
      break;
    }

    default: {
      qcore_panic("Unknown type kind");
    }
  }

  return message;
}

SyntaxTree::Base *AstWriter::From(const FlowPtr<Base> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Base>(m_arena);

  message->set_allocated_location(FromSource(in));

  return message;
}

SyntaxTree::ExprStmt *AstWriter::From(const FlowPtr<ExprStmt> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::ExprStmt>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_expression(From(in->GetExpr()));

  return message;
}

SyntaxTree::LambdaExpr *AstWriter::From(const FlowPtr<LambdaExpr> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::LambdaExpr>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_function(From(in->GetFunc()));

  return message;
}

SyntaxTree::NamedTy *AstWriter::From(const FlowPtr<NamedTy> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::NamedTy>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::InferTy *AstWriter::From(const FlowPtr<InferTy> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::InferTy>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::TemplateType *AstWriter::From(const FlowPtr<TemplateType> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::TemplateType>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_base(From(in->GetTemplate()));
  SetTypeMetadata(message, in);

  { /* Add all arguments */
    const auto &args = in->GetArgs();
    auto *arg_list = message->mutable_arguments();
    arg_list->Reserve(args.size());

    for (const auto &arg : args) {
      auto *argument = Pool::CreateMessage<SyntaxTree::CallArgument>(m_arena);
      argument->set_name(arg.first.Get());
      argument->set_allocated_value(From(arg.second));
      arg_list->AddAllocated(argument);
    }
  }

  return message;
}

SyntaxTree::U1 *AstWriter::From(const FlowPtr<U1> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::U1>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::U8 *AstWriter::From(const FlowPtr<U8> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::U8>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::U16 *AstWriter::From(const FlowPtr<U16> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::U16>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::U32 *AstWriter::From(const FlowPtr<U32> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::U32>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::U64 *AstWriter::From(const FlowPtr<U64> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::U64>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::U128 *AstWriter::From(const FlowPtr<U128> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::U128>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::I8 *AstWriter::From(const FlowPtr<I8> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::I8>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::I16 *AstWriter::From(const FlowPtr<I16> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::I16>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::I32 *AstWriter::From(const FlowPtr<I32> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::I32>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::I64 *AstWriter::From(const FlowPtr<I64> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::I64>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::I128 *AstWriter::From(const FlowPtr<I128> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::I128>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::F16 *AstWriter::From(const FlowPtr<F16> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::F16>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::F32 *AstWriter::From(const FlowPtr<F32> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::F32>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::F64 *AstWriter::From(const FlowPtr<F64> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::F64>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::F128 *AstWriter::From(const FlowPtr<F128> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::F128>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::VoidTy *AstWriter::From(const FlowPtr<VoidTy> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::VoidTy>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::PtrTy *AstWriter::From(const FlowPtr<PtrTy> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::PtrTy>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_pointee(From(in->GetItem()));
  message->set_volatile_(in->IsVolatile());
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::OpaqueTy *AstWriter::From(const FlowPtr<OpaqueTy> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::OpaqueTy>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::TupleTy *AstWriter::From(const FlowPtr<TupleTy> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::TupleTy>(m_arena);

  message->set_allocated_location(FromSource(in));
  SetTypeMetadata(message, in);

  { /* Add all elements */
    const auto &items = in->GetItems();

    message->mutable_elements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](auto item) { message->mutable_elements()->AddAllocated(From(item)); });
  }

  return message;
}

SyntaxTree::ArrayTy *AstWriter::From(const FlowPtr<ArrayTy> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::ArrayTy>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_element_type(From(in->GetItem()));
  message->set_allocated_element_count(From(in->GetSize()));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::RefTy *AstWriter::From(const FlowPtr<RefTy> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::RefTy>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_pointee(From(in->GetItem()));
  SetTypeMetadata(message, in);

  return message;
}

SyntaxTree::FuncTy *AstWriter::From(const FlowPtr<FuncTy> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::FuncTy>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_return_type(From(in->GetReturn()));
  message->set_variadic(in->IsVariadic());
  message->set_purity(FromPurity(in->GetPurity()));
  SetTypeMetadata(message, in);

  { /* Add all parameters */
    const auto &params = in->GetParams();
    auto *param_list = message->mutable_parameters();
    param_list->Reserve(params.size());

    for (const auto &param : params) {
      auto *parameter = Pool::CreateMessage<SyntaxTree::FunctionParameter>(m_arena);
      const auto &[name, type, default_] = param;
      parameter->set_name(name.Get());
      parameter->set_allocated_type(From(type));
      if (default_.has_value()) {
        parameter->set_allocated_default_value(From(default_.value()));
      }

      param_list->AddAllocated(parameter);
    }
  }

  { /* Add all attributes */
    const auto &attrs = in->GetAttributes();
    message->mutable_attributes()->Reserve(attrs.size());
    std::for_each(attrs.begin(), attrs.end(),
                  [&](auto attr) { message->mutable_attributes()->AddAllocated(From(attr)); });
  }

  return message;
}

SyntaxTree::Unary *AstWriter::From(const FlowPtr<Unary> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Unary>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_operator_(FromOperator(in->GetOp()));
  message->set_allocated_operand(From(in->GetRHS()));

  return message;
}

SyntaxTree::Binary *AstWriter::From(const FlowPtr<Binary> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Binary>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_operator_(FromOperator(in->GetOp()));
  message->set_allocated_left(From(in->GetLHS()));
  message->set_allocated_right(From(in->GetRHS()));

  return message;
}

SyntaxTree::PostUnary *AstWriter::From(const FlowPtr<PostUnary> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::PostUnary>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_operator_(FromOperator(in->GetOp()));
  message->set_allocated_operand(From(in->GetLHS()));

  return message;
}

SyntaxTree::Ternary *AstWriter::From(const FlowPtr<Ternary> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Ternary>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_condition(From(in->GetCond()));
  message->set_allocated_true_branch(From(in->GetLHS()));
  message->set_allocated_false_branch(From(in->GetRHS()));

  return message;
}

SyntaxTree::Integer *AstWriter::From(const FlowPtr<Integer> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Integer>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_number(in->GetValue().Get());

  return message;
}

SyntaxTree::Float *AstWriter::From(const FlowPtr<Float> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Float>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_number(in->GetValue().Get());

  return message;
}

SyntaxTree::Boolean *AstWriter::From(const FlowPtr<Boolean> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Boolean>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_value(in->GetValue());

  return message;
}

SyntaxTree::String *AstWriter::From(const FlowPtr<String> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::String>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_text(in->GetValue().Get());

  return message;
}

SyntaxTree::Character *AstWriter::From(const FlowPtr<Character> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Character>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_char_(in->GetValue());

  return message;
}

SyntaxTree::Null *AstWriter::From(const FlowPtr<Null> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Null>(m_arena);

  message->set_allocated_location(FromSource(in));

  return message;
}

SyntaxTree::Undefined *AstWriter::From(const FlowPtr<Undefined> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Undefined>(m_arena);

  message->set_allocated_location(FromSource(in));

  return message;
}

SyntaxTree::Call *AstWriter::From(const FlowPtr<Call> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Call>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_callee(From(in->GetFunc()));

  { /* Add all arguments */
    const auto &args = in->GetArgs();
    message->mutable_arguments()->Reserve(args.size());

    std::for_each(args.begin(), args.end(), [&](auto arg) {
      auto *argument = Pool::CreateMessage<SyntaxTree::CallArgument>(m_arena);
      argument->set_name(arg.first.Get());
      argument->set_allocated_value(From(arg.second));
      message->mutable_arguments()->AddAllocated(argument);
    });
  }

  return message;
}

SyntaxTree::TemplateCall *AstWriter::From(const FlowPtr<TemplateCall> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::TemplateCall>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_callee(From(in->GetFunc()));

  { /* Add all arguments */
    const auto &args = in->GetArgs();
    message->mutable_arguments()->Reserve(args.size());

    std::for_each(args.begin(), args.end(), [&](auto arg) {
      auto *argument = Pool::CreateMessage<SyntaxTree::CallArgument>(m_arena);
      argument->set_name(arg.first.Get());
      argument->set_allocated_value(From(arg.second));
      message->mutable_arguments()->AddAllocated(argument);
    });
  }

  { /* Add all template arguments */
    const auto &args = in->GetTemplateArgs();
    message->mutable_template_arguments()->Reserve(args.size());

    std::for_each(args.begin(), args.end(), [&](auto arg) {
      auto *argument = Pool::CreateMessage<SyntaxTree::CallArgument>(m_arena);
      argument->set_name(arg.first.Get());
      argument->set_allocated_value(From(arg.second));
      message->mutable_template_arguments()->AddAllocated(argument);
    });
  }

  return message;
}

SyntaxTree::List *AstWriter::From(const FlowPtr<List> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::List>(m_arena);

  message->set_allocated_location(FromSource(in));

  { /* Add all elements */
    const auto &items = in->GetItems();

    message->mutable_elements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](auto item) { message->mutable_elements()->AddAllocated(From(item)); });
  }

  return message;
}

SyntaxTree::Assoc *AstWriter::From(const FlowPtr<Assoc> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Assoc>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_key(From(in->GetKey()));
  message->set_allocated_value(From(in->GetValue()));

  return message;
}

SyntaxTree::Index *AstWriter::From(const FlowPtr<Index> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Index>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_base(From(in->GetBase()));
  message->set_allocated_index(From(in->GetIndex()));

  return message;
}

SyntaxTree::Slice *AstWriter::From(const FlowPtr<Slice> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Slice>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_base(From(in->GetBase()));
  message->set_allocated_start(From(in->GetStart()));
  message->set_allocated_end(From(in->GetEnd()));

  return message;
}

SyntaxTree::FString *AstWriter::From(const FlowPtr<FString> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::FString>(m_arena);

  message->set_allocated_location(FromSource(in));

  { /* Add all elements */
    const auto &items = in->GetItems();
    message->mutable_elements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *element = Pool::CreateMessage<SyntaxTree::FString::FStringTerm>(m_arena);
      if (std::holds_alternative<FlowPtr<Expr>>(item)) {
        element->set_allocated_expr(From(std::get<FlowPtr<Expr>>(item)));
      } else {
        element->set_text(std::get<ncc::String>(item).Get());
      }

      message->mutable_elements()->AddAllocated(element);
    });
  }

  return message;
}

SyntaxTree::Identifier *AstWriter::From(const FlowPtr<Identifier> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Identifier>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());

  return message;
}

SyntaxTree::Sequence *AstWriter::From(const FlowPtr<Sequence> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Sequence>(m_arena);

  message->set_allocated_location(FromSource(in));

  { /* Add all elements */
    const auto &items = in->GetItems();

    message->mutable_elements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](auto item) { message->mutable_elements()->AddAllocated(From(item)); });
  }

  return message;
}

SyntaxTree::Block *AstWriter::From(const FlowPtr<Block> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Block>(m_arena);

  message->set_allocated_location(FromSource(in));

  switch (in->GetSafety()) {
    case SafetyMode::Unknown: {
      message->set_safety(SyntaxTree::Block_Safety_None);
      break;
    }

    case SafetyMode::Safe: {
      message->set_safety(SyntaxTree::Block_Safety_Safe);
      break;
    }

    case SafetyMode::Unsafe: {
      message->set_safety(SyntaxTree::Block_Safety_Unsafe);
      break;
    }
  }

  { /* Add all statements */
    const auto &items = in->GetStatements();

    message->mutable_statements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](auto item) { message->mutable_statements()->AddAllocated(From(item)); });
  }

  return message;
}

SyntaxTree::Variable *AstWriter::From(const FlowPtr<Variable> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Variable>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());
  if (in->GetType().has_value()) {
    message->set_allocated_type(From(in->GetType().value()));
  }
  if (in->GetInitializer().has_value()) {
    message->set_allocated_initial_value(From(in->GetInitializer().value()));
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
    std::for_each(items.begin(), items.end(),
                  [&](auto item) { message->mutable_attributes()->AddAllocated(From(item)); });
  }

  return message;
}

SyntaxTree::Assembly *AstWriter::From(const FlowPtr<Assembly> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Assembly>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_code(in->GetCode().Get());

  { /* Add all arguments */
    const auto &items = in->GetArguments();

    message->mutable_arguments()->Reserve(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](auto item) { message->mutable_arguments()->AddAllocated(From(item)); });
  }

  return message;
}

SyntaxTree::If *AstWriter::From(const FlowPtr<If> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::If>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_condition(From(in->GetCond()));
  message->set_allocated_true_branch(From(in->GetThen()));

  if (in->GetElse().has_value()) {
    message->set_allocated_false_branch(From(in->GetElse().value()));
  }

  return message;
}

SyntaxTree::While *AstWriter::From(const FlowPtr<While> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::While>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_condition(From(in->GetCond()));
  message->set_allocated_body(From(in->GetBody()));

  return message;
}

SyntaxTree::For *AstWriter::From(const FlowPtr<For> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::For>(m_arena);

  message->set_allocated_location(FromSource(in));

  if (in->GetInit().has_value()) {
    message->set_allocated_init(From(in->GetInit().value()));
  }

  if (in->GetCond().has_value()) {
    message->set_allocated_condition(From(in->GetCond().value()));
  }

  if (in->GetStep().has_value()) {
    message->set_allocated_step(From(in->GetStep().value()));
  }

  message->set_allocated_body(From(in->GetBody()));

  return message;
}

SyntaxTree::Foreach *AstWriter::From(const FlowPtr<Foreach> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Foreach>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_index_name(in->GetIndex().Get());
  message->set_value_name(in->GetValue().Get());
  message->set_allocated_expression(From(in->GetExpr()));
  message->set_allocated_body(From(in->GetBody()));

  return message;
}

SyntaxTree::Break *AstWriter::From(const FlowPtr<Break> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Break>(m_arena);

  message->set_allocated_location(FromSource(in));

  return message;
}

SyntaxTree::Continue *AstWriter::From(const FlowPtr<Continue> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Continue>(m_arena);

  message->set_allocated_location(FromSource(in));

  return message;
}

SyntaxTree::Return *AstWriter::From(const FlowPtr<Return> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Return>(m_arena);

  message->set_allocated_location(FromSource(in));
  if (in->GetValue().has_value()) {
    message->set_allocated_value(From(in->GetValue().value()));
  }

  return message;
}

SyntaxTree::ReturnIf *AstWriter::From(const FlowPtr<ReturnIf> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::ReturnIf>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_condition(From(in->GetCond()));
  message->set_allocated_value(From(in->GetValue()));

  return message;
}

SyntaxTree::Case *AstWriter::From(const FlowPtr<Case> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Case>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_condition(From(in->GetCond()));
  message->set_allocated_body(From(in->GetBody()));

  return message;
}

SyntaxTree::Switch *AstWriter::From(const FlowPtr<Switch> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Switch>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_condition(From(in->GetCond()));

  { /* Add all cases */
    const auto &items = in->GetCases();

    message->mutable_cases()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) { message->mutable_cases()->AddAllocated(From(item)); });
  }

  if (in->GetDefault().has_value()) {
    message->set_allocated_default_(From(in->GetDefault().value()));
  }

  return message;
}

SyntaxTree::Typedef *AstWriter::From(const FlowPtr<Typedef> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Typedef>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());
  message->set_allocated_type(From(in->GetType()));

  return message;
}

SyntaxTree::Function *AstWriter::From(const FlowPtr<Function> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Function>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_allocated_return_type(From(in->GetReturn()));
  message->set_name(in->GetName().Get());
  message->set_purity(FromPurity(in->GetPurity()));
  message->set_variadic(in->IsVariadic());
  if (in->GetPrecond().has_value()) {
    message->set_allocated_precondition(From(in->GetPrecond().value()));
  }

  if (in->GetPostcond().has_value()) {
    message->set_allocated_postcondition(From(in->GetPostcond().value()));
  }

  if (in->GetBody().has_value()) {
    message->set_allocated_body(From(in->GetBody().value()));
  }

  { /* Add all attributes */
    const auto &items = in->GetAttributes();

    message->mutable_attributes()->Reserve(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](auto item) { message->mutable_attributes()->AddAllocated(From(item)); });
  }

  { /* Add all captures */
    const auto &items = in->GetCaptures();

    message->mutable_captures()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *capture = Pool::CreateMessage<SyntaxTree::Function_Capture>(m_arena);
      capture->set_name(item.first.Get());
      capture->set_is_reference(item.second);

      message->mutable_captures()->AddAllocated(capture);
    });
  }

  /* Add all template parameters */
  if (in->GetTemplateParams().has_value()) {
    auto params = in->GetTemplateParams().value();
    auto param_list = message->mutable_template_parameters()->parameters();
    param_list.Reserve(params.size());

    for (const auto &param : params) {
      auto *parameter = Pool::CreateMessage<SyntaxTree::TemplateParameters::TemplateParameter>(m_arena);
      const auto &[name, type, default_] = param;
      parameter->set_name(name.Get());
      parameter->set_allocated_type(From(type));
      if (default_.has_value()) {
        parameter->set_allocated_default_value(From(default_.value()));
      }

      param_list.AddAllocated(parameter);
    }
  }

  { /* Add all parameters */
    const auto &params = in->GetParams();
    auto *param_list = message->mutable_parameters();
    param_list->Reserve(params.size());

    for (const auto &param : params) {
      auto *parameter = Pool::CreateMessage<SyntaxTree::FunctionParameter>(m_arena);
      const auto &[name, type, default_] = param;
      parameter->set_name(name.Get());
      parameter->set_allocated_type(From(type));
      if (default_.has_value()) {
        parameter->set_allocated_default_value(From(default_.value()));
      }

      param_list->AddAllocated(parameter);
    }
  }

  if (in->GetTemplateParams().has_value()) {
    auto items = in->GetTemplateParams().value();

    message->mutable_template_parameters()->mutable_parameters()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *parameter = Pool::CreateMessage<SyntaxTree::TemplateParameters_TemplateParameter>(m_arena);
      const auto &param_name = std::get<0>(item);
      const auto &param_type = std::get<1>(item);
      const auto &param_default = std::get<2>(item);

      parameter->set_name(param_name.Get());
      parameter->set_allocated_type(From(param_type));
      if (param_default.has_value()) {
        parameter->set_allocated_default_value(From(param_default.value()));
      }

      message->mutable_template_parameters()->mutable_parameters()->AddAllocated(parameter);
    });
  }

  return message;
}

SyntaxTree::Struct *AstWriter::From(const FlowPtr<Struct> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Struct>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());
  message->set_kind(FromStructKind(in->GetCompositeType()));

  if (in->GetTemplateParams().has_value()) {
    auto items = in->GetTemplateParams().value();

    message->mutable_template_parameters()->mutable_parameters()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *parameter = Pool::CreateMessage<SyntaxTree::TemplateParameters_TemplateParameter>(m_arena);
      const auto &param_name = std::get<0>(item);
      const auto &param_type = std::get<1>(item);
      const auto &param_default = std::get<2>(item);

      parameter->set_name(param_name.Get());
      parameter->set_allocated_type(From(param_type));
      if (param_default.has_value()) {
        parameter->set_allocated_default_value(From(param_default.value()));
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
    std::for_each(items.begin(), items.end(),
                  [&](auto item) { message->mutable_attributes()->AddAllocated(From(item)); });
  }

  { /* Add all fields */
    const auto &items = in->GetFields();

    message->mutable_fields()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *field = Pool::CreateMessage<SyntaxTree::Struct_Field>(m_arena);
      field->set_name(item.GetName().Get());
      field->set_allocated_type(From(item.GetType()));
      field->set_visibility(FromVisibility(item.GetVis()));
      field->set_is_static(item.IsStatic());
      if (item.GetValue().has_value()) {
        field->set_allocated_default_value(From(item.GetValue().value()));
      }

      message->mutable_fields()->AddAllocated(field);
    });
  }

  { /* Add all methods */
    const auto &items = in->GetMethods();

    message->mutable_methods()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *method = Pool::CreateMessage<SyntaxTree::Struct_Method>(m_arena);
      method->set_visibility(FromVisibility(item.m_vis));
      method->set_allocated_func(From(item.m_func));

      message->mutable_methods()->AddAllocated(method);
    });
  }

  { /* Add all methods */
    const auto &items = in->GetStaticMethods();

    message->mutable_static_methods()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *method = Pool::CreateMessage<SyntaxTree::Struct_Method>(m_arena);
      method->set_visibility(FromVisibility(item.m_vis));
      method->set_allocated_func(From(item.m_func));

      message->mutable_static_methods()->AddAllocated(method);
    });
  }

  return message;
}

SyntaxTree::Enum *AstWriter::From(const FlowPtr<Enum> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Enum>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_name(in->GetName().Get());

  if (in->GetType().has_value()) {
    message->set_allocated_base_type(From(in->GetType().value()));
  }

  { /* Add all elements */
    const auto &items = in->GetFields();

    message->mutable_items()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      auto *element = Pool::CreateMessage<SyntaxTree::Enum_Field>(m_arena);
      element->set_name(item.first.Get());
      if (item.second.has_value()) {
        element->set_allocated_value(From(item.second.value()));
      }
      message->mutable_items()->AddAllocated(element);
    });
  }

  return message;
}

SyntaxTree::Scope *AstWriter::From(const FlowPtr<Scope> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Scope>(m_arena);

  message->set_allocated_location(FromSource(in));

  { /* Add all dependencies */
    const auto &items = in->GetDeps();
    std::vector<std::string_view> names(items.size());
    std::transform(items.begin(), items.end(), names.begin(), [](auto item) { return item.Get(); });

    message->mutable_dependencies()->Assign(names.begin(), names.end());
  }

  message->set_name(in->GetName().Get());
  message->set_allocated_body(From(in->GetBody()));

  return message;
}

SyntaxTree::Export *AstWriter::From(const FlowPtr<Export> &in) {
  auto *message = Pool::CreateMessage<SyntaxTree::Export>(m_arena);

  message->set_allocated_location(FromSource(in));
  message->set_abi_name(in->GetAbiName().Get());
  message->set_allocated_body(From(in->GetBody()));
  message->set_visibility(FromVisibility(in->GetVis()));

  { /* Add all attributes */
    const auto &items = in->GetAttributes();

    message->mutable_attributes()->Reserve(items.size());
    std::for_each(items.begin(), items.end(),
                  [&](auto item) { message->mutable_attributes()->AddAllocated(From(item)); });
  }

  return message;
}

////////////////////////////////////////////////////////////////////////////////

#define SEND(__message, __node_name)                             \
  {                                                              \
    auto *message = From(n);                                     \
    message->CheckInitialized();                                 \
    auto *root = Pool::CreateMessage<SyntaxTree::Root>(m_arena); \
    root->set_allocated_##__node_name(message);                  \
    root->CheckInitialized();                                    \
    if (m_plaintext_mode) {                                      \
      m_os << root->Utf8DebugString();                           \
    } else {                                                     \
      if (!root->SerializeToOstream(&m_os)) [[unlikely]] {       \
        qcore_panic("Failed to serialize protobuf message");     \
      }                                                          \
    }                                                            \
  }

void AstWriter::Visit(FlowPtr<Base> n) { SEND(From(n), base); }
void AstWriter::Visit(FlowPtr<ExprStmt> n) { SEND(From(n), expr); }
void AstWriter::Visit(FlowPtr<LambdaExpr> n) { SEND(From(n), lambda_expr); }
void AstWriter::Visit(FlowPtr<NamedTy> n) { SEND(From(n), named); }
void AstWriter::Visit(FlowPtr<InferTy> n) { SEND(From(n), infer); }
void AstWriter::Visit(FlowPtr<TemplateType> n) { SEND(From(n), template_); }
void AstWriter::Visit(FlowPtr<U1> n) { SEND(From(n), u1); }
void AstWriter::Visit(FlowPtr<U8> n) { SEND(From(n), u8); }
void AstWriter::Visit(FlowPtr<U16> n) { SEND(From(n), u16); }
void AstWriter::Visit(FlowPtr<U32> n) { SEND(From(n), u32); }
void AstWriter::Visit(FlowPtr<U64> n) { SEND(From(n), u64); }
void AstWriter::Visit(FlowPtr<U128> n) { SEND(From(n), u128); }
void AstWriter::Visit(FlowPtr<I8> n) { SEND(From(n), i8); }
void AstWriter::Visit(FlowPtr<I16> n) { SEND(From(n), i16); }
void AstWriter::Visit(FlowPtr<I32> n) { SEND(From(n), i32); }
void AstWriter::Visit(FlowPtr<I64> n) { SEND(From(n), i64); }
void AstWriter::Visit(FlowPtr<I128> n) { SEND(From(n), i128); }
void AstWriter::Visit(FlowPtr<F16> n) { SEND(From(n), f16); }
void AstWriter::Visit(FlowPtr<F32> n) { SEND(From(n), f32); }
void AstWriter::Visit(FlowPtr<F64> n) { SEND(From(n), f64); }
void AstWriter::Visit(FlowPtr<F128> n) { SEND(From(n), f128); }
void AstWriter::Visit(FlowPtr<VoidTy> n) { SEND(From(n), void_); }
void AstWriter::Visit(FlowPtr<PtrTy> n) { SEND(From(n), ptr); }
void AstWriter::Visit(FlowPtr<OpaqueTy> n) { SEND(From(n), opaque); }
void AstWriter::Visit(FlowPtr<TupleTy> n) { SEND(From(n), tuple); }
void AstWriter::Visit(FlowPtr<ArrayTy> n) SEND(From(n), array);
void AstWriter::Visit(FlowPtr<RefTy> n) { SEND(From(n), ref); }
void AstWriter::Visit(FlowPtr<FuncTy> n) { SEND(From(n), func); }
void AstWriter::Visit(FlowPtr<Unary> n) { SEND(From(n), unary); }
void AstWriter::Visit(FlowPtr<Binary> n) { SEND(From(n), binary); }
void AstWriter::Visit(FlowPtr<PostUnary> n) { SEND(From(n), post_unary); }
void AstWriter::Visit(FlowPtr<Ternary> n) { SEND(From(n), ternary); }
void AstWriter::Visit(FlowPtr<Integer> n) { SEND(From(n), integer); }
void AstWriter::Visit(FlowPtr<Float> n) { SEND(From(n), float_); }
void AstWriter::Visit(FlowPtr<Boolean> n) { SEND(From(n), boolean); }
void AstWriter::Visit(FlowPtr<String> n) { SEND(From(n), string); }
void AstWriter::Visit(FlowPtr<Character> n) { SEND(From(n), character); }
void AstWriter::Visit(FlowPtr<Null> n) { SEND(From(n), null); }
void AstWriter::Visit(FlowPtr<Undefined> n) { SEND(From(n), undefined); }
void AstWriter::Visit(FlowPtr<Call> n) { SEND(From(n), call); }
void AstWriter::Visit(FlowPtr<TemplateCall> n) { SEND(From(n), template_call); }
void AstWriter::Visit(FlowPtr<List> n) { SEND(From(n), list); }
void AstWriter::Visit(FlowPtr<Assoc> n) { SEND(From(n), assoc); }
void AstWriter::Visit(FlowPtr<Index> n) { SEND(From(n), index); }
void AstWriter::Visit(FlowPtr<Slice> n) { SEND(From(n), slice); }
void AstWriter::Visit(FlowPtr<FString> n) { SEND(From(n), fstring); }
void AstWriter::Visit(FlowPtr<Identifier> n) { SEND(From(n), identifier); }
void AstWriter::Visit(FlowPtr<Sequence> n) { SEND(From(n), sequence); }
void AstWriter::Visit(FlowPtr<Block> n) { SEND(From(n), block); }
void AstWriter::Visit(FlowPtr<Variable> n) { SEND(From(n), variable); }
void AstWriter::Visit(FlowPtr<Assembly> n) { SEND(From(n), assembly); }
void AstWriter::Visit(FlowPtr<If> n) { SEND(From(n), if_); }
void AstWriter::Visit(FlowPtr<While> n) { SEND(From(n), while_); }
void AstWriter::Visit(FlowPtr<For> n) { SEND(From(n), for_); }
void AstWriter::Visit(FlowPtr<Foreach> n) { SEND(From(n), foreach); }
void AstWriter::Visit(FlowPtr<Break> n) { SEND(From(n), break_); }
void AstWriter::Visit(FlowPtr<Continue> n) { SEND(From(n), continue_); }
void AstWriter::Visit(FlowPtr<Return> n) { SEND(From(n), return_); }
void AstWriter::Visit(FlowPtr<ReturnIf> n) { SEND(From(n), return_if); }
void AstWriter::Visit(FlowPtr<Case> n) { SEND(From(n), case_); }
void AstWriter::Visit(FlowPtr<Switch> n) { SEND(From(n), switch_); }
void AstWriter::Visit(FlowPtr<Typedef> n) { SEND(From(n), typedef_); }
void AstWriter::Visit(FlowPtr<Function> n) { SEND(From(n), function); }
void AstWriter::Visit(FlowPtr<Struct> n) { SEND(From(n), struct_); }
void AstWriter::Visit(FlowPtr<Enum> n) { SEND(From(n), enum_); }
void AstWriter::Visit(FlowPtr<Scope> n) { SEND(From(n), scope); }
void AstWriter::Visit(FlowPtr<Export> n) { SEND(From(n), export_); }

AstWriter::AstWriter(std::ostream &os, WriterSourceProvider rd, bool plaintext_mode)
    : m_arena(new google::protobuf::Arena), m_os(os), m_rd(rd), m_plaintext_mode(plaintext_mode) {}

AstWriter::~AstWriter() { delete m_arena; }
