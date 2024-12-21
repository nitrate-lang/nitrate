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
#include <nitrate-lexer/Token.hh>

using namespace ncc::lex;

CPP_EXPORT int ncc::lex::GetOperatorPrecedence(Operator op, OpMode type) {
  using Key = std::pair<Operator, OpMode>;

  struct KeyHash {
    size_t operator()(const Key &k) const {
      return std::hash<Operator>()(k.first) ^ std::hash<OpMode>()(k.second);
    }
  };

  /// TODO: Write this table

  static const std::unordered_map<Key, int, KeyHash> precedence = {
      {{qOpPlus, OpMode::Binary}, 5},
      {{qOpMinus, OpMode::Binary}, 2},
      {{qOpTimes, OpMode::Binary}, 7},
      {{qOpSlash, OpMode::Binary}, 2},
      {{qOpPercent, OpMode::Binary}, 2},
      {{qOpBitAnd, OpMode::Binary}, 2},
      {{qOpBitOr, OpMode::Binary}, 2},
      {{qOpBitXor, OpMode::Binary}, 2},
      {{qOpBitNot, OpMode::PreUnary}, 2},
      {{qOpLShift, OpMode::Binary}, 2},
      {{qOpRShift, OpMode::Binary}, 2},
      {{qOpROTL, OpMode::Binary}, 2},
      {{qOpROTR, OpMode::Binary}, 2},
      {{qOpLogicAnd, OpMode::Binary}, 2},
      {{qOpLogicOr, OpMode::Binary}, 2},
      {{qOpLogicXor, OpMode::Binary}, 2},
      {{qOpLogicNot, OpMode::PreUnary}, 2},
      {{qOpLT, OpMode::Binary}, 2},
      {{qOpGT, OpMode::Binary}, 2},
      {{qOpLE, OpMode::Binary}, 2},
      {{qOpGE, OpMode::Binary}, 2},
      {{qOpEq, OpMode::Binary}, 2},
      {{qOpNE, OpMode::Binary}, 2},
      {{qOpSet, OpMode::Binary}, 2},
      {{qOpPlusSet, OpMode::Binary}, 2},
      {{qOpMinusSet, OpMode::Binary}, 2},
      {{qOpTimesSet, OpMode::Binary}, 2},
      {{qOpSlashSet, OpMode::Binary}, 2},
      {{qOpPercentSet, OpMode::Binary}, 2},
      {{qOpBitAndSet, OpMode::Binary}, 2},
      {{qOpBitOrSet, OpMode::Binary}, 2},
      {{qOpBitXorSet, OpMode::Binary}, 2},
      {{qOpLogicAndSet, OpMode::Binary}, 2},
      {{qOpLogicOrSet, OpMode::Binary}, 2},
      {{qOpLogicXorSet, OpMode::Binary}, 2},
      {{qOpLShiftSet, OpMode::Binary}, 2},
      {{qOpRShiftSet, OpMode::Binary}, 2},
      {{qOpROTLSet, OpMode::Binary}, 2},
      {{qOpROTRSet, OpMode::Binary}, 2},
      {{qOpInc, OpMode::PreUnary}, 2},
      {{qOpDec, OpMode::PreUnary}, 2},
      {{qOpAs, OpMode::Binary}, 2},
      {{qOpBitcastAs, OpMode::Binary}, 2},
      {{qOpIn, OpMode::Binary}, 2},
      {{qOpOut, OpMode::Binary}, 2},
      {{qOpSizeof, OpMode::PreUnary}, 2},
      {{qOpBitsizeof, OpMode::PreUnary}, 2},
      {{qOpAlignof, OpMode::PreUnary}, 2},
      {{qOpTypeof, OpMode::PreUnary}, 2},
      {{qOpDot, OpMode::Binary}, 2},
      {{qOpRange, OpMode::Binary}, 2},
      {{qOpEllipsis, OpMode::PreUnary}, 2},
      {{qOpArrow, OpMode::Binary}, 2},
      {{qOpTernary, OpMode::Ternary}, 2},
  };

  auto it = precedence.find(Key(op, type));
  if (it != precedence.end()) [[likely]] {
    return it->second;
  }

  return -1;
}

CPP_EXPORT OpAssoc ncc::lex::GetOperatorAssociativity(Operator op,
                                                      OpMode type) {
  return OpAssoc::Left;
  /// TODO: Implement this function
  qcore_implement();
}

CPP_EXPORT std::string_view ncc::lex::to_string(TokenType ty, TokenData v) {
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
      R = v.str.get();
      break;
    }

    case qIntL: {
      R = v.str.get();
      break;
    }

    case qNumL: {
      R = v.str.get();
      break;
    }

    case qText: {
      R = v.str.get();
      break;
    }

    case qChar: {
      R = v.str.get();
      break;
    }

    case qMacB: {
      R = v.str.get();
      break;
    }

    case qMacr: {
      R = v.str.get();
      break;
    }

    case qNote: {
      R = v.str.get();
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
        m_ready.push_back(Token::EndOfFile());
      }
      break;
    }
  }
}

CPP_EXPORT void IScanner::SyncState(Token tok) {
  /// TODO: Implement this function
  m_current = tok;
}

CPP_EXPORT Token IScanner::Next() {
  while (true) {
    if (m_ready.empty()) {
      FillTokenBuffer();
    }

    Token tok = m_ready.front();
    m_ready.pop_front();

    if (GetSkipCommentsState() && tok.is(qNote)) {
      continue;
    }

    SyncState(tok);
    m_last = m_current;

    return tok;
  }
}

CPP_EXPORT Token IScanner::Peek() {
  if (m_ready.empty()) [[unlikely]] {
    FillTokenBuffer();
  }

  Token tok = m_ready.front();
  SyncState(tok);

  return tok;
}

CPP_EXPORT void IScanner::Undo() {
  if (!m_last.has_value()) {
    return;
  }

  m_ready.push_front(m_last.value());
  SyncState(m_last.value());
}

CPP_EXPORT std::string_view IScanner::Filename(Token t) {
  /// TODO:
  return "?";
}

CPP_EXPORT uint32_t IScanner::StartLine(Token t) {
  /// TODO:
  return QLEX_EOFF;
}

CPP_EXPORT uint32_t IScanner::StartColumn(Token t) {
  /// TODO:
  return QLEX_EOFF;
}

CPP_EXPORT uint32_t IScanner::EndLine(Token t) {
  /// TODO:
  return QLEX_EOFF;
}

CPP_EXPORT uint32_t IScanner::EndColumn(Token t) {
  /// TODO:
  return QLEX_EOFF;
}
