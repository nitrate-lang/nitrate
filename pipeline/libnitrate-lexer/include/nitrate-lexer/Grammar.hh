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

#ifndef __NITRATE_LEXER_GRAMMAR_HH__
#define __NITRATE_LEXER_GRAMMAR_HH__

#include <boost/bimap.hpp>
#include <cstdint>
#include <nitrate-lexer/Enums.hh>

namespace ncc::lex {
  namespace detail {
    template <typename L, typename R>
    auto MakeBimap(std::initializer_list<typename boost::bimap<L, R>::value_type> list) -> boost::bimap<L, R> {
      return boost::bimap<L, R>(list.begin(), list.end());
    }

    enum OpType : uint8_t { Both, Unary, Binary, Ternary };

    struct OpConfig {
      OpType m_type;
      bool m_overloadable;

      OpConfig(OpType t, bool o) : m_type(t), m_overloadable(o) {}

      auto operator<(const OpConfig &rhs) const -> bool {
        if (m_type != rhs.m_type) {
          return m_type < rhs.m_type;
        }

        return (int)m_overloadable < (int)rhs.m_overloadable;
      }
    };
  }  // namespace detail

  enum class OpMode : uint8_t { Binary, PreUnary, PostUnary, Ternary };
  enum Associativity : uint8_t { Left, Right };

  inline static const auto LEXICAL_KEYWORDS = detail::MakeBimap<std::string, Keyword>({
      {"scope", Scope},
      {"pub", Pub},
      {"sec", Sec},
      {"pro", Pro},
      {"import", Import},
      {"type", Type},
      {"let", Let},
      {"var", Var},
      {"const", Const},
      {"static", Static},
      {"struct", Struct},
      {"region", Region},
      {"group", Group},
      {"class", Class},
      {"union", Union},
      {"opaque", Opaque},
      {"enum", Enum},
      {"__fstring", __FString},
      {"fn", Fn},
      {"safe", Safe},
      {"unsafe", Unsafe},
      {"pure", Pure},
      {"impure", Impure},
      {"quasi", Quasi},
      {"retro", Retro},
      {"inline", Inline},
      {"foreign", Foreign},
      {"promise", Promise},
      {"if", If},
      {"else", Else},
      {"for", For},
      {"while", While},
      {"do", Do},
      {"switch", Switch},
      {"break", Break},
      {"continue", Continue},
      {"ret", Return},
      {"foreach", Foreach},
      {"try", Try},
      {"catch", Catch},
      {"throw", Throw},
      {"async", Async},
      {"await", Await},
      {"__asm__", __Asm__},
      {"undef", Undef},
      {"null", Null},
      {"true", True},
      {"false", False},
      {"escape_block", EscapeBlock},
      {"unit_assert", UnitAssert},
  });

  inline static const auto LEXICAL_OPERATORS = detail::MakeBimap<std::string, Operator>({
      {"+", OpPlus},
      {"-", OpMinus},
      {"*", OpTimes},
      {"/", OpSlash},
      {"%", OpPercent},
      {"&", OpBitAnd},
      {"|", OpBitOr},
      {"^", OpBitXor},
      {"~", OpBitNot},
      {"<<", OpLShift},
      {">>", OpRShift},
      {"<<<", OpROTL},
      {">>>", OpROTR},
      {"&&", OpLogicAnd},
      {"||", OpLogicOr},
      {"^^", OpLogicXor},
      {"!", OpLogicNot},
      {"<", OpLT},
      {">", OpGT},
      {"<=", OpLE},
      {">=", OpGE},
      {"==", OpEq},
      {"!=", OpNE},
      {"=", OpSet},
      {"+=", OpPlusSet},
      {"-=", OpMinusSet},
      {"*=", OpTimesSet},
      {"/=", OpSlashSet},
      {"%=", OpPercentSet},
      {"&=", OpBitAndSet},
      {"|=", OpBitOrSet},
      {"^=", OpBitXorSet},
      {"&&=", OpLogicAndSet},
      {"||=", OpLogicOrSet},
      {"^^=", OpLogicXorSet},
      {"<<=", OpLShiftSet},
      {">>=", OpRShiftSet},
      {"<<<=", OpROTLSet},
      {">>>=", OpROTRSet},
      {"++", OpInc},
      {"--", OpDec},
      {"as", OpAs},
      {"bitcast_as", OpBitcastAs},
      {"in", OpIn},
      {"out", OpOut},
      {"sizeof", OpSizeof},
      {"bitsizeof", OpBitsizeof},
      {"alignof", OpAlignof},
      {"typeof", OpTypeof},
      {"comptime", OpComptime},
      {".", OpDot},
      {"..", OpRange},
      {"...", OpEllipsis},
      {"=>", OpArrow},
      {"?", OpTernary},
  });

  inline static const auto LEXICAL_PUNCTORS = detail::MakeBimap<std::string, Punctor>({
      {"(", PuncLPar},
      {")", PuncRPar},
      {"[", PuncLBrk},
      {"]", PuncRBrk},
      {"{", PuncLCur},
      {"}", PuncRCur},
      {",", PuncComa},
      {":", PuncColn},
      {";", PuncSemi},
      {"::", PuncScope},
  });

  auto GetOperatorPrecedence(Operator op, OpMode type) -> short;
  auto GetOperatorAssociativity(Operator op, OpMode type) -> Associativity;

  static inline const char *op_repr(Operator op) {  // NOLINT
    return LEXICAL_OPERATORS.right.at(op).c_str();
  }

  static inline const char *kw_repr(Keyword kw) {  // NOLINT
    return LEXICAL_KEYWORDS.right.at(kw).c_str();
  }

  static inline const char *punct_repr(Punctor punct) {  // NOLINT
    return LEXICAL_PUNCTORS.right.at(punct).c_str();
  }

  inline auto operator<<(std::ostream &os, Operator op) -> std::ostream & {
    os << op_repr(op);
    return os;
  }

  inline auto operator<<(std::ostream &os, Keyword kw) -> std::ostream & {
    os << kw_repr(kw);
    return os;
  }

  inline auto operator<<(std::ostream &os, Punctor punct) -> std::ostream & {
    os << punct_repr(punct);
    return os;
  }
}  // namespace ncc::lex

#endif