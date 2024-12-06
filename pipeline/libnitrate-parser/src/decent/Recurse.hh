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

#include <nitrate-core/Macro.h>
#include <nitrate-lexer/Token.h>
#include <nitrate-parser/Node.h>
#include <nitrate-parser/Parser.h>

#include <core/ParseReport.hh>
#include <core/ParserStruct.hh>
#include <set>
#include <unordered_set>

namespace qparse {
  Stmt *recurse_pub(qparse_t &S, qlex_t &rd);
  Stmt *recurse_sec(qparse_t &S, qlex_t &rd);
  Stmt *recurse_pro(qparse_t &S, qlex_t &rd);
  std::vector<Stmt *> recurse_let(qparse_t &S, qlex_t &rd);
  std::vector<Stmt *> recurse_const(qparse_t &S, qlex_t &rd);
  std::vector<Stmt *> recurse_var(qparse_t &S, qlex_t &rd);
  Stmt *recurse_enum(qparse_t &S, qlex_t &rd);
  Stmt *recurse_struct(qparse_t &S, qlex_t &rd, CompositeType type);
  Stmt *recurse_subsystem(qparse_t &S, qlex_t &rd);
  Stmt *recurse_function(qparse_t &S, qlex_t &rd);
  Type *recurse_type(qparse_t &S, qlex_t &rd);
  Stmt *recurse_typedef(qparse_t &S, qlex_t &rd);
  Stmt *recurse_return(qparse_t &S, qlex_t &rd);
  Stmt *recurse_retif(qparse_t &S, qlex_t &rd);
  Stmt *recurse_if(qparse_t &S, qlex_t &rd);
  Stmt *recurse_while(qparse_t &S, qlex_t &rd);
  Stmt *recurse_for(qparse_t &S, qlex_t &rd);
  Stmt *recurse_foreach(qparse_t &S, qlex_t &rd);
  Stmt *recurse_switch(qparse_t &S, qlex_t &rd);
  Stmt *recurse_inline_asm(qparse_t &S, qlex_t &rd);
  Decl *recurse_composite_field(qparse_t &S, qlex_t &rd);

  bool recurse_attributes(qparse_t &S, qlex_t &rd,
                          std::set<Expr *> &attributes);

  Stmt *recurse_block(qparse_t &S, qlex_t &rd, bool expect_braces = true,
                      bool single_stmt = false);

  struct tok_hash {
    std::size_t operator()(qlex_tok_t const &v) const {
      union {
        uint64_t w;
        struct {
          qlex_ty_t ty;
          uint32_t val;
        } st;
      } u = {.st = {.ty = v.ty, .val = v.v.str_idx}};

      return u.w;
    }
  };

  Expr *recurse_expr(qparse_t &S, qlex_t &rd,
                     std::unordered_set<qlex_tok_t, tok_hash> terminators,
                     size_t depth = 0);

#define next() qlex_next(&rd)
#define peek() qlex_peek(&rd)
#define current() qlex_current(&rd)

};  // namespace qparse

#endif  // __NITRATE_PARSE_H__
