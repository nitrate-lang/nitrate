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

using namespace qparse;
using namespace qparse::parser;

static bool asm_parse_param(qparse_t &job, qlex_t *rd,
                            std::vector<std::pair<std::string, Expr *>> &result) {
  qlex_tok_t tok;

  tok = qlex_next(rd);
  if (!tok.is<qPuncLCur>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  while (!(tok = qlex_next(rd)).is<qPuncRCur>()) {
    if (!tok.is(qText)) {
      /// TODO: Write the ERROR message
      return false;
    }

    std::string name = tok.as_string();

    tok = qlex_next(rd);
    if (!tok.is<qPuncColn>()) {
      /// TODO: Write the ERROR message
      return false;
    }

    Expr *expr = nullptr;
    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRCur)},
                    &expr)) {
      /// TODO: Write the ERROR message
      return false;
    }

    result.push_back({name, expr});

    tok = qlex_next(rd);
    if (tok.is<qPuncRCur>()) {
      break;
    } else if (!tok.is<qPuncComa>()) {
      /// TODO: Write the ERROR message
      return false;
    }
  }

  return true;
}

static bool asm_parse_clobbers(qparse_t &job, qlex_t *rd, std::vector<std::string> &result) {
  // ['a', 'b', 'c']

  qlex_tok_t tok;

  tok = qlex_next(rd);
  if (!tok.is<qPuncLBrk>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  while (true) {
    tok = qlex_next(rd);
    if (tok.is<qPuncRBrk>()) {
      break;
    }

    if (!tok.is(qText)) {
      /// TODO: Write the ERROR message
      return false;
    }

    result.push_back(tok.as_string());

    tok = qlex_next(rd);
    if (tok.is<qPuncRBrk>()) {
      break;
    } else if (!tok.is<qPuncComa>()) {
      /// TODO: Write the ERROR message
      return false;
    }
  }

  return true;
}

bool qparse::parser::parse_inline_asm(qparse_t &job, qlex_t *rd, Stmt **node) {
  qlex_tok_t tok = qlex_next(rd);
  if (!tok.is<qPuncLPar>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  tok = qlex_next(rd);
  if (!tok.is(qText)) {
    /// TODO: Write the ERROR message
    return false;
  }

  std::string asmcode = tok.as_string();

  tok = qlex_next(rd);
  if (!tok.is<qPuncComa>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  std::vector<std::pair<std::string, Expr *>> outputs;
  std::vector<std::pair<std::string, Expr *>> inputs;

  if (!asm_parse_param(job, rd, outputs)) {
    /// TODO: Write the ERROR message
    return false;
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncComa>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  if (!asm_parse_param(job, rd, inputs)) {
    /// TODO: Write the ERROR message
    return false;
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncComa>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  std::vector<std::string> clobbers;
  if (!asm_parse_clobbers(job, rd, clobbers)) {
    /// TODO: Write the ERROR message
    return false;
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncRPar>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  tok = qlex_next(rd);
  if (!tok.is<qPuncSemi>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  // node = std::make_shared<InlineAsmNode>(asmcode, outputs, inputs, clobbers);
  // *node = InlineAsm::get(asmcode, outputs, inputs, clobbers);
  /// TODO: Implement InlineAsm::get
  throw std::runtime_error("InlineAsm::get not implemented");

  return true;
}