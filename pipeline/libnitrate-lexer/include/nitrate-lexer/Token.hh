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

#ifndef __NITRATE_LEXER_TOKEN_HH__
#define __NITRATE_LEXER_TOKEN_HH__

#include <boost/bimap.hpp>
#include <cstdint>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/String.hh>
#include <type_traits>

namespace ncc::lex {
  enum TokenType : uint8_t {
    EofF = 1, /* End of file */
    KeyW,     /* Keyword */
    Oper,     /* Operator */
    Punc,     /* Punctuation */
    Name,     /* Identifier */
    IntL,     /* Integer literal */
    NumL,     /* Floating-point literal */
    Text,     /* String literal */
    Char,     /* Character literal */
    MacB,     /* Macro block */
    Macr,     /* Macro call */
    Note,     /* Comment */
  };

  enum Punctor : uint8_t {
    PuncLPar, /* Left parenthesis */
    PuncRPar, /* Right parenthesis */
    PuncLBrk, /* Left bracket */
    PuncRBrk, /* Right bracket */
    PuncLCur, /* Left curly brace */
    PuncRCur, /* Right curly brace */
    PuncComa, /* Comma */
    PuncColn, /* Colon */
    PuncSemi, /* Semicolon */
    PuncScope /* Scope resolution */
  };

  enum Operator : uint8_t {
    OpPlus,        /* '+':    Addition operator */
    OpMinus,       /* '-':    Subtraction operator */
    OpTimes,       /* '*':    Multiplication operator */
    OpSlash,       /* '/':    Division operator */
    OpPercent,     /* '%':    Modulus operator */
    OpBitAnd,      /* '&':    Bitwise AND operator */
    OpBitOr,       /* '|':    Bitwise OR operator */
    OpBitXor,      /* '^':    Bitwise XOR operator */
    OpBitNot,      /* '~':    Bitwise NOT operator */
    OpLShift,      /* '<<':   Left shift operator */
    OpRShift,      /* '>>':   Right shift operator */
    OpROTL,        /* '<<<':  Rotate left operator */
    OpROTR,        /* '>>>':  Rotate right operator */
    OpLogicAnd,    /* '&&':   Logical AND operator */
    OpLogicOr,     /* '||':   Logical OR operator */
    OpLogicXor,    /* '^^':   Logical XOR operator */
    OpLogicNot,    /* '!':    Logical NOT operator */
    OpLT,          /* '<':    Less than operator */
    OpGT,          /* '>':    Greater than operator */
    OpLE,          /* '<=':   Less than or equal to operator */
    OpGE,          /* '>=':   Greater than or equal to operator */
    OpEq,          /* '==':   Equal to operator */
    OpNE,          /* '!=':   Not equal to operator */
    OpSet,         /* '=':    Assignment operator */
    OpPlusSet,     /* '+=':   Addition assignment operator */
    OpMinusSet,    /* '-=':   Subtraction assignment operator */
    OpTimesSet,    /* '*=':   Multiplication assignment operator */
    OpSlashSet,    /* '/=':   Division assignment operator */
    OpPercentSet,  /* '%=':   Modulus assignment operator */
    OpBitAndSet,   /* '&=':   Bitwise AND assignment operator */
    OpBitOrSet,    /* '|=':   Bitwise OR assignment operator */
    OpBitXorSet,   /* '^=':   Bitwise XOR assignment operator */
    OpLogicAndSet, /* '&&=':  Logical AND assignment operator */
    OpLogicOrSet,  /* '||=':  Logical OR assignment operator */
    OpLogicXorSet, /* '^^=':  Logical XOR assignment operator */
    OpLShiftSet,   /* '<<=':  Left shift assignment operator */
    OpRShiftSet,   /* '>>=':  Right shift assignment operator */
    OpROTLSet,     /* '<<<=': Rotate left assignment operator */
    OpROTRSet,     /* '>>>=': Rotate right assignment operator */
    OpInc,         /* '++':   Increment operator */
    OpDec,         /* '--':   Decrement operator */
    OpAs,          /* 'as':   Type cast operator */
    OpBitcastAs,   /* 'bitcast_as': Bitcast operator */
    OpIn,          /* 'in':         Generic membership operator */
    OpOut,         /* 'out':        Output operator */
    OpSizeof,      /* 'sizeof':     Size of operator */
    OpBitsizeof,   /* 'bitsizeof':  Bit size of operator */
    OpAlignof,     /* 'alignof':    Alignment of operator */
    OpTypeof,      /* 'typeof':     Type of operator */
    OpComptime,    /* 'comptime':   Compile-time operator */
    OpDot,         /* '.':          Dot operator */
    OpRange,       /* '..':         Range operator */
    OpEllipsis,    /* '...':        Ellipsis operator */
    OpArrow,       /* '=>':         Arrow operator */
    OpTernary,     /* '?':          Ternary operator */
  };

