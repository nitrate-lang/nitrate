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
  class BinaryExpression;
  class UnaryExpression;
  class TernaryExpression;
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
  class PostUnaryExpression;
  class StmtExpr;
  class TypeExpr;
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
}  // namespace nitrate::parser::SyntaxTree

namespace ncc::parse {
  using namespace nitrate::parser;  // NOLINT

  using WriterSourceProvider =
      std::optional<std::reference_wrapper<lex::IScanner>>;

  class NCC_EXPORT AstWriter : public ASTVisitor {
    std::ostream &m_os;
    WriterSourceProvider m_rd;

    void Visit(const FlowPtr<Base> &in, SyntaxTree::Base &out);
    void Visit(const FlowPtr<ExprStmt> &in, SyntaxTree::ExprStmt &out);
    void Visit(const FlowPtr<StmtExpr> &in, SyntaxTree::StmtExpr &out);
    void Visit(const FlowPtr<TypeExpr> &in, SyntaxTree::TypeExpr &out);
    void Visit(const FlowPtr<NamedTy> &in, SyntaxTree::NamedTy &out);
    void Visit(const FlowPtr<InferTy> &in, SyntaxTree::InferTy &out);
    void Visit(const FlowPtr<TemplateType> &in, SyntaxTree::TemplateType &out);
    void Visit(const FlowPtr<U1> &in, SyntaxTree::U1 &out);
    void Visit(const FlowPtr<U8> &in, SyntaxTree::U8 &out);
    void Visit(const FlowPtr<U16> &in, SyntaxTree::U16 &out);
    void Visit(const FlowPtr<U32> &in, SyntaxTree::U32 &out);
    void Visit(const FlowPtr<U64> &in, SyntaxTree::U64 &out);
    void Visit(const FlowPtr<U128> &in, SyntaxTree::U128 &out);
    void Visit(const FlowPtr<I8> &in, SyntaxTree::I8 &out);
    void Visit(const FlowPtr<I16> &in, SyntaxTree::I16 &out);
    void Visit(const FlowPtr<I32> &in, SyntaxTree::I32 &out);
    void Visit(const FlowPtr<I64> &in, SyntaxTree::I64 &out);
    void Visit(const FlowPtr<I128> &in, SyntaxTree::I128 &out);
    void Visit(const FlowPtr<F16> &in, SyntaxTree::F16 &out);
    void Visit(const FlowPtr<F32> &in, SyntaxTree::F32 &out);
    void Visit(const FlowPtr<F64> &in, SyntaxTree::F64 &out);
    void Visit(const FlowPtr<F128> &in, SyntaxTree::F128 &out);
    void Visit(const FlowPtr<VoidTy> &in, SyntaxTree::VoidTy &out);
    void Visit(const FlowPtr<PtrTy> &in, SyntaxTree::PtrTy &out);
    void Visit(const FlowPtr<OpaqueTy> &in, SyntaxTree::OpaqueTy &out);
    void Visit(const FlowPtr<TupleTy> &in, SyntaxTree::TupleTy &out);
    void Visit(const FlowPtr<ArrayTy> &in, SyntaxTree::ArrayTy &out);
    void Visit(const FlowPtr<RefTy> &in, SyntaxTree::RefTy &out);
    void Visit(const FlowPtr<FuncTy> &in, SyntaxTree::FuncTy &out);
    void Visit(const FlowPtr<UnaryExpression> &in,
               SyntaxTree::UnaryExpression &out);
    void Visit(const FlowPtr<BinaryExpression> &in,
               SyntaxTree::BinaryExpression &out);
    void Visit(const FlowPtr<PostUnaryExpression> &in,
               SyntaxTree::PostUnaryExpression &out);
    void Visit(const FlowPtr<TernaryExpression> &in,
               SyntaxTree::TernaryExpression &out);
    void Visit(const FlowPtr<Integer> &in, SyntaxTree::Integer &out);
    void Visit(const FlowPtr<Float> &in, SyntaxTree::Float &out);
    void Visit(const FlowPtr<Boolean> &in, SyntaxTree::Boolean &out);
    void Visit(const FlowPtr<String> &in, SyntaxTree::String &out);
    void Visit(const FlowPtr<Character> &in, SyntaxTree::Character &out);
    void Visit(const FlowPtr<Null> &in, SyntaxTree::Null &out);
    void Visit(const FlowPtr<Undefined> &in, SyntaxTree::Undefined &out);
    void Visit(const FlowPtr<Call> &in, SyntaxTree::Call &out);
    void Visit(const FlowPtr<TemplateCall> &in, SyntaxTree::TemplateCall &out);
    void Visit(const FlowPtr<List> &in, SyntaxTree::List &out);
    void Visit(const FlowPtr<Assoc> &in, SyntaxTree::Assoc &out);
    void Visit(const FlowPtr<Index> &in, SyntaxTree::Index &out);
    void Visit(const FlowPtr<Slice> &in, SyntaxTree::Slice &out);
    void Visit(const FlowPtr<FString> &in, SyntaxTree::FString &out);
    void Visit(const FlowPtr<Identifier> &in, SyntaxTree::Identifier &out);
    void Visit(const FlowPtr<Sequence> &in, SyntaxTree::Sequence &out);
    void Visit(const FlowPtr<Block> &in, SyntaxTree::Block &out);
    void Visit(const FlowPtr<Variable> &in, SyntaxTree::Variable &out);
    void Visit(const FlowPtr<Assembly> &in, SyntaxTree::Assembly &out);
    void Visit(const FlowPtr<If> &in, SyntaxTree::If &out);
    void Visit(const FlowPtr<While> &in, SyntaxTree::While &out);
    void Visit(const FlowPtr<For> &in, SyntaxTree::For &out);
    void Visit(const FlowPtr<Foreach> &in, SyntaxTree::Foreach &out);
    void Visit(const FlowPtr<Break> &in, SyntaxTree::Break &out);
    void Visit(const FlowPtr<Continue> &in, SyntaxTree::Continue &out);
    void Visit(const FlowPtr<Return> &in, SyntaxTree::Return &out);
    void Visit(const FlowPtr<ReturnIf> &in, SyntaxTree::ReturnIf &out);
    void Visit(const FlowPtr<Case> &in, SyntaxTree::Case &out);
    void Visit(const FlowPtr<Switch> &in, SyntaxTree::Switch &out);
    void Visit(const FlowPtr<Typedef> &in, SyntaxTree::Typedef &out);
    void Visit(const FlowPtr<Function> &in, SyntaxTree::Function &out);
    void Visit(const FlowPtr<Struct> &in, SyntaxTree::Struct &out);
    void Visit(const FlowPtr<Enum> &in, SyntaxTree::Enum &out);
    void Visit(const FlowPtr<Scope> &in, SyntaxTree::Scope &out);
    void Visit(const FlowPtr<Export> &in, SyntaxTree::Export &out);

  protected:
    void Visit(FlowPtr<Base> n) override;
    void Visit(FlowPtr<ExprStmt> n) override;
    void Visit(FlowPtr<StmtExpr> n) override;
    void Visit(FlowPtr<TypeExpr> n) override;
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
    void Visit(FlowPtr<UnaryExpression> n) override;
    void Visit(FlowPtr<BinaryExpression> n) override;
    void Visit(FlowPtr<PostUnaryExpression> n) override;
    void Visit(FlowPtr<TernaryExpression> n) override;
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
    AstWriter(std::ostream &os, WriterSourceProvider rd = std::nullopt)
        : m_os(os), m_rd(rd) {}
    ~AstWriter() override;
  };
}  // namespace ncc::parse

#endif
