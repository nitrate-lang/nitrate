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

#include <charconv>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>

using namespace ncc::lex;
using namespace ncc::lex::detail;

static constexpr size_t kTokenBufferSize = 1024;

static inline auto StringToUint32(std::string_view str,
                                  uint32_t sentinal) -> uint32_t {
  uint32_t result = sentinal;
  std::from_chars(str.data(), str.data() + str.size(), result);
  return result;
}

class IScanner::Impl {
public:
  static void FillTokenBuffer(IScanner &self) {
    for (size_t i = 0; i < kTokenBufferSize; i++) {
      try {
        self.m_ready.push_back(self.GetNext());
      } catch (ScannerEOF &) {
        if (i == 0) {
          self.m_eof = true;
          self.m_ready.push_back(Token::EndOfFile());
        }
        break;
      }
    }
  }
};

IScanner::IScanner(std::shared_ptr<Environment> env) : m_env(std::move(env)) {
  constexpr size_t kInitialLocationReserve = 0xffff;

  m_location_interned.reserve(kInitialLocationReserve);
  m_location_interned.emplace_back(Location::EndOfFile());
}

IScanner::~IScanner() = default;

auto IScanner::GetEofLocation() -> Location {
  uint32_t offset = kLexEof;
  uint32_t line = kLexEof;
  uint32_t column = kLexEof;
  string filename;

  if (auto off = m_env->Get("this.file.eof.offset"); off.has_value()) {
    offset = StringToUint32(off.value(), kLexEof);
  }

  if (auto ln = m_env->Get("this.file.eof.line"); ln.has_value()) {
    line = StringToUint32(ln.value(), kLexEof);
  }

  if (auto col = m_env->Get("this.file.eof.column"); col.has_value()) {
    column = StringToUint32(col.value(), kLexEof);
  }

  if (auto fn = m_env->Get("this.file.eof.filename"); fn.has_value()) {
    filename = fn.value();
  }

  return {offset, line, column, filename};
}

auto IScanner::SetFailBit(bool fail) -> bool {
  auto old = m_ebit;
  m_ebit = fail;
  return old;
}

auto IScanner::Next() -> Token {
  while (true) {
    if (m_ready.empty()) [[unlikely]] {
      Impl::FillTokenBuffer(*this);
    }

    auto tok = m_ready.front();
    m_ready.pop_front();

    if (m_skip && tok.is(Note)) [[unlikely]] {
      m_comments.push_back(tok);
      continue;
    }

    m_current = tok;

    return tok;
  }
}

auto IScanner::Peek() -> Token {
  while (true) {
    if (m_ready.empty()) [[unlikely]] {
      Impl::FillTokenBuffer(*this);
    }

    auto tok = m_ready.front();

    if (m_skip && tok.is(Note)) [[unlikely]] {
      m_comments.push_back(tok);
      m_ready.pop_front();
      continue;
    }

    m_current = tok;

    return tok;
  }
}

auto IScanner::Insert(Token tok) -> void {
  m_ready.push_front(tok);
  m_current = tok;
}

auto IScanner::Start(Token t) -> Location { return t.GetStart().Get(*this); }

auto IScanner::End(Token) -> Location {  /// NOLINT
  /// TODO: Support relexing to get the end location
  return Location::EndOfFile();
}

auto IScanner::StartLine(Token t) -> uint32_t { return Start(t).GetRow(); }

auto IScanner::StartColumn(Token t) -> uint32_t { return Start(t).GetCol(); }

auto IScanner::EndLine(Token t) -> uint32_t { return End(t).GetRow(); }

auto IScanner::EndColumn(Token t) -> uint32_t { return End(t).GetCol(); }

auto IScanner::GetLocation(LocationID id) -> Location {
  if (id.GetId() < m_location_interned.size()) {
    return m_location_interned[id.GetId()];
  }

  return GetLocationFallback(id.GetId()).value_or(Location::EndOfFile());
}

auto IScanner::GetCurrentFilename() const -> string { return m_filename; }

auto IScanner::SetCurrentFilename(string filename) -> string {
  auto old = m_filename;
  m_filename = filename;
  return old;
}

auto IScanner::SkipCommentsState(bool skip) -> bool {
  auto old = m_skip;
  m_skip = skip;
  return old;
}

auto IScanner::GetEnvironment() const -> std::shared_ptr<Environment> {
  return m_env;
}
