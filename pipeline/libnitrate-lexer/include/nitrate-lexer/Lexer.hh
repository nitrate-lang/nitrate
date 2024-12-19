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

#ifndef __NITRATE_LEXER_LEX_HH__
#define __NITRATE_LEXER_LEX_HH__

#include <cstdint>
#include <deque>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Token.hh>
#include <ostream>

///============================================================================///

#define QLEX_FLAG_NONE 0
#define QLEX_NO_COMMENTS 0x01

namespace ncc::lex {
  const char *qlex_ty_str(qlex_ty_t ty);

  class ISourceFile {
  public:
    virtual ~ISourceFile() = default;
  };

  struct ScannerEOF final {};

  class IScanner {
    Token m_current{}, m_last{};
    std::string_view m_filename = "?";
    uint32_t m_line{}, m_column{}, m_offset{};
    std::deque<Token> m_ready;
    static constexpr size_t TOKEN_BUFFER_SIZE = 256;

    void FillTokenBuffer();
    void SyncState(Token tok);

  protected:
    void UpdateLocation(uint32_t line, uint32_t column, uint32_t offset,
                        std::string_view filename) {
      m_line = line;
      m_column = column;
      m_offset = offset;
      m_filename = filename;
    }

    virtual Token GetNext() = 0;

  public:
    virtual ~IScanner() = default;

    Token Next();
    Token Peek();
    void Undo();

    Token Current() { return m_current; }

    constexpr std::string_view GetCurrentFilename() const { return m_filename; }
    constexpr uint32_t GetCurrentLine() const { return m_line; }
    constexpr uint32_t GetCurrentColumn() const { return m_column; }
    constexpr uint32_t GetCurrentOffset() const { return m_offset; }
    constexpr bool IsEof() const { return m_current.is(qEofF); }

    std::string_view Filename(Token t);
    uint32_t StartLine(Token t);
    uint32_t StartColumn(Token t);
    uint32_t EndLine(Token t);
    uint32_t EndColumn(Token t);
  };

  class CPP_EXPORT Tokenizer final : public IScanner {
    std::istream &m_file;
    std::shared_ptr<core::Environment> m_env;
    std::deque<char> m_pushback;

    char nextc();
    void reset_state();

    Token GetNext() override;

  public:
    Tokenizer(std::istream &source_file, std::shared_ptr<core::Environment> env)
        : m_file(source_file), m_env(env) {}
    virtual ~Tokenizer() override {}

    std::shared_ptr<core::Environment> GetEnvironment() const { return m_env; }
  };

  const char *op_repr(qlex_op_t op);
  const char *kw_repr(qlex_key_t kw);
  const char *punct_repr(qlex_punc_t punct);

  std::ostream &operator<<(std::ostream &os, qlex_ty_t ty);
  std::ostream &operator<<(std::ostream &os, Token tok);

  inline std::ostream &operator<<(std::ostream &os, qlex_op_t op) {
    os << op_repr(op);
    return os;
  }

  inline std::ostream &operator<<(std::ostream &os, qlex_key_t kw) {
    os << kw_repr(kw);
    return os;
  }

  inline std::ostream &operator<<(std::ostream &os, qlex_punc_t punct) {
    os << punct_repr(punct);
    return os;
  }

  void qlex_tok_fromstr(IScanner *lexer, qlex_ty_t ty, const char *str,
                        Token *out);
}  // namespace ncc::lex

#endif