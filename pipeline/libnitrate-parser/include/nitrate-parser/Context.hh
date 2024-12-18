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

#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/AST.hh>

typedef struct npar_t npar_t;

namespace ncc::parse {
  class Parser;

  class ASTRoot final {
    std::shared_ptr<Parser> m_ref;
    Base *m_base;

  public:
    ASTRoot(std::shared_ptr<Parser> ref, ncc::parse::Base *base)
        : m_ref(ref), m_base(base) {}

    Base *get() const { return m_base; }

    bool check() const;
  };

  class Parser final {
    std::shared_ptr<ncc::core::Environment> m_env;
    std::unique_ptr<ncc::core::IMemory> m_allocator; /* The Main allocator */
    qlex_t *rd;                                      /* Polymporphic lexer */
    bool m_failed; /* Whether the parser failed (ie syntax errors) */

    Stmt *recurse_pub();
    Stmt *recurse_sec();
    Stmt *recurse_pro();
    std::vector<Stmt *> recurse_variable(VarDeclType type);
    Stmt *recurse_enum();
    Stmt *recurse_struct(CompositeType type);
    Stmt *recurse_scope();
    Stmt *recurse_function(bool restrict_decl_only);
    Type *recurse_type();
    Stmt *recurse_typedef();
    Stmt *recurse_return();
    Stmt *recurse_retif();
    Stmt *recurse_if();
    Stmt *recurse_while();
    Stmt *recurse_for();
    Stmt *recurse_foreach();
    Stmt *recurse_switch();
    Stmt *recurse_inline_asm();
    Stmt *recurse_block(bool expect_braces, bool single_stmt,
                        SafetyMode safety);
    Expr *recurse_expr(std::set<qlex_tok_t> terminators, size_t depth = 0);

  public:
    Parser(std::shared_ptr<ncc::core::Environment> env);
    ~Parser();

    ASTRoot parse(qlex_t *lexer);

    static ASTRoot FromLexer(qlex_t *lexer,
                             std::shared_ptr<ncc::core::Environment> env);

    static ASTRoot FromString(std::string_view str,
                              std::shared_ptr<ncc::core::Environment> env);

    static ASTRoot FromStream(std::istream &stream,
                              std::shared_ptr<ncc::core::Environment> env);
  };
}  // namespace ncc::parse

/**
 * @brief Check if the parse tree is valid.
 *
 * @param parser The parser instance to use for parsing or NULL.
 * @param base The base node of the parse tree to check.
 *
 * @return True if the AST is valid, false otherwise.
 *
 * @note If base is NULL, false is returned.
 *
 * @note This function is thread safe.
 */
bool npar_check(npar_t *parser, const ncc::parse::Base *base);

#endif  // __NITRATE_AST_PARSER_H__
