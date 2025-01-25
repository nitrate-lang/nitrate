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
using namespace google::protobuf;
using namespace nitrate::parser::SyntaxTree;

SyntaxTree::Base AstWriter::From(const FlowPtr<Base> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::ExprStmt AstWriter::From(const FlowPtr<ExprStmt> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::StmtExpr AstWriter::From(const FlowPtr<StmtExpr> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::TypeExpr AstWriter::From(const FlowPtr<TypeExpr> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::NamedTy AstWriter::From(const FlowPtr<NamedTy> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::InferTy AstWriter::From(const FlowPtr<InferTy> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::TemplateType AstWriter::From(const FlowPtr<TemplateType> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::U1 AstWriter::From(const FlowPtr<U1> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::U8 AstWriter::From(const FlowPtr<U8> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::U16 AstWriter::From(const FlowPtr<U16> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::U32 AstWriter::From(const FlowPtr<U32> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::U64 AstWriter::From(const FlowPtr<U64> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::U128 AstWriter::From(const FlowPtr<U128> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::I8 AstWriter::From(const FlowPtr<I8> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::I16 AstWriter::From(const FlowPtr<I16> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::I32 AstWriter::From(const FlowPtr<I32> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::I64 AstWriter::From(const FlowPtr<I64> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::I128 AstWriter::From(const FlowPtr<I128> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::F16 AstWriter::From(const FlowPtr<F16> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::F32 AstWriter::From(const FlowPtr<F32> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::F64 AstWriter::From(const FlowPtr<F64> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::F128 AstWriter::From(const FlowPtr<F128> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::VoidTy AstWriter::From(const FlowPtr<VoidTy> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::PtrTy AstWriter::From(const FlowPtr<PtrTy> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::OpaqueTy AstWriter::From(const FlowPtr<OpaqueTy> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::TupleTy AstWriter::From(const FlowPtr<TupleTy> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::ArrayTy AstWriter::From(const FlowPtr<ArrayTy> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::RefTy AstWriter::From(const FlowPtr<RefTy> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::FuncTy AstWriter::From(const FlowPtr<FuncTy> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::UnaryExpr AstWriter::From(const FlowPtr<UnaryExpr> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
  (void)m_os;
}

SyntaxTree::BinExpr AstWriter::From(const FlowPtr<BinExpr> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::PostUnary AstWriter::From(const FlowPtr<PostUnary> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::TernaryExpr AstWriter::From(const FlowPtr<TernaryExpr> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Integer AstWriter::From(const FlowPtr<Integer> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Float AstWriter::From(const FlowPtr<Float> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Boolean AstWriter::From(const FlowPtr<Boolean> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::String AstWriter::From(const FlowPtr<String> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Character AstWriter::From(const FlowPtr<Character> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Null AstWriter::From(const FlowPtr<Null> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Undefined AstWriter::From(const FlowPtr<Undefined> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Call AstWriter::From(const FlowPtr<Call> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::TemplateCall AstWriter::From(const FlowPtr<TemplateCall> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::List AstWriter::From(const FlowPtr<List> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Assoc AstWriter::From(const FlowPtr<Assoc> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Index AstWriter::From(const FlowPtr<Index> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Slice AstWriter::From(const FlowPtr<Slice> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::FString AstWriter::From(const FlowPtr<FString> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Identifier AstWriter::From(const FlowPtr<Identifier> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Sequence AstWriter::From(const FlowPtr<Sequence> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Block AstWriter::From(const FlowPtr<Block> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Variable AstWriter::From(const FlowPtr<Variable> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Assembly AstWriter::From(const FlowPtr<Assembly> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::If AstWriter::From(const FlowPtr<If> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::While AstWriter::From(const FlowPtr<While> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::For AstWriter::From(const FlowPtr<For> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Foreach AstWriter::From(const FlowPtr<Foreach> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Break AstWriter::From(const FlowPtr<Break> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Continue AstWriter::From(const FlowPtr<Continue> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Return AstWriter::From(const FlowPtr<Return> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::ReturnIf AstWriter::From(const FlowPtr<ReturnIf> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Case AstWriter::From(const FlowPtr<Case> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Switch AstWriter::From(const FlowPtr<Switch> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Typedef AstWriter::From(const FlowPtr<Typedef> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Function AstWriter::From(const FlowPtr<Function> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Struct AstWriter::From(const FlowPtr<Struct> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Enum AstWriter::From(const FlowPtr<Enum> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Scope AstWriter::From(const FlowPtr<Scope> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

SyntaxTree::Export AstWriter::From(const FlowPtr<Export> &in) {
  /// TODO: From input to protobuf structure
  qcore_implement();
  (void)in;
}

////////////////////////////////////////////////////////////////////////////////

#define SEND(__object, __node_name)                         \
  {                                                         \
    auto object = From(n);                                  \
    object.CheckInitialized();                              \
    Root root;                                              \
    root.unsafe_arena_set_allocated_##__node_name(&object); \
    root.CheckInitialized();                                \
    if (!root.SerializeToOstream(&m_os)) [[unlikely]] {     \
      qcore_panic("Failed to serialize protobuf message");  \
    }                                                       \
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
