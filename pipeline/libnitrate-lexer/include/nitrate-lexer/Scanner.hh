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

#ifndef __NITRATE_LEXER_SCANNER_HH__
#define __NITRATE_LEXER_SCANNER_HH__

#include <memory>
#include <nitrate-core/EnvironmentFwd.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/Testing.hh>
#include <nitrate-lexer/Token.hh>

namespace ncc::lex {
  namespace detail {
    struct ScannerEOF final {};
  }  // namespace detail

  class ISourceFile {
  public:
    virtual ~ISourceFile() = default;
  };

  /**
   * @brief Generic scanner interface.
   *
   * The scanner is based on the Virtual Token Deque (VTQ) model. The VTQ model
   * is an abstract machine that behaves like a std::deque<Token> of infinite
   * size. If the deque is empty, the scanner will read from the source file and
   * fill the deque with tokens. If the input reaches the end of the file, the
   * VTQ model will return a continuous stream of `qEof` type tokens.
   */
  class NCC_EXPORT IScanner {
    friend class TestAttorney;

    class PImpl;
    std::unique_ptr<PImpl> m_impl;

  protected:
    std::shared_ptr<IEnvironment> m_env;

    virtual auto GetNext() -> Token = 0;

    /**
     * @brief Provide fallback resolution for location IDs.
     * @note The internal map is checked first, it the ID is not found, this
     *       method is called.
     */
    virtual auto GetLocationFallback(LocationID) -> std::optional<Location>;

  public:
    IScanner(std::shared_ptr<IEnvironment> env);
    IScanner(const IScanner&) = delete;
    IScanner(IScanner&&) = default;
    virtual ~IScanner();

    /** Check if the VTQ model is empty. */
    [[nodiscard]] auto IsEof() const -> bool;

    /** Check if the error bit is set. */
    [[nodiscard]] virtual auto HasError() const -> bool;

    /** Set the lexer error bit */
    virtual auto SetFailBit(bool fail = true) -> bool;

    /**
     * Consumes the next token in the VTQ model. If comments are disabled,
     * `qNote` tokens are saved to the comment buffer, and the next non-comment
     * token is actually returned.
     *
     * @return The next token from the deque.
     * @note This method is reentrant and thread safe.
     */
    auto Next() -> Token;

    /**
     * Copies the next token in the VTQ model without consuming it.
     * If comments are disabled, `qNote` tokens are saved to the comment buffer,
     * and the peeked token is consumed and the next non-comment token is
     * returned.
     *
     * @return The next token from the deque.
     * @note This method is reentrant and thread safe.
     */
    [[nodiscard]] auto Peek() -> Token;

    /**
     * Inserts a token into the front of the VTQ model.
     *
     * @param tok The token to insert.
     * @note An unlimited number of tokens can be inserted.
     * @note This method is thread safe.
     */
    auto Insert(Token tok) -> void;

    /** @brief Return the last token emitted by either `Next` or `Peek`. */
    [[nodiscard]] auto Current() -> Token;

    /** @brief Get the source location at the start of the token. */
    [[nodiscard]] auto Start(Token t) -> Location;

    /** @brief Get the source location at the end of the token. */
    [[nodiscard]] auto End(Token t) -> Location;

    /** @brief Get the source line at the start of the token. */
    [[nodiscard]] auto StartLine(Token t) -> uint32_t;

    /** @brief Get the source column at the start of the token. */
    [[nodiscard]] auto StartColumn(Token t) -> uint32_t;

    /** @brief Get the source line at the end of the token. */
    [[nodiscard]] auto EndLine(Token t) -> uint32_t;

    /** @brief Get the source column at the end of the token. */
    [[nodiscard]] auto EndColumn(Token t) -> uint32_t;

    /** @brief Convert a LocationID alias into a Location object. */
    [[nodiscard]] auto GetLocation(LocationID id) -> Location;

    /** @brief Get the comment buffer. */
    [[nodiscard]] auto CommentBuffer() -> const std::vector<Token>&;

    /** @brief Clear the comment buffer. */
    auto ClearCommentBuffer() -> void;

    [[nodiscard]] auto GetSkipCommentsState() const -> bool;
    auto SkipCommentsState(bool skip) -> bool;

    struct Point {
      long m_col = 0, m_line = 0;

      Point(long line, long col) : m_col(col), m_line(line) {}
    };

    virtual auto GetSourceWindow(Point start, Point end,
                                 char fillchar = ' ') -> std::optional<std::vector<std::string>> = 0;

    [[nodiscard]] auto GetEnvironment() const -> std::shared_ptr<IEnvironment>;

    /** Create a new LocationID from a Location */
    auto InternLocation(Location loc) -> LocationID;
  };
}  // namespace ncc::lex

#endif