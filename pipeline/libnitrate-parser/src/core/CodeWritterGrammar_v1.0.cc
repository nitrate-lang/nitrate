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

#include <boost/multiprecision/cpp_int.hpp>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Grammar.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>
#include <nitrate-parser/CodeWriter.hh>

using namespace ncc::lex;
using namespace ncc::parse;
using namespace ncc::parse::detail;

static constexpr auto kNumberOfOperators = Op_Last - Op_First + 1;

/// FIXME: Optimize this lookup table to minimize redundant whitespace
static constexpr std::array<char, (kNumberOfOperators * kNumberOfOperators) + 1> kOpOnOpAction = {
    /*
     OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
     ppppppppppppppppppppppppppppppppppppppppppppppppppppppp
     PMTSPBBBBLRRRLLLLLGLGENSPMTSPBBBLLLLRRRIDABIOSBATCDREAT
     liileiiiiSSOOooooTTEEqEeliileiiioooSSOOnesinuiilyooalre
     unmartttthhTTgggg      tunmartttggghhTTcc t tztipmtnlrr
     suescAOXNiiLRiiii       suescAOXiiiiiLR   c  esgep gion
      sshenrooff  cccc       SsshenrocccffSS   a  oinot epwa
         nd rttt  AOXN       eSSSndSrAOXttee   s  fzofi  s r
         t        nroo       teeetSeSnroSStt   t   ef m  i y
                  d rt        tttSetedSree     A   o  e  s
                                 et tSeStt     s   f
                                 t   ete
                                     t t                                */
    "                                                       " /* OpPlus */
    "                                                       " /* OpMinus */
    "                                                       " /* OpTimes */
    "                                                       " /* OpSlash */
    "                                                       " /* OpPercent */
    "                                                       " /* OpBitAnd */
    "                                                       " /* OpBitOr */
    "                                                       " /* OpBitXor */
    "                                                       " /* OpBitNot */
    "                                                       " /* OpLShift */
    "                                                       " /* OpRShift */
    "                                                       " /* OpROTL */
    "                                                       " /* OpROTR */
    "                                                       " /* OpLogicAnd */
    "                                                       " /* OpLogicOr */
    "                                                       " /* OpLogicXor */
    "                                                       " /* OpLogicNot */
    "                                                       " /* OpLT */
    "                                                       " /* OpGT */
    "                                                       " /* OpLE */
    "                                                       " /* OpGE */
    "                                                       " /* OpEq */
    "                                                       " /* OpNE */
    "                                                       " /* OpSet */
    "                                                       " /* OpPlusSet */
    "                                                       " /* OpMinusSet */
    "                                                       " /* OpTimesSet */
    "                                                       " /* OpSlashSet */
    "                                                       " /* OpPercentSet */
    "                                                       " /* OpBitAndSet */
    "                                                       " /* OpBitOrSet */
    "                                                       " /* OpBitXorSet */
    "                                                       " /* OpLogicAndSet */
    "                                                       " /* OpLogicOrSet */
    "                                                       " /* OpLogicXorSet */
    "                                                       " /* OpLShiftSet */
    "                                                       " /* OpRShiftSet */
    "                                                       " /* OpROTLSet */
    "                                                       " /* OpROTRSet */
    "                                                       " /* OpInc */
    "                                                       " /* OpDec */
    "                                                       " /* OpAs */
    "                                                       " /* OpBitcastAs */
    "                                                       " /* OpIn */
    "                                                       " /* OpOut */
    "                                                       " /* OpSizeof */
    "                                                       " /* OpBitsizeof */
    "                                                       " /* OpAlignof */
    "                                                       " /* OpTypeof */
    "                                                       " /* OpComptime */
    "                                                       " /* OpDot */
    "                                                       " /* OpRange */
    "                                                       " /* OpEllipsis */
    "                                                       " /* OpArrow */
    "                                                       " /* OpTernary */
};

static bool IsWordOperator(Operator op) {
  switch (op) {
    case OpAs:
    case OpBitcastAs:
    case OpIn:
    case OpOut:
    case OpSizeof:
    case OpBitsizeof:
    case OpAlignof:
    case OpTypeof:
    case OpComptime:
      return true;

    default:
      return false;
  }
}

