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
#include <memory>
#include <nitrate-core/IEnvironment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/Context.hh>
#include <set>

namespace ncc::parse {
  class Parser::PImpl final {
    friend class Parser;

    std::shared_ptr<IEnvironment> m_env;
    std::unique_ptr<IMemory> m_allocator;
    lex::IScanner &m_rd;
    bool m_failed = false;
    std::shared_ptr<void> m_lifetime;

    /****************************************************************************
     * @brief
     *  Primary language constructs
     ****************************************************************************/

    auto RecurseExport(Vis vis) -> FlowPtr<Stmt>;
    auto RecurseVariable(VariableType type) -> std::vector<FlowPtr<Stmt>>;
    auto RecurseEnum() -> FlowPtr<Stmt>;
    auto RecurseStruct(CompositeType type) -> FlowPtr<Stmt>;
    auto RecurseScope() -> FlowPtr<Stmt>;
    auto RecurseFunction(bool parse_declaration_only) -> FlowPtr<Stmt>;
    auto RecurseType() -> FlowPtr<Type>;
    auto RecurseTypedef() -> FlowPtr<Stmt>;
    auto RecurseReturn() -> FlowPtr<Stmt>;
    auto RecurseRetif() -> FlowPtr<Stmt>;
    auto RecurseIf() -> FlowPtr<Stmt>;
    auto RecurseWhile() -> FlowPtr<Stmt>;
    auto RecurseFor() -> FlowPtr<Stmt>;
    auto RecurseForeach() -> FlowPtr<Stmt>;
    auto RecurseSwitch() -> FlowPtr<Stmt>;
    auto RecurseAssembly() -> FlowPtr<Stmt>;
    auto RecurseTry() -> FlowPtr<Stmt>;
    auto RecurseThrow() -> FlowPtr<Stmt>;
    auto RecurseAwait() -> FlowPtr<Stmt>;
    auto RecurseBlock(bool expect_braces, bool single_stmt, SafetyMode safety) -> FlowPtr<Stmt>;
    auto RecurseExpr(const std::set<lex::Token> &terminators) -> FlowPtr<Expr>;
    auto RecurseExprPrimary(bool is_type) -> NullableFlowPtr<Expr>;
    auto RecurseExprKeyword(lex::Keyword key) -> NullableFlowPtr<Expr>;
    auto RecurseExprPunctor(lex::Punctor punc) -> NullableFlowPtr<Expr>;
    auto RecurseExprTypeSuffix(FlowPtr<Expr> base) -> FlowPtr<Expr>;

    /****************************************************************************
     * @brief
     *  Helper functions
     ****************************************************************************/

    auto MockStmt(std::optional<npar_ty_t> expected = std::nullopt) -> FlowPtr<Stmt>;
    auto MockExpr(std::optional<npar_ty_t> expected = std::nullopt) -> FlowPtr<Expr>;
    auto MockType() -> FlowPtr<Type>;

    auto RecurseName() -> string;

    auto RecurseEnumType() -> NullableFlowPtr<Type>;
    auto RecurseEnumItem() -> std::optional<EnumItem>;
    auto RecurseEnumItems() -> std::optional<EnumItems>;

    auto RecurseAbiName() -> string;
    auto RecurseExportAttributes() -> std::optional<ExpressionList>;
    auto RecurseExportBody() -> FlowPtr<Stmt>;

    auto RecurseCallArguments(const std::set<lex::Token> &terminators, bool type_by_default) -> CallArgs;
    auto RecurseFstring() -> FlowPtr<Expr>;

    auto RecurseForInitExpr() -> NullableFlowPtr<Stmt>;
    auto RecurseForCondition() -> NullableFlowPtr<Expr>;
    auto RecurseForStepExpr(bool has_paren) -> NullableFlowPtr<Expr>;
    auto RecurseForBody() -> FlowPtr<Stmt>;

    auto RecurseForeachNames() -> std::optional<std::pair<string, string>>;
    auto RecurseForeachExpr(bool has_paren) -> FlowPtr<Expr>;
    auto RecurseForeachBody() -> FlowPtr<Stmt>;

    auto RecurseFunctionParameterType() -> FlowPtr<Type>;
    auto RecurseFunctionParameterValue() -> NullableFlowPtr<Expr>;
    auto RecurseFunctionParameter() -> std::optional<FuncParam>;
    auto RecurseTemplateParameters() -> std::optional<TemplateParameters>;
    auto RecurseFunctionParameters() -> std::pair<FuncParams, bool>;
    auto RecurseFunctionBody(bool parse_declaration_only) -> NullableFlowPtr<Stmt>;
    auto RecurseFunctionReturnType() -> FlowPtr<Type>;
    static auto GetPuritySpecifier(lex::Token start_pos, bool is_thread_safe, bool is_pure, bool is_impure,
                                   bool is_quasi, bool is_retro) -> Purity;
    auto RecurseFunctionCapture() -> std::optional<std::pair<string, bool>>;
    auto RecurseFunctionAmbigouis() -> std::tuple<ExpressionList, FnCaptures, Purity, string>;

    auto RecurseIfThen() -> FlowPtr<Stmt>;
    auto RecurseIfElse() -> NullableFlowPtr<Stmt>;

    auto RecurseScopeDeps() -> std::optional<ScopeDeps>;
    auto RecurseScopeBlock() -> FlowPtr<Stmt>;

    struct StructContent {
      StructFields m_fields;
      StructMethods m_methods;
      StructStaticMethods m_static_methods;
    };
    auto RecurseStructAttributes() -> ExpressionList;
    auto RecurseStructTerms() -> StructNames;
    auto RecurseStructFieldDefaultValue() -> NullableFlowPtr<Expr>;
    void RecurseStructField(Vis vis, bool is_static, StructFields &fields);
    void RecurseStructMethodOrField(StructContent &body);
    auto RecurseStructBody() -> StructContent;

    auto RecurseSwitchCaseBody() -> FlowPtr<Stmt>;
    auto RecurseSwitchCase() -> std::pair<FlowPtr<Stmt>, bool>;
    auto RecurseSwitchBody() -> std::optional<std::pair<SwitchCases, NullableFlowPtr<Stmt>>>;

    auto RecurseTypeRangeStart() -> NullableFlowPtr<Expr>;
    auto RecurseTypeRangeEnd() -> NullableFlowPtr<Expr>;
    auto RecurseTypeTemplateArguments() -> std::optional<CallArgs>;
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

    auto RecurseVariableAttributes() -> std::optional<ExpressionList>;
    auto RecurseVariableType() -> NullableFlowPtr<Type>;
    auto RecurseVariableValue() -> NullableFlowPtr<Expr>;
    auto RecurseVariableInstance(VariableType decl_type) -> NullableFlowPtr<Stmt>;

    auto RecurseWhileCond() -> FlowPtr<Expr>;
    auto RecurseWhileBody() -> FlowPtr<Stmt>;

  public:
    PImpl(lex::IScanner &lexer, std::shared_ptr<IEnvironment> env, std::shared_ptr<void> lifetime)
        : m_env(std::move(env)),
          m_allocator(std::make_unique<DynamicArena>()),
          m_rd(lexer),
          m_lifetime(std::move(lifetime)) {}
    ~PImpl() = default;
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_PIMPL_H__
