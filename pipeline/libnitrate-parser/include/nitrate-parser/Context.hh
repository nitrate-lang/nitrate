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

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/AST.hh>
#include <sstream>

namespace ncc::parse {
  class Parser;

  class ASTRoot final {
    FlowPtr<Base> m_base;
    std::shared_ptr<void> m_allocator;
    bool m_failed;

  public:
    ASTRoot(FlowPtr<Base> base, std::shared_ptr<void> allocator, bool failed)
        : m_base(base), m_allocator(allocator), m_failed(failed) {}

    FlowPtr<Base> &get() { return m_base; }
    FlowPtr<Base> get() const { return m_base; }

    bool check() const;
  };

  class Parser final {
    std::shared_ptr<ncc::Environment> m_env;
    std::unique_ptr<ncc::IMemory> m_allocator;
    ncc::lex::IScanner &rd;
    bool m_failed;
    std::shared_ptr<void> m_lifetime;

    /****************************************************************************
     * @brief
     *  Primary language constructs
     ****************************************************************************/

    FlowPtr<Stmt> recurse_pub();
    FlowPtr<Stmt> recurse_sec();
    FlowPtr<Stmt> recurse_pro();
    std::vector<FlowPtr<Stmt>> recurse_variable(VarDeclType type);
    FlowPtr<Stmt> recurse_enum();
    FlowPtr<Stmt> recurse_struct(CompositeType type);
    FlowPtr<Stmt> recurse_scope();
    FlowPtr<Stmt> recurse_function(bool restrict_decl_only);
    FlowPtr<Type> recurse_type();
    FlowPtr<Stmt> recurse_typedef();
    FlowPtr<Stmt> recurse_return();
    FlowPtr<Stmt> recurse_retif();
    FlowPtr<Stmt> recurse_if();
    FlowPtr<Stmt> recurse_while();
    FlowPtr<Stmt> recurse_for();
    FlowPtr<Stmt> recurse_foreach();
    FlowPtr<Stmt> recurse_switch();
    FlowPtr<Stmt> recurse_inline_asm();
    FlowPtr<Stmt> recurse_block(bool expect_braces, bool single_stmt,
                                SafetyMode safety);
    FlowPtr<Expr> recurse_expr(const std::set<ncc::lex::Token> &terminators);
    std::optional<FlowPtr<Expr>> recurse_expr_primary(bool isType);
    std::optional<FlowPtr<Expr>> recurse_expr_keyword(lex::Keyword key);
    std::optional<FlowPtr<Expr>> recurse_expr_punctor(lex::Punctor punc);
    FlowPtr<Expr> recurse_expr_type_suffix(FlowPtr<Expr> base);

    /****************************************************************************
     * @brief
     *  Helper functions
     ****************************************************************************/

    std::string_view recurse_enum_name();
    std::optional<FlowPtr<Type>> recurse_enum_type();
    std::optional<FlowPtr<Expr>> recurse_enum_item_value();
    std::optional<EnumItem> recurse_enum_item();
    std::optional<EnumDefItems> recurse_enum_items();

    std::string_view recurse_abi_name();
    std::optional<ExpressionList> recurse_export_attributes();
    FlowPtr<Stmt> recurse_export_body();

    CallArgs recurse_call_arguments(ncc::lex::Token terminator);
    FlowPtr<Expr> recurse_fstring();

    std::optional<FlowPtr<Stmt>> recurse_for_init_expr();
    std::optional<FlowPtr<Expr>> recurse_for_cond_expr();
    std::optional<FlowPtr<Expr>> recurse_for_step_expr(bool has_paren);
    FlowPtr<Stmt> recurse_for_body();

    std::optional<std::pair<std::string_view, std::string_view>>
    recurse_foreach_names();
    FlowPtr<Expr> recurse_foreach_expr(bool has_paren);
    FlowPtr<Stmt> recurse_foreach_body();

