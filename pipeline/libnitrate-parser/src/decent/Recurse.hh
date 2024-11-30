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

#ifndef __NITRATE_PARSE_H__
#define __NITRATE_PARSE_H__

#include <nitrate-lexer/Token.h>
#include <nitrate-parser/Node.h>
#include <nitrate-parser/Parser.h>

#include <core/ParseReport.hh>
#include <core/ParserStruct.hh>
#include <set>

namespace qparse {
  bool recurse_pub(qparse_t &S, qlex_t &rd, Stmt **node);
  bool recurse_sec(qparse_t &S, qlex_t &rd, Stmt **node);
  bool recurse_pro(qparse_t &S, qlex_t &rd, Stmt **node);
  bool recurse_let(qparse_t &S, qlex_t &rd, std::vector<Stmt *> &node);
  bool recurse_const(qparse_t &S, qlex_t &rd, std::vector<Stmt *> &node);
  bool recurse_var(qparse_t &S, qlex_t &rd, std::vector<Stmt *> &node);
  bool recurse_enum(qparse_t &S, qlex_t &rd, Stmt **node);
  bool recurse_struct(qparse_t &S, qlex_t &rd, Stmt **node);
  bool recurse_subsystem(qparse_t &S, qlex_t &rd, Stmt **node);
  bool recurse_function(qparse_t &S, qlex_t &rd, Stmt **node);
  bool recurse_type(qparse_t &S, qlex_t &rd, Type **node);
  Stmt *recurse_typedef(qparse_t &S, qlex_t &rd);
  Stmt *recurse_return(qparse_t &S, qlex_t &rd);
  Stmt *recurse_retif(qparse_t &S, qlex_t &rd);
  Stmt *recurse_if(qparse_t &S, qlex_t &rd);
  Stmt *recurse_while(qparse_t &S, qlex_t &rd);
  Stmt *recurse_for(qparse_t &S, qlex_t &rd);
  bool recurse_foreach(qparse_t &S, qlex_t &rd, Stmt **node);
  bool recurse_case(qparse_t &S, qlex_t &rd, Stmt **node);
  bool recurse_switch(qparse_t &S, qlex_t &rd, Stmt **node);
  Stmt *recurse_inline_asm(qparse_t &S, qlex_t &rd);
  bool recurse_composite_field(qparse_t &S, qlex_t &rd, StructField **node);

  bool recurse_attributes(qparse_t &S, qlex_t &rd,
                          std::set<Expr *> &attributes);

  Stmt *recurse(qparse_t &S, qlex_t &rd, bool expect_braces = true,
                bool single_stmt = false);

  bool recurse_expr(qparse_t &S, qlex_t &rd, std::set<qlex_tok_t> terminators,
                    Expr **node, size_t depth = 0);

#define next() qlex_next(&rd)
#define peek() qlex_peek(&rd)

};  // namespace qparse

#endif  // __NITRATE_PARSE_H__
