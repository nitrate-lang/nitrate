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
    size_t m_recursion_depth = 0;

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

    [[nodiscard]] auto RecurseExport(Vis vis) -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseVariable(VariableType type) -> std::vector<FlowPtr<Expr>>;
    [[nodiscard]] auto RecurseEnum() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseStruct(CompositeType type) -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseScope() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseFunction(bool parse_declaration_only) -> FlowPtr<Function>;
    [[nodiscard]] auto RecurseType() -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseTypedef() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseReturn() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseReturnIf() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseIf() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseWhile() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseFor() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseForeach() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseSwitch() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseAssembly() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseTry() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseThrow() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseAwait() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseImport() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseBlock(bool expect_braces, bool single_stmt, BlockMode safety) -> FlowPtr<Block>;
    [[nodiscard]] auto RecurseExpr(const std::set<lex::Token> &terminators) -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseExprPrimary(bool is_type) -> NullableFlowPtr<Expr>;
    [[nodiscard]] auto RecurseExprKeyword(lex::Keyword key) -> NullableFlowPtr<Expr>;
    [[nodiscard]] auto RecurseExprPunctor(lex::Punctor punc) -> NullableFlowPtr<Expr>;
    [[nodiscard]] auto RecurseExprTypeSuffix(FlowPtr<Expr> base) -> FlowPtr<Expr>;
    auto RecurseEscapeBlock() -> void;
    [[nodiscard]] auto RecurseUnitAssert() -> FlowPtr<Expr>;

    /****************************************************************************
     * @brief
     *  Helper functions
     ****************************************************************************/

    struct StructContent {
      std::vector<StructField> m_fields;
      std::vector<StructFunction> m_methods;
      std::vector<StructFunction> m_static_methods;
    };

    [[nodiscard]] auto RecurseName() -> string;
    [[nodiscard]] auto RecurseEnumField() -> std::pair<string, NullableFlowPtr<Expr>>;
    [[nodiscard]] auto RecurseEnumFields() -> std::vector<std::pair<string, NullableFlowPtr<Expr>>>;
    [[nodiscard]] auto RecurseAbiName() -> string;
    [[nodiscard]] auto RecurseExportAttributes() -> std::vector<FlowPtr<Expr>>;
    [[nodiscard]] auto RecurseExportBody() -> FlowPtr<Block>;
    [[nodiscard]] auto RecurseCallArguments(const std::set<lex::Token> &terminators,
                                            bool type_by_default) -> std::vector<CallArg>;
    [[nodiscard]] auto ParseFStringExpression(std::string_view source) -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseFstring() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseForInitExpr() -> NullableFlowPtr<Expr>;
    [[nodiscard]] auto RecurseForCondition() -> NullableFlowPtr<Expr>;
    [[nodiscard]] auto RecurseForStepExpr(bool has_paren) -> NullableFlowPtr<Expr>;
    [[nodiscard]] auto RecurseForeachNames() -> std::optional<std::pair<string, string>>;
    [[nodiscard]] auto RecurseForeachExpr(bool has_paren) -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseForeachBody() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseFunctionParameterType() -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseFunctionParameterValue() -> NullableFlowPtr<Expr>;
    [[nodiscard]] auto RecurseFunctionParameter() -> std::optional<FuncParam>;
    [[nodiscard]] auto RecurseTemplateParameters() -> std::optional<std::vector<TemplateParameter>>;
    [[nodiscard]] auto RecurseFunctionParameters()
        -> std::pair<std::vector<ASTFactory::FactoryFunctionParameter>, bool>;
    [[nodiscard]] auto RecurseFunctionBody(bool parse_declaration_only) -> NullableFlowPtr<Expr>;
    [[nodiscard]] auto RecurseFunctionReturnType() -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseFunctionAttributes() -> std::vector<FlowPtr<Expr>>;
    [[nodiscard]] auto RecurseIfElse() -> NullableFlowPtr<Expr>;
    [[nodiscard]] auto RecurseScopeDeps() -> std::vector<string>;
    [[nodiscard]] auto RecurseScopeBlock() -> FlowPtr<Expr>;
    [[nodiscard]] [[nodiscard]] auto RecurseStructAttributes() -> std::vector<FlowPtr<Expr>>;
    [[nodiscard]] auto RecurseStructTerms() -> std::vector<string>;
    [[nodiscard]] auto RecurseStructFieldDefaultValue() -> NullableFlowPtr<Expr>;
    void RecurseStructField(Vis vis, bool is_static, std::vector<StructField> &fields);
    void RecurseStructMethodOrField(StructContent &body);
    [[nodiscard]] auto RecurseStructBody() -> StructContent;
    [[nodiscard]] auto RecurseSwitchCaseBody() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseSwitchCase() -> std::pair<FlowPtr<Expr>, bool>;
    [[nodiscard]] auto RecurseSwitchBody()
        -> std::optional<std::pair<std::vector<FlowPtr<Case>>, NullableFlowPtr<Expr>>>;
    [[nodiscard]] auto RecurseTypeRangeStart() -> NullableFlowPtr<Expr>;
    [[nodiscard]] auto RecurseTypeRangeEnd() -> NullableFlowPtr<Expr>;
    [[nodiscard]] auto RecurseTypeTemplateArguments() -> std::optional<std::vector<CallArg>>;
    [[nodiscard]] auto RecurseTypeSuffix(FlowPtr<Type> base) -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseFunctionType() -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseOpaqueType() -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseTypeByKeyword(lex::Keyword key) -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseTypeByOperator(lex::Operator op) -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseArrayOrVector() -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseSetType() -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseTupleType() -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseTypeByPunctuation(lex::Punctor punc) -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseTypeByName(string name) -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseVariableAttributes() -> std::vector<FlowPtr<Expr>>;
    [[nodiscard]] auto RecurseVariableType() -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseVariableValue() -> NullableFlowPtr<Expr>;
    [[nodiscard]] auto RecurseVariableInstance(VariableType decl_type) -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseWhileCond() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseImportName() -> std::pair<string, ImportMode>;
    [[nodiscard]] auto RecurseImportRegularFile(string import_file, ImportMode import_mode) -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseImportPackage(string import_name, ImportMode import_mode) -> FlowPtr<Expr>;
    void PrepareImportSubgraph(const FlowPtr<Expr> &root);

  public:
    PImpl(lex::IScanner &lexer, std::shared_ptr<IEnvironment> env, std::pmr::memory_resource &pool)
        : m_env(std::move(env)), m_pool(pool), m_fac(m_pool), m_rd(lexer) {}
    ~PImpl() = default;
  };

  void ParserSwapScanner(lex::IScanner *&value);
}  // namespace ncc::parse

#endif  // __NITRATE_AST_PIMPL_H__
