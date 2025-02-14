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

#ifndef __NITRATE_AST_WRITER_H__
#define __NITRATE_AST_WRITER_H__

#include <functional>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-lexer/Token.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <ostream>

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

namespace ncc::parse {
  using namespace nitrate::parser;  // NOLINT

  using WriterSourceProvider = std::optional<std::reference_wrapper<lex::IScanner>>;

  class NCC_EXPORT AstWriter : public ASTVisitor {
    google::protobuf::Arena *m_arena;
    std::ostream &m_os;
    WriterSourceProvider m_rd;
    bool m_plaintext_mode;

    void SetTypeMetadata(auto *message, const FlowPtr<Type> &in);
    SyntaxTree::SourceLocationRange *FromSource(const FlowPtr<Base> &in);
    SyntaxTree::Expr *From(const FlowPtr<Expr> &in);
    SyntaxTree::Stmt *From(const FlowPtr<Stmt> &in);
    SyntaxTree::Type *From(const FlowPtr<Type> &in);
    SyntaxTree::Base *From(const FlowPtr<Base> &in);
    SyntaxTree::ExprStmt *From(const FlowPtr<ExprStmt> &in);
    SyntaxTree::LambdaExpr *From(const FlowPtr<LambdaExpr> &in);
    SyntaxTree::NamedTy *From(const FlowPtr<NamedTy> &in);
    SyntaxTree::InferTy *From(const FlowPtr<InferTy> &in);
    SyntaxTree::TemplateType *From(const FlowPtr<TemplateType> &in);
    SyntaxTree::U1 *From(const FlowPtr<U1> &in);
    SyntaxTree::U8 *From(const FlowPtr<U8> &in);
    SyntaxTree::U16 *From(const FlowPtr<U16> &in);
    SyntaxTree::U32 *From(const FlowPtr<U32> &in);
    SyntaxTree::U64 *From(const FlowPtr<U64> &in);
    SyntaxTree::U128 *From(const FlowPtr<U128> &in);
    SyntaxTree::I8 *From(const FlowPtr<I8> &in);
    SyntaxTree::I16 *From(const FlowPtr<I16> &in);
    SyntaxTree::I32 *From(const FlowPtr<I32> &in);
    SyntaxTree::I64 *From(const FlowPtr<I64> &in);
    SyntaxTree::I128 *From(const FlowPtr<I128> &in);
    SyntaxTree::F16 *From(const FlowPtr<F16> &in);
    SyntaxTree::F32 *From(const FlowPtr<F32> &in);
    SyntaxTree::F64 *From(const FlowPtr<F64> &in);
    SyntaxTree::F128 *From(const FlowPtr<F128> &in);
    SyntaxTree::VoidTy *From(const FlowPtr<VoidTy> &in);
    SyntaxTree::PtrTy *From(const FlowPtr<PtrTy> &in);
    SyntaxTree::OpaqueTy *From(const FlowPtr<OpaqueTy> &in);
    SyntaxTree::TupleTy *From(const FlowPtr<TupleTy> &in);
    SyntaxTree::ArrayTy *From(const FlowPtr<ArrayTy> &in);
    SyntaxTree::RefTy *From(const FlowPtr<RefTy> &in);
    SyntaxTree::FuncTy *From(const FlowPtr<FuncTy> &in);
    SyntaxTree::Unary *From(const FlowPtr<Unary> &in);
    SyntaxTree::Binary *From(const FlowPtr<Binary> &in);
    SyntaxTree::PostUnary *From(const FlowPtr<PostUnary> &in);
    SyntaxTree::Ternary *From(const FlowPtr<Ternary> &in);
    SyntaxTree::Integer *From(const FlowPtr<Integer> &in);
    SyntaxTree::Float *From(const FlowPtr<Float> &in);
    SyntaxTree::Boolean *From(const FlowPtr<Boolean> &in);
    SyntaxTree::String *From(const FlowPtr<String> &in);
    SyntaxTree::Character *From(const FlowPtr<Character> &in);
    SyntaxTree::Null *From(const FlowPtr<Null> &in);
    SyntaxTree::Undefined *From(const FlowPtr<Undefined> &in);
    SyntaxTree::Call *From(const FlowPtr<Call> &in);
    SyntaxTree::TemplateCall *From(const FlowPtr<TemplateCall> &in);
    SyntaxTree::List *From(const FlowPtr<List> &in);
    SyntaxTree::Assoc *From(const FlowPtr<Assoc> &in);
    SyntaxTree::Index *From(const FlowPtr<Index> &in);
    SyntaxTree::Slice *From(const FlowPtr<Slice> &in);
    SyntaxTree::FString *From(const FlowPtr<FString> &in);
    SyntaxTree::Identifier *From(const FlowPtr<Identifier> &in);
    SyntaxTree::Sequence *From(const FlowPtr<Sequence> &in);
    SyntaxTree::Block *From(const FlowPtr<Block> &in);
    SyntaxTree::Variable *From(const FlowPtr<Variable> &in);
    SyntaxTree::Assembly *From(const FlowPtr<Assembly> &in);
    SyntaxTree::If *From(const FlowPtr<If> &in);
    SyntaxTree::While *From(const FlowPtr<While> &in);
    SyntaxTree::For *From(const FlowPtr<For> &in);
    SyntaxTree::Foreach *From(const FlowPtr<Foreach> &in);
    SyntaxTree::Break *From(const FlowPtr<Break> &in);
    SyntaxTree::Continue *From(const FlowPtr<Continue> &in);
    SyntaxTree::Return *From(const FlowPtr<Return> &in);
    SyntaxTree::ReturnIf *From(const FlowPtr<ReturnIf> &in);
    SyntaxTree::Case *From(const FlowPtr<Case> &in);
    SyntaxTree::Switch *From(const FlowPtr<Switch> &in);
    SyntaxTree::Typedef *From(const FlowPtr<Typedef> &in);
    SyntaxTree::Function *From(const FlowPtr<Function> &in);
    SyntaxTree::Struct *From(const FlowPtr<Struct> &in);
    SyntaxTree::Enum *From(const FlowPtr<Enum> &in);
    SyntaxTree::Scope *From(const FlowPtr<Scope> &in);
    SyntaxTree::Export *From(const FlowPtr<Export> &in);

