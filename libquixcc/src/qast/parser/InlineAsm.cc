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

#define QUIXCC_INTERNAL

#include <core/Macro.h>
#include <qast/Parser.h>
#include <quixcc/core/Logger.h>

using namespace libquixcc;
using namespace libquixcc::qast;
using namespace libquixcc::qast::parser;

static bool asm_parse_param(quixcc_cc_job_t &job, Scanner *scanner,
                            std::vector<std::pair<std::string, Expr *>> &result) {
  Token tok;

  tok = scanner->next();
  if (!tok.is<Punctor>(OpenBrace)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_LEFT_BRACE] << tok.serialize() << tok << std::endl;
    return false;
  }

  while (!(tok = scanner->next()).is<Punctor>(CloseBrace)) {
    if (!tok.is(tText)) {
      LOG(ERROR) << core::feedback[ASM_PARAM_EXPECTED_STRING_LITERAL] << tok.serialize() << tok
                 << std::endl;
      return false;
    }

    std::string name = tok.as_string();

    tok = scanner->next();
    if (!tok.is<Punctor>(Colon)) {
      LOG(ERROR) << core::feedback[ASM_PARAM_EXPECTED_COLON] << tok.serialize() << tok << std::endl;
      return false;
    }

    Expr *expr = nullptr;
    if (!parse_expr(job, scanner, {Token(tPunc, Comma), Token(tPunc, CloseBrace)}, &expr)) {
      LOG(ERROR) << core::feedback[ASM_PARAM_EXPECTED_EXPR] << tok.serialize() << tok << std::endl;
      return false;
    }

    result.push_back({name, expr});

    tok = scanner->next();
    if (tok.is<Punctor>(CloseBrace)) {
      break;
    } else if (!tok.is<Punctor>(Comma)) {
      LOG(ERROR) << core::feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok << std::endl;
      return false;
    }
  }

  return true;
}

static bool asm_parse_clobbers(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                               std::vector<std::string> &result) {
  // ['a', 'b', 'c']

  Token tok;

  tok = scanner->next();
  if (!tok.is<Punctor>(OpenBracket)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_LEFT_BRACE] << tok.serialize() << tok << std::endl;
    return false;
  }

  while (true) {
    tok = scanner->next();
    if (tok.is<Punctor>(CloseBracket)) {
      break;
    }

    if (!tok.is(tText)) {
      LOG(ERROR) << core::feedback[ASM_PARAM_EXPECTED_STRING_LITERAL] << tok.serialize() << tok
                 << std::endl;
      return false;
    }

    result.push_back(tok.as_string());

    tok = scanner->next();
    if (tok.is<Punctor>(CloseBracket)) {
      break;
    } else if (!tok.is<Punctor>(Comma)) {
      LOG(ERROR) << core::feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok << std::endl;
      return false;
    }
  }

  return true;
}

bool libquixcc::qast::parser::parse_inline_asm(quixcc_cc_job_t &job, libquixcc::Scanner *scanner,
                                               Stmt **node) {
  Token tok = scanner->next();
  if (!tok.is<Punctor>(OpenParen)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_LEFT_PAREN] << tok.serialize() << tok << std::endl;
    return false;
  }

  tok = scanner->next();
  if (!tok.is(tText)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_STRING_LITERAL] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  std::string asmcode = tok.as_string();

  tok = scanner->next();
  if (!tok.is<Punctor>(Comma)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok << std::endl;
    return false;
  }

  std::vector<std::pair<std::string, Expr *>> outputs;
  std::vector<std::pair<std::string, Expr *>> inputs;

  if (!asm_parse_param(job, scanner, outputs)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_OUTPUTS] << tok.serialize() << tok << std::endl;
    return false;
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(Comma)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok << std::endl;
    return false;
  }

  if (!asm_parse_param(job, scanner, inputs)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_INPUTS] << tok.serialize() << tok << std::endl;
    return false;
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(Comma)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok << std::endl;
    return false;
  }

  std::vector<std::string> clobbers;
  if (!asm_parse_clobbers(job, scanner, clobbers)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_CLOBBERS] << tok.serialize() << tok << std::endl;
    return false;
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(CloseParen)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_RIGHT_PAREN] << tok.serialize() << tok << std::endl;
    return false;
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(Semicolon)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_SEMICOLON] << tok.serialize() << tok << std::endl;
    return false;
  }

  // node = std::make_shared<InlineAsmNode>(asmcode, outputs, inputs, clobbers);
  // *node = InlineAsm::get(asmcode, outputs, inputs, clobbers);
  /// TODO: Implement InlineAsm::get
  throw std::runtime_error("InlineAsm::get not implemented");

  return true;
}