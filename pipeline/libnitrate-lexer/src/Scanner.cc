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

#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Lexer.hh>

#include "nitrate-lexer/Token.hh"

using namespace ncc::lex;

CPP_EXPORT std::string_view NCCToken::as_string() const {
  std::string_view R;

  switch (ty) {
    case qEofF: {
      R = "";
      break;
    }

    case qKeyW: {
      R = ncc::lex::kw_repr(v.key);
      break;
    }

    case qOper: {
      R = ncc::lex::op_repr(v.op);
      break;
    }

    case qPunc: {
      R = ncc::lex::punct_repr(v.punc);
      break;
    }

    case qName: {
      R = v.str_idx.get();
      break;
    }

    case qIntL: {
      R = v.str_idx.get();
      break;
    }

    case qNumL: {
      R = v.str_idx.get();
      break;
    }

    case qText: {
      R = v.str_idx.get();
      break;
    }

    case qChar: {
      R = v.str_idx.get();
      break;
    }

    case qMacB: {
      R = v.str_idx.get();
      break;
    }

    case qMacr: {
      R = v.str_idx.get();
      break;
    }

    case qNote: {
      R = v.str_idx.get();
      break;
    }
  }

  return R;
}

CPP_EXPORT void IScanner::FillTokenBuffer() {
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

CPP_EXPORT void IScanner::SyncState(NCCToken tok) {
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

CPP_EXPORT std::string_view IScanner::Filename(NCCToken t) {
  /// TODO:
  return "?";
}

CPP_EXPORT uint32_t IScanner::StartLine(NCCToken t) {
  /// TODO:
  return QLEX_EOFF;
}

CPP_EXPORT uint32_t IScanner::StartColumn(NCCToken t) {
  /// TODO:
  return QLEX_EOFF;
}

CPP_EXPORT uint32_t IScanner::EndLine(NCCToken t) {
  /// TODO:
  return QLEX_EOFF;
}

CPP_EXPORT uint32_t IScanner::EndColumn(NCCToken t) {
  /// TODO:
  return QLEX_EOFF;
}
