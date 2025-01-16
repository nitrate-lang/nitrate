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
using namespace ncc::lex::detail;

// Lower index means higher precedence
static const std::vector<
    std::vector<std::tuple<Operator, OpMode, Associativity>>>
    PRECEDENCE_GROUPS = {
        {
            {OpDot, OpMode::Binary, Left},
        },

        {
            {OpInc, OpMode::PostUnary, Left},
            {OpDec, OpMode::PostUnary, Left},
        },

        {
            /* Yee, we have enough overloadable operators to last a lifetime */
            {OpPlus, OpMode::PreUnary, Right},
            {OpMinus, OpMode::PreUnary, Right},
            {OpTimes, OpMode::PreUnary, Right},
            {OpSlash, OpMode::PreUnary, Right},
            {OpPercent, OpMode::PreUnary, Right},
            {OpBitAnd, OpMode::PreUnary, Right},
            {OpBitOr, OpMode::PreUnary, Right},
            {OpBitXor, OpMode::PreUnary, Right},
            {OpBitNot, OpMode::PreUnary, Right},
            {OpLShift, OpMode::PreUnary, Right},
            {OpRShift, OpMode::PreUnary, Right},
            {OpROTL, OpMode::PreUnary, Right},
            {OpROTR, OpMode::PreUnary, Right},
            {OpLogicAnd, OpMode::PreUnary, Right},
            {OpLogicOr, OpMode::PreUnary, Right},
            {OpLogicXor, OpMode::PreUnary, Right},
            {OpLogicNot, OpMode::PreUnary, Right},
            {OpLT, OpMode::PreUnary, Right},
            {OpGT, OpMode::PreUnary, Right},
            {OpLE, OpMode::PreUnary, Right},
            {OpGE, OpMode::PreUnary, Right},
            {OpEq, OpMode::PreUnary, Right},
            {OpNE, OpMode::PreUnary, Right},
            {OpSet, OpMode::PreUnary, Right},
            {OpPlusSet, OpMode::PreUnary, Right},
            {OpMinusSet, OpMode::PreUnary, Right},
            {OpTimesSet, OpMode::PreUnary, Right},
            {OpSlashSet, OpMode::PreUnary, Right},
            {OpPercentSet, OpMode::PreUnary, Right},
            {OpBitAndSet, OpMode::PreUnary, Right},
            {OpBitOrSet, OpMode::PreUnary, Right},
            {OpBitXorSet, OpMode::PreUnary, Right},
            {OpLogicAndSet, OpMode::PreUnary, Right},
            {OpLogicOrSet, OpMode::PreUnary, Right},
            {OpLogicXorSet, OpMode::PreUnary, Right},
            {OpLShiftSet, OpMode::PreUnary, Right},
            {OpRShiftSet, OpMode::PreUnary, Right},
            {OpROTLSet, OpMode::PreUnary, Right},
            {OpROTRSet, OpMode::PreUnary, Right},
            {OpInc, OpMode::PreUnary, Right},
            {OpDec, OpMode::PreUnary, Right},
            {OpAs, OpMode::PreUnary, Right},
            {OpBitcastAs, OpMode::PreUnary, Right},
            {OpIn, OpMode::PreUnary, Right},
            {OpOut, OpMode::PreUnary, Right},
            {OpSizeof, OpMode::PreUnary, Right},
            {OpBitsizeof, OpMode::PreUnary, Right},
            {OpAlignof, OpMode::PreUnary, Right},
            {OpTypeof, OpMode::PreUnary, Right},
            {OpComptime, OpMode::PreUnary, Right},
            {OpDot, OpMode::PreUnary, Right},
            {OpRange, OpMode::PreUnary, Right},
            {OpEllipsis, OpMode::PreUnary, Right},
            {OpArrow, OpMode::PreUnary, Right},
            {OpTernary, OpMode::PreUnary, Right},
        },

        {
            {OpAs, OpMode::Binary, Left},
            {OpBitcastAs, OpMode::Binary, Left},
        },

        {
            {OpTimes, OpMode::Binary, Left},
            {OpSlash, OpMode::Binary, Left},
            {OpPercent, OpMode::Binary, Left},
        },

        {
            {OpPlus, OpMode::Binary, Left},
            {OpMinus, OpMode::Binary, Left},
        },

        {
            {OpLShift, OpMode::Binary, Left},
            {OpRShift, OpMode::Binary, Left},
            {OpROTL, OpMode::Binary, Left},
            {OpROTR, OpMode::Binary, Left},
        },

        {
            {OpBitAnd, OpMode::Binary, Left},
        },

        {
            {OpBitXor, OpMode::Binary, Left},
        },

        {
            {OpBitOr, OpMode::Binary, Left},
        },

        {
            /*
              Add soup of new operators like '$>', '~?', etc. Maybe like 90 of
              them? This way we can have a lot of fun with overloading and
              creating basically custom syntax that is meme worthy.
             */
        },

        {
            {OpEq, OpMode::Binary, Left},
            {OpNE, OpMode::Binary, Left},
            {OpLT, OpMode::Binary, Left},
            {OpGT, OpMode::Binary, Left},
            {OpLE, OpMode::Binary, Left},
            {OpGE, OpMode::Binary, Left},
        },

        {
            {OpLogicAnd, OpMode::Binary, Left},
        },

        {
            {OpLogicOr, OpMode::Binary, Left},
        },

        {
            {OpLogicXor, OpMode::Binary, Left},
        },

        {
            {OpTernary, OpMode::Ternary, Right},
        },

        {
            {OpIn, OpMode::Binary, Left},
            {OpOut, OpMode::Binary, Left},
        },

        {
            {OpRange, OpMode::Binary, Left},
            {OpArrow, OpMode::Binary, Left},
        },

        {
            {OpSet, OpMode::Binary, Right},
            {OpPlusSet, OpMode::Binary, Right},
            {OpMinusSet, OpMode::Binary, Right},
            {OpTimesSet, OpMode::Binary, Right},
            {OpSlashSet, OpMode::Binary, Right},
            {OpPercentSet, OpMode::Binary, Right},
            {OpBitAndSet, OpMode::Binary, Right},
            {OpBitOrSet, OpMode::Binary, Right},
            {OpBitXorSet, OpMode::Binary, Right},
            {OpLogicAndSet, OpMode::Binary, Right},
            {OpLogicOrSet, OpMode::Binary, Right},
            {OpLogicXorSet, OpMode::Binary, Right},
            {OpLShiftSet, OpMode::Binary, Right},
            {OpRShiftSet, OpMode::Binary, Right},
            {OpROTLSet, OpMode::Binary, Right},
            {OpROTRSet, OpMode::Binary, Right},
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

        for (size_t i = 0; i < PRECEDENCE_GROUPS.size(); i++) {
          for (let[op, mode, _] : PRECEDENCE_GROUPS[i]) {
            precedence[{op, mode}] = (PRECEDENCE_GROUPS.size() - i);
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

NCC_EXPORT Associativity ncc::lex::GetOperatorAssociativity(Operator op,
                                                            OpMode type) {
  using Key = std::pair<Operator, OpMode>;

  struct KeyHash {
    size_t operator()(const Key &k) const {
      return std::hash<Operator>()(k.first) ^ std::hash<OpMode>()(k.second);
    }
  };

  static const std::unordered_map<Key, Associativity, KeyHash> associativity =
      [] {
        std::unordered_map<Key, Associativity, KeyHash> associativity;

        for (const auto &group : PRECEDENCE_GROUPS) {
          for (let[op, mode, assoc] : group) {
            associativity[{op, mode}] = assoc;
          }
        }

        return associativity;
      }();

  auto it = associativity.find({op, type});
  if (it != associativity.end()) [[likely]] {
    return it->second;
  }

  return Left;
}

NCC_EXPORT ncc::string ncc::lex::to_string(TokenType ty, TokenData v) {
  string r;

  switch (ty) {
    case EofF: {
      r = "";
      break;
    }

    case KeyW: {
      r = ncc::lex::kw_repr(v.m_key);
      break;
    }

    case Oper: {
      r = ncc::lex::op_repr(v.m_op);
      break;
    }

    case Punc: {
      r = ncc::lex::punct_repr(v.m_punc);
      break;
    }

    case Name: {
      r = v.m_str;
      break;
    }

    case IntL: {
      r = v.m_str;
      break;
    }

    case NumL: {
      r = v.m_str;
      break;
    }

    case Text: {
      r = v.m_str;
      break;
    }

    case Char: {
      r = v.m_str;
      break;
    }

    case MacB: {
      r = v.m_str;
      break;
    }

    case Macr: {
      r = v.m_str;
      break;
    }

    case Note: {
      r = v.m_str;
      break;
    }
  }

  return r;
}

NCC_EXPORT Location LocationID::Get(IScanner &l) const {
  return l.GetLocation(m_id);
}

class IScanner::StaticImpl {
public:
  static NCC_FORCE_INLINE void FillTokenBuffer(IScanner &l) {
    for (size_t i = 0; i < kTokenBufferSize; i++) {
      try {
        l.m_ready.push_back(l.GetNext());
      } catch (ScannerEOF &) {
        if (i == 0) {
          l.m_eof = true;
          l.m_ready.push_back(Token::EndOfFile());
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

static uint32_t StringToUint32(std::string_view str, uint32_t sentinal) {
  uint32_t result = sentinal;
  std::from_chars(str.data(), str.data() + str.size(), result);
  return result;
}

Location IScanner::GetEofLocation() {
  uint32_t offset = kLexEof;
  uint32_t line = kLexEof;
  uint32_t column = kLexEof;
  string filename;

  if (auto off = m_env->Get("this.file.eof.offset"); off.has_value()) {
    offset = StringToUint32(off.value(), kLexEof);
  }

  if (auto ln = m_env->Get("this.file.eof.line"); ln.has_value()) {
    line = StringToUint32(ln.value(), kLexEof);
  }

  if (auto col = m_env->Get("this.file.eof.column"); col.has_value()) {
    column = StringToUint32(col.value(), kLexEof);
  }

  if (auto fn = m_env->Get("this.file.eof.filename"); fn.has_value()) {
    filename = fn.value();
  }

  return {offset, line, column, filename};
}

Location IScanner::GetLocation(LocationID id) {
  if (id.GetId() < m_location_interned.size()) {
    return m_location_interned[id.GetId()];
  }

  return GetLocationFallback(id.GetId()).value_or(Location::EndOfFile());
}

Location IScanner::Start(Token t) { return t.get_start().Get(*this); }

Location IScanner::End(Token) {  /// NOLINT
  /// TODO: Support relexing to get the end location
  return Location::EndOfFile();
}

uint32_t IScanner::StartLine(Token t) { return Start(t).GetRow(); }
uint32_t IScanner::StartColumn(Token t) { return Start(t).GetCol(); }
uint32_t IScanner::EndLine(Token t) { return End(t).GetRow(); }
uint32_t IScanner::EndColumn(Token t) { return End(t).GetCol(); }
