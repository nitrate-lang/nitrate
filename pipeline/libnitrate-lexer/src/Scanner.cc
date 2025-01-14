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

#include <charconv>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-lexer/Token.hh>

using namespace ncc::lex;

// Lower index means higher precedence
static const std::vector<std::vector<std::tuple<Operator, OpMode, OpAssoc>>>
    precedence_groups = {
        {
            {OpDot, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpInc, OpMode::PostUnary, OpAssoc::Left},
            {OpDec, OpMode::PostUnary, OpAssoc::Left},
        },

        {
            /* Yee, we have enough overloadable operators to last a lifetime */
            {OpPlus, OpMode::PreUnary, OpAssoc::Right},
            {OpMinus, OpMode::PreUnary, OpAssoc::Right},
            {OpTimes, OpMode::PreUnary, OpAssoc::Right},
            {OpSlash, OpMode::PreUnary, OpAssoc::Right},
            {OpPercent, OpMode::PreUnary, OpAssoc::Right},
            {OpBitAnd, OpMode::PreUnary, OpAssoc::Right},
            {OpBitOr, OpMode::PreUnary, OpAssoc::Right},
            {OpBitXor, OpMode::PreUnary, OpAssoc::Right},
            {OpBitNot, OpMode::PreUnary, OpAssoc::Right},
            {OpLShift, OpMode::PreUnary, OpAssoc::Right},
            {OpRShift, OpMode::PreUnary, OpAssoc::Right},
            {OpROTL, OpMode::PreUnary, OpAssoc::Right},
            {OpROTR, OpMode::PreUnary, OpAssoc::Right},
            {OpLogicAnd, OpMode::PreUnary, OpAssoc::Right},
            {OpLogicOr, OpMode::PreUnary, OpAssoc::Right},
            {OpLogicXor, OpMode::PreUnary, OpAssoc::Right},
            {OpLogicNot, OpMode::PreUnary, OpAssoc::Right},
            {OpLT, OpMode::PreUnary, OpAssoc::Right},
            {OpGT, OpMode::PreUnary, OpAssoc::Right},
            {OpLE, OpMode::PreUnary, OpAssoc::Right},
            {OpGE, OpMode::PreUnary, OpAssoc::Right},
            {OpEq, OpMode::PreUnary, OpAssoc::Right},
            {OpNE, OpMode::PreUnary, OpAssoc::Right},
            {OpSet, OpMode::PreUnary, OpAssoc::Right},
            {OpPlusSet, OpMode::PreUnary, OpAssoc::Right},
            {OpMinusSet, OpMode::PreUnary, OpAssoc::Right},
            {OpTimesSet, OpMode::PreUnary, OpAssoc::Right},
            {OpSlashSet, OpMode::PreUnary, OpAssoc::Right},
            {OpPercentSet, OpMode::PreUnary, OpAssoc::Right},
            {OpBitAndSet, OpMode::PreUnary, OpAssoc::Right},
            {OpBitOrSet, OpMode::PreUnary, OpAssoc::Right},
            {OpBitXorSet, OpMode::PreUnary, OpAssoc::Right},
            {OpLogicAndSet, OpMode::PreUnary, OpAssoc::Right},
            {OpLogicOrSet, OpMode::PreUnary, OpAssoc::Right},
            {OpLogicXorSet, OpMode::PreUnary, OpAssoc::Right},
            {OpLShiftSet, OpMode::PreUnary, OpAssoc::Right},
            {OpRShiftSet, OpMode::PreUnary, OpAssoc::Right},
            {OpROTLSet, OpMode::PreUnary, OpAssoc::Right},
            {OpROTRSet, OpMode::PreUnary, OpAssoc::Right},
            {OpInc, OpMode::PreUnary, OpAssoc::Right},
            {OpDec, OpMode::PreUnary, OpAssoc::Right},
            {OpAs, OpMode::PreUnary, OpAssoc::Right},
            {OpBitcastAs, OpMode::PreUnary, OpAssoc::Right},
            {OpIn, OpMode::PreUnary, OpAssoc::Right},
            {OpOut, OpMode::PreUnary, OpAssoc::Right},
            {OpSizeof, OpMode::PreUnary, OpAssoc::Right},
            {OpBitsizeof, OpMode::PreUnary, OpAssoc::Right},
            {OpAlignof, OpMode::PreUnary, OpAssoc::Right},
            {OpTypeof, OpMode::PreUnary, OpAssoc::Right},
            {OpComptime, OpMode::PreUnary, OpAssoc::Right},
            {OpDot, OpMode::PreUnary, OpAssoc::Right},
            {OpRange, OpMode::PreUnary, OpAssoc::Right},
            {OpEllipsis, OpMode::PreUnary, OpAssoc::Right},
            {OpArrow, OpMode::PreUnary, OpAssoc::Right},
            {OpTernary, OpMode::PreUnary, OpAssoc::Right},
        },

        {
            {OpAs, OpMode::Binary, OpAssoc::Left},
            {OpBitcastAs, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpTimes, OpMode::Binary, OpAssoc::Left},
            {OpSlash, OpMode::Binary, OpAssoc::Left},
            {OpPercent, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpPlus, OpMode::Binary, OpAssoc::Left},
            {OpMinus, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpLShift, OpMode::Binary, OpAssoc::Left},
            {OpRShift, OpMode::Binary, OpAssoc::Left},
            {OpROTL, OpMode::Binary, OpAssoc::Left},
            {OpROTR, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpBitAnd, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpBitXor, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpBitOr, OpMode::Binary, OpAssoc::Left},
        },

        {
            /*
              Add soup of new operators like '$>', '~?', etc. Maybe like 90 of
              them? This way we can have a lot of fun with overloading and
              creating basically custom syntax that is meme worthy.
             */
        },

        {
            {OpEq, OpMode::Binary, OpAssoc::Left},
            {OpNE, OpMode::Binary, OpAssoc::Left},
            {OpLT, OpMode::Binary, OpAssoc::Left},
            {OpGT, OpMode::Binary, OpAssoc::Left},
            {OpLE, OpMode::Binary, OpAssoc::Left},
            {OpGE, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpLogicAnd, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpLogicOr, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpLogicXor, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpTernary, OpMode::Ternary, OpAssoc::Right},
        },

        {
            {OpIn, OpMode::Binary, OpAssoc::Left},
            {OpOut, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpRange, OpMode::Binary, OpAssoc::Left},
            {OpArrow, OpMode::Binary, OpAssoc::Left},
        },

        {
            {OpSet, OpMode::Binary, OpAssoc::Right},
            {OpPlusSet, OpMode::Binary, OpAssoc::Right},
            {OpMinusSet, OpMode::Binary, OpAssoc::Right},
            {OpTimesSet, OpMode::Binary, OpAssoc::Right},
            {OpSlashSet, OpMode::Binary, OpAssoc::Right},
            {OpPercentSet, OpMode::Binary, OpAssoc::Right},
            {OpBitAndSet, OpMode::Binary, OpAssoc::Right},
            {OpBitOrSet, OpMode::Binary, OpAssoc::Right},
            {OpBitXorSet, OpMode::Binary, OpAssoc::Right},
            {OpLogicAndSet, OpMode::Binary, OpAssoc::Right},
            {OpLogicOrSet, OpMode::Binary, OpAssoc::Right},
            {OpLogicXorSet, OpMode::Binary, OpAssoc::Right},
            {OpLShiftSet, OpMode::Binary, OpAssoc::Right},
            {OpRShiftSet, OpMode::Binary, OpAssoc::Right},
            {OpROTLSet, OpMode::Binary, OpAssoc::Right},
            {OpROTRSet, OpMode::Binary, OpAssoc::Right},
        },
};

