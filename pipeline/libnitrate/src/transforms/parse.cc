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
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>
#include <unordered_set>

using namespace ncc::lex;
using namespace ncc::parse;

static inline Token EofTok() { return Token::EndOfFile(); }

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
  } m_mode;

  uint64_t m_ele_count;
  bool m_eof_bit;
  std::istream &m_file;

  Token Decode(TokenType t, const std::string &data) {
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

  Token NextImplJson() {
    if (m_eof_bit) [[unlikely]] {
      return EofTok();
    }

    uint32_t ty, a, b, c, d;
    char *str = nullptr;

    { /* Read the token array */
      size_t str_len = 0;

      if (m_file.get() != '[') return EofTok();
      m_file >> ty;
      ty &= 0xff;
      if (m_file.get() != ',') return EofTok();

      if (!ReadJsonString(m_file, &str, str_len)) [[unlikely]] {
        return EofTok();
      }

      if (m_file.get() != ',') return EofTok();
      m_file >> a;
      if (m_file.get() != ',') return EofTok();
      m_file >> b;
      if (m_file.get() != ',') return EofTok();
      m_file >> c;
      if (m_file.get() != ',') return EofTok();
      m_file >> d;
      if (m_file.get() != ']') return EofTok();
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
    if (kValidTyIdTab[ty]) [[likely]] {
      Token t = Decode(static_cast<TokenType>(ty), str);

      free(str);
      return t;
    }

    free(str);
    return EofTok();
  }

  Token NextImplMsgpack() {
    if (m_eof_bit || !m_ele_count) [[unlikely]] {
      return EofTok();
    }

    uint64_t ty, a, b, c, d;
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

      if (!MsgpackReadUint(m_file, a) || !MsgpackReadUint(m_file, b) ||
          !MsgpackReadUint(m_file, c) || !MsgpackReadUint(m_file, d))
          [[unlikely]] {
        free(str);
        return EofTok();
      }
    }

    m_ele_count--;

    /* Validate the token type */
    if (kValidTyIdTab[ty]) [[likely]] {
      Token t = Decode(static_cast<TokenType>(ty), str);
      free(str);
      return t;
    }

    free(str);
    return EofTok();
  }

  virtual Token GetNext() override {
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
  DeserializerAdapterLexer(std::istream &file,
                           std::shared_ptr<ncc::Environment> env)
      : ncc::lex::IScanner(env),
        m_mode(InMode::BadCodec),
        m_ele_count(0),
        m_eof_bit(false),
        m_file(file) {
    int ch = file.get();

    m_mode = InMode::BadCodec;
    m_eof_bit = false;

    if (ch == EOF) {
      return;
    } else if (ch == '[') {
      m_mode = InMode::JSON;
      return;
    } else if (ch == 0xdd) {
      m_ele_count = 0;

      if ((ch = file.get()) == EOF) return;
      m_ele_count |= ch << 24;
      if ((ch = file.get()) == EOF) return;
      m_ele_count |= ch << 16;
      if ((ch = file.get()) == EOF) return;
      m_ele_count |= ch << 8;
      if ((ch = file.get()) == EOF) return;
      m_ele_count |= ch;

      m_mode = InMode::MsgPack;
    } else if (ch == 0xdc) {
      m_ele_count = 0;

      if ((ch = file.get()) == EOF) return;
      m_ele_count |= ch << 8;
      if ((ch = file.get()) == EOF) return;
      m_ele_count |= ch;

      m_mode = InMode::MsgPack;
    } else if ((ch & 0b10010000) == 0b10010000) {
      m_mode = InMode::MsgPack;
      m_ele_count = ch & 0b00001111;
    }
  }

  virtual ~DeserializerAdapterLexer() override = default;
};

CREATE_TRANSFORM(nit::parse) {
  enum class OutMode {
    JSON,
    MsgPack,
  } out_mode = OutMode::JSON;

  if (opts.contains("-fuse-json") && opts.contains("-fuse-msgpack")) {
    qcore_logf(QCORE_ERROR, "Cannot use both JSON and MsgPack output.");
    return false;
  }

  if (opts.contains("-fuse-msgpack")) {
    out_mode = OutMode::MsgPack;
  }

  DeserializerAdapterLexer lexer(source, env);
  auto parser = Parser::Create(lexer, env);

  let root = parser->Parse();

  switch (out_mode) {
    case OutMode::JSON: {
      auto writter = AstJsonWriter(output);
      root.Get().Accept(writter);
      return true;
    }
    case OutMode::MsgPack: {
      auto writter = AstMsgPackWriter(output);
      root.Get().Accept(writter);
      return true;
    }
    default: {
      return false;
    }
  }
}