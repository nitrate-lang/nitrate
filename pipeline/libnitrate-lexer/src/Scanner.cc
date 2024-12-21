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

/// FIXME: Verify and test this operator precedence table's semantics

// Lower index means higher precedence
static const std::vector<std::vector<std::tuple<Operator, OpMode, OpAssoc>>>
    precedence_groups = {
        {
            {qOpInc, OpMode::PostUnary, OpAssoc::Left},
            {qOpDec, OpMode::PostUnary, OpAssoc::Left},
            {qOpDot, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpInc, OpMode::PreUnary, OpAssoc::Right},
            {qOpDec, OpMode::PreUnary, OpAssoc::Right},
            {qOpPlus, OpMode::PreUnary, OpAssoc::Right},
            {qOpMinus, OpMode::PreUnary, OpAssoc::Right},
            {qOpLogicNot, OpMode::PreUnary, OpAssoc::Right},
            {qOpBitNot, OpMode::PreUnary, OpAssoc::Right},
            {qOpTimes, OpMode::PreUnary, OpAssoc::Right},
            {qOpBitAnd, OpMode::PreUnary, OpAssoc::Right},
            {qOpSizeof, OpMode::PreUnary, OpAssoc::Right},
            {qOpBitsizeof, OpMode::PreUnary, OpAssoc::Right},
            {qOpAlignof, OpMode::PreUnary, OpAssoc::Right},
            {qOpTypeof, OpMode::PreUnary, OpAssoc::Right},
            {qOpAs, OpMode::Binary, OpAssoc::Right},
            {qOpBitcastAs, OpMode::Binary, OpAssoc::Right},
        },

        {
            {qOpTimes, OpMode::Binary, OpAssoc::Left},
            {qOpSlash, OpMode::Binary, OpAssoc::Left},
            {qOpPercent, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpPlus, OpMode::Binary, OpAssoc::Left},
            {qOpMinus, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpLShift, OpMode::Binary, OpAssoc::Left},
            {qOpRShift, OpMode::Binary, OpAssoc::Left},
            {qOpROTL, OpMode::Binary, OpAssoc::Left},
            {qOpROTR, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpLT, OpMode::Binary, OpAssoc::Left},
            {qOpGT, OpMode::Binary, OpAssoc::Left},
            {qOpLE, OpMode::Binary, OpAssoc::Left},
            {qOpGE, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpEq, OpMode::Binary, OpAssoc::Left},
            {qOpNE, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpBitAnd, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpBitXor, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpBitOr, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpLogicAnd, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpLogicOr, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpLogicXor, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpTernary, OpMode::Ternary, OpAssoc::Right},
            {qOpSet, OpMode::Binary, OpAssoc::Right},
            {qOpPlusSet, OpMode::Binary, OpAssoc::Right},
            {qOpMinusSet, OpMode::Binary, OpAssoc::Right},
            {qOpTimesSet, OpMode::Binary, OpAssoc::Right},
            {qOpSlashSet, OpMode::Binary, OpAssoc::Right},
            {qOpPercentSet, OpMode::Binary, OpAssoc::Right},
            {qOpBitAndSet, OpMode::Binary, OpAssoc::Right},
            {qOpBitOrSet, OpMode::Binary, OpAssoc::Right},
            {qOpBitXorSet, OpMode::Binary, OpAssoc::Right},
            {qOpLogicAndSet, OpMode::Binary, OpAssoc::Right},
            {qOpLogicOrSet, OpMode::Binary, OpAssoc::Right},
            {qOpLogicXorSet, OpMode::Binary, OpAssoc::Right},
            {qOpLShiftSet, OpMode::Binary, OpAssoc::Right},
            {qOpRShiftSet, OpMode::Binary, OpAssoc::Right},
            {qOpROTLSet, OpMode::Binary, OpAssoc::Right},
            {qOpROTRSet, OpMode::Binary, OpAssoc::Right},
        },

        {
            {qOpEllipsis, OpMode::PreUnary, OpAssoc::Right},
        },
};

CPP_EXPORT int ncc::lex::GetOperatorPrecedence(Operator op, OpMode type) {
  using Key = std::pair<Operator, OpMode>;

  struct KeyHash {
    size_t operator()(const Key &k) const {
      return std::hash<Operator>()(k.first) ^ std::hash<OpMode>()(k.second);
    }
  };

  static const std::unordered_map<std::pair<Operator, OpMode>, int, KeyHash>
      precedence = [] {
        std::unordered_map<std::pair<Operator, OpMode>, int, KeyHash>
            precedence;

        for (size_t i = 0; i < precedence_groups.size(); i++) {
          for (let[op, mode, _] : precedence_groups[i]) {
            precedence[{op, mode}] = (precedence_groups.size() - i) * 10;
          }
        }

        return precedence;
      }();

  auto it = precedence.find({op, type});
  if (it != precedence.end()) [[likely]] {
    return it->second;
  }

  return -1;
}

CPP_EXPORT OpAssoc ncc::lex::GetOperatorAssociativity(Operator op,
                                                      OpMode type) {
  using Key = std::pair<Operator, OpMode>;

  struct KeyHash {
    size_t operator()(const Key &k) const {
      return std::hash<Operator>()(k.first) ^ std::hash<OpMode>()(k.second);
    }
  };

  static const std::unordered_map<Key, OpAssoc, KeyHash> associativity = [] {
    std::unordered_map<Key, OpAssoc, KeyHash> associativity;

    for (size_t i = 0; i < precedence_groups.size(); i++) {
      for (let[op, mode, assoc] : precedence_groups[i]) {
        associativity[{op, mode}] = assoc;
      }
    }

    return associativity;
  }();

  auto it = associativity.find({op, type});
  if (it != associativity.end()) [[likely]] {
    return it->second;
  }

  return OpAssoc::Left;
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
