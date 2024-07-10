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

#include <LibMacro.h>
#include <core/Logger.h>
#include <parsetree/Parser.h>

bool libquixcc::parse_subsystem(quixcc_job_t &job, libquixcc::Scanner *scanner,
                                std::shared_ptr<libquixcc::StmtNode> &node) {
  Token tok = scanner->next();
  if (tok.type() != tName) {
    LOG(ERROR) << core::feedback[SUBSYSTEM_MISSING_IDENTIFIER] << tok
               << std::endl;
    return false;
  }

  std::string name = tok.as<std::string>();

  if (name.find("::") != std::string::npos) {
    /*
    subsystem a::b::c: hi, world {

    }

    /// ->

    subsystem a {
        subsystem b {
            subsystem c: hi, world {
            }
        }
    }
    */

    std::shared_ptr<StmtNode> sub;

    std::string subname = name.substr(0, name.find("::"));

    scanner->push(Token(tName, name.substr(name.find("::") + 2)));

    if (!parse_subsystem(job, scanner, sub)) return false;

    std::shared_ptr<BlockNode> block = std::make_shared<BlockNode>();
    block->m_stmts.push_back(sub);

    node = std::make_shared<SubsystemNode>(subname, std::set<std::string>(),
                                           block);

    return true;
  }

  std::set<std::string> deps;

  tok = scanner->peek();

  // check if : item1, item2, item3
  if (tok.is<Punctor>(Punctor::Colon)) {
    scanner->next();  // consume colon
    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::OpenBracket)) {
      LOG(ERROR) << "Expected '[' before subsystem dependencies" << tok
                 << std::endl;
      return false;
    }
    tok = scanner->next();

    if (tok.type() != tName) {
      LOG(ERROR) << core::feedback[SUBSYSTEM_EXPECTED_IDENTIFIER] << tok
                 << std::endl;
      return false;
    }
    deps.insert(tok.as<std::string>());

    tok = scanner->peek();
    while (tok.is<Punctor>(Punctor::Comma)) {
      scanner->next();  // consume comma
      tok = scanner->next();
      if (tok.type() != tName) {
        LOG(ERROR) << core::feedback[SUBSYSTEM_EXPECTED_IDENTIFIER] << tok
                   << std::endl;
        return false;
      }
      deps.insert(tok.as<std::string>());
      tok = scanner->peek();
    }

    tok = scanner->next();
    if (!tok.is<Punctor>(Punctor::CloseBracket)) {
      LOG(ERROR) << "Expected ']' after subsystem dependencies" << tok
                 << std::endl;
      return false;
    }
  }

  std::shared_ptr<BlockNode> block;
  if (!parse(job, scanner, block, true)) return false;

  node = std::make_shared<SubsystemNode>(name, deps, block);

  return true;
}