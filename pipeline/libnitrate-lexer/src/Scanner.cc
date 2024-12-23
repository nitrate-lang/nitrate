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

// Lower index means higher precedence
static const std::vector<std::vector<std::tuple<Operator, OpMode, OpAssoc>>>
    precedence_groups = {
        {
            {qOpDot, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpInc, OpMode::PostUnary, OpAssoc::Left},
            {qOpDec, OpMode::PostUnary, OpAssoc::Left},
        },

        {
            /* Yee, we have enough overloadable operators to last a lifetime */
            {qOpPlus, OpMode::PreUnary, OpAssoc::Right},
            {qOpMinus, OpMode::PreUnary, OpAssoc::Right},
            {qOpTimes, OpMode::PreUnary, OpAssoc::Right},
            {qOpSlash, OpMode::PreUnary, OpAssoc::Right},
            {qOpPercent, OpMode::PreUnary, OpAssoc::Right},
            {qOpBitAnd, OpMode::PreUnary, OpAssoc::Right},
            {qOpBitOr, OpMode::PreUnary, OpAssoc::Right},
            {qOpBitXor, OpMode::PreUnary, OpAssoc::Right},
            {qOpBitNot, OpMode::PreUnary, OpAssoc::Right},
            {qOpLShift, OpMode::PreUnary, OpAssoc::Right},
            {qOpRShift, OpMode::PreUnary, OpAssoc::Right},
            {qOpROTL, OpMode::PreUnary, OpAssoc::Right},
            {qOpROTR, OpMode::PreUnary, OpAssoc::Right},
            {qOpLogicAnd, OpMode::PreUnary, OpAssoc::Right},
            {qOpLogicOr, OpMode::PreUnary, OpAssoc::Right},
            {qOpLogicXor, OpMode::PreUnary, OpAssoc::Right},
            {qOpLogicNot, OpMode::PreUnary, OpAssoc::Right},
            {qOpLT, OpMode::PreUnary, OpAssoc::Right},
            {qOpGT, OpMode::PreUnary, OpAssoc::Right},
            {qOpLE, OpMode::PreUnary, OpAssoc::Right},
            {qOpGE, OpMode::PreUnary, OpAssoc::Right},
            {qOpEq, OpMode::PreUnary, OpAssoc::Right},
            {qOpNE, OpMode::PreUnary, OpAssoc::Right},
            {qOpSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpPlusSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpMinusSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpTimesSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpSlashSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpPercentSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpBitAndSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpBitOrSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpBitXorSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpLogicAndSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpLogicOrSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpLogicXorSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpLShiftSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpRShiftSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpROTLSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpROTRSet, OpMode::PreUnary, OpAssoc::Right},
            {qOpInc, OpMode::PreUnary, OpAssoc::Right},
            {qOpDec, OpMode::PreUnary, OpAssoc::Right},
            {qOpAs, OpMode::PreUnary, OpAssoc::Right},
            {qOpBitcastAs, OpMode::PreUnary, OpAssoc::Right},
            {qOpIn, OpMode::PreUnary, OpAssoc::Right},
            {qOpOut, OpMode::PreUnary, OpAssoc::Right},
            {qOpSizeof, OpMode::PreUnary, OpAssoc::Right},
            {qOpBitsizeof, OpMode::PreUnary, OpAssoc::Right},
            {qOpAlignof, OpMode::PreUnary, OpAssoc::Right},
            {qOpTypeof, OpMode::PreUnary, OpAssoc::Right},
            {qOpDot, OpMode::PreUnary, OpAssoc::Right},
            {qOpRange, OpMode::PreUnary, OpAssoc::Right},
            {qOpEllipsis, OpMode::PreUnary, OpAssoc::Right},
            {qOpArrow, OpMode::PreUnary, OpAssoc::Right},
            {qOpTernary, OpMode::PreUnary, OpAssoc::Right},
        },

        {
            {qOpAs, OpMode::Binary, OpAssoc::Left},
            {qOpBitcastAs, OpMode::Binary, OpAssoc::Left},
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
            {qOpBitAnd, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpBitXor, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpBitOr, OpMode::Binary, OpAssoc::Left},
        },

        {
            /*
              Add soup of new operators like '$>', '~?', etc. Maybe like 90 of
              them? This way we can have a lot of fun with overloading and
              creating basically custom syntax that is meme worthy.
             */
        },

        {
            {qOpEq, OpMode::Binary, OpAssoc::Left},
            {qOpNE, OpMode::Binary, OpAssoc::Left},
            {qOpLT, OpMode::Binary, OpAssoc::Left},
            {qOpGT, OpMode::Binary, OpAssoc::Left},
            {qOpLE, OpMode::Binary, OpAssoc::Left},
            {qOpGE, OpMode::Binary, OpAssoc::Left},
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
        },

        {
            {qOpIn, OpMode::Binary, OpAssoc::Left},
            {qOpOut, OpMode::Binary, OpAssoc::Left},
        },

        {
            {qOpRange, OpMode::Binary, OpAssoc::Left},
            {qOpArrow, OpMode::Binary, OpAssoc::Left},
        },

        {
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
};

CPP_EXPORT short ncc::lex::GetOperatorPrecedence(Operator op, OpMode type) {
  using Key = std::pair<Operator, OpMode>;

  struct KeyHash {
    size_t operator()(const Key &k) const {
      return std::hash<Operator>()(k.first) ^ std::hash<OpMode>()(k.second);
    }
  };

  static const std::unordered_map<std::pair<Operator, OpMode>, short, KeyHash>
      precedence = [] {
        std::unordered_map<std::pair<Operator, OpMode>, short, KeyHash>
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

class IScanner::StaticImpl {
public:
  static FORCE_INLINE void FillTokenBuffer(IScanner &L) {
    for (size_t i = 0; i < TOKEN_BUFFER_SIZE; i++) {
      try {
        L.m_ready.push_back(L.GetNext());
      } catch (ScannerEOF &) {
        if (i == 0) {
          L.m_ready.push_back(Token::EndOfFile());
        }
        break;
      }
    }
  }

  static FORCE_INLINE void FlushInternedBuffering(IScanner &L) {
    L.m_location_interned.insert(L.m_location_interned_buffer.begin(),
                                 L.m_location_interned_buffer.end());

    L.m_location_interned_buffer.clear();
  }
};

CPP_EXPORT Token IScanner::Next() {
  while (true) {
    if (m_ready.empty()) {
      StaticImpl::FillTokenBuffer(*this);
    }

    Token tok = m_ready.front();
    m_ready.pop_front();

    if (GetSkipCommentsState() && tok.is(qNote)) {
      continue;
    }

    m_current = tok;
    m_last = m_current;

    return tok;
  }
}

CPP_EXPORT Token IScanner::Peek() {
  if (m_ready.empty()) [[unlikely]] {
    StaticImpl::FillTokenBuffer(*this);
  }

  Token tok = m_ready.front();
  m_current = tok;

  return tok;
}

CPP_EXPORT void IScanner::Undo() {
  if (!m_last.has_value()) {
    return;
  }

  m_ready.push_front(m_last.value());
  m_current = m_last;
}

CPP_EXPORT Location LocationID::Get(IScanner &L) const {
  return L.GetLocation(m_id);
}

CPP_EXPORT Location IScanner::GetLocation(LocationID id) {
  IScanner::StaticImpl::FlushInternedBuffering(*this);

  auto it = m_location_interned.find(id);
  if (it == m_location_interned.end()) [[unlikely]] {
    return Location::EndOfFile();
  }

  return it->second;
}

CPP_EXPORT Location IScanner::Start(Token t) {
  IScanner::StaticImpl::FlushInternedBuffering(*this);

  return t.get_start().Get(*this);
}

CPP_EXPORT Location IScanner::End(Token t) {
  IScanner::StaticImpl::FlushInternedBuffering(*this);
  (void)t;

  /// TODO: Implement this
  return Location::EndOfFile();
}

CPP_EXPORT uint32_t IScanner::StartLine(Token t) { return Start(t).GetRow(); }
CPP_EXPORT uint32_t IScanner::StartColumn(Token t) { return Start(t).GetCol(); }
CPP_EXPORT uint32_t IScanner::EndLine(Token t) { return End(t).GetRow(); }
CPP_EXPORT uint32_t IScanner::EndColumn(Token t) { return End(t).GetCol(); }
