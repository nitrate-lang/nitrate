////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_LEX_TOKEN_H__
#define __QUIXCC_LEX_TOKEN_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <variant>

namespace libquixcc {
enum class Keyword {
  Subsystem = 10,
  Import = 11,
  Pub = 12,
  Sec = 13,
  Pro = 14,

  Type = 20,
  Let = 21,
  Var = 22,
  Const = 23,
  Static = 24,

  Struct = 40,
  Region = 41,
  Group = 42,
  Union = 43,
  Opaque = 44,
  Enum = 45,
  FString = 46,
  Impl = 47,

  Fn = 60,
  Noexcept = 61,
  Foreign = 62,
  Impure = 63,
  Tsafe = 64,
  Pure = 65,
  Quasipure = 66,
  Retropure = 67,
  CrashPoint = 68,
  Inline = 69,
  Unsafe = 70,
  Safe = 71,
  In = 72,
  Out = 73,
  Req = 74,

  If = 90,
  Else = 91,
  For = 92,
  While = 93,
  Do = 94,
  Switch = 95,
  Case = 96,
  Default = 97,
  Break = 98,
  Continue = 99,
  Return = 100,
  Retif = 101,
  Retz = 102,
  Retv = 103,
  Form = 104,
  Foreach = 105,

  __Asm__ = 130,

  Void = 140,
  Undef = 141,
  Null = 142,
  True = 143,
  False = 144,
};

enum Punctor {
  OpenParen = 1,
  CloseParen = 2,
  OpenBrace = 3,
  CloseBrace = 4,
  OpenBracket = 5,
  CloseBracket = 6,
  Comma = 8,
  Colon = 9,
  Semicolon = 10,
};

enum class Operator {
  At = 1,
  Question = 2,
  Arrow = 3,
  Dot = 4,

  Plus = 10,
  Minus = 11,
  Multiply = 12,
  Divide = 13,
  Modulo = 14,

  BitwiseAnd = 20,
  BitwiseOr = 21,
  BitwiseXor = 22,
  BitwiseNot = 23,
  LeftShift = 24,
  RightShift = 25,
  RotateRight = 26,
  RotateLeft = 27,

  Increment = 30,
  Decrement = 31,
  Assign = 32,
  PlusAssign = 33,
  MinusAssign = 34,
  MultiplyAssign = 35,
  DivideAssign = 36,
  ModuloAssign = 37,
  BitwiseOrAssign = 38,
  BitwiseAndAssign = 39,
  BitwiseXorAssign = 40,
  XorAssign = 41,
  OrAssign = 42,
  AndAssign = 43,
  LeftShiftAssign = 44,
  RightShiftAssign = 45,

  LogicalNot = 50,
  LogicalAnd = 51,
  LogicalOr = 52,
  LogicalXor = 53,

  LessThan = 60,
  GreaterThan = 61,
  LessThanEqual = 62,
  GreaterThanEqual = 63,
  Equal = 64,
  NotEqual = 65,

  As = 80,
  Is = 81,
  In = 82,
  NotIn = 83,
  Sizeof = 84,
  Alignof = 85,
  Typeof = 86,
  Offsetof = 87,
  Range = 88,
  Ellipsis = 89,
  Spaceship = 90,
  BitcastAs = 91,
  ReinterpretAs = 92,
  Bitsizeof = 93,
};

enum TT {
  tEofF = 0,
  tErro = 1,
  tName = 2,
  tKeyW = 3,
  tOper = 4,
  tPunc = 5,
  tIntL = 6,
  tNumL = 7,
  tText = 8,
  tChar = 9,
  tMacB = 10,
  tMacr = 11,
  tNote = 12,
};

class TLCString {
  static thread_local std::map<std::string, std::unique_ptr<char[]>> m_data;

 public:
  /// @brief Create a new Constant C string whose lifetime is that of the
  /// current thread
  /// @param data Data to copy into the new C string
  /// @return const char* Pointer to the new C string
  static const char *get(const std::string &data) {
    if (!m_data.contains(data)) {
      m_data[data] = std::make_unique<char[]>(data.size() + 1);
      std::copy(data.begin(), data.end(), m_data[data].get());
      m_data[data][data.size()] = '\0';
    }
    return m_data[data].get();
  }
};

class Loc {
 public:
  typedef uint32_t LocPosType;

  Loc(LocPosType line = 1, LocPosType col = 1, std::string_view file = "")
      : m_file(file.data()), m_line(line), m_col(col) {}

