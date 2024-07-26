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
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_PARSE_H__
#define __QUIXCC_PARSE_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include "Nodes.h"
#include <quixcc/Library.h>
#include <qast/Lexer.h>

namespace libquixcc::qast::parser {
  bool parse(quixcc_cc_job_t &job, Scanner *scanner, Block **node, bool expect_braces = true,
             bool single_stmt = false);
  bool parse_pub(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_let(quixcc_cc_job_t &job, Scanner *scanner, StmtListItems &node);
  bool parse_const(quixcc_cc_job_t &job, Scanner *scanner, StmtListItems &node);
  bool parse_var(quixcc_cc_job_t &job, Scanner *scanner, StmtListItems &node);
  bool parse_enum(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_struct(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_region(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_group(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_union(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_subsystem(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_function(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_expr(quixcc_cc_job_t &job, Scanner *scanner, std::set<Token> terminators, Expr **node,
                  size_t depth = 0);
  bool parse_type(quixcc_cc_job_t &job, Scanner *scanner, Type **node);
  bool parse_typedef(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_return(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_retif(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_retz(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_retv(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_if(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_while(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_for(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_form(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_foreach(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_case(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_switch(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
  bool parse_inline_asm(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node);
};  // namespace libquixcc::qast::parser

#endif  // __QUIXCC_PARSE_H__
