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
#include <nitrate-core/String.hh>
#include <nitrate-lexer/Enums.hh>
#include <nitrate-lexer/Location.hh>
#include <nitrate-lexer/TokenData.hh>
#include <type_traits>

namespace ncc::lex {
  class TokenBase {
    TokenType m_type;
    LocationID m_location_id;

  public:
    TokenData m_v;

    constexpr TokenBase() : m_type(EofF), m_v{OpPlus} {}

    template <class T = Operator>
    constexpr TokenBase(TokenType ty, T val, LocationID start = LocationID())
        : m_type(ty), m_location_id(start), m_v{val} {}

    constexpr TokenBase(TokenType ty, size_t str_len, const char *str_ptr, LocationID start = LocationID())
        : m_type(ty), m_location_id(start), m_v{std::string_view(str_ptr, str_len)} {}

    constexpr TokenBase(Operator op, LocationID start = LocationID()) : m_type(Oper), m_location_id(start), m_v{op} {}

    constexpr TokenBase(Punctor punc, LocationID start = LocationID())
        : m_type(Punc), m_location_id(start), m_v{punc} {}

    constexpr TokenBase(Keyword key, LocationID start = LocationID()) : m_type(KeyW), m_location_id(start), m_v{key} {}

    constexpr TokenBase(uint64_t num, LocationID start = LocationID())
        : m_type(IntL), m_location_id(start), m_v{std::to_string(num)} {}

    constexpr TokenBase(string identifier, LocationID start = LocationID())
        : m_type(Name), m_location_id(start), m_v{identifier} {}

    constexpr static auto EndOfFile() { return TokenBase(); }

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

    [[nodiscard]] constexpr auto Is(TokenType val) const -> bool { return m_type == val; }

    template <auto V>
    [[nodiscard]] constexpr auto Is() const -> bool {
      if constexpr (std::is_same_v<decltype(V), Keyword>) {
        return m_type == KeyW && m_v.m_key == V;
      } else if constexpr (std::is_same_v<decltype(V), Punctor>) {
        return m_type == Punc && m_v.m_punc == V;
      } else if constexpr (std::is_same_v<decltype(V), Operator>) {
        return m_type == Oper && m_v.m_op == V;
      }
    }

    [[nodiscard]] constexpr auto AsString() const { return to_string(m_type, m_v); }

    template <auto V>
    [[nodiscard]] constexpr auto Is(string value) const -> bool {
      static_assert(V == IntL || V == NumL || V == Text || V == Name || V == Char || V == MacB || V == Macr ||
                    V == Note);
      return m_type == V && AsString() == value;
    }

    [[nodiscard]] constexpr auto GetString() const {
      qcore_assert(m_type == IntL || m_type == NumL || m_type == Text || m_type == Name || m_type == Char ||
                   m_type == MacB || m_type == Macr || m_type == Note);
      return m_v.m_str;
    }

    [[nodiscard]] constexpr auto GetKeyword() const {
      qcore_assert(m_type == KeyW);
      return m_v.m_key;
    }

    [[nodiscard]] constexpr auto GetOperator() const {
      qcore_assert(m_type == Oper);
      return m_v.m_op;
    }

    [[nodiscard]] constexpr auto GetPunctor() const {
      qcore_assert(m_type == Punc);
      return m_v.m_punc;
    }

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

    constexpr operator bool() const { return m_type != EofF; }
  } __attribute__((packed));

  using Token = TokenBase;

  string to_string(TokenType ty);  // NOLINT
  static inline auto operator<<(std::ostream &os, TokenType ty) -> std::ostream & {
    os << to_string(ty);
    return os;
  }

  auto operator<<(std::ostream &os, Token tok) -> std::ostream &;
}  // namespace ncc::lex

namespace std {
  template <>
  struct hash<ncc::lex::LocationID> {
    auto operator()(const ncc::lex::LocationID &loc) const -> size_t { return loc.GetId(); }
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