  protected:
    void Visit(FlowPtr<Base> n) override;
    void Visit(FlowPtr<ExprStmt> n) override;
    void Visit(FlowPtr<LambdaExpr> n) override;
    void Visit(FlowPtr<NamedTy> n) override;
    void Visit(FlowPtr<InferTy> n) override;
    void Visit(FlowPtr<TemplateType> n) override;
    void Visit(FlowPtr<U1> n) override;
    void Visit(FlowPtr<U8> n) override;
    void Visit(FlowPtr<U16> n) override;
    void Visit(FlowPtr<U32> n) override;
    void Visit(FlowPtr<U64> n) override;
    void Visit(FlowPtr<U128> n) override;
    void Visit(FlowPtr<I8> n) override;
    void Visit(FlowPtr<I16> n) override;
    void Visit(FlowPtr<I32> n) override;
    void Visit(FlowPtr<I64> n) override;
    void Visit(FlowPtr<I128> n) override;
    void Visit(FlowPtr<F16> n) override;
    void Visit(FlowPtr<F32> n) override;
    void Visit(FlowPtr<F64> n) override;
    void Visit(FlowPtr<F128> n) override;
    void Visit(FlowPtr<VoidTy> n) override;
    void Visit(FlowPtr<PtrTy> n) override;
    void Visit(FlowPtr<OpaqueTy> n) override;
    void Visit(FlowPtr<TupleTy> n) override;
    void Visit(FlowPtr<ArrayTy> n) override;
    void Visit(FlowPtr<RefTy> n) override;
    void Visit(FlowPtr<FuncTy> n) override;
    void Visit(FlowPtr<Unary> n) override;
    void Visit(FlowPtr<Binary> n) override;
    void Visit(FlowPtr<PostUnary> n) override;
    void Visit(FlowPtr<Ternary> n) override;
    void Visit(FlowPtr<Integer> n) override;
    void Visit(FlowPtr<Float> n) override;
    void Visit(FlowPtr<Boolean> n) override;
    void Visit(FlowPtr<String> n) override;
    void Visit(FlowPtr<Character> n) override;
    void Visit(FlowPtr<Null> n) override;
    void Visit(FlowPtr<Undefined> n) override;
    void Visit(FlowPtr<Call> n) override;
    void Visit(FlowPtr<TemplateCall> n) override;
    void Visit(FlowPtr<List> n) override;
    void Visit(FlowPtr<Assoc> n) override;
    void Visit(FlowPtr<Index> n) override;
    void Visit(FlowPtr<Slice> n) override;
    void Visit(FlowPtr<FString> n) override;
    void Visit(FlowPtr<Identifier> n) override;
    void Visit(FlowPtr<Sequence> n) override;
    void Visit(FlowPtr<Block> n) override;
    void Visit(FlowPtr<Variable> n) override;
    void Visit(FlowPtr<Assembly> n) override;
    void Visit(FlowPtr<If> n) override;
    void Visit(FlowPtr<While> n) override;
    void Visit(FlowPtr<For> n) override;
    void Visit(FlowPtr<Foreach> n) override;
    void Visit(FlowPtr<Break> n) override;
    void Visit(FlowPtr<Continue> n) override;
    void Visit(FlowPtr<Return> n) override;
    void Visit(FlowPtr<ReturnIf> n) override;
    void Visit(FlowPtr<Case> n) override;
    void Visit(FlowPtr<Switch> n) override;
    void Visit(FlowPtr<Typedef> n) override;
    void Visit(FlowPtr<Function> n) override;
    void Visit(FlowPtr<Struct> n) override;
    void Visit(FlowPtr<Enum> n) override;
    void Visit(FlowPtr<Scope> n) override;
    void Visit(FlowPtr<Export> n) override;

  public:
    AstWriter(std::ostream &os, WriterSourceProvider rd = std::nullopt, bool plaintext_mode = false);
    ~AstWriter() override;
  };
}  // namespace ncc::parse

#endif
