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

#ifndef __NITRATE_AST_SERIALIZER_H__
#define __NITRATE_AST_SERIALIZER_H__

#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/ScannerFwd.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <nitrate-parser/ProtobufFwd.hh>

namespace ncc::parse {
  using namespace nitrate::parser;

  class NCC_EXPORT ASTWriter : public ASTVisitor {
    class PImpl;
    std::unique_ptr<PImpl> m_impl;

    void SetTypeMetadata(auto *message, const FlowPtr<Type> &in);
    SyntaxTree::SourceLocationRange *FromSource(FlowPtr<Expr> in);

    SyntaxTree::Expr *From(FlowPtr<Expr> in);
    SyntaxTree::Type *From(FlowPtr<Type> in);
    SyntaxTree::NamedTy *From(FlowPtr<NamedTy> in);
    SyntaxTree::InferTy *From(FlowPtr<InferTy> in);
    SyntaxTree::TemplateType *From(FlowPtr<TemplateType> in);
    SyntaxTree::PtrTy *From(FlowPtr<PtrTy> in);
    SyntaxTree::OpaqueTy *From(FlowPtr<OpaqueTy> in);
    SyntaxTree::TupleTy *From(FlowPtr<TupleTy> in);
    SyntaxTree::ArrayTy *From(FlowPtr<ArrayTy> in);
    SyntaxTree::RefTy *From(FlowPtr<RefTy> in);
    SyntaxTree::FuncTy *From(FlowPtr<FuncTy> in);
    SyntaxTree::Unary *From(FlowPtr<Unary> in);
    SyntaxTree::Binary *From(FlowPtr<Binary> in);
    SyntaxTree::Integer *From(FlowPtr<Integer> in);
    SyntaxTree::Float *From(FlowPtr<Float> in);
    SyntaxTree::Boolean *From(FlowPtr<Boolean> in);
    SyntaxTree::String *From(FlowPtr<String> in);
    SyntaxTree::Character *From(FlowPtr<Character> in);
    SyntaxTree::Call *From(FlowPtr<Call> in);
    SyntaxTree::TemplateCall *From(FlowPtr<TemplateCall> in);
    SyntaxTree::Import *From(FlowPtr<Import> in);
    SyntaxTree::List *From(FlowPtr<List> in);
    SyntaxTree::Assoc *From(FlowPtr<Assoc> in);
    SyntaxTree::Index *From(FlowPtr<Index> in);
    SyntaxTree::Slice *From(FlowPtr<Slice> in);
    SyntaxTree::FString *From(FlowPtr<FString> in);
    SyntaxTree::Identifier *From(FlowPtr<Identifier> in);
    SyntaxTree::Block *From(FlowPtr<Block> in);
    SyntaxTree::Variable *From(FlowPtr<Variable> in);
    SyntaxTree::Assembly *From(FlowPtr<Assembly> in);
    SyntaxTree::If *From(FlowPtr<If> in);
    SyntaxTree::While *From(FlowPtr<While> in);
    SyntaxTree::For *From(FlowPtr<For> in);
    SyntaxTree::Foreach *From(FlowPtr<Foreach> in);
    SyntaxTree::Break *From(FlowPtr<Break> in);
    SyntaxTree::Continue *From(FlowPtr<Continue> in);
    SyntaxTree::Return *From(FlowPtr<Return> in);
    SyntaxTree::Case *From(FlowPtr<Case> in);
    SyntaxTree::Switch *From(FlowPtr<Switch> in);
    SyntaxTree::Typedef *From(FlowPtr<Typedef> in);
    SyntaxTree::Function *From(FlowPtr<Function> in);
    SyntaxTree::Struct *From(FlowPtr<Struct> in);
    SyntaxTree::Enum *From(FlowPtr<Enum> in);
    SyntaxTree::Scope *From(FlowPtr<Scope> in);
    SyntaxTree::Export *From(FlowPtr<Export> in);

  protected:
    void Visit(FlowPtr<NamedTy> n) override;
    void Visit(FlowPtr<InferTy> n) override;
    void Visit(FlowPtr<TemplateType> n) override;
    void Visit(FlowPtr<PtrTy> n) override;
    void Visit(FlowPtr<OpaqueTy> n) override;
    void Visit(FlowPtr<TupleTy> n) override;
    void Visit(FlowPtr<ArrayTy> n) override;
    void Visit(FlowPtr<RefTy> n) override;
    void Visit(FlowPtr<FuncTy> n) override;
    void Visit(FlowPtr<Unary> n) override;
    void Visit(FlowPtr<Binary> n) override;
    void Visit(FlowPtr<Integer> n) override;
    void Visit(FlowPtr<Float> n) override;
    void Visit(FlowPtr<Boolean> n) override;
    void Visit(FlowPtr<String> n) override;
    void Visit(FlowPtr<Character> n) override;
    void Visit(FlowPtr<Call> n) override;
    void Visit(FlowPtr<TemplateCall> n) override;
    void Visit(FlowPtr<Import> n) override;
    void Visit(FlowPtr<List> n) override;
    void Visit(FlowPtr<Assoc> n) override;
    void Visit(FlowPtr<Index> n) override;
    void Visit(FlowPtr<Slice> n) override;
    void Visit(FlowPtr<FString> n) override;
    void Visit(FlowPtr<Identifier> n) override;
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
    void Visit(FlowPtr<Case> n) override;
    void Visit(FlowPtr<Switch> n) override;
    void Visit(FlowPtr<Typedef> n) override;
    void Visit(FlowPtr<Function> n) override;
    void Visit(FlowPtr<Struct> n) override;
    void Visit(FlowPtr<Enum> n) override;
    void Visit(FlowPtr<Scope> n) override;
    void Visit(FlowPtr<Export> n) override;

  public:
    enum class Format { JSON, PROTO };

    ASTWriter(std::ostream &os, Format format, OptionalSourceProvider rd = std::nullopt);
    ~ASTWriter() override;
  };
}  // namespace ncc::parse

#endif
