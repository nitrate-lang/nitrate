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

#ifndef __QUIXCC_LEX_H__
#define __QUIXCC_LEX_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/lexer/Token.h>
#include <stdio.h>

#include <array>
#include <deque>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace libquixcc {
constexpr std::array<char, 10> punctors = {'(', ')', '{', '}', '[',
                                           ']', ',', ':', ';'};

extern const std::unordered_map<std::string_view, libquixcc::Keyword>
    keyword_map;
extern const std::unordered_map<libquixcc::Keyword, std::string_view>
    keyword_map_inverse;
extern const std::unordered_map<std::string_view, libquixcc::Punctor>
    punctor_map;
extern const std::unordered_map<libquixcc::Punctor, std::string_view>
    punctor_map_inverse;
extern const std::unordered_map<std::string_view, libquixcc::Operator>
    operator_map;
extern const std::unordered_map<libquixcc::Operator, std::string_view>
    operator_map_inverse;

class Scanner {
 public:
  Scanner() = default;
  virtual ~Scanner() = default;

  virtual Token next() = 0;
  virtual const Token &peek() = 0;
  static std::string escape_string(std::string_view str);
  virtual void push(Token tok) = 0;
  virtual bool set_source(FILE *src, const std::string &filename) = 0;
  virtual void comments(bool ignore) = 0;
};

class StreamLexer : public Scanner {
  constexpr static size_t GETC_BUFFER_SIZE = 1024;
  std::array<char, GETC_BUFFER_SIZE> m_buffer;
  std::optional<Token> m_tok;
  std::deque<char> m_pushback;
  Loc m_loc_curr;
  Loc m_loc;
  size_t m_buf_pos = 0;
  FILE *m_src = nullptr;
  bool added_newline = false;
  bool ingore_comments = true;

 protected:
  std::string m_filename;
  char getc();
  const libquixcc::Token &read_token();
  inline void reset_state() { m_pushback = std::deque<char>(); }

 public:
  StreamLexer();
  virtual ~StreamLexer() = default;

  /// @brief Set the source file
  /// @param src C FILE pointer
  /// @return true if the source file is set successfully
  virtual bool set_source(FILE *src, const std::string &filename) override;

  Token next() override;
  const Token &peek() override;

  void comments(bool ignore) override { ingore_comments = ignore; }

  inline void push(Token tok) override { m_tok = tok; }
};

class StringLexer : public StreamLexer {
 private:
  std::string m_src;
  FILE *m_file;

 public:
  StringLexer() : m_file(nullptr) {}
  StringLexer(const std::string &source_code) {
    set_source(source_code, "stringlexer");
  }
  virtual ~StringLexer();

  using StreamLexer::set_source;

  /// @brief Set the source file
  /// @param src C FILE pointer
  /// @return true if the source file is set successfully
  bool set_source(const std::string &source_code, const std::string &filename);

  /// @brief Lex the source code
  /// @param source_code
  /// @param tokens
  /// @return Returns false if the source code is invalid
  static bool QuickLex(const std::string &source_code,
                       std::vector<Token> &tokens,
                       const std::string &filename = "quicklex");
};

class MockScanner : public Scanner {
 private:
  std::deque<Token> m_tokens;
  Token m_tok;

 public:
  MockScanner() : m_tok(Token(tEofF, "")) {}
  MockScanner(const std::vector<Token> &tokens) {
    for (const auto &tok : tokens) {
      m_tokens.push_back(tok);
    }

    if (!m_tokens.empty()) {
      m_tok = m_tokens.front();
      m_tokens.pop_front();
    } else {
      m_tok = Token(tEofF, "");
    }
  }
  virtual ~MockScanner() = default;

  bool set_source(FILE *src, const std::string &filename) override {
    return false;
  }

  Token next() override {
    if (m_tokens.empty()) {
      m_tok = Token(tEofF, "");
    } else {
      m_tok = m_tokens.front();
      m_tokens.pop_front();
    }

    return m_tok;
  }

  const Token &peek() override { return m_tok; }

  void push(Token tok) override { m_tokens.push_front(tok); }
};
};  // namespace libquixcc

#endif  // __QUIXCC_LEX_H__