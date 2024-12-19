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

#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Lexer.hh>

using namespace ncc::lex;

void IScanner::FillTokenBuffer() {
  for (size_t i = 0; i < TOKEN_BUFFER_SIZE; i++) {
    try {
      m_ready.push_back(GetNext());
    } catch (ScannerEOF &) {
      if (i == 0) {
        m_ready.push_back(NCCToken::eof(GetCurrentOffset()));
      }
      break;
    }
  }
}

void IScanner::SyncState(NCCToken tok) {
  /// TODO:
  m_current = tok;
}

CPP_EXPORT NCCToken IScanner::Next() {
  if (m_ready.empty()) [[unlikely]] {
    FillTokenBuffer();
  }

  NCCToken tok = m_ready.front();
  m_ready.pop_front();
  SyncState(tok);
  m_last = m_current;

  return tok;
}

CPP_EXPORT NCCToken IScanner::Peek() {
  if (m_ready.empty()) [[unlikely]] {
    FillTokenBuffer();
  }

  NCCToken tok = m_ready.front();
  SyncState(tok);

  return tok;
}

CPP_EXPORT void IScanner::Undo() {
  m_ready.push_front(m_last);
  SyncState(m_last);
}
