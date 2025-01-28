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

#ifndef __NITRATE_AST_READER_H__
#define __NITRATE_AST_READER_H__

#include <istream>
#include <memory>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/NullableFlowPtr.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <optional>

namespace ncc::parse {
  using ReaderSourceManager =
      std::optional<std::reference_wrapper<lex::IScanner>>;

  class NCC_EXPORT AstReader final {
    template <typename T>
    using Result = NullableFlowPtr<Base>;

    Result<Base> m_root;
    ReaderSourceManager m_rd;
    std::unique_ptr<IMemory> m_mm;

    void UnmarshalLocationLocation(const SyntaxTree::SourceLocationRange &in,
                                   FlowPtr<Base> out);

    void UnmarshalCodeComment(
        const ::google::protobuf::RepeatedPtrField<
            ::nitrate::parser::SyntaxTree::UserComment> &in,
        FlowPtr<Base> out);

    auto Unmarshal(const SyntaxTree::Expr &in) -> Result<Expr>;
    auto Unmarshal(const SyntaxTree::Root &in) -> Result<Base>;
    auto Unmarshal(const SyntaxTree::Stmt &in) -> Result<Stmt>;
    auto Unmarshal(const SyntaxTree::Type &in) -> Result<Type>;
    auto Unmarshal(const SyntaxTree::Base &in) -> Result<Base>;
    auto Unmarshal(const SyntaxTree::ExprStmt &in) -> Result<ExprStmt>;
    auto Unmarshal(const SyntaxTree::StmtExpr &in) -> Result<StmtExpr>;
    auto Unmarshal(const SyntaxTree::TypeExpr &in) -> Result<TypeExpr>;
    auto Unmarshal(const SyntaxTree::NamedTy &in) -> Result<NamedTy>;
    auto Unmarshal(const SyntaxTree::InferTy &in) -> Result<InferTy>;
    auto Unmarshal(const SyntaxTree::TemplateType &in) -> Result<TemplateType>;
    auto Unmarshal(const SyntaxTree::U1 &in) -> Result<U1>;
    auto Unmarshal(const SyntaxTree::U8 &in) -> Result<U8>;
    auto Unmarshal(const SyntaxTree::U16 &in) -> Result<U16>;
    auto Unmarshal(const SyntaxTree::U32 &in) -> Result<U32>;
    auto Unmarshal(const SyntaxTree::U64 &in) -> Result<U64>;
    auto Unmarshal(const SyntaxTree::U128 &in) -> Result<U128>;
    auto Unmarshal(const SyntaxTree::I8 &in) -> Result<I8>;
    auto Unmarshal(const SyntaxTree::I16 &in) -> Result<I16>;
    auto Unmarshal(const SyntaxTree::I32 &in) -> Result<I32>;
    auto Unmarshal(const SyntaxTree::I64 &in) -> Result<I64>;
    auto Unmarshal(const SyntaxTree::I128 &in) -> Result<I128>;
    auto Unmarshal(const SyntaxTree::F16 &in) -> Result<F16>;
    auto Unmarshal(const SyntaxTree::F32 &in) -> Result<F32>;
    auto Unmarshal(const SyntaxTree::F64 &in) -> Result<F64>;
    auto Unmarshal(const SyntaxTree::F128 &in) -> Result<F128>;
    auto Unmarshal(const SyntaxTree::VoidTy &in) -> Result<VoidTy>;
    auto Unmarshal(const SyntaxTree::PtrTy &in) -> Result<PtrTy>;
    auto Unmarshal(const SyntaxTree::OpaqueTy &in) -> Result<OpaqueTy>;
    auto Unmarshal(const SyntaxTree::TupleTy &in) -> Result<TupleTy>;
    auto Unmarshal(const SyntaxTree::ArrayTy &in) -> Result<ArrayTy>;
    auto Unmarshal(const SyntaxTree::RefTy &in) -> Result<RefTy>;
    auto Unmarshal(const SyntaxTree::FuncTy &in) -> Result<FuncTy>;
    auto Unmarshal(const SyntaxTree::Unary &in) -> Result<Unary>;
    auto Unmarshal(const SyntaxTree::Binary &in) -> Result<Binary>;
    auto Unmarshal(const SyntaxTree::PostUnary &in) -> Result<PostUnary>;
    auto Unmarshal(const SyntaxTree::Ternary &in) -> Result<Ternary>;
    auto Unmarshal(const SyntaxTree::Integer &in) -> Result<Integer>;
    auto Unmarshal(const SyntaxTree::Float &in) -> Result<Float>;
    auto Unmarshal(const SyntaxTree::Boolean &in) -> Result<Boolean>;
    auto Unmarshal(const SyntaxTree::String &in) -> Result<String>;
    auto Unmarshal(const SyntaxTree::Character &in) -> Result<Character>;
    auto Unmarshal(const SyntaxTree::Null &in) -> Result<Null>;
    auto Unmarshal(const SyntaxTree::Undefined &in) -> Result<Undefined>;
    auto Unmarshal(const SyntaxTree::Call &in) -> Result<Call>;
    auto Unmarshal(const SyntaxTree::TemplateCall &in) -> Result<TemplateCall>;
    auto Unmarshal(const SyntaxTree::List &in) -> Result<List>;
    auto Unmarshal(const SyntaxTree::Assoc &in) -> Result<Assoc>;
    auto Unmarshal(const SyntaxTree::Index &in) -> Result<Index>;
    auto Unmarshal(const SyntaxTree::Slice &in) -> Result<Slice>;
    auto Unmarshal(const SyntaxTree::FString &in) -> Result<FString>;
    auto Unmarshal(const SyntaxTree::Identifier &in) -> Result<Identifier>;
    auto Unmarshal(const SyntaxTree::Sequence &in) -> Result<Sequence>;
    auto Unmarshal(const SyntaxTree::Block &in) -> Result<Block>;
    auto Unmarshal(const SyntaxTree::Variable &in) -> Result<Variable>;
    auto Unmarshal(const SyntaxTree::Assembly &in) -> Result<Assembly>;
    auto Unmarshal(const SyntaxTree::If &in) -> Result<If>;
    auto Unmarshal(const SyntaxTree::While &in) -> Result<While>;
    auto Unmarshal(const SyntaxTree::For &in) -> Result<For>;
    auto Unmarshal(const SyntaxTree::Foreach &in) -> Result<Foreach>;
    auto Unmarshal(const SyntaxTree::Break &in) -> Result<Break>;
    auto Unmarshal(const SyntaxTree::Continue &in) -> Result<Continue>;
    auto Unmarshal(const SyntaxTree::Return &in) -> Result<Return>;
    auto Unmarshal(const SyntaxTree::ReturnIf &in) -> Result<ReturnIf>;
    auto Unmarshal(const SyntaxTree::Case &in) -> Result<Case>;
    auto Unmarshal(const SyntaxTree::Switch &in) -> Result<Switch>;
    auto Unmarshal(const SyntaxTree::Typedef &in) -> Result<Typedef>;
    auto Unmarshal(const SyntaxTree::Function &in) -> Result<Function>;
    auto Unmarshal(const SyntaxTree::Struct &in) -> Result<Struct>;
    auto Unmarshal(const SyntaxTree::Enum &in) -> Result<Enum>;
    auto Unmarshal(const SyntaxTree::Scope &in) -> Result<Scope>;
    auto Unmarshal(const SyntaxTree::Export &in) -> Result<Export>;

  public:
    AstReader(std::istream &protobuf_data,
              ReaderSourceManager source_manager = std::nullopt);
    AstReader(std::string_view protobuf_data,
              ReaderSourceManager source_manager = std::nullopt);
    ~AstReader() = default;

    auto Get() -> std::optional<ASTRoot>;
  };
}  // namespace ncc::parse

#endif
