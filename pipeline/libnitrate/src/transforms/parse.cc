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

#include <nitrate/code.h>

#include <core/SerialUtil.hh>
#include <core/Transform.hh>
#include <cstdint>
#include <memory>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Grammar.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>
#include <utility>

using namespace ncc::lex;
using namespace ncc::parse;

static inline auto EofTok() -> Token { return Token::EndOfFile(); }

class DeserializerAdapterLexer final : public ncc::lex::IScanner {
  static constexpr std::array<uint8_t, 256> kValidTyIdTab = []() {
    std::array<uint8_t, 256> tab = {};
    tab.fill(0);

    tab[EofF] = 1;
    tab[KeyW] = 1;
    tab[Oper] = 1;
    tab[Punc] = 1;
    tab[Name] = 1;
    tab[IntL] = 1;
    tab[NumL] = 1;
    tab[Text] = 1;
    tab[Char] = 1;
    tab[MacB] = 1;
    tab[Macr] = 1;
    tab[Note] = 1;

    return tab;
  }();

  enum class InMode {
    JSON,
    MsgPack,
    BadCodec,
  } m_mode = InMode::BadCodec;

  uint64_t m_ele_count = 0;
  bool m_eof_bit = false;
  std::istream &m_file;

  static auto Decode(TokenType t, const std::string &data) -> Token {
    Token r;

    switch (t) {
      case TokenType::EofF: {
        r = Token::EndOfFile();
        break;
      }

      case TokenType::KeyW: {
        r = Token(t, ncc::lex::LEXICAL_KEYWORDS.left.at(data));
        break;
      }

      case TokenType::Oper: {
        r = Token(t, ncc::lex::LEXICAL_OPERATORS.left.at(data));
        break;
      }

      case TokenType::Punc: {
        r = Token(t, ncc::lex::LEXICAL_PUNCTORS.left.at(data));
        break;
      }

      case TokenType::Name: {
        r = Token(t, ncc::string(data));
        break;
      }

      case TokenType::IntL: {
        r = Token(t, ncc::string(data));
        break;
      }

      case TokenType::NumL: {
        r = Token(t, ncc::string(data));
        break;
      }

      case TokenType::Text: {
        r = Token(t, ncc::string(data));
        break;
      }

      case TokenType::Char: {
        r = Token(t, ncc::string(data));
        break;
      }

      case TokenType::MacB: {
        r = Token(t, ncc::string(data));
        break;
      }

      case TokenType::Macr: {
        r = Token(t, ncc::string(data));
        break;
      }

      case TokenType::Note: {
        r = Token(t, ncc::string(data));
        break;
      }
    }

    return r;
  }

  auto NextImplJson() -> Token {
    if (m_eof_bit) [[unlikely]] {
      return EofTok();
    }

    uint32_t ty;
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    char *str = nullptr;

    { /* Read the token array */
      size_t str_len = 0;

      if (m_file.get() != '[') {
        return EofTok();
      }
      m_file >> ty;
      ty &= 0xff;
      if (m_file.get() != ',') {
        return EofTok();
      }

      if (!ReadJsonString(m_file, &str, str_len)) [[unlikely]] {
        return EofTok();
      }

      if (m_file.get() != ',') {
        return EofTok();
      }
      m_file >> a;
      if (m_file.get() != ',') {
        return EofTok();
      }
      m_file >> b;
      if (m_file.get() != ',') {
        return EofTok();
      }
      m_file >> c;
      if (m_file.get() != ',') {
        return EofTok();
      }
      m_file >> d;
      if (m_file.get() != ']') {
        return EofTok();
      }
    }

    { /* Check the delimiter */
      char delim = m_file.get();

      if (delim == ']') [[unlikely]] {
        m_eof_bit = true;
        free(str);
        return EofTok();
      } else if (delim != ',') [[unlikely]] {
        free(str);
        return EofTok();
      }
    }

    /* Validate the token type */
    if (kValidTyIdTab[ty] != 0U) [[likely]] {
      Token t = Decode(static_cast<TokenType>(ty), str);

      free(str);
      return t;
    }

    free(str);
    return EofTok();
  }

  auto NextImplMsgpack() -> Token {
    if (m_eof_bit || (m_ele_count == 0U)) [[unlikely]] {
      return EofTok();
    }

    uint64_t ty;
    uint64_t a;
    uint64_t b;
    uint64_t c;
    uint64_t d;
    char *str = nullptr;

    { /* Read the token array */
      // Array start byte for 6 elements
      if (m_file.get() != 0x96) {
        return EofTok();
      }

      size_t str_len;

      if (!MsgpackReadUint(m_file, ty)) [[unlikely]] {
        return EofTok();
      }
      ty &= 0xff;

      if (!MsgpackReadStr(m_file, &str, str_len)) [[unlikely]] {
        return EofTok();
      }

      if (!MsgpackReadUint(m_file, a) || !MsgpackReadUint(m_file, b) || !MsgpackReadUint(m_file, c) ||
          !MsgpackReadUint(m_file, d)) [[unlikely]] {
        free(str);
        return EofTok();
      }
    }

    m_ele_count--;

    /* Validate the token type */
    if (kValidTyIdTab[ty] != 0U) [[likely]] {
      Token t = Decode(static_cast<TokenType>(ty), str);
      free(str);
      return t;
    }

    free(str);
    return EofTok();
  }

  auto GetNext() -> Token override {
    switch (m_mode) {
      case InMode::JSON: {
        return NextImplJson();
      }
      case InMode::MsgPack: {
        return NextImplMsgpack();
      }
      case InMode::BadCodec: {
        return EofTok();
      }
    }
  }

public:
  DeserializerAdapterLexer(std::istream &file, std::shared_ptr<ncc::Environment> env)
      : ncc::lex::IScanner(std::move(env)), m_file(file) {
    int ch = file.get();

    m_mode = InMode::BadCodec;
    m_eof_bit = false;

    if (ch == EOF) {
      return;
    }
    if (ch == '[') {
      m_mode = InMode::JSON;
      return;
    }
    if (ch == 0xdd) {
      m_ele_count = 0;

      if ((ch = file.get()) == EOF) {
        return;
      }
      m_ele_count |= ch << 24;
      if ((ch = file.get()) == EOF) {
        return;
      }
      m_ele_count |= ch << 16;
      if ((ch = file.get()) == EOF) {
        return;
      }
      m_ele_count |= ch << 8;
      if ((ch = file.get()) == EOF) {
        return;
      }
      m_ele_count |= ch;

      m_mode = InMode::MsgPack;
    } else if (ch == 0xdc) {
      m_ele_count = 0;

      if ((ch = file.get()) == EOF) {
        return;
      }
      m_ele_count |= ch << 8;
      if ((ch = file.get()) == EOF) {
        return;
      }
      m_ele_count |= ch;

      m_mode = InMode::MsgPack;
    } else if ((ch & 0b10010000) == 0b10010000) {
      m_mode = InMode::MsgPack;
      m_ele_count = ch & 0b00001111;
    }
  }

  ~DeserializerAdapterLexer() override = default;

  auto GetSourceWindow(Point, Point, char = ' ') -> std::optional<std::vector<std::string>> override {
    return std::nullopt;
  }
};

CREATE_TRANSFORM(nit::parse) {
  (void)opts;

  DeserializerAdapterLexer lexer(source, env);
  auto parser = Parser::Create(lexer, env);

  let root = parser->Parse();

  output << root.Get()->Serialize();

  return true;
}
