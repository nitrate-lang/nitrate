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

#ifndef __NITRATE_AST_PIMPL_H__
#define __NITRATE_AST_PIMPL_H__

#include <boost/shared_ptr.hpp>
#include <core/SyntaxDiagnostics.hh>
#include <memory>
#include <memory_resource>
#include <nitrate-core/IEnvironment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTFactory.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>
#include <nitrate-parser/Context.hh>
#include <set>

namespace ncc::parse {
  using namespace ec;

  class GeneralParser::PImpl final {
    friend class GeneralParser;

    std::shared_ptr<IEnvironment> m_env;
    std::pmr::memory_resource &m_pool;
    ASTFactory m_fac;
    lex::IScanner &m_rd;
    bool m_failed = false;

    lex::Token Next() { return m_rd.Next(); }
    lex::Token Peek() { return m_rd.Peek(); }
    lex::Token Current() { return m_rd.Current(); }

    template <auto tok>
    auto NextIf(string value) -> std::optional<ncc::lex::Token> {
      static_assert(std::is_same_v<decltype(tok), ncc::lex::TokenType>);
      auto t = m_rd.Peek();
      if (t.Is<tok>(value)) {
        m_rd.Next();
        return t;
      }

      return std::nullopt;
    }

    template <auto tok>
    auto NextIf() -> std::optional<ncc::lex::Token> {
      auto t = m_rd.Peek();
      if constexpr (std::is_same_v<decltype(tok), ncc::lex::TokenType>) {
        if (t.Is(tok)) {
          m_rd.Next();
          return t;
        }

        return std::nullopt;
      } else {
        if (t.Is<tok>()) {
          m_rd.Next();
          return t;
        }

        return std::nullopt;
      }
    }

    static inline auto BindComments(auto node, auto comments) {
      std::vector<string> comments_strs(comments.size());
      std::transform(comments.begin(), comments.end(), comments_strs.begin(),
                     [](const auto &c) { return c.GetString(); });
      node->SetComments(std::move(comments_strs));
      return node;
    }

    /****************************************************************************
     * @brief
     *  Primary language constructs
     ****************************************************************************/

    auto RecurseExport(Vis vis) -> FlowPtr<Expr>;
    auto RecurseVariable(VariableType type) -> std::vector<FlowPtr<Expr>>;
    auto RecurseEnum() -> FlowPtr<Expr>;
    auto RecurseStruct(CompositeType type) -> FlowPtr<Expr>;
    auto RecurseScope() -> FlowPtr<Expr>;
    auto RecurseFunction(bool parse_declaration_only) -> FlowPtr<Expr>;
    auto RecurseType() -> FlowPtr<Type>;
    auto RecurseTypedef() -> FlowPtr<Expr>;
    auto RecurseReturn() -> FlowPtr<Expr>;
    auto RecurseReturnIf() -> FlowPtr<Expr>;
    auto RecurseIf() -> FlowPtr<Expr>;
    auto RecurseWhile() -> FlowPtr<Expr>;
    auto RecurseFor() -> FlowPtr<Expr>;
    auto RecurseForeach() -> FlowPtr<Expr>;
    auto RecurseSwitch() -> FlowPtr<Expr>;
    auto RecurseAssembly() -> FlowPtr<Expr>;
    auto RecurseTry() -> FlowPtr<Expr>;
    auto RecurseThrow() -> FlowPtr<Expr>;
    auto RecurseAwait() -> FlowPtr<Expr>;
    auto RecurseBlock(bool expect_braces, bool single_stmt, BlockMode safety) -> FlowPtr<Expr>;
    auto RecurseExpr(const std::set<lex::Token> &terminators) -> FlowPtr<Expr>;
    auto RecurseExprPrimary(bool is_type) -> NullableFlowPtr<Expr>;
    auto RecurseExprKeyword(lex::Keyword key) -> NullableFlowPtr<Expr>;
    auto RecurseExprPunctor(lex::Punctor punc) -> NullableFlowPtr<Expr>;
    auto RecurseExprTypeSuffix(FlowPtr<Expr> base) -> FlowPtr<Expr>;

    /****************************************************************************
     * @brief
     *  Helper functions
     ****************************************************************************/

    auto RecurseName() -> string;

    auto RecurseEnumType() -> NullableFlowPtr<Type>;
    auto RecurseEnumItem() -> std::optional<std::pair<string, NullableFlowPtr<Expr>>>;
    auto RecurseEnumItems() -> std::optional<std::vector<std::pair<string, NullableFlowPtr<Expr>>>>;

    auto RecurseAbiName() -> string;
    auto RecurseExportAttributes() -> std::optional<std::vector<FlowPtr<Expr>>>;
    auto RecurseExportBody() -> FlowPtr<Expr>;

