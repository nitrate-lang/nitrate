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

void AstWriter::AttachTypeMetadata(auto *object, const FlowPtr<Type> &in) {
  if (in->GetWidth().has_value()) [[unlikely]] {
    object->set_allocated_bit_width(From(in->GetWidth().value()));
  }

  if (in->GetRangeBegin().has_value()) [[unlikely]] {
    object->set_allocated_minimum_value(From(in->GetRangeBegin().value()));
  }

  if (in->GetRangeEnd().has_value()) [[unlikely]] {
    object->set_allocated_maximum_value(From(in->GetRangeEnd().value()));
  }
}

SyntaxTree::SourceLocationRange *AstWriter::FromSource(FlowPtr<Base> in) {
  auto *object = Pool::Create<SyntaxTree::SourceLocationRange>(&m_arena);

  /// TODO: From input to protobuf structure

  return object;
}

SyntaxTree::Expr *AstWriter::From(FlowPtr<Expr> in) {
  auto *object = Pool::Create<SyntaxTree::Expr>(&m_arena);

  switch (in->GetKind()) {
    case QAST_BINEXPR: {
      object->set_allocated_binary(From(in.As<BinExpr>()));
      break;
    }

    case QAST_UNEXPR: {
      object->set_allocated_unary(From(in.As<UnaryExpr>()));
      break;
    }

    case QAST_POST_UNEXPR: {
      object->set_allocated_post_unary(From(in.As<PostUnary>()));
      break;
    }

    case QAST_TEREXPR: {
      object->set_allocated_ternary(From(in.As<TernaryExpr>()));
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
      object->set_allocated_stmt(From(in.As<StmtExpr>()));
      break;
    }

    case QAST_TEXPR: {
      object->set_allocated_type(From(in.As<TypeExpr>()));
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
  auto *object = Pool::Create<SyntaxTree::Stmt>(&m_arena);

  switch (in->GetKind()) {
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
      object->set_allocated_expr(From(in.As<ExprStmt>()));
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
  auto *object = Pool::Create<SyntaxTree::Type>(&m_arena);

  switch (in->GetKind()) {
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
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::ExprStmt *AstWriter::From(FlowPtr<ExprStmt> in) {
  auto *object = Pool::Create<SyntaxTree::ExprStmt>(&m_arena);

  object->set_allocated_from(FromSource(in));
  object->set_allocated_expr(From(in->GetExpr()));

  return object;
}

SyntaxTree::StmtExpr *AstWriter::From(FlowPtr<StmtExpr> in) {
  auto *object = Pool::Create<SyntaxTree::StmtExpr>(&m_arena);

  object->set_allocated_from(FromSource(in));
  object->set_allocated_stmt(From(in->GetStmt()));

  return object;
}

SyntaxTree::TypeExpr *AstWriter::From(FlowPtr<TypeExpr> in) {
  auto *object = Pool::Create<SyntaxTree::TypeExpr>(&m_arena);

  object->set_allocated_from(FromSource(in));
  object->set_allocated_type(From(in->GetType()));

  return object;
}

SyntaxTree::NamedTy *AstWriter::From(FlowPtr<NamedTy> in) {
  auto *object = Pool::Create<SyntaxTree::NamedTy>(&m_arena);

  object->set_allocated_from(FromSource(in));
  object->set_type_name(in->GetName().Get());
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::InferTy *AstWriter::From(FlowPtr<InferTy> in) {
  auto *object = Pool::Create<SyntaxTree::InferTy>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::TemplateType *AstWriter::From(FlowPtr<TemplateType> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::U1 *AstWriter::From(FlowPtr<U1> in) {
  auto *object = Pool::Create<SyntaxTree::U1>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::U8 *AstWriter::From(FlowPtr<U8> in) {
  auto *object = Pool::Create<SyntaxTree::U8>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::U16 *AstWriter::From(FlowPtr<U16> in) {
  auto *object = Pool::Create<SyntaxTree::U16>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::U32 *AstWriter::From(FlowPtr<U32> in) {
  auto *object = Pool::Create<SyntaxTree::U32>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::U64 *AstWriter::From(FlowPtr<U64> in) {
  auto *object = Pool::Create<SyntaxTree::U64>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::U128 *AstWriter::From(FlowPtr<U128> in) {
  auto *object = Pool::Create<SyntaxTree::U128>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::I8 *AstWriter::From(FlowPtr<I8> in) {
  auto *object = Pool::Create<SyntaxTree::I8>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::I16 *AstWriter::From(FlowPtr<I16> in) {
  auto *object = Pool::Create<SyntaxTree::I16>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::I32 *AstWriter::From(FlowPtr<I32> in) {
  auto *object = Pool::Create<SyntaxTree::I32>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::I64 *AstWriter::From(FlowPtr<I64> in) {
  auto *object = Pool::Create<SyntaxTree::I64>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::I128 *AstWriter::From(FlowPtr<I128> in) {
  auto *object = Pool::Create<SyntaxTree::I128>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::F16 *AstWriter::From(FlowPtr<F16> in) {
  auto *object = Pool::Create<SyntaxTree::F16>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::F32 *AstWriter::From(FlowPtr<F32> in) {
  auto *object = Pool::Create<SyntaxTree::F32>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::F64 *AstWriter::From(FlowPtr<F64> in) {
  auto *object = Pool::Create<SyntaxTree::F64>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::F128 *AstWriter::From(FlowPtr<F128> in) {
  auto *object = Pool::Create<SyntaxTree::F128>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::VoidTy *AstWriter::From(FlowPtr<VoidTy> in) {
  auto *object = Pool::Create<SyntaxTree::VoidTy>(&m_arena);

  object->set_allocated_from(FromSource(in));
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::PtrTy *AstWriter::From(FlowPtr<PtrTy> in) {
  auto *object = Pool::Create<SyntaxTree::PtrTy>(&m_arena);

  object->set_allocated_from(FromSource(in));
  object->set_allocated_pointee(From(in->GetItem()));
  object->set_is_volatile(in->IsVolatile());
  AttachTypeMetadata(object, in);

  return object;
}

SyntaxTree::OpaqueTy *AstWriter::From(FlowPtr<OpaqueTy> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::TupleTy *AstWriter::From(FlowPtr<TupleTy> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::ArrayTy *AstWriter::From(FlowPtr<ArrayTy> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::RefTy *AstWriter::From(FlowPtr<RefTy> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::FuncTy *AstWriter::From(FlowPtr<FuncTy> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::UnaryExpr *AstWriter::From(FlowPtr<UnaryExpr> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
  (void)m_os;
}

SyntaxTree::BinExpr *AstWriter::From(FlowPtr<BinExpr> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::PostUnary *AstWriter::From(FlowPtr<PostUnary> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::TernaryExpr *AstWriter::From(FlowPtr<TernaryExpr> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Integer *AstWriter::From(FlowPtr<Integer> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Float *AstWriter::From(FlowPtr<Float> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Boolean *AstWriter::From(FlowPtr<Boolean> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::String *AstWriter::From(FlowPtr<String> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Character *AstWriter::From(FlowPtr<Character> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Null *AstWriter::From(FlowPtr<Null> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Undefined *AstWriter::From(FlowPtr<Undefined> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Call *AstWriter::From(FlowPtr<Call> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::TemplateCall *AstWriter::From(FlowPtr<TemplateCall> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::List *AstWriter::From(FlowPtr<List> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Assoc *AstWriter::From(FlowPtr<Assoc> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Index *AstWriter::From(FlowPtr<Index> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Slice *AstWriter::From(FlowPtr<Slice> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::FString *AstWriter::From(FlowPtr<FString> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Identifier *AstWriter::From(FlowPtr<Identifier> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Sequence *AstWriter::From(FlowPtr<Sequence> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Block *AstWriter::From(FlowPtr<Block> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Variable *AstWriter::From(FlowPtr<Variable> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Assembly *AstWriter::From(FlowPtr<Assembly> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::If *AstWriter::From(FlowPtr<If> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::While *AstWriter::From(FlowPtr<While> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::For *AstWriter::From(FlowPtr<For> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Foreach *AstWriter::From(FlowPtr<Foreach> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Break *AstWriter::From(FlowPtr<Break> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Continue *AstWriter::From(FlowPtr<Continue> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Return *AstWriter::From(FlowPtr<Return> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::ReturnIf *AstWriter::From(FlowPtr<ReturnIf> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Case *AstWriter::From(FlowPtr<Case> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Switch *AstWriter::From(FlowPtr<Switch> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Typedef *AstWriter::From(FlowPtr<Typedef> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
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
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Export *AstWriter::From(FlowPtr<Export> in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

////////////////////////////////////////////////////////////////////////////////

#define SEND(__object, __node_name)                        \
  {                                                        \
    auto object = From(n);                                 \
    object->CheckInitialized();                            \
    Root root;                                             \
    root.unsafe_arena_set_allocated_##__node_name(object); \
    root.CheckInitialized();                               \
    if (!root.SerializeToOstream(&m_os)) [[unlikely]] {    \
      qcore_panic("Failed to serialize protobuf message"); \
    }                                                      \
  }

void AstWriter::Visit(FlowPtr<Base> n) { SEND(From(n), base); }
void AstWriter::Visit(FlowPtr<ExprStmt> n) { SEND(From(n), expr); }
void AstWriter::Visit(FlowPtr<StmtExpr> n) { SEND(From(n), stmt); }
void AstWriter::Visit(FlowPtr<TypeExpr> n) { SEND(From(n), type); }
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
void AstWriter::Visit(FlowPtr<UnaryExpr> n) { SEND(From(n), unary); }
void AstWriter::Visit(FlowPtr<BinExpr> n) { SEND(From(n), binary); }
void AstWriter::Visit(FlowPtr<PostUnary> n) { SEND(From(n), post_unary); }
void AstWriter::Visit(FlowPtr<TernaryExpr> n) { SEND(From(n), ternary); }
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
