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

#define __QUIX_IMPL__

#include "LibMacro.h"
#include "parser/Parse.h"

using namespace qparse::parser;

namespace qparse::parser {
  static bool parse_decl(qparse_t &job, qlex_tok_t tok, qlex_t *rd,
                         std::pair<std::string, Type *> &decl) {
    std::string name = tok.as_string(rd);

    tok = qlex_peek(rd);
    if (!tok.is<qPuncColn>()) {
      decl = std::make_pair(name, nullptr);
      return true;
    }

    qlex_next(rd);

    Type *type = nullptr;

    if (!parse_type(job, rd, &type)) {
      /// TODO: Write the ERROR message
      return false;
    }

    decl = std::make_pair(name, type);
    return true;
  }
}  // namespace qparse::parser

bool qparse::parser::parse_const(qparse_t &job, qlex_t *rd, StmtListItems &nodes) {
  qlex_tok_t tok = qlex_next(rd);

  std::vector<std::pair<std::string, Type *>> decls;
  bool multi_decl = false;
  if (tok.is<qPuncLBrk>()) {
    multi_decl = true;

    while (true) {
      tok = qlex_next(rd);

      std::pair<std::string, Type *> decl;
      if (!parse_decl(job, tok, rd, decl)) return false;

      decls.push_back(decl);

      tok = qlex_next(rd);
      if (tok.is<qPuncComa>())
        continue;
      else if (tok.is<qPuncRBrk>())
        break;
      else {
        /// TODO: Write the ERROR message
        return false;
      }
    }
  } else if (tok.is(qName)) {
    // Parse single variable declaration
    std::pair<std::string, Type *> decl;
    if (!parse_decl(job, tok, rd, decl)) return false;

    decls.push_back(decl);
  } else {
    /// TODO: Write the ERROR message
    return false;
  }

  if (decls.empty()) {
    /// TODO: Write the ERROR message
    return false;
  }

  tok = qlex_next(rd);
  if (tok.is<qPuncSemi>()) {
    // No initializer
    for (auto &decl : decls) {
      ConstDecl *const_decl = ConstDecl::get(decl.first, decl.second, nullptr);
      nodes.push_back(const_decl);
    }
  } else if (tok.is<qOpSet>()) {
    if (multi_decl)
      throw std::runtime_error("Initializer not implemented for multiple declarations");

    // Parse initializer
    Expr *init = nullptr;
    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncSemi)}, &init)) return false;

    tok = qlex_next(rd);
    if (!tok.is<qPuncSemi>()) {
      /// TODO: Write the ERROR message
      return false;
    }

    ConstDecl *const_decl = ConstDecl::get(decls[0].first, decls[0].second, init);
    nodes.push_back(const_decl);
  } else {
    /// TODO: Write the ERROR message
    return false;
  }

  return true;
}