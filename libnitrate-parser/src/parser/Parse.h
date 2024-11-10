////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
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

#ifndef __NITRATE_PARSE_H__
#define __NITRATE_PARSE_H__

#define __NITRATE_LEXER_IMPL__
#define __QPARSE_IMPL__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <Impl.h>
#include <ParseReport.h>
#include <nitrate-lexer/Token.h>
#include <nitrate-parser/Node.h>
#include <nitrate-parser/Parser.h>

#include <ParserStruct.hh>
#include <set>

namespace qparse::parser {
  bool parse(qparse_t &job, qlex_t *rd, Block **node, bool expect_braces = true,
             bool single_stmt = false);

  bool parse_pub(qparse_t &job, qlex_t *rd, Stmt **node);
  bool parse_sec(qparse_t &job, qlex_t *rd, Stmt **node);
  bool parse_pro(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_let(qparse_t &job, qlex_t *rd, std::vector<Stmt *> &node);

  bool parse_const(qparse_t &job, qlex_t *rd, std::vector<Stmt *> &node);

  bool parse_var(qparse_t &job, qlex_t *rd, std::vector<Stmt *> &node);

  bool parse_enum(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_struct(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_region(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_group(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_union(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_subsystem(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_function(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_expr(qparse_t &job, qlex_t *rd, std::set<qlex_tok_t> terminators, Expr **node,
                  size_t depth = 0);

  bool parse_type(qparse_t &job, qlex_t *rd, Type **node);

  bool parse_typedef(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_return(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_retif(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_retz(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_retv(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_if(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_while(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_for(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_form(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_foreach(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_case(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_switch(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_inline_asm(qparse_t &job, qlex_t *rd, Stmt **node);

  bool parse_attributes(qparse_t &job, qlex_t *rd, std::set<ConstExpr *> &attributes);
};  // namespace qparse::parser

#endif  // __NITRATE_PARSE_H__
