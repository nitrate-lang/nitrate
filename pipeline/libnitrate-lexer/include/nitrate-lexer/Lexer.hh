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

#include <boost/bimap.hpp>
#include <cstdint>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <queue>

namespace ncc::lex {
  class NCC_EXPORT Tokenizer final : public IScanner {
    static constexpr size_t kGetcBufferSize = 1024;

    uint64_t m_offset = 0, m_line = 0, m_column = 0;
    std::queue<char> m_fifo;
    size_t m_getc_buffer_pos = kGetcBufferSize;
    std::array<char, kGetcBufferSize> m_getc_buffer;
    std::istream &m_file;
    bool m_eof = false;

    class StaticImpl;
    friend class StaticImpl;

    void RefillCharacterBuffer();
    auto GetChar() -> char;
    void ResetAutomaton();
    auto GetNext() -> Token override;

  public:
    Tokenizer(std::istream &source_file, std::shared_ptr<Environment> env)
        : IScanner(std::move(env)), m_file(source_file) {
      if (auto filename = m_env->Get("FILE"); filename.has_value()) {
        SetCurrentFilename(filename.value());
      }
    }

    ~Tokenizer() override = default;

    auto GetSourceWindow(Point start, Point end, char fillchar = ' ')
        -> std::optional<std::vector<std::string>> override;
  };
}  // namespace ncc::lex

#endif