  enum Keyword : uint8_t {
    Scope,     /* 'scope' */
    Pub,       /* 'pub' */
    Sec,       /* 'sec' */
    Pro,       /* 'pro' */
    Type,      /* 'type' */
    Let,       /* 'let' */
    Var,       /* 'var' */
    Const,     /* 'const' */
    Static,    /* 'static' */
    Struct,    /* 'struct' */
    Region,    /* 'region' */
    Group,     /* 'group' */
    Class,     /* 'class' */
    Union,     /* 'union' */
    Opaque,    /* 'opaque' */
    Enum,      /* 'enum' */
    __FString, /* '__fstring' */
    Fn,        /* 'fn' */
    Unsafe,    /* 'unsafe' */
    Safe,      /* 'safe' */
    Promise,   /* 'promise' */
    If,        /* 'if' */
    Else,      /* 'else' */
    For,       /* 'for' */
    While,     /* 'while' */
    Do,        /* 'do' */
    Switch,    /* 'switch' */
    Break,     /* 'break' */
    Continue,  /* 'continue' */
    Return,    /* 'ret' */
    Retif,     /* 'retif' */
    Foreach,   /* 'foreach' */
    Try,       /* 'try' */
    Catch,     /* 'catch' */
    Throw,     /* 'throw' */
    Async,     /* 'async' */
    Await,     /* 'await' */
    __Asm__,   /* '__asm__' */
    Undef,     /* 'undef' */
    Null,      /* 'null' */
    True,      /* 'true' */
    False,     /* 'false' */
  };

  constexpr size_t kLexEof = UINT32_MAX;
  constexpr size_t kLexNoFile = 16777215;

  class IScanner;

  class Location {
    uint32_t m_offset = kLexEof, m_line = kLexEof, m_column = kLexEof;
    string m_filename;

  public:
    constexpr Location() = default;

    constexpr Location(uint32_t offset, uint32_t line, uint32_t column,
                       string filename)
        : m_offset(offset),
          m_line(line),
          m_column(column),
          m_filename(filename) {}

    static constexpr auto EndOfFile() {
      return Location(kLexEof, kLexEof, kLexEof, "");
    }

    [[nodiscard]] constexpr auto GetOffset() const { return m_offset; }
    [[nodiscard]] constexpr auto GetRow() const { return m_line; }
    [[nodiscard]] constexpr auto GetCol() const { return m_column; }
    [[nodiscard]] constexpr auto GetFilename() const -> string {
      return m_filename;
    }
  } __attribute__((packed));

  class LocationID {
  public:
    using Counter = uint32_t;

    constexpr LocationID(Counter id = 0) : m_id(id) {}

    auto Get(IScanner &l) const -> Location;
    [[nodiscard]] constexpr auto GetId() const -> Counter { return m_id; }

    constexpr auto operator==(const LocationID &rhs) const -> bool {
      return m_id == rhs.m_id;
    }

    constexpr auto operator<(const LocationID &rhs) const -> bool {
      return m_id < rhs.m_id;
    }

  private:
    Counter m_id;
  } __attribute__((packed));

  using LocationRange = std::pair<LocationID, LocationID>;

  union TokenData {
    Punctor m_punc;
    Operator m_op;
    Keyword m_key;
    string m_str;

    constexpr TokenData(Punctor punc) : m_punc(punc) {}
    constexpr TokenData(Operator op) : m_op(op) {}
    constexpr TokenData(Keyword key) : m_key(key) {}
    constexpr TokenData(string str) : m_str(str) {}
  } __attribute__((packed));

  string to_string(TokenType, TokenData);  /// NOLINT

  class TokenBase {
    TokenType m_type;
    LocationID m_location_id = 0;

  public:
    TokenData m_v;

    constexpr TokenBase() : m_type(EofF), m_v{OpPlus} {}

    template <class T = Operator>
    constexpr TokenBase(TokenType ty, T val, LocationID start = LocationID())
        : m_type(ty), m_location_id(start), m_v{val} {}