    auto RecurseCallArguments(const std::set<lex::Token> &terminators, bool type_by_default) -> std::vector<CallArg>;
    auto RecurseFstring() -> FlowPtr<Expr>;

    auto RecurseForInitExpr() -> NullableFlowPtr<Expr>;
    auto RecurseForCondition() -> NullableFlowPtr<Expr>;
    auto RecurseForStepExpr(bool has_paren) -> NullableFlowPtr<Expr>;

    auto RecurseForeachNames() -> std::optional<std::pair<string, string>>;
    auto RecurseForeachExpr(bool has_paren) -> FlowPtr<Expr>;
    auto RecurseForeachBody() -> FlowPtr<Expr>;

    auto RecurseFunctionParameterType() -> FlowPtr<Type>;
    auto RecurseFunctionParameterValue() -> NullableFlowPtr<Expr>;
    auto RecurseFunctionParameter() -> std::optional<FuncParam>;
    auto RecurseTemplateParameters() -> std::optional<std::vector<TemplateParameter>>;
    auto RecurseFunctionParameters() -> std::pair<std::vector<ASTFactory::FactoryFunctionParameter>, bool>;
    auto RecurseFunctionBody(bool parse_declaration_only) -> NullableFlowPtr<Expr>;
    auto RecurseFunctionReturnType() -> FlowPtr<Type>;
    static auto GetPuritySpecifier(lex::Token start_pos, bool is_thread_safe, bool is_pure, bool is_impure,
                                   bool is_quasi, bool is_retro) -> Purity;
    auto RecurseFunctionCapture() -> std::optional<std::pair<string, bool>>;
    auto RecurseFunctionAmbigouis()
        -> std::tuple<std::vector<FlowPtr<Expr>>, std::vector<std::pair<string, bool>>, Purity, string>;

    auto RecurseIfElse() -> NullableFlowPtr<Expr>;

    auto RecurseScopeDeps() -> std::optional<std::vector<string>>;
    auto RecurseScopeBlock() -> FlowPtr<Expr>;

    struct StructContent {
      std::vector<StructField> m_fields;
      std::vector<StructFunction> m_methods;
      std::vector<StructFunction> m_static_methods;
    };
    auto RecurseStructAttributes() -> std::vector<FlowPtr<Expr>>;
    auto RecurseStructTerms() -> std::vector<string>;
    auto RecurseStructFieldDefaultValue() -> NullableFlowPtr<Expr>;
    void RecurseStructField(Vis vis, bool is_static, std::vector<StructField> &fields);
    void RecurseStructMethodOrField(StructContent &body);
    auto RecurseStructBody() -> StructContent;

    auto RecurseSwitchCaseBody() -> FlowPtr<Expr>;
    auto RecurseSwitchCase() -> std::pair<FlowPtr<Expr>, bool>;
    auto RecurseSwitchBody() -> std::optional<std::pair<std::vector<FlowPtr<Case>>, NullableFlowPtr<Expr>>>;

    auto RecurseTypeRangeStart() -> NullableFlowPtr<Expr>;
    auto RecurseTypeRangeEnd() -> NullableFlowPtr<Expr>;
    auto RecurseTypeTemplateArguments() -> std::optional<std::vector<CallArg>>;
    auto RecurseTypeSuffix(FlowPtr<Type> base) -> FlowPtr<Type>;
    auto RecurseFunctionType() -> FlowPtr<Type>;
    auto RecurseOpaqueType() -> FlowPtr<Type>;
    auto RecurseTypeByKeyword(lex::Keyword key) -> FlowPtr<Type>;
    auto RecurseTypeByOperator(lex::Operator op) -> FlowPtr<Type>;
    auto RecurseArrayOrVector() -> FlowPtr<Type>;
    auto RecurseSetType() -> FlowPtr<Type>;
    auto RecurseTupleType() -> FlowPtr<Type>;
    auto RecurseTypeByPunctuation(lex::Punctor punc) -> FlowPtr<Type>;
    auto RecurseTypeByName(string name) -> FlowPtr<Type>;

    auto RecurseVariableAttributes() -> std::optional<std::vector<FlowPtr<Expr>>>;
    auto RecurseVariableType() -> FlowPtr<Type>;
    auto RecurseVariableValue() -> NullableFlowPtr<Expr>;
    auto RecurseVariableInstance(VariableType decl_type) -> NullableFlowPtr<Expr>;

    auto RecurseWhileCond() -> FlowPtr<Expr>;

  public:
    PImpl(lex::IScanner &lexer, std::shared_ptr<IEnvironment> env, std::pmr::memory_resource &pool)
        : m_env(std::move(env)), m_pool(pool), m_fac(m_pool), m_rd(lexer) {}
    ~PImpl() = default;
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_PIMPL_H__
