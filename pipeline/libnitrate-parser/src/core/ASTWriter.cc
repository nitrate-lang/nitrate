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

#include <google/protobuf/message.h>

#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>

#include "core/SyntaxTree.pb.h"

using namespace ncc::parse;
using namespace google;
using namespace nitrate::parser::SyntaxTree;

using Pool = google::protobuf::Arena;

static SyntaxTree::Operator FromOperator(ncc::lex::Operator op) {
  using LexOp = ncc::lex::Operator;

  switch (op) {
    case LexOp::OpPlus:
      return SyntaxTree::Plus;

    case LexOp::OpMinus:
      return SyntaxTree::Minus;

    case LexOp::OpTimes:
      return SyntaxTree::Times;

    case LexOp::OpSlash:
      return SyntaxTree::Slash;

    case LexOp::OpPercent:
      return SyntaxTree::Percent;

    case LexOp::OpBitAnd:
      return SyntaxTree::BitAnd;

    case LexOp::OpBitOr:
      return SyntaxTree::BitOr;

    case LexOp::OpBitXor:
      return SyntaxTree::BitXor;

    case LexOp::OpBitNot:
      return SyntaxTree::BitNot;

    case LexOp::OpLShift:
      return SyntaxTree::LShift;

    case LexOp::OpRShift:
      return SyntaxTree::RShift;

    case LexOp::OpROTL:
      return SyntaxTree::ROTL;

    case LexOp::OpROTR:
      return SyntaxTree::ROTR;

    case LexOp::OpLogicAnd:
      return SyntaxTree::LogicAnd;

    case LexOp::OpLogicOr:
      return SyntaxTree::LogicOr;

    case LexOp::OpLogicXor:
      return SyntaxTree::LogicXor;

    case LexOp::OpLogicNot:
      return SyntaxTree::LogicNot;

    case LexOp::OpLT:
      return SyntaxTree::LT;

    case LexOp::OpGT:
      return SyntaxTree::GT;

    case LexOp::OpLE:
      return SyntaxTree::LE;

    case LexOp::OpGE:
      return SyntaxTree::GE;

    case LexOp::OpEq:
      return SyntaxTree::Eq;

    case LexOp::OpNE:
      return SyntaxTree::NE;

    case LexOp::OpSet:
      return SyntaxTree::Set;

    case LexOp::OpPlusSet:
      return SyntaxTree::PlusSet;

    case LexOp::OpMinusSet:
      return SyntaxTree::MinusSet;

    case LexOp::OpTimesSet:
      return SyntaxTree::TimesSet;

    case LexOp::OpSlashSet:
      return SyntaxTree::SlashSet;

    case LexOp::OpPercentSet:
      return SyntaxTree::PercentSet;

    case LexOp::OpBitAndSet:
      return SyntaxTree::BitAndSet;

    case LexOp::OpBitOrSet:
      return SyntaxTree::BitOrSet;

    case LexOp::OpBitXorSet:
      return SyntaxTree::BitXorSet;

    case LexOp::OpLogicAndSet:
      return SyntaxTree::LogicAndSet;

    case LexOp::OpLogicOrSet:
      return SyntaxTree::LogicOrSet;

    case LexOp::OpLogicXorSet:
      return SyntaxTree::LogicXorSet;

    case LexOp::OpLShiftSet:
      return SyntaxTree::LShiftSet;

    case LexOp::OpRShiftSet:
      return SyntaxTree::RShiftSet;

    case LexOp::OpROTLSet:
      return SyntaxTree::ROTLSet;

    case LexOp::OpROTRSet:
      return SyntaxTree::ROTRSet;

    case LexOp::OpInc:
      return SyntaxTree::Inc;

    case LexOp::OpDec:
      return SyntaxTree::Dec;

    case LexOp::OpAs:
      return SyntaxTree::As;

    case LexOp::OpBitcastAs:
      return SyntaxTree::BitcastAs;

    case LexOp::OpIn:
      return SyntaxTree::In;

    case LexOp::OpOut:
      return SyntaxTree::Out;

    case LexOp::OpSizeof:
      return SyntaxTree::Sizeof;

    case LexOp::OpBitsizeof:
      return SyntaxTree::Bitsizeof;

    case LexOp::OpAlignof:
      return SyntaxTree::Alignof;

    case LexOp::OpTypeof:
      return SyntaxTree::Typeof;

    case LexOp::OpComptime:
      return SyntaxTree::Comptime;

    case LexOp::OpDot:
      return SyntaxTree::Dot;

    case LexOp::OpRange:
      return SyntaxTree::Range;

    case LexOp::OpEllipsis:
      return SyntaxTree::Ellipsis;

    case LexOp::OpArrow:
      return SyntaxTree::Arrow;

    case LexOp::OpTernary:
      return SyntaxTree::Question;
  }
}