    constexpr static auto EndOfFile() { return TokenBase(); }

    constexpr bool is(TokenType val) const {  /// NOLINT
      return m_type == val;
    }

    constexpr auto operator==(const TokenBase &rhs) const -> bool {
      if (m_type != rhs.m_type) {
        return false;
      }

      switch (m_type) {
        case EofF:
        case Punc:
          return m_v.m_punc == rhs.m_v.m_punc;
        case Oper:
          return m_v.m_op == rhs.m_v.m_op;
        case KeyW:
          return m_v.m_key == rhs.m_v.m_key;
        case IntL:
        case NumL:
        case Text:
        case Name:
        case Char:
        case MacB:
        case Macr:
        case Note:
          return m_v.m_str == rhs.m_v.m_str;
      }
    }

    template <auto V>
    [[nodiscard]] constexpr bool is() const {  /// NOLINT
      if constexpr (std::is_same_v<decltype(V), Keyword>) {
        return m_type == KeyW && m_v.m_key == V;
      } else if constexpr (std::is_same_v<decltype(V), Punctor>) {
        return m_type == Punc && m_v.m_punc == V;
      } else if constexpr (std::is_same_v<decltype(V), Operator>) {
        return m_type == Oper && m_v.m_op == V;
      }
    }

    [[nodiscard]] constexpr auto GetString() const {
      return to_string(m_type, m_v);
    }
    [[nodiscard]] constexpr auto GetKeyword() const { return m_v.m_key; }
    [[nodiscard]] constexpr auto GetOperator() const { return m_v.m_op; }
    [[nodiscard]] constexpr auto GetPunctor() const { return m_v.m_punc; }
    [[nodiscard]] constexpr auto GetStart() const { return m_location_id; }
    [[nodiscard]] constexpr auto GetKind() const { return m_type; }

    constexpr auto operator<(const TokenBase &rhs) const -> bool {
      if (m_type != rhs.m_type) {
        return m_type < rhs.m_type;
      }

      switch (m_type) {
        case EofF:
          return false;
        case Punc:
          return m_v.m_punc < rhs.m_v.m_punc;
        case Oper:
          return m_v.m_op < rhs.m_v.m_op;
        case KeyW:
          return m_v.m_key < rhs.m_v.m_key;
        case IntL:
        case NumL:
        case Text:
        case Name:
        case Char:
        case MacB:
        case Macr:
        case Note:
          return m_v.m_str < rhs.m_v.m_str;
      }
    }
  } __attribute__((packed));

  using Token = TokenBase;

  namespace detail {
    template <typename L, typename R>
    auto MakeBimap(
        std::initializer_list<typename boost::bimap<L, R>::value_type> list)
        -> boost::bimap<L, R> {
      return boost::bimap<L, R>(list.begin(), list.end());
    }

    struct ScannerEOF final {};

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

  inline static const auto LEXICAL_KEYWORDS =
      detail::MakeBimap<std::string, Keyword>({
          {"scope", Scope},
          {"pub", Pub},
          {"sec", Sec},
          {"pro", Pro},
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
          {"unsafe", Unsafe},
          {"safe", Safe},
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
          {"retif", Retif},
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
      });