    FlowPtr<Type> recurse_function_parameter_type();
    std::optional<FlowPtr<Expr>> recurse_function_parameter_value();
    std::optional<FuncParam> recurse_function_parameter();
    std::optional<TemplateParameters> recurse_template_parameters();
    std::pair<FuncParams, bool> recurse_function_parameters();
    std::optional<FlowPtr<Stmt>> recurse_function_body(bool restrict_decl_only);
    FlowPtr<Type> recurse_function_return_type();
    FuncPurity get_purity_specifier(ncc::lex::Token &start_pos,
                                    bool is_thread_safe, bool is_pure,
                                    bool is_impure, bool is_quasi,
                                    bool is_retro);
    std::optional<std::pair<std::string_view, bool>> recurse_function_capture();
    void recurse_function_ambigouis(ExpressionList &attributes,
                                    FnCaptures &captures, FuncPurity &purity,
                                    std::string_view &function_name);

    FlowPtr<Stmt> recurse_if_then();
    std::optional<FlowPtr<Stmt>> recurse_if_else();

    std::string_view recurse_scope_name();
    std::optional<ScopeDeps> recurse_scope_deps();
    FlowPtr<Stmt> recurse_scope_block();

    struct StructContent {
      StructDefFields fields;
      StructDefMethods methods;
      StructDefStaticMethods static_methods;
    };
    ExpressionList recurse_struct_attributes();
    std::string_view recurse_struct_name();
    StructDefNames recurse_struct_terms();
    std::optional<FlowPtr<Expr>> recurse_struct_field_default_value();
    void recurse_struct_field(Vis vis, bool is_static, StructDefFields &fields);
    void recurse_struct_method_or_field(StructContent &body);
    StructContent recurse_struct_body();

    FlowPtr<Stmt> recurse_switch_case_body();
    std::variant<FlowPtr<CaseStmt>, FlowPtr<Stmt>> recurse_switch_case();
    std::optional<std::pair<SwitchCases, std::optional<FlowPtr<CaseStmt>>>>
    recurse_switch_body();

    std::optional<FlowPtr<Expr>> recurse_type_range_start();
    std::optional<FlowPtr<Expr>> recurse_type_range_end();
    std::optional<CallArgs> recurse_type_template_arguments();
    FlowPtr<Type> recurse_type_suffix(FlowPtr<Type> base);
    FlowPtr<Type> recurse_function_type();
    FlowPtr<Type> recurse_opaque_type();
    FlowPtr<Type> recurse_type_by_keyword(ncc::lex::Keyword key);
    FlowPtr<Type> recurse_type_by_operator(ncc::lex::Operator op);
    FlowPtr<Type> recurse_array_or_vector();
    FlowPtr<Type> recurse_set_type();
    FlowPtr<Type> recurse_tuple_type();
    FlowPtr<Type> recurse_type_by_punctuation(ncc::lex::Punctor punc);
    FlowPtr<Type> recurse_type_by_name(std::string_view name);

    std::optional<ExpressionList> recurse_variable_attributes();
    std::optional<FlowPtr<Type>> recurse_variable_type();
    std::optional<FlowPtr<Expr>> recurse_variable_value();
    std::optional<FlowPtr<Stmt>> recurse_variable_instance(
        VarDeclType decl_type);

    FlowPtr<Expr> recurse_while_cond();
    FlowPtr<Stmt> recurse_while_body();

    Parser(ncc::lex::IScanner &lexer, std::shared_ptr<ncc::Environment> env,
           std::shared_ptr<void> lifetime);

  public:
    static boost::shared_ptr<Parser> Create(
        ncc::lex::IScanner &lexer, std::shared_ptr<ncc::Environment> env,
        std::shared_ptr<void> lifetime = nullptr) {
      return boost::shared_ptr<Parser>(new Parser(lexer, env, lifetime));
    }

    ~Parser() = default;

    ASTRoot parse();

    void SetFailBit() { m_failed = true; }
    ncc::lex::IScanner &GetLexer() { return rd; }

    template <typename Scanner = ncc::lex::Tokenizer>
    static boost::shared_ptr<Parser> FromString(
        std::string_view str, std::shared_ptr<ncc::Environment> env) {
      auto state = std::make_shared<
          std::pair<std::stringstream, std::unique_ptr<Scanner>>>(
          std::stringstream(std::string(str)), nullptr);
      state->second = std::make_unique<Scanner>(state->first, env);

      return Create(*state->second, env, state);
    }

    template <typename Scanner = lex::Tokenizer>
    static boost::shared_ptr<Parser> FromStream(
        std::istream &stream, std::shared_ptr<ncc::Environment> env) {
      auto lexer = std::make_shared<Scanner>(stream, env);
      return Create(lexer, env, lexer);
    }
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_PARSER_H__
