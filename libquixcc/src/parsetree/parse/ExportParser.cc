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

bool libquixcc::parse_pub(quixcc_job_t &job, libquixcc::Scanner *scanner,
                          std::shared_ptr<libquixcc::StmtNode> &node) {
  Token tok = scanner->peek();

  ExportLangType langType = ExportLangType::Default;

  if (tok.type == TT::String) {
    scanner->next();

    std::string lang = tok.as<std::string>();

    std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);

    if (lang == "c")
      langType = ExportLangType::C;
    else if (lang == "c++" || lang == "cxx")
      langType = ExportLangType::CXX;
    else if (lang == "d" || lang == "dlang")
      langType = ExportLangType::DLang;
    else {
      LOG(ERROR) << core::feedback[PARSER_UNKNOWN_LANGUAGE] << lang << tok
                 << std::endl;
      return false;
    }

    tok = scanner->peek();
  }

  if (tok.is<Punctor>(Punctor::OpenBrace)) {
    std::shared_ptr<libquixcc::BlockNode> block;
    if (!parse(job, scanner, block, true)) return false;

    node = std::make_shared<libquixcc::ExportNode>(block->m_stmts, langType);
    return true;
  }

  scanner->next();
  if (tok.type != TT::Keyword) {
    LOG(ERROR) << core::feedback[PARSER_EXPECTED_KEYWORD] << tok.serialize() << tok
               << std::endl;
    return false;
  }

  std::vector<std::shared_ptr<libquixcc::StmtNode>> stmts;
  std::shared_ptr<libquixcc::StmtNode> stmt;

  switch (tok.as<Keyword>()) {
    case Keyword::Var:
      if (!parse_var(job, scanner, stmts)) return false;
      break;
    case Keyword::Let:
      if (!parse_let(job, scanner, stmts)) return false;
      break;
    case Keyword::Subsystem:
      if (!parse_subsystem(job, scanner, stmt)) return false;
      break;
    case Keyword::Fn:
      if (!parse_function(job, scanner, stmt)) return false;
      break;
    default:
      LOG(ERROR) << core::feedback[PARSER_EXPECTED_KEYWORD] << tok.serialize() << tok
                 << std::endl;
      return false;
  }
  if (stmt) stmts.push_back(stmt);

  node = std::make_shared<libquixcc::ExportNode>(stmts, langType);

  return true;
}