  inline static const auto LEXICAL_OPERATORS =
      detail::MakeBimap<std::string, Operator>({
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

  inline static const auto LEXICAL_OPERATORS_CONFIG =
      detail::MakeBimap<Operator, detail::OpConfig>({
          {OpPlus, {detail::Both, true}},
          {OpMinus, {detail::Both, true}},
          {OpTimes, {detail::Both, true}},
          {OpSlash, {detail::Binary, true}},
          {OpPercent, {detail::Binary, true}},
          {OpBitAnd, {detail::Both, true}},
          {OpBitOr, {detail::Binary, true}},
          {OpBitXor, {detail::Binary, true}},
          {OpBitNot, {detail::Unary, true}},
          {OpLShift, {detail::Binary, true}},
          {OpRShift, {detail::Binary, true}},
          {OpROTL, {detail::Binary, true}},
          {OpROTR, {detail::Binary, true}},
          {OpLogicAnd, {detail::Binary, true}},
          {OpLogicOr, {detail::Binary, true}},
          {OpLogicXor, {detail::Binary, true}},
          {OpLogicNot, {detail::Unary, true}},
          {OpLT, {detail::Binary, true}},
          {OpGT, {detail::Binary, true}},
          {OpLE, {detail::Binary, true}},
          {OpGE, {detail::Binary, true}},
          {OpEq, {detail::Binary, true}},
          {OpNE, {detail::Binary, true}},
          {OpSet, {detail::Binary, true}},
          {OpPlusSet, {detail::Binary, true}},
          {OpMinusSet, {detail::Binary, true}},
          {OpTimesSet, {detail::Binary, true}},
          {OpSlashSet, {detail::Binary, true}},
          {OpPercentSet, {detail::Binary, true}},
          {OpBitAndSet, {detail::Binary, true}},
          {OpBitOrSet, {detail::Binary, true}},
          {OpBitXorSet, {detail::Binary, true}},
          {OpLogicAndSet, {detail::Binary, true}},
          {OpLogicOrSet, {detail::Binary, true}},
          {OpLogicXorSet, {detail::Binary, true}},
          {OpLShiftSet, {detail::Binary, true}},
          {OpRShiftSet, {detail::Binary, true}},
          {OpROTLSet, {detail::Binary, true}},
          {OpROTRSet, {detail::Binary, true}},
          {OpInc, {detail::Unary, true}},
          {OpDec, {detail::Unary, true}},
          {OpAs, {detail::Binary, true}},
          {OpBitcastAs, {detail::Binary, false}},
          {OpIn, {detail::Both, false}},
          {OpOut, {detail::Both, false}},
          {OpSizeof, {detail::Unary, false}},
          {OpBitsizeof, {detail::Unary, false}},
          {OpAlignof, {detail::Unary, false}},
          {OpTypeof, {detail::Unary, false}},
          {OpComptime, {detail::Unary, false}},
          {OpDot, {detail::Binary, false}},
          {OpRange, {detail::Binary, true}},
          {OpEllipsis, {detail::Unary, false}},
          {OpArrow, {detail::Binary, false}},
          {OpTernary, {detail::Ternary, false}},
      });

  inline static const auto LEXICAL_PUNCTORS =
      detail::MakeBimap<std::string, Punctor>({
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
  string to_string(TokenType ty);  /// NOLINT

  static inline const char *op_repr(Operator op) {  /// NOLINT
    return LEXICAL_OPERATORS.right.at(op).c_str();
  }

  static inline const char *kw_repr(Keyword kw) {  /// NOLINT
    return LEXICAL_KEYWORDS.right.at(kw).c_str();
  }

  static inline const char *punct_repr(Punctor punct) {  /// NOLINT
    return LEXICAL_PUNCTORS.right.at(punct).c_str();
  }

  auto operator<<(std::ostream &os, TokenType ty) -> std::ostream &;
  auto operator<<(std::ostream &os, Token tok) -> std::ostream &;

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

namespace std {
  template <>
  struct hash<ncc::lex::LocationID> {
    auto operator()(const ncc::lex::LocationID &loc) const -> size_t {
      return loc.GetId();
    }
  };

  template <>
  struct hash<ncc::lex::Token> {
    constexpr auto operator()(const ncc::lex::Token &tok) const -> size_t {
      size_t h = 0;

      switch (tok.GetKind()) {
        case ncc::lex::TokenType::EofF: {
          h = std::hash<uint8_t>{}(0);
          break;
        }

        case ncc::lex::TokenType::KeyW: {
          h = std::hash<uint8_t>{}(1);
          h ^= std::hash<uint8_t>{}(tok.GetKeyword());
          break;
        }

        case ncc::lex::TokenType::Oper: {
          h = std::hash<uint8_t>{}(2);
          h ^= std::hash<uint8_t>{}(tok.GetOperator());
          break;
        }

        case ncc::lex::TokenType::Punc: {
          h = std::hash<uint8_t>{}(3);
          h ^= std::hash<uint8_t>{}(tok.GetPunctor());
          break;
        }

        case ncc::lex::TokenType::Name:
        case ncc::lex::TokenType::IntL:
        case ncc::lex::TokenType::NumL:
        case ncc::lex::TokenType::Text:
        case ncc::lex::TokenType::Char:
        case ncc::lex::TokenType::MacB:
        case ncc::lex::TokenType::Macr:
        case ncc::lex::TokenType::Note: {
          h = std::hash<uint8_t>{}(4);
          h ^= std::hash<ncc::string>{}(tok.GetString());
          break;
        }
      }

      return h;
    }
  };
}  // namespace std

#endif  // __NITRATE_LEXER_TOKEN_H__
