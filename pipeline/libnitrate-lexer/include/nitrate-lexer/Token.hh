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
    [[nodiscard]] constexpr string GetFilename() const { return m_filename; }
  } __attribute__((packed));

  class LocationID {
  public:
    using Counter = uint32_t;

    constexpr LocationID(Counter id = 0) : m_id(id) {}

    Location Get(IScanner &l) const;
    [[nodiscard]] constexpr Counter GetId() const { return m_id; }

    constexpr bool operator==(const LocationID &rhs) const {
      return m_id == rhs.m_id;
    }

    constexpr bool operator<(const LocationID &rhs) const {
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
    LocationID m_location_id = 0;
    TokenType m_type;

  public:
    TokenData m_v;

    constexpr TokenBase() : m_type(EofF), m_v{OpPlus} {}

    template <class T = Operator>
    constexpr TokenBase(TokenType ty, T val, LocationID start = LocationID())
        : m_location_id(start), m_type(ty), m_v{val} {}

    constexpr static auto EndOfFile() { return TokenBase(); }

    constexpr bool is(TokenType val) const {  /// NOLINT
      return m_type == val;
    }

    constexpr bool operator==(const TokenBase &rhs) const {
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

    [[nodiscard]] auto as_string() const {  /// NOLINT
      return to_string(m_type, m_v);
    }
    [[nodiscard]] auto as_key() const {  /// NOLINT
      return m_v.m_key;
    }
    [[nodiscard]] auto as_op() const {  /// NOLINT
      return m_v.m_op;
    }
    [[nodiscard]] auto as_punc() const {  /// NOLINT
      return m_v.m_punc;
    }
    [[nodiscard]] auto get_start() const {  /// NOLINT
      return m_location_id;
    }
    [[nodiscard]] auto get_type() const {  /// NOLINT
      return m_type;
    }

    constexpr bool operator<(const TokenBase &rhs) const {
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
}  // namespace ncc::lex

namespace std {
  template <>
  struct hash<ncc::lex::LocationID> {
    size_t operator()(const ncc::lex::LocationID &loc) const {
      return loc.GetId();
    }
  };

}  // namespace std

#endif  // __NITRATE_LEXER_TOKEN_H__
