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

#ifndef __NITRATE_AST_PARSER_H__
#define __NITRATE_AST_PARSER_H__

#include <boost/shared_ptr.hpp>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/AST.hh>
#include <set>
#include <sstream>
#include <unordered_set>

namespace ncc::parse {
  class Parser;

  class ASTRoot final {
    FlowPtr<Base> m_base;
    std::shared_ptr<void> m_allocator;
    bool m_failed;

  public:
    ASTRoot(FlowPtr<Base> base, std::shared_ptr<void> allocator, bool failed)
        : m_base(std::move(base)),
          m_allocator(std::move(allocator)),
          m_failed(failed) {}

    FlowPtr<Base> &Get() { return m_base; }
    [[nodiscard]] FlowPtr<Base> Get() const { return m_base; }

    [[nodiscard]] bool Check() const;
  };

  class Parser final {
    std::shared_ptr<ncc::Environment> m_env;
    std::unique_ptr<ncc::IMemory> m_allocator;
    ncc::lex::IScanner &m_rd;
    bool m_failed;
    std::shared_ptr<void> m_lifetime;

    /****************************************************************************
     * @brief
     *  Primary language constructs
     ****************************************************************************/

    FlowPtr<Stmt> RecurseExport(Vis vis);
    std::vector<FlowPtr<Stmt>> RecurseVariable(VarDeclType type);
    FlowPtr<Stmt> RecurseEnum();
    FlowPtr<Stmt> RecurseStruct(CompositeType type);
    FlowPtr<Stmt> RecurseScope();
    FlowPtr<Stmt> RecurseFunction(bool parse_declaration_only);
    FlowPtr<Type> RecurseType();
    FlowPtr<Stmt> RecurseTypedef();
    FlowPtr<Stmt> RecurseReturn();
    FlowPtr<Stmt> RecurseRetif();
    FlowPtr<Stmt> RecurseIf();
    FlowPtr<Stmt> RecurseWhile();
    FlowPtr<Stmt> RecurseFor();
    FlowPtr<Stmt> RecurseForeach();
    FlowPtr<Stmt> RecurseSwitch();
    FlowPtr<Stmt> RecurseInlineAsm();
    FlowPtr<Stmt> RecurseTry();
    FlowPtr<Stmt> RecurseThrow();
    FlowPtr<Stmt> RecurseAwait();
    FlowPtr<Stmt> RecurseBlock(bool expect_braces, bool single_stmt,
                               SafetyMode safety);
    FlowPtr<Expr> RecurseExpr(
        const std::unordered_set<ncc::lex::Token> &terminators);
    NullableFlowPtr<Expr> RecurseExprPrimary(bool is_type);
    NullableFlowPtr<Expr> RecurseExprKeyword(lex::Keyword key);
    NullableFlowPtr<Expr> RecurseExprPunctor(lex::Punctor punc);
    FlowPtr<Expr> RecurseExprTypeSuffix(FlowPtr<Expr> base);

    /****************************************************************************
     * @brief
     *  Helper functions
     ****************************************************************************/

    FlowPtr<Stmt> MockStmt(std::optional<npar_ty_t> expected = std::nullopt);
    FlowPtr<Expr> MockExpr(std::optional<npar_ty_t> expected = std::nullopt);
    FlowPtr<Type> MockType();

    string RecurseEnumName();
    NullableFlowPtr<Type> RecurseEnumType();
    NullableFlowPtr<Expr> RecurseEnumItemValue();
    std::optional<EnumItem> RecurseEnumItem();
    std::optional<EnumDefItems> RecurseEnumItems();

    string RecurseAbiName();
    std::optional<ExpressionList> RecurseExportAttributes();
    FlowPtr<Stmt> RecurseExportBody();

    CallArgs RecurseCallArguments(
        const std::unordered_set<lex::Token> &terminators,
        bool type_by_default);
    FlowPtr<Expr> RecurseFstring();

    NullableFlowPtr<Stmt> RecurseForInitExpr();
    NullableFlowPtr<Expr> RecurseForCondition();
    NullableFlowPtr<Expr> RecurseForStepExpr(bool has_paren);
    FlowPtr<Stmt> RecurseForBody();

    std::optional<std::pair<string, string>> RecurseForeachNames();
    FlowPtr<Expr> RecurseForeachExpr(bool has_paren);
    FlowPtr<Stmt> RecurseForeachBody();

