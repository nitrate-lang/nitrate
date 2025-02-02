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
#include <nlohmann/json.hpp>

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

using PrecedenceKey = std::pair<Operator, OpMode>;

struct PrecedenceKeyHash {
  auto operator()(const PrecedenceKey &k) const -> size_t {
    return std::hash<Operator>()(k.first) ^ std::hash<OpMode>()(k.second);
  }
};

static const std::unordered_map<PrecedenceKey, short, PrecedenceKeyHash>
    PRECEDENCE = [] {
      std::unordered_map<PrecedenceKey, short, PrecedenceKeyHash> precedence;

      for (size_t i = 0; i < PRECEDENCE_GROUPS.size(); i++) {
        for (let[op, mode, _] : PRECEDENCE_GROUPS[i]) {
          precedence[{op, mode}] = (PRECEDENCE_GROUPS.size() - i) * 10;
        }
      }

      return precedence;
    }();

NCC_EXPORT auto ncc::lex::GetOperatorPrecedence(Operator op,
                                                OpMode type) -> short {
  auto it = PRECEDENCE.find({op, type});
  if (it != PRECEDENCE.end()) [[likely]] {
    return it->second;
  }

  return -1;
}

using AssociativityKey = std::pair<Operator, OpMode>;

struct AssociativityKeyHash {
  auto operator()(const AssociativityKey &k) const -> size_t {
    return std::hash<Operator>()(k.first) ^ std::hash<OpMode>()(k.second);
  }
};

static const std::unordered_map<AssociativityKey, Associativity,
                                AssociativityKeyHash>
    ASSOCIATIVITY = [] {
      std::unordered_map<AssociativityKey, Associativity, AssociativityKeyHash>
          associativity;

      for (const auto &group : PRECEDENCE_GROUPS) {
        for (let[op, mode, assoc] : group) {
          associativity[{op, mode}] = assoc;
        }
      }

      return associativity;
    }();

NCC_EXPORT auto ncc::lex::GetOperatorAssociativity(Operator op, OpMode type)
    -> Associativity {
  auto it = ASSOCIATIVITY.find({op, type});
  if (it != ASSOCIATIVITY.end()) [[likely]] {
    return it->second;
  }

  return Left;
}

NCC_EXPORT auto ncc::lex::to_string(TokenType ty, TokenData v) -> ncc::string {
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

NCC_EXPORT auto LocationID::Get(IScanner &l) const -> Location {
  return l.GetLocation(LocationID(m_id));
}

NCC_EXPORT auto ncc::lex::to_string(TokenType ty) -> string {
  switch (ty) {
    case EofF:
      return "eof";
    case KeyW:
      return "key";
    case Oper:
      return "op";
    case Punc:
      return "sym";
    case Name:
      return "name";
    case IntL:
      return "int";
    case NumL:
      return "num";
    case Text:
      return "str";
    case Char:
      return "char";
    case MacB:
      return "macb";
    case Macr:
      return "macr";
    case Note:
      return "note";
  }
}

NCC_EXPORT auto ncc::lex::operator<<(std::ostream &os,
                                     Token tok) -> std::ostream & {
  // Serialize the token so that the core logger system can use it

  {
    nlohmann::json j;
    j["type"] = (int)tok.GetKind();

    if (tok.GetStart().has_value()) {
      j["pos"] = tok.GetStart().GetId();
    } else {
      j["pos"] = nullptr;
    }

    std::string s = j.dump();
    os << "$TOKEN{" << s.size() << s << "}";
  }

  return os;
}