static SyntaxTree::Vis FromVisibility(ncc::parse::Vis vis) {
  switch (vis) {
    case ncc::parse::Vis::Pub:
      return SyntaxTree::Public;

    case ncc::parse::Vis::Pro:
      return SyntaxTree::Protected;

    case ncc::parse::Vis::Sec:
      return SyntaxTree::Private;
  }
}

static SyntaxTree::FunctionPurity FromPurity(ncc::parse::Purity purity) {
  switch (purity) {
    case ncc::parse::Purity::Impure:
      return SyntaxTree::Impure;

    case ncc::parse::Purity::Impure_TSafe:
      return SyntaxTree::Impure_TSafe;

    case ncc::parse::Purity::Pure:
      return SyntaxTree::Pure;

    case ncc::parse::Purity::Quasi:
      return SyntaxTree::Quasi;

    case ncc::parse::Purity::Retro:
      return SyntaxTree::Retro;
  }
}

void AstWriter::AttachTypeMetadata(auto *object, const FlowPtr<Type> &in) {
  if (in->GetWidth().has_value()) [[unlikely]] {
    object->set_allocated_bit_width(From(in->GetWidth().value()));
  }

  if (in->GetRangeBegin().has_value()) [[unlikely]] {
    object->set_allocated_minimum(From(in->GetRangeBegin().value()));
  }

  if (in->GetRangeEnd().has_value()) [[unlikely]] {
    object->set_allocated_maximum(From(in->GetRangeEnd().value()));
  }
}

SyntaxTree::SourceLocationRange *AstWriter::FromSource(
    const FlowPtr<Base> &in) {
  if (!m_rd.has_value()) {
    return nullptr;
  }

  const auto &pos = in->GetPos();
  auto start_pos = m_rd->get().GetLocation(pos.first);
  auto end_pos = m_rd->get().GetLocation(pos.second);

  if (start_pos == lex::Location::EndOfFile() &&
      end_pos == lex::Location::EndOfFile()) {
    return nullptr;
  }

  auto *object = Pool::CreateMessage<SyntaxTree::SourceLocationRange>(&m_arena);

  if (start_pos != lex::Location::EndOfFile()) {
    auto *start = Pool::CreateMessage<SyntaxTree::SourceLocation>(&m_arena);
    start->set_line(start_pos.GetRow() + 1);
    start->set_column(start_pos.GetCol() + 1);
    start->set_offset(start_pos.GetOffset());
    start->set_file(start_pos.GetFilename().Get());

    object->set_allocated_start(start);
  }

  if (end_pos != lex::Location::EndOfFile()) {
    auto *end = Pool::CreateMessage<SyntaxTree::SourceLocation>(&m_arena);
    end->set_line(end_pos.GetRow() + 1);
    end->set_column(end_pos.GetCol() + 1);
    end->set_offset(end_pos.GetOffset());
    end->set_file(end_pos.GetFilename().Get());

    object->set_allocated_end(end);
  }

  return object;
}