NCC_EXPORT short ncc::lex::GetOperatorPrecedence(Operator op, OpMode type) {
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

NCC_EXPORT OpAssoc ncc::lex::GetOperatorAssociativity(Operator op,
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

NCC_EXPORT ncc::string ncc::lex::to_string(TokenType ty, TokenData v) {
  string R;

  switch (ty) {
    case EofF: {
      R = "";
      break;
    }

    case KeyW: {
      R = ncc::lex::kw_repr(v.key);
      break;
    }

    case Oper: {
      R = ncc::lex::op_repr(v.op);
      break;
    }

    case Punc: {
      R = ncc::lex::punct_repr(v.punc);
      break;
    }

    case Name: {
      R = v.str.get();
      break;
    }

    case IntL: {
      R = v.str.get();
      break;
    }

    case NumL: {
      R = v.str.get();
      break;
    }

    case Text: {
      R = v.str.get();
      break;
    }

    case Char: {
      R = v.str.get();
      break;
    }

    case MacB: {
      R = v.str.get();
      break;
    }

    case Macr: {
      R = v.str.get();
      break;
    }

    case Note: {
      R = v.str.get();
      break;
    }
  }

  return R;
}

NCC_EXPORT Location LocationID::Get(IScanner &L) const {
  return L.GetLocation(m_id);
}

class IScanner::StaticImpl {
public:
  static NCC_FORCE_INLINE void FillTokenBuffer(IScanner &L) {
    for (size_t i = 0; i < TOKEN_BUFFER_SIZE; i++) {
      try {
        L.m_ready.push_back(L.GetNext());
      } catch (ScannerEOF &) {
        if (i == 0) {
          L.m_eof = true;
          L.m_ready.push_back(Token::EndOfFile());
        }
        break;
      }
    }
  }
};

Token IScanner::Next() {
  while (true) {
    if (m_ready.empty()) [[unlikely]] {
      StaticImpl::FillTokenBuffer(*this);
    }

    Token tok = m_ready.front();
    m_ready.pop_front();

    if (GetSkipCommentsState() && tok.is(Note)) {
      m_comments.push_back(tok);
      continue;
    }

    m_current = tok;

    return tok;
  }
}

Token IScanner::Peek() {
  while (true) {
    if (m_ready.empty()) [[unlikely]] {
      StaticImpl::FillTokenBuffer(*this);
    }

    Token tok = m_ready.front();

    if (GetSkipCommentsState() && tok.is(Note)) {
      m_comments.push_back(tok);
      m_ready.pop_front();
      continue;
    }

    m_current = tok;

    return tok;
  }
}

void IScanner::Insert(Token tok) {
  m_ready.push_front(tok);
  m_current = tok;
}

static uint32_t strtou64(std::string_view str, uint32_t sentinal) {
  uint32_t result = sentinal;
  std::from_chars(str.data(), str.data() + str.size(), result);
  return result;
}

Location IScanner::GetEofLocation() {
  uint32_t offset = QLEX_EOFF, line = QLEX_EOFF, column = QLEX_EOFF;
  string filename;

  if (auto off = m_env->get("this.file.eof.offset"); off.has_value()) {
    offset = strtou64(off.value(), QLEX_EOFF);
  }

  if (auto ln = m_env->get("this.file.eof.line"); ln.has_value()) {
    line = strtou64(ln.value(), QLEX_EOFF);
  }

  if (auto col = m_env->get("this.file.eof.column"); col.has_value()) {
    column = strtou64(col.value(), QLEX_EOFF);
  }

  if (auto fn = m_env->get("this.file.eof.filename"); fn.has_value()) {
    filename = fn.value();
  }

  return Location(offset, line, column, filename);
}

Location IScanner::GetLocation(LocationID id) {
  if (id.GetId() < m_location_interned.size()) {
    return m_location_interned[id.GetId()];
  } else {
    return GetLocationFallback(id.GetId()).value_or(Location::EndOfFile());
  }
}

Location IScanner::Start(Token t) { return t.get_start().Get(*this); }

Location IScanner::End(Token) {
  /// TODO: Support relexing to get the end location
  return Location::EndOfFile();
}

uint32_t IScanner::StartLine(Token t) { return Start(t).GetRow(); }
uint32_t IScanner::StartColumn(Token t) { return Start(t).GetCol(); }
uint32_t IScanner::EndLine(Token t) { return End(t).GetRow(); }
uint32_t IScanner::EndColumn(Token t) { return End(t).GetCol(); }