  Loc(const Loc &loc) = default;
  Loc(Loc &&loc) = default;
  Loc &operator=(const Loc &loc) = default;

  inline const char *file() const {
    if (m_file == nullptr) return "";
    return m_file;
  }

  inline LocPosType line() const { return m_line; }
  inline LocPosType col() const { return m_col; }
  inline LocPosType &line() { return m_line; }
  inline LocPosType &col() { return m_col; }

  Loc operator-(LocPosType rhs) const;

 private:
  const char *m_file;
  LocPosType m_line;
  LocPosType m_col;
};

#if !defined(NDEBUG)
typedef std::variant<std::string, Punctor, Keyword, Operator> TokVal;
#else
struct TokVal {
  std::string str;
  union TokValVal {
    int val;
    Punctor punctor;
    Keyword keyword;
    Operator op;

    TokValVal(int val = 0) : val(val) {}
  } val;

  TokVal() : val(0) {}
  TokVal(std::string str) : str(str) {}
  TokVal(const char *str) : str(str) {}
  TokVal(Punctor punctor) : val(static_cast<int>(punctor)) {}
  TokVal(Keyword keyword) : val(static_cast<int>(keyword)) {}
  TokVal(Operator op) : val(static_cast<int>(op)) {}

  TokVal(const TokVal &val) {
    str = val.str;
    this->val = val.val;
  }

  TokVal(TokVal &&val) {
    str = std::move(val.str);
    this->val = val.val;
  }
};
#endif

class Token {
  TokVal m_value;
  Loc m_loc;
  TT m_type;

  std::string serialize_human_readable() const;

 public:
  Token(TT type = tErro, TokVal value = TokVal(), Loc loc = Loc());

  Token(const Token &tok) = default;
  Token(Token &&tok) {
    m_value = std::move(tok.m_value);
    m_loc = std::move(tok.m_loc);
    m_type = tok.m_type;
  }

  Token &operator=(const Token &tok) = default;

  inline bool is(TT val) const { return m_type == val; }

  template <typename T, typename V = T>
  bool is(V val) const {
    if constexpr (std::is_same_v<T, std::string>)
      return m_type == tName && as<std::string>() == val;
    else if constexpr (std::is_same_v<T, Keyword>)
      return m_type == tKeyW && as<Keyword>() == val;
    else if constexpr (std::is_same_v<T, Punctor>)
      return m_type == tPunc && as<Punctor>() == val;
    else if constexpr (std::is_same_v<T, Operator>)
      return m_type == tOper && as<Operator>() == val;

    static_assert(std::is_same_v<T, T>, "Invalid type");
  }

  template <typename T>
  const T &as() const {
#if !defined(NDEBUG)
    return std::get<T>(m_value);
#else
    if constexpr (std::is_same_v<T, std::string>) {
      return m_value.str;
    } else if constexpr (std::is_same_v<T, Punctor>) {
      return m_value.val.punctor;
    } else if constexpr (std::is_same_v<T, Keyword>) {
      return m_value.val.keyword;
    } else if constexpr (std::is_same_v<T, Operator>) {
      return m_value.val.op;
    }

    static_assert(std::is_same_v<T, T>, "Invalid type");
#endif
  }

  const Loc &loc() const { return m_loc; }
  inline TT type() const { return m_type; }

  std::string serialize(bool human_readable = true) const;

  inline bool operator==(const Token &rhs) const {
    if (m_type != rhs.m_type) return false;

    switch (m_type) {
      case tName:
      case tText:
      case tChar:
      case tIntL:
        return as<std::string>() == rhs.as<std::string>();
      case tKeyW:
        return as<Keyword>() == rhs.as<Keyword>();
      case tOper:
        return as<Operator>() == rhs.as<Operator>();
      case tPunc:
        return as<Punctor>() == rhs.as<Punctor>();
      default:
        return true;
    }
  }

  inline bool operator<(const Token &rhs) const {
    if (m_type != rhs.m_type) return m_type < rhs.m_type;
    switch (m_type) {
      case tName:
      case tText:
      case tChar:
      case tIntL:
        return as<std::string>() < rhs.as<std::string>();
      case tKeyW:
        return as<Keyword>() < rhs.as<Keyword>();
      case tOper:
        return as<Operator>() < rhs.as<Operator>();
      case tPunc:
        return as<Punctor>() < rhs.as<Punctor>();
      default:
        return false;
    }
  }
};
};  // namespace libquixcc

#endif  // __QUIXCC_LEX_TOKEN_H__