SyntaxTree::Expr *AstWriter::From(FlowPtr<Expr> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Expr>(&m_arena);

  switch (in->GetKind()) {
    case QAST_BASE: {
      object->set_allocated_base(From(in.As<Base>()));
      break;
    }

    case QAST_BINEXPR: {
      object->set_allocated_binary(From(in.As<Binary>()));
      break;
    }

    case QAST_UNEXPR: {
      object->set_allocated_unary(From(in.As<Unary>()));
      break;
    }

    case QAST_POST_UNEXPR: {
      object->set_allocated_post_unary(From(in.As<PostUnary>()));
      break;
    }

    case QAST_TEREXPR: {
      object->set_allocated_ternary(From(in.As<Ternary>()));
      break;
    }

    case QAST_INT: {
      object->set_allocated_integer(From(in.As<Integer>()));
      break;
    }

    case QAST_FLOAT: {
      object->set_allocated_float_(From(in.As<Float>()));
      break;
    }

    case QAST_STRING: {
      object->set_allocated_string(From(in.As<String>()));
      break;
    }

    case QAST_CHAR: {
      object->set_allocated_character(From(in.As<Character>()));
      break;
    }

    case QAST_BOOL: {
      object->set_allocated_boolean(From(in.As<Boolean>()));
      break;
    }

    case QAST_NULL: {
      object->set_allocated_null(From(in.As<Null>()));
      break;
    }

    case QAST_UNDEF: {
      object->set_allocated_undefined(From(in.As<Undefined>()));
      break;
    }

    case QAST_CALL: {
      object->set_allocated_call(From(in.As<Call>()));
      break;
    }

    case QAST_LIST: {
      object->set_allocated_list(From(in.As<List>()));
      break;
    }

    case QAST_ASSOC: {
      object->set_allocated_assoc(From(in.As<Assoc>()));
      break;
    }

    case QAST_INDEX: {
      object->set_allocated_index(From(in.As<Index>()));
      break;
    }

    case QAST_SLICE: {
      object->set_allocated_slice(From(in.As<Slice>()));
      break;
    }

    case QAST_FSTRING: {
      object->set_allocated_fstring(From(in.As<FString>()));
      break;
    }

    case QAST_IDENT: {
      object->set_allocated_identifier(From(in.As<Identifier>()));
      break;
    }

    case QAST_SEXPR: {
      object->set_allocated_stmt_expr(From(in.As<StmtExpr>()));
      break;
    }

    case QAST_TEXPR: {
      object->set_allocated_type_expr(From(in.As<TypeExpr>()));
      break;
    }

    case QAST_TEMPL_CALL: {
      object->set_allocated_template_call(From(in.As<TemplateCall>()));
      break;
    }

    default: {
      qcore_panic("Unknown expression kind");
    }
  }

  return object;
}

SyntaxTree::Stmt *AstWriter::From(FlowPtr<Stmt> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Stmt>(&m_arena);

  switch (in->GetKind()) {
    case QAST_BASE: {
      object->set_allocated_base(From(in.As<Base>()));
      break;
    }

    case QAST_IF: {
      object->set_allocated_if_(From(in.As<If>()));
      break;
    }

    case QAST_RETIF: {
      object->set_allocated_return_if(From(in.As<ReturnIf>()));
      break;
    }

    case QAST_SWITCH: {
      object->set_allocated_switch_(From(in.As<Switch>()));
      break;
    }

    case QAST_CASE: {
      object->set_allocated_case_(From(in.As<Case>()));
      break;
    }

    case QAST_RETURN: {
      object->set_allocated_return_(From(in.As<Return>()));
      break;
    }

    case QAST_BREAK: {
      object->set_allocated_break_(From(in.As<Break>()));
      break;
    }

    case QAST_CONTINUE: {
      object->set_allocated_continue_(From(in.As<Continue>()));
      break;
    }

    case QAST_WHILE: {
      object->set_allocated_while_(From(in.As<While>()));
      break;
    }

    case QAST_FOR: {
      object->set_allocated_for_(From(in.As<For>()));
      break;
    }

    case QAST_FOREACH: {
      object->set_allocated_foreach(From(in.As<Foreach>()));
      break;
    }

    case QAST_INLINE_ASM: {
      object->set_allocated_assembly(From(in.As<Assembly>()));
      break;
    }

    case QAST_ESTMT: {
      object->set_allocated_expr_stmt(From(in.As<ExprStmt>()));
      break;
    }

    case QAST_TYPEDEF: {
      object->set_allocated_typedef_(From(in.As<Typedef>()));
      break;
    }

    case QAST_STRUCT: {
      object->set_allocated_struct_(From(in.As<Struct>()));
      break;
    }

    case QAST_ENUM: {
      object->set_allocated_enum_(From(in.As<Enum>()));
      break;
    }

    case QAST_SCOPE: {
      object->set_allocated_scope(From(in.As<Scope>()));
      break;
    }

    case QAST_BLOCK: {
      object->set_allocated_block(From(in.As<Block>()));
      break;
    }

    case QAST_EXPORT: {
      object->set_allocated_export_(From(in.As<Export>()));
      break;
    }

    case QAST_VAR: {
      object->set_allocated_variable(From(in.As<Variable>()));
      break;
    }

    case QAST_FUNCTION: {
      object->set_allocated_function(From(in.As<Function>()));
      break;
    }

    default: {
      qcore_panic("Unknown statement kind");
    }
  }

  return object;
}

