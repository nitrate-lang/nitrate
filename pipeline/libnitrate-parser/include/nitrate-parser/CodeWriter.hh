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

#ifndef __NITRATE_CODE_WRITER_H__
#define __NITRATE_CODE_WRITER_H__

#include <memory>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Token.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <ostream>

namespace ncc::parse {
  class ICodeWriter : public ASTVisitor {
  public:
    ~ICodeWriter() override = default;
  };

  class NCC_EXPORT CodeWriterFactory final {
  public:
    static std::unique_ptr<ICodeWriter> Create(std::ostream &os, SyntaxVersion ver = SyntaxVersion::NITRATE_1_0);
  };

  namespace detail {
    class NCC_EXPORT CodeWriter_v1_0 final : public ICodeWriter {  // NOLINT(readability-identifier-naming)
      std::ostream &m_os;
      lex::TokenType m_last;
      lex::TokenData m_ldata;
      bool m_did_root;

      void PutKeyword(lex::Keyword kw);
      void PutOperator(lex::Operator op);
      void PutPunctor(lex::Punctor punc);
      void PutIdentifier(std::string_view name);
      void PutInteger(std::string_view num);
      void PutFloat(std::string_view num);
      void PutString(std::string_view str);
      void PutCharacter(std::string_view ch);
      void PutMacroBlock(std::string_view macro);
      void PutMacroCall(std::string_view macro);
      void PutComment(std::string_view note);

      void PutTypeStuff(const FlowPtr<Type> &n);

    protected:
      void Visit(FlowPtr<Base> n) override;
      void Visit(FlowPtr<ExprStmt> n) override;
      void Visit(FlowPtr<LambdaExpr> n) override;
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
      CodeWriter_v1_0(std::ostream &os);
      ~CodeWriter_v1_0() override = default;
    };
  }  // namespace detail

}  // namespace ncc::parse

#endif
