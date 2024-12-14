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

#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <nitrate-parser/Parser.h>

#include <core/Context.hh>
#include <sstream>

using namespace npar;

CPP_EXPORT thread_local DiagnosticManager *npar::diagnostic;

///============================================================================///

std::string DiagnosticManager::mint_plain_message(
    const DiagMessage &msg) const {
  std::stringstream ss;
  ss << qlex_filename(m_parser->lexer) << ":";
  uint32_t line = qlex_line(m_parser->lexer, qlex_begin(&msg.tok));
  uint32_t col = qlex_col(m_parser->lexer, qlex_begin(&msg.tok));

  if (line != QLEX_EOFF) {
    ss << line << ":";
  } else {
    ss << "?:";
  }

  if (col != QLEX_EOFF) {
    ss << col << ": ";
  } else {
    ss << "?: ";
  }
  ss << "error: " << msg.msg << " [";

  ss << "SyntaxError";

  ss << "]\n";

  uint32_t offset;
  char *snippet = qlex_snippet(m_parser->lexer, msg.tok, &offset);
  if (!snippet) {
    return ss.str();
  }

  ss << snippet << "\n";
  for (uint32_t i = 0; i < offset; i++) {
    ss << " ";
  }
  ss << "^\n";
  free(snippet);

  return ss.str();
}

std::string DiagnosticManager::mint_clang16_message(
    const DiagMessage &msg) const {
  std::stringstream ss;
  ss << "\x1b[37;1m" << qlex_filename(m_parser->lexer) << ":";
  uint32_t line = qlex_line(m_parser->lexer, qlex_begin(&msg.tok));
  uint32_t col = qlex_col(m_parser->lexer, qlex_begin(&msg.tok));

  if (line != QLEX_EOFF) {
    ss << line << ":";
  } else {
    ss << "?:";
  }

  if (col != QLEX_EOFF) {
    ss << col << ":\x1b[0m ";
  } else {
    ss << "?:\x1b[0m ";
  }

  ss << "\x1b[31;1merror:\x1b[0m \x1b[37;1m" << msg.msg << " [";

  ss << "SyntaxError";

  ss << "]\x1b[0m\n";

  uint32_t offset;
  char *snippet = qlex_snippet(m_parser->lexer, msg.tok, &offset);
  if (!snippet) {
    return ss.str();
  }

  ss << snippet << "\n";
  for (uint32_t i = 0; i < offset; i++) {
    ss << " ";
  }
  ss << "\x1b[32;1m^\x1b[0m\n";
  free(snippet);

  return ss.str();
}

///============================================================================///

void DiagnosticManager::push(DiagMessage &&msg) {
  m_msgs.push_back(std::move(msg));
  m_parser->failed = true;
}

size_t DiagnosticManager::render(DiagnosticMessageHandler handler,
                                 FormatStyle style) const {
  switch (style) {
    case FormatStyle::ClangPlain:
      for (const auto &msg : m_msgs) {
        handler(mint_plain_message(msg).c_str());
      }
      break;
    case FormatStyle::Clang16Color:
      for (const auto &msg : m_msgs) {
        handler(mint_clang16_message(msg).c_str());
      }
      break;
    default:
      qcore_panicf("Unsupported diagnostic format style: %d",
                   static_cast<int>(style));
  }

  return m_msgs.size();
}

void npar::install_reference(npar_t *parser) {
  diagnostic = parser ? &parser->diag : nullptr;
}