SyntaxTree::Type *AstWriter::From(FlowPtr<Type> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Type>(&m_arena);

  switch (in->GetKind()) {
    case QAST_BASE: {
      object->set_allocated_base(From(in.As<Base>()));
      break;
    }

    case QAST_U1: {
      object->set_allocated_u1(From(in.As<U1>()));
      break;
    }

    case QAST_U8: {
      object->set_allocated_u8(From(in.As<U8>()));
      break;
    }

    case QAST_U16: {
      object->set_allocated_u16(From(in.As<U16>()));
      break;
    }

    case QAST_U32: {
      object->set_allocated_u32(From(in.As<U32>()));
      break;
    }

    case QAST_U64: {
      object->set_allocated_u64(From(in.As<U64>()));
      break;
    }

    case QAST_U128: {
      object->set_allocated_u128(From(in.As<U128>()));
      break;
    }

    case QAST_I8: {
      object->set_allocated_i8(From(in.As<I8>()));
      break;
    }

    case QAST_I16: {
      object->set_allocated_i16(From(in.As<I16>()));
      break;
    }

    case QAST_I32: {
      object->set_allocated_i32(From(in.As<I32>()));
      break;
    }

    case QAST_I64: {
      object->set_allocated_i64(From(in.As<I64>()));
      break;
    }

    case QAST_I128: {
      object->set_allocated_i128(From(in.As<I128>()));
      break;
    }

    case QAST_F16: {
      object->set_allocated_f16(From(in.As<F16>()));
      break;
    }

    case QAST_F32: {
      object->set_allocated_f32(From(in.As<F32>()));
      break;
    }

    case QAST_F64: {
      object->set_allocated_f64(From(in.As<F64>()));
      break;
    }

    case QAST_F128: {
      object->set_allocated_f128(From(in.As<F128>()));
      break;
    }

    case QAST_VOID: {
      object->set_allocated_void_(From(in.As<VoidTy>()));
      break;
    }

    case QAST_INFER: {
      object->set_allocated_infer(From(in.As<InferTy>()));
      break;
    }

    case QAST_OPAQUE: {
      object->set_allocated_opaque(From(in.As<OpaqueTy>()));
      break;
    }

    case QAST_NAMED: {
      object->set_allocated_named(From(in.As<NamedTy>()));
      break;
    }

    case QAST_REF: {
      object->set_allocated_ref(From(in.As<RefTy>()));
      break;
    }

    case QAST_PTR: {
      object->set_allocated_ptr(From(in.As<PtrTy>()));
      break;
    }

    case QAST_ARRAY: {
      object->set_allocated_array(From(in.As<ArrayTy>()));
      break;
    }

    case QAST_TUPLE: {
      object->set_allocated_tuple(From(in.As<TupleTy>()));
      break;
    }

    case QAST_TEMPLATE: {
      object->set_allocated_template_(From(in.As<TemplateType>()));
      break;
    }

    case QAST_FUNCTOR: {
      object->set_allocated_func(From(in.As<FuncTy>()));
      break;
    }

    default: {
      qcore_panic("Unknown type kind");
    }
  }

  return object;
}

SyntaxTree::Base *AstWriter::From(FlowPtr<Base> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Base>(&m_arena);

  object->set_allocated_location(FromSource(in));

  return object;
}

SyntaxTree::ExprStmt *AstWriter::From(FlowPtr<ExprStmt> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::ExprStmt>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_expression(From(in->GetExpr()));

  return object;
}

SyntaxTree::StmtExpr *AstWriter::From(FlowPtr<StmtExpr> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::StmtExpr>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_statement(From(in->GetStmt()));

  return object;
}

SyntaxTree::TypeExpr *AstWriter::From(FlowPtr<TypeExpr> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::TypeExpr>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_type(From(in->GetType()));

  return object;
}