    FlowPtr<Type> RecurseFunctionParameterType();
    NullableFlowPtr<Expr> RecurseFunctionParameterValue();
    std::optional<FuncParam> RecurseFunctionParameter();
    std::optional<TemplateParameters> RecurseTemplateParameters();
    std::pair<FuncParams, bool> RecurseFunctionParameters();
    NullableFlowPtr<Stmt> RecurseFunctionBody(bool parse_declaration_only);
    FlowPtr<Type> RecurseFunctionReturnType();
    Purity GetPuritySpecifier(lex::Token start_pos, bool is_thread_safe,
                              bool is_pure, bool is_impure, bool is_quasi,
                              bool is_retro);
    std::optional<std::pair<string, bool>> RecurseFunctionCapture();
    std::tuple<ExpressionList, FnCaptures, Purity, string>
    RecurseFunctionAmbigouis();

    FlowPtr<Stmt> RecurseIfThen();
    NullableFlowPtr<Stmt> RecurseIfElse();

    string RecurseScopeName();
    std::optional<ScopeDeps> RecurseScopeDeps();
    FlowPtr<Stmt> RecurseScopeBlock();

    struct StructContent {
      StructDefFields m_fields;
      StructDefMethods m_methods;
      StructDefStaticMethods m_static_methods;
    };
    ExpressionList RecurseStructAttributes();
    string RecurseStructName();
    StructDefNames RecurseStructTerms();
    NullableFlowPtr<Expr> RecurseStructFieldDefaultValue();
    void RecurseStructField(Vis vis, bool is_static, StructDefFields &fields);
    void RecurseStructMethodOrField(StructContent &body);
    StructContent RecurseStructBody();

    FlowPtr<Stmt> RecurseSwitchCaseBody();
    std::pair<FlowPtr<Stmt>, bool> RecurseSwitchCase();
    std::optional<std::pair<SwitchCases, NullableFlowPtr<Stmt>>>
    RecurseSwitchBody();

    NullableFlowPtr<Expr> RecurseTypeRangeStart();
    NullableFlowPtr<Expr> RecurseTypeRangeEnd();
    std::optional<CallArgs> RecurseTypeTemplateArguments();
    FlowPtr<Type> RecurseTypeSuffix(FlowPtr<Type> base);
    FlowPtr<Type> RecurseFunctionType();
    FlowPtr<Type> RecurseOpaqueType();
    FlowPtr<Type> RecurseTypeByKeyword(lex::Keyword key);
    FlowPtr<Type> RecurseTypeByOperator(lex::Operator op);
    FlowPtr<Type> RecurseArrayOrVector();
    FlowPtr<Type> RecurseSetType();
    FlowPtr<Type> RecurseTupleType();
    FlowPtr<Type> RecurseTypeByPunctuation(lex::Punctor punc);
    FlowPtr<Type> RecurseTypeByName(string name);

    std::optional<ExpressionList> RecurseVariableAttributes();
    NullableFlowPtr<Type> RecurseVariableType();
    NullableFlowPtr<Expr> RecurseVariableValue();
    NullableFlowPtr<Stmt> RecurseVariableInstance(VarDeclType decl_type);

    FlowPtr<Expr> RecurseWhileCond();
    FlowPtr<Stmt> RecurseWhileBody();

    Parser(lex::IScanner &lexer, std::shared_ptr<Environment> env,
           std::shared_ptr<void> lifetime);

  public:
    static boost::shared_ptr<Parser> Create(
        lex::IScanner &lexer, std::shared_ptr<Environment> env,
        std::shared_ptr<void> lifetime = nullptr) {
      return boost::shared_ptr<Parser>(new Parser(lexer, env, lifetime));
    }

    ~Parser() = default;

    ASTRoot Parse();

    void SetFailBit() { m_failed = true; }
    lex::IScanner &GetLexer() { return m_rd; }

    template <typename Scanner = lex::Tokenizer>
    static boost::shared_ptr<Parser> FromString(
        std::string_view str, std::shared_ptr<Environment> env) {
      auto state = std::make_shared<
          std::pair<std::stringstream, std::unique_ptr<Scanner>>>(
          std::stringstream(std::string(str)), nullptr);
      state->second = std::make_unique<Scanner>(state->first, env);

      return Create(*state->second, env, state);
    }

    template <typename Scanner = lex::Tokenizer>
    static boost::shared_ptr<Parser> FromStream(
        std::istream &stream, std::shared_ptr<Environment> env) {
      auto lexer = std::make_shared<Scanner>(stream, env);
      return Create(lexer, env, lexer);
    }
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_PARSER_H__
