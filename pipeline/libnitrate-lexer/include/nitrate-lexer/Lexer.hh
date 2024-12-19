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

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include <cstdint>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-lexer/Token.hh>
#include <ostream>
#include <queue>

#include "nitrate-core/Logger.hh"

///============================================================================///

typedef struct NCCLexer NCCLexer;

#define QLEX_FLAG_NONE 0
#define QLEX_NO_COMMENTS 0x01

void qlex_insert(NCCLexer *lexer, NCCToken tok);

static inline uint32_t qlex_begin(const NCCToken *tok) { return tok->start; }
uint32_t qlex_end(NCCLexer *L, NCCToken tok);

const char *qlex_filename(NCCLexer *lexer);
uint32_t qlex_line(NCCLexer *lexer, uint32_t loc);
uint32_t qlex_col(NCCLexer *lexer, uint32_t loc);

char *qlex_snippet(NCCLexer *lexer, NCCToken loc, uint32_t *offset);

const char *qlex_ty_str(qlex_ty_t ty);

/**
 * @brief Get the internal string value for a token.
 *
 * @param lexer Lexer context.
 * @param tok Token.
 * @param len Pointer to store the length of the string.
 *
 * @return The internal string value for the token or empty string if this
 * operation is applicable for this token type.
 * @note This function is thread-safe.
 * @warning The lifetime shall exist for the duration of the lexer context.
 * @warning DO NOT MODIFY THE RETURNED STRING.
 * @warning The returned string is NULL-terminated, however, it may contain any
 * bytes within the data including NULL bytes.
 */
const char *qlex_str(NCCLexer *lexer, const NCCToken *tok, size_t *len);
const char *qlex_opstr(qlex_op_t op);
const char *qlex_kwstr(qlex_key_t kw);
const char *qlex_punctstr(qlex_punc_t punct);

namespace ncc::lex {
  class ISourceFile {
  protected:
    std::string m_filename = "<unknown>";

  public:
    virtual ~ISourceFile() = default;

    virtual std::istream &GetStream() = 0;
    constexpr std::string_view GetFilename() const { return m_filename; }

    void Seek(size_t offset) {
      auto &stream = GetStream();
      stream.seekg(offset);
    }
  };

  std::unique_ptr<ISourceFile> SourceFileFromSeekableStream(
      std::istream &stream, size_t begin_offset = 0);

  class IScanner {
  protected:
    NCCToken m_current;
    std::string_view m_filename;
    uint32_t m_line, m_column, m_offset;

  public:
    virtual ~IScanner() = default;

    virtual NCCToken Next() = 0;

    NCCToken Peek() {
      /// TODO: Implement
      qcore_implement();
    }

    void Undo() {
      /// TODO: Implement
      qcore_implement();
    }

    constexpr NCCToken Current() const { return m_current; }

    constexpr std::string_view GetCurrentFilename() const { return m_filename; }
    constexpr uint32_t GetCurrentLine() const { return m_line; }
    constexpr uint32_t GetCurrentColumn() const { return m_column; }
    constexpr uint32_t GetCurrentOffset() const { return m_offset; }

    constexpr bool IsEof() const { return m_current.is(qEofF); }

    std::string_view Filename(NCCToken) { return "?"; }
    uint32_t StartLine(NCCToken) { return UINT32_MAX; }
    uint32_t StartColumn(NCCToken) { return UINT32_MAX; }
    uint32_t EndLine(NCCToken) { return UINT32_MAX; }
    uint32_t EndColumn(NCCToken) { return UINT32_MAX; }
  };

  class Tokenizer final : public IScanner {
    std::shared_ptr<ISourceFile> m_source_file;
    std::shared_ptr<core::Environment> m_env;

    std::queue<NCCToken> m_token_buffer;

    void refill_buffer();

  public:
    Tokenizer(std::shared_ptr<ISourceFile> source_file,
              std::shared_ptr<core::Environment> env);
    virtual ~Tokenizer() override;

    NCCToken Next() override;
  };

  class RefactorWrapper final : public IScanner {
    NCCLexer *m_lexer;

  public:
    RefactorWrapper(NCCLexer *lexer) : m_lexer(lexer) {}
    virtual ~RefactorWrapper() override {}

    NCCToken Next() override;
  };

  std::ostream &operator<<(std::ostream &os, qlex_ty_t ty);
  std::ostream &operator<<(std::ostream &os, NCCToken tok);
  std::ostream &operator<<(std::ostream &os, qlex_op_t op);
  std::ostream &operator<<(std::ostream &os, qlex_key_t kw);
  std::ostream &operator<<(std::ostream &os, qlex_punc_t punct);
}  // namespace ncc::lex

void qlex_tok_fromstr(ncc::lex::IScanner *lexer, qlex_ty_t ty, const char *str,
                      NCCToken *out);

#endif