SyntaxTree::NamedTy *AstWriter::From(FlowPtr<NamedTy> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::NamedTy>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_name(in->GetName().Get());
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::InferTy *AstWriter::From(FlowPtr<InferTy> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::InferTy>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::TemplateType *AstWriter::From(FlowPtr<TemplateType> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::TemplateType>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_base(From(in->GetTemplate()));
  AttachTypeMetadata(object, in);

  { /* Add all arguments */
    const auto &args = in->GetArgs();
    auto *arg_list = object->mutable_arguments();
    arg_list->Reserve(args.size());

    for (const auto &arg : args) {
      auto *argument = Pool::CreateMessage<SyntaxTree::CallArgument>(&m_arena);
      argument->set_name(arg.first.Get());
      argument->set_allocated_value(From(arg.second));
      arg_list->AddAllocated(argument);
    }
  }

  return object;
}

SyntaxTree::U1 *AstWriter::From(FlowPtr<U1> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::U1>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::U8 *AstWriter::From(FlowPtr<U8> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::U8>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::U16 *AstWriter::From(FlowPtr<U16> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::U16>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::U32 *AstWriter::From(FlowPtr<U32> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::U32>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::U64 *AstWriter::From(FlowPtr<U64> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::U64>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::U128 *AstWriter::From(FlowPtr<U128> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::U128>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::I8 *AstWriter::From(FlowPtr<I8> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::I8>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::I16 *AstWriter::From(FlowPtr<I16> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::I16>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::I32 *AstWriter::From(FlowPtr<I32> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::I32>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::I64 *AstWriter::From(FlowPtr<I64> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::I64>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::I128 *AstWriter::From(FlowPtr<I128> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::I128>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::F16 *AstWriter::From(FlowPtr<F16> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::F16>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::F32 *AstWriter::From(FlowPtr<F32> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::F32>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::F64 *AstWriter::From(FlowPtr<F64> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::F64>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::F128 *AstWriter::From(FlowPtr<F128> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::F128>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::VoidTy *AstWriter::From(FlowPtr<VoidTy> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::VoidTy>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::PtrTy *AstWriter::From(FlowPtr<PtrTy> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::PtrTy>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_pointee(From(in->GetItem()));
  object->set_volatile_(in->IsVolatile());
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::OpaqueTy *AstWriter::From(FlowPtr<OpaqueTy> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::OpaqueTy>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_name(in->GetName().Get());
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::TupleTy *AstWriter::From(FlowPtr<TupleTy> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::TupleTy>(&m_arena);

  object->set_allocated_location(FromSource(in));
  AttachTypeMetadata(object, in);

  { /* Add all elements */
    const auto &items = in->GetItems();

    object->mutable_elements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      object->mutable_elements()->AddAllocated(From(item));
    });
  }

  return object;
}

SyntaxTree::ArrayTy *AstWriter::From(FlowPtr<ArrayTy> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::ArrayTy>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_element_type(From(in->GetItem()));
  object->set_allocated_element_count(From(in->GetSize()));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::RefTy *AstWriter::From(FlowPtr<RefTy> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::RefTy>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_pointee(From(in->GetItem()));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::FuncTy *AstWriter::From(FlowPtr<FuncTy> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::FuncTy>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_return_type(From(in->GetReturn()));
  object->set_variadic(in->IsVariadic());
  object->set_purity(FromPurity(in->GetPurity()));
  AttachTypeMetadata(object, in);

  { /* Add all parameters */
    const auto &params = in->GetParams();
    auto *param_list = object->mutable_parameters();
    param_list->Reserve(params.size());

    for (const auto &param : params) {
      auto *parameter =
          Pool::CreateMessage<SyntaxTree::FunctionParameter>(&m_arena);
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
    object->mutable_attributes()->Reserve(attrs.size());
    std::for_each(attrs.begin(), attrs.end(), [&](auto attr) {
      object->mutable_attributes()->AddAllocated(From(attr));
    });
  }

  return object;
}

SyntaxTree::Unary *AstWriter::From(FlowPtr<Unary> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Unary>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_operator_(FromOperator(in->GetOp()));
  object->set_allocated_operand(From(in->GetRHS()));

  return object;
}

SyntaxTree::Binary *AstWriter::From(FlowPtr<Binary> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Binary>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_operator_(FromOperator(in->GetOp()));
  object->set_allocated_left(From(in->GetLHS()));
  object->set_allocated_right(From(in->GetRHS()));

  return object;
}

SyntaxTree::PostUnary *AstWriter::From(FlowPtr<PostUnary> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::PostUnary>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_operator_(FromOperator(in->GetOp()));
  object->set_allocated_operand(From(in->GetLHS()));

  return object;
}

SyntaxTree::Ternary *AstWriter::From(FlowPtr<Ternary> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Ternary>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_condition(From(in->GetCond()));
  object->set_allocated_true_branch(From(in->GetLHS()));
  object->set_allocated_false_branch(From(in->GetRHS()));

  return object;
}

SyntaxTree::Integer *AstWriter::From(FlowPtr<Integer> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Integer>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_number(in->GetValue().Get());

  return object;
}

SyntaxTree::Float *AstWriter::From(FlowPtr<Float> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Float>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_number(in->GetValue().Get());

  return object;
}

SyntaxTree::Boolean *AstWriter::From(FlowPtr<Boolean> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Boolean>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_value(in->GetValue());

  return object;
}

SyntaxTree::String *AstWriter::From(FlowPtr<String> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::String>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_text(in->GetValue().Get());

  return object;
}

SyntaxTree::Character *AstWriter::From(FlowPtr<Character> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Character>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_char_(in->GetValue());

  return object;
}

SyntaxTree::Null *AstWriter::From(FlowPtr<Null> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Null>(&m_arena);

  object->set_allocated_location(FromSource(in));

  return object;
}

SyntaxTree::Undefined *AstWriter::From(FlowPtr<Undefined> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Undefined>(&m_arena);

  object->set_allocated_location(FromSource(in));

  return object;
}

SyntaxTree::Call *AstWriter::From(FlowPtr<Call> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Call>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_callee(From(in->GetFunc()));

  { /* Add all arguments */
    const auto &args = in->GetArgs();
    object->mutable_arguments()->Reserve(args.size());

    std::for_each(args.begin(), args.end(), [&](auto arg) {
      auto *argument = Pool::CreateMessage<SyntaxTree::CallArgument>(&m_arena);
      argument->set_name(arg.first.Get());
      argument->set_allocated_value(From(arg.second));
      object->mutable_arguments()->AddAllocated(argument);
    });
  }

  return object;
}

SyntaxTree::TemplateCall *AstWriter::From(FlowPtr<TemplateCall> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::TemplateCall>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_callee(From(in->GetFunc()));

  { /* Add all arguments */
    const auto &args = in->GetArgs();
    object->mutable_arguments()->Reserve(args.size());

    std::for_each(args.begin(), args.end(), [&](auto arg) {
      auto *argument = Pool::CreateMessage<SyntaxTree::CallArgument>(&m_arena);
      argument->set_name(arg.first.Get());
      argument->set_allocated_value(From(arg.second));
      object->mutable_arguments()->AddAllocated(argument);
    });
  }

  { /* Add all template arguments */
    const auto &args = in->GetTemplateArgs();
    object->mutable_template_arguments()->Reserve(args.size());

    std::for_each(args.begin(), args.end(), [&](auto arg) {
      auto *argument = Pool::CreateMessage<SyntaxTree::CallArgument>(&m_arena);
      argument->set_name(arg.first.Get());
      argument->set_allocated_value(From(arg.second));
      object->mutable_template_arguments()->AddAllocated(argument);
    });
  }

  return object;
}

SyntaxTree::List *AstWriter::From(FlowPtr<List> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::List>(&m_arena);

  object->set_allocated_location(FromSource(in));

  { /* Add all elements */
    const auto &items = in->GetItems();

    object->mutable_elements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      object->mutable_elements()->AddAllocated(From(item));
    });
  }

  return object;
}

SyntaxTree::Assoc *AstWriter::From(FlowPtr<Assoc> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Assoc>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_key(From(in->GetKey()));
  object->set_allocated_value(From(in->GetValue()));

  return object;
}

SyntaxTree::Index *AstWriter::From(FlowPtr<Index> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Index>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_base(From(in->GetBase()));
  object->set_allocated_index(From(in->GetIndex()));

  return object;
}

SyntaxTree::Slice *AstWriter::From(FlowPtr<Slice> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Slice>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_base(From(in->GetBase()));
  object->set_allocated_start(From(in->GetStart()));
  object->set_allocated_end(From(in->GetEnd()));

  return object;
}

SyntaxTree::FString *AstWriter::From(FlowPtr<FString> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Identifier *AstWriter::From(FlowPtr<Identifier> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Identifier>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_name(in->GetName().Get());

  return object;
}

SyntaxTree::Sequence *AstWriter::From(FlowPtr<Sequence> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Sequence>(&m_arena);

  object->set_allocated_location(FromSource(in));

  { /* Add all elements */
    const auto &items = in->GetItems();

    object->mutable_elements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      object->mutable_elements()->AddAllocated(From(item));
    });
  }

  return object;
}

SyntaxTree::Block *AstWriter::From(FlowPtr<Block> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Block>(&m_arena);

  object->set_allocated_location(FromSource(in));

  switch (in->GetSafety()) {
    case SafetyMode::Unknown: {
      object->set_guarantor(SyntaxTree::SafetyMode::Unspecified);
      break;
    }

    case SafetyMode::Safe: {
      object->set_guarantor(SyntaxTree::SafetyMode::Safe);
      break;
    }

    case SafetyMode::Unsafe: {
      object->set_guarantor(SyntaxTree::SafetyMode::Unsafe);
      break;
    }
  }

  { /* Add all statements */
    const auto &items = in->GetItems();

    object->mutable_statements()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      object->mutable_statements()->AddAllocated(From(item));
    });
  }

  return object;
}

SyntaxTree::Variable *AstWriter::From(FlowPtr<Variable> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Variable>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_name(in->GetName().Get());
  if (in->GetType().has_value()) {
    object->set_allocated_type(From(in->GetType().value()));
  }
  if (in->GetValue().has_value()) {
    object->set_allocated_initial_value(From(in->GetValue().value()));
  }

  switch (in->GetDeclType()) {
    case VariableType::Var:
      object->set_kind(SyntaxTree::VariableKind::Var);
      break;

    case VariableType::Let:
      object->set_kind(SyntaxTree::VariableKind::Let);
      break;

    case VariableType::Const:
      object->set_kind(SyntaxTree::VariableKind::Const);
      break;
  }

  { /* Add all attributes */
    const auto &items = in->GetAttributes();

    object->mutable_attributes()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      object->mutable_attributes()->AddAllocated(From(item));
    });
  }

  return object;
}

SyntaxTree::Assembly *AstWriter::From(FlowPtr<Assembly> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Assembly>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_code(in->GetCode().Get());

  { /* Add all arguments */
    const auto &items = in->GetArgs();

    object->mutable_arguments()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      object->mutable_arguments()->AddAllocated(From(item));
    });
  }

  return object;
}

SyntaxTree::If *AstWriter::From(FlowPtr<If> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::If>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_condition(From(in->GetCond()));
  object->set_allocated_true_branch(From(in->GetThen()));

  if (in->GetElse().has_value()) {
    object->set_allocated_false_branch(From(in->GetElse().value()));
  }

  return object;
}

SyntaxTree::While *AstWriter::From(FlowPtr<While> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::While>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_condition(From(in->GetCond()));
  object->set_allocated_body(From(in->GetBody()));

  return object;
}

SyntaxTree::For *AstWriter::From(FlowPtr<For> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::For>(&m_arena);

  object->set_allocated_location(FromSource(in));

  if (in->GetInit().has_value()) {
    object->set_allocated_init(From(in->GetInit().value()));
  }

  if (in->GetCond().has_value()) {
    object->set_allocated_condition(From(in->GetCond().value()));
  }

  if (in->GetStep().has_value()) {
    object->set_allocated_step(From(in->GetStep().value()));
  }

  object->set_allocated_body(From(in->GetBody()));

  return object;
}

SyntaxTree::Foreach *AstWriter::From(FlowPtr<Foreach> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Foreach>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_index_name(in->GetIdxIdentifier().Get());
  object->set_value_name(in->GetValIdentifier().Get());
  object->set_allocated_expression(From(in->GetExpr()));
  object->set_allocated_body(From(in->GetBody()));

  return object;
}

SyntaxTree::Break *AstWriter::From(FlowPtr<Break> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Break>(&m_arena);

  object->set_allocated_location(FromSource(in));

  return object;
}

SyntaxTree::Continue *AstWriter::From(FlowPtr<Continue> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Continue>(&m_arena);

  object->set_allocated_location(FromSource(in));

  return object;
}

SyntaxTree::Return *AstWriter::From(FlowPtr<Return> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Return>(&m_arena);

  object->set_allocated_location(FromSource(in));
  if (in->GetValue().has_value()) {
    object->set_allocated_value(From(in->GetValue().value()));
  }

  return object;
}

SyntaxTree::ReturnIf *AstWriter::From(FlowPtr<ReturnIf> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::ReturnIf>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_condition(From(in->GetCond()));
  object->set_allocated_value(From(in->GetValue()));

  return object;
}

SyntaxTree::Case *AstWriter::From(FlowPtr<Case> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Case>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_condition(From(in->GetCond()));
  object->set_allocated_body(From(in->GetBody()));

  return object;
}

SyntaxTree::Switch *AstWriter::From(FlowPtr<Switch> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Switch>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_allocated_condition(From(in->GetCond()));

  { /* Add all cases */
    const auto &items = in->GetCases();

    object->mutable_cases()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      object->mutable_cases()->AddAllocated(From(item));
    });
  }

  if (in->GetDefault().has_value()) {
    object->set_allocated_default_(From(in->GetDefault().value()));
  }

  return object;
}

SyntaxTree::Typedef *AstWriter::From(FlowPtr<Typedef> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Typedef>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_name(in->GetName().Get());
  object->set_allocated_type(From(in->GetType()));

  return object;
}

SyntaxTree::Function *AstWriter::From(FlowPtr<Function> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Struct *AstWriter::From(FlowPtr<Struct> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Enum *AstWriter::From(FlowPtr<Enum> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Scope *AstWriter::From(FlowPtr<Scope> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Scope>(&m_arena);

  object->set_allocated_location(FromSource(in));

  { /* Add all dependencies */
    const auto &items = in->GetDeps();
    std::vector<std::string_view> names(items.size());
    std::transform(items.begin(), items.end(), names.begin(),
                   [](auto item) { return item.Get(); });

    object->mutable_dependencies()->Assign(names.begin(), names.end());
  }

  object->set_name(in->GetName().Get());
  object->set_allocated_body(From(in->GetBody()));

  return object;
}

SyntaxTree::Export *AstWriter::From(FlowPtr<Export> in) {
  auto *object = Pool::CreateMessage<SyntaxTree::Export>(&m_arena);

  object->set_allocated_location(FromSource(in));
  object->set_abi_name(in->GetAbiName().Get());
  object->set_allocated_body(From(in->GetBody()));
  object->set_visibility(FromVisibility(in->GetVis()));

  { /* Add all attributes */
    const auto &items = in->GetAttrs();

    object->mutable_attributes()->Reserve(items.size());
    std::for_each(items.begin(), items.end(), [&](auto item) {
      object->mutable_attributes()->AddAllocated(From(item));
    });
  }

  return object;
}

////////////////////////////////////////////////////////////////////////////////

#define SEND(__object, __node_name)                               \
  {                                                               \
    auto *object = From(n);                                       \
    object->CheckInitialized();                                   \
    auto *root = Pool::CreateMessage<SyntaxTree::Root>(&m_arena); \
    root->set_allocated_##__node_name(object);                    \
    root->CheckInitialized();                                     \
    if (!root->SerializeToOstream(&m_os)) [[unlikely]] {          \
      qcore_panic("Failed to serialize protobuf message");        \
    }                                                             \
  }

void AstWriter::Visit(FlowPtr<Base> n) { SEND(From(n), base); }
void AstWriter::Visit(FlowPtr<ExprStmt> n) { SEND(From(n), expr); }
void AstWriter::Visit(FlowPtr<StmtExpr> n) { SEND(From(n), stmt_expr); }
void AstWriter::Visit(FlowPtr<TypeExpr> n) { SEND(From(n), type_expr); }
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

AstWriter::~AstWriter() = default;
