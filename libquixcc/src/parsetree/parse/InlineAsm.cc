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

#include <LibMacro.h>
#include <core/Logger.h>
#include <parsetree/Parser.h>

using namespace libquixcc;

/*
    Syntax: __asm__("mov x0, $0", {}, {'$0': x}, ["x0"]);
*/

static bool asm_parse_param(
    quixcc_job_t &job, libquixcc::Scanner *scanner,
    std::vector<std::pair<std::string, std::shared_ptr<ExprNode>>> &result) {
  Token tok;

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::OpenBrace)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_LEFT_BRACE] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  while (!(tok = scanner->next()).is<Punctor>(Punctor::CloseBrace)) {
    if (tok.type != TT::String) {
      LOG(ERROR) << core::feedback[ASM_PARAM_EXPECTED_STRING_LITERAL]
                 << tok.serialize() << tok << std::endl;
      return false;
    }

    std::string name = tok.as<std::string>();

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::Colon)) {
      LOG(ERROR) << core::feedback[ASM_PARAM_EXPECTED_COLON] << tok.serialize() << tok
                 << std::endl;
      return false;
    }

    std::shared_ptr<ExprNode> expr;
    if (!parse_expr(job, scanner,
                    {Token(TT::Punctor, Punctor::Comma),
                     Token(TT::Punctor, Punctor::CloseBrace)},
                    expr)) {
      LOG(ERROR) << core::feedback[ASM_PARAM_EXPECTED_EXPR] << tok.serialize() << tok
                 << std::endl;
      return false;
    }

    result.push_back({name, expr});

    tok = scanner->next();
    if (tok.is<Punctor>(Punctor::CloseBrace)) {
      break;
    } else if (!tok.is<Punctor>(Punctor::Comma)) {
      LOG(ERROR) << core::feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok
                 << std::endl;
      return false;
    }
  }

  return true;
}

static bool asm_parse_clobbers(quixcc_job_t &job, libquixcc::Scanner *scanner,
                               std::vector<std::string> &result) {
  // ['a', 'b', 'c']

  Token tok;

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::OpenBracket)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_LEFT_BRACE] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  while (true) {
    tok = scanner->next();
    if (tok.is<Punctor>(Punctor::CloseBracket)) {
      break;
    }

    if (tok.type != TT::String) {
      LOG(ERROR) << core::feedback[ASM_PARAM_EXPECTED_STRING_LITERAL]
                 << tok.serialize() << tok << std::endl;
      return false;
    }

    result.push_back(tok.as<std::string>());

    tok = scanner->next();
    if (tok.is<Punctor>(Punctor::CloseBracket)) {
      break;
    } else if (!tok.is<Punctor>(Punctor::Comma)) {
      LOG(ERROR) << core::feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok
                 << std::endl;
      return false;
    }
  }

  return true;
}

bool libquixcc::parse_inline_asm(quixcc_job_t &job, libquixcc::Scanner *scanner,
                                 std::shared_ptr<libquixcc::StmtNode> &node) {
  Token tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::OpenParen)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_LEFT_PAREN] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  tok = scanner->next();
  if (tok.type != TT::String) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_STRING_LITERAL] << tok.serialize()
               << tok << std::endl;
    return false;
  }

  std::string asmcode = tok.as<std::string>();

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::Comma)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  std::vector<std::pair<std::string, std::shared_ptr<ExprNode>>> outputs;
  std::vector<std::pair<std::string, std::shared_ptr<ExprNode>>> inputs;

  if (!asm_parse_param(job, scanner, outputs)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_OUTPUTS] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::Comma)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  if (!asm_parse_param(job, scanner, inputs)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_INPUTS] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::Comma)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_COMMA] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  std::vector<std::string> clobbers;
  if (!asm_parse_clobbers(job, scanner, clobbers)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_CLOBBERS] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::CloseParen)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_RIGHT_PAREN] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  tok = scanner->next();
  if (!tok.is<Punctor>(Punctor::Semicolon)) {
    LOG(ERROR) << core::feedback[ASM_EXPECTED_SEMICOLON] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  node = std::make_shared<InlineAsmNode>(asmcode, outputs, inputs, clobbers);

  return true;
}