static std::string StringEscape(std::string_view str) {
  std::stringstream ss;

  for (char ch : str) {
    switch (ch) {
      case '\n': {
        ss << "\\n";
        break;
      }

      case '\t': {
        ss << "\\t";
        break;
      }

      case '\r': {
        ss << "\\r";
        break;
      }

      case '\v': {
        ss << "\\v";
        break;
      }

      case '\f': {
        ss << "\\f";
        break;
      }

      case '\b': {
        ss << "\\b";
        break;
      }

      case '\a': {
        ss << "\\a";
        break;
      }

      case '\\': {
        ss << "\\\\";
        break;
      }

      case '"': {
        ss << "\\\"";
        break;
      }

      default: {
        ss << ch;
        break;
      }
    }
  }

  return ss.str();
}

void CodeWriter_v1_0::PutKeyword(Keyword kw) {
  switch (m_last) {
    case KeyW:
    case Name:
    case Macr:
    case NumL: {
      m_os << ' ' << kw;
      break;
    }

    case Oper: {
      if (IsWordOperator(m_ldata.m_op)) {
        m_os << ' ' << kw;
      } else {
        m_os << kw;
      }
      break;
    }

    case EofF:
    case Punc:
    case IntL:
    case Text:
    case Char:
    case MacB:
    case Note: {
      m_os << kw;
      break;
    }
  }

  m_last = KeyW;
  m_ldata = kw;
}

void CodeWriter_v1_0::PutOperator(Operator op) {
  switch (m_last) {
    case KeyW:
    case Name:
    case Macr:
    case IntL:
    case NumL: {
      if (IsWordOperator(op)) {
        m_os << ' ' << op;
      } else {
        m_os << op;
      }
      break;
    }

    case Oper: {
      auto table_row = static_cast<size_t>(m_ldata.m_op) - static_cast<size_t>(Op_First);
      auto table_col = static_cast<size_t>(op) - static_cast<size_t>(Op_First);
      auto table_idx = table_row * kNumberOfOperators + table_col;

      if (kOpOnOpAction[table_idx] == ' ') {
        m_os << ' ' << op;
      } else {
        m_os << op;
      }
      break;
    }

    case Punc: {
      m_os << op;
      break;
    }

    case EofF:
    case Text:
    case Char:
    case MacB:
    case Note: {
      m_os << op;
      break;
    }
  }

  m_last = Oper;
  m_ldata = op;
}

void CodeWriter_v1_0::PutPunctor(Punctor punc) {
  switch (m_last) {
    case Oper: {
      m_os << punc;
      break;
    }

    case Punc: {
      if ((punc == PuncColn || punc == PuncScope) && m_ldata.m_punc == PuncColn) {
        m_os << ' ' << punc;
      } else {
        m_os << punc;
      }

      break;
    }

    case IntL:
    case NumL:
    case Macr:
    case EofF:
    case KeyW:
    case Name:
    case Text:
    case Char:
    case MacB:
    case Note: {
      m_os << punc;
      break;
    }
  }

  m_last = Punc;
  m_ldata = punc;
}

void CodeWriter_v1_0::PutIdentifier(std::string_view name) {
  switch (m_last) {
    case Oper: {
      if (IsWordOperator(m_ldata.m_op)) {
        m_os << ' ' << name;
      } else {
        m_os << name;
      }
      break;
    }

    case KeyW:
    case Name:
    case Macr: {
      m_os << ' ' << name;
      break;
    }

    case NumL: {
      /// FIXME: Minimize redundant whitespace
      m_os << ' ' << name;
      break;
    }

    case IntL: {
      bool was_it_hex = m_ldata.m_str.Get().starts_with("0x");
      if (was_it_hex) {
        if (std::isxdigit(name.front()) == 0) {
          m_os << name;
        } else {
          m_os << ' ' << name;
        }
      } else {
        /**
         * Otherwise, it was in decimal. Therefore the allowed character is [0-9].
         * Because identifiers cannot start with a digit, we don't require whitespace.
         */

        m_os << name;
      }
      break;
    }

    case EofF:
    case Punc:
    case Text:
    case Char:
    case MacB:
    case Note: {
      m_os << name;
      break;
    }
  }

  m_last = Name;
  m_ldata = string(name);
}

static const boost::multiprecision::uint128_t FORMAT_SWITCH_THRESHOLD = 1000000000000;

static void WriteSmallestInteger(std::ostream& os, std::string_view num) {
  boost::multiprecision::uint128_t number(num);

  if (number < FORMAT_SWITCH_THRESHOLD) {
    os << num;
    return;
  }

  os << "0x" << std::hex << number << std::dec;
}

void CodeWriter_v1_0::PutInteger(std::string_view num) {
  switch (m_last) {
    case Oper: {
      if (IsWordOperator(m_ldata.m_op)) {
        m_os << ' ';
        WriteSmallestInteger(m_os, num);
      } else {
        WriteSmallestInteger(m_os, num);
      }
      break;
    }

    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr: {
      m_os << ' ';
      WriteSmallestInteger(m_os, num);
      break;
    }

    case EofF:
    case Punc:
    case Text:
    case Char:
    case MacB:
    case Note: {
      WriteSmallestInteger(m_os, num);
      break;
    }
  }

  m_last = IntL;
  m_ldata = string(num);
}

void CodeWriter_v1_0::PutFloat(std::string_view num) {
  /// FIXME: Find algorigm to compress floating point numbers

  switch (m_last) {
    case Oper: {
      if (IsWordOperator(m_ldata.m_op)) {
        m_os << ' ' << num;
      } else {
        m_os << num;
      }
      break;
    }

    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr: {
      m_os << ' ' << num;
      break;
    }

    case EofF:
    case Punc:
    case Text:
    case Char:
    case MacB:
    case Note: {
      m_os << num;
      break;
    }
  }

  m_last = NumL;
  m_ldata = string(num);
}

void CodeWriter_v1_0::PutString(std::string_view str) {
  switch (m_last) {
    case Text: {
      /* Adjacent strings are always concatenated */
      m_os << '"' << StringEscape(str) << '"';
      break;
    }

    case Char:
    case Oper:
    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr:
    case EofF:
    case Punc:
    case MacB:
    case Note: {
      m_os << '"' << StringEscape(str) << '"';
      break;
    }
  }

  m_last = Text;
  m_ldata = string(str);
}

void CodeWriter_v1_0::PutCharacter(uint8_t ch) {
  std::string_view str(reinterpret_cast<const char*>(&ch), 1);

  switch (m_last) {
    case Char: {
      m_os << '\'' << StringEscape(str) << '\'';
      break;
    }

    case Text:
    case Oper:
    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr:
    case EofF:
    case Punc:
    case MacB:
    case Note: {
      m_os << '\'' << StringEscape(str) << '\'';
      break;
    }
  }

  m_last = Char;
  m_ldata = string(str);
}

void CodeWriter_v1_0::PutMacroBlock(std::string_view macro) {
  switch (m_last) {
    case Text:
    case Char:
    case Oper:
    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr:
    case EofF:
    case Punc:
    case MacB:
    case Note: {
      m_os << "@(" << macro << ")";
      break;
    }
  }

  m_last = MacB;
  m_ldata = string(macro);
}

void CodeWriter_v1_0::PutMacroCall(std::string_view macro) {
  switch (m_last) {
    case Text:
    case Char:
    case Oper:
    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr:
    case EofF:
    case Punc:
    case MacB:
    case Note: {
      m_os << "@" << macro;
      break;
    }
  }

  m_last = Macr;
  m_ldata = string(macro);
}

void CodeWriter_v1_0::PutComment(std::string_view note) {
  switch (m_last) {
    case Text:
    case Char:
    case Oper:
    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr:
    case EofF:
    case Punc:
    case MacB:
    case Note: {
      m_os << "/*" << note << "*/";
      break;
    }
  }

  m_last = Note;
  m_ldata = string(note);
}
