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

#include <cstdint>
#include <deque>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/Testing.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-lexer/Token.hh>

using namespace ncc::lex;
using namespace ncc::lex::detail;

class IScanner::PImpl {
public:
  std::deque<Token> m_ready;
  std::vector<Token> m_comments;
  std::vector<Location> m_location_interned;
  Token m_current;
  bool m_skip = false, m_ebit = false, m_eof = false;
};

auto IScanner::GetLocationFallback(LocationID) -> std::optional<Location> { return std::nullopt; };

IScanner::IScanner(std::shared_ptr<IEnvironment> env) : m_impl(std::make_unique<PImpl>()), m_env(std::move(env)) {
  constexpr size_t kInitialLocationReserve = 0xffff;

  m_impl->m_location_interned.reserve(kInitialLocationReserve);
  m_impl->m_location_interned.emplace_back(Location::EndOfFile());
}

IScanner::~IScanner() = default;

auto IScanner::IsEof() const -> bool { return m_impl->m_eof; }

auto IScanner::HasError() const -> bool { return m_impl->m_ebit; }

auto IScanner::SetFailBit(bool fail) -> bool {
  auto old = m_impl->m_ebit;
  m_impl->m_ebit = fail;
  return old;
}

[[nodiscard]] auto IScanner::Current() -> Token { return m_impl->m_current; }
[[nodiscard]] auto IScanner::CommentBuffer() -> const std::vector<Token> & { return m_impl->m_comments; }

auto IScanner::ClearCommentBuffer() -> void { m_impl->m_comments.clear(); }

[[nodiscard]] auto IScanner::GetSkipCommentsState() const -> bool { return m_impl->m_skip; }

auto IScanner::Next() -> Token {
  Token tok;
  PImpl &m = *m_impl;

  try {
    while (true) {
      if (m.m_ready.empty()) {
        tok = GetNext();
      } else {
        tok = m.m_ready.front();
        m.m_ready.pop_front();
      }

      /* Handle comment token buffering */
      if (m.m_skip && tok.Is(Note)) [[unlikely]] {
        m.m_comments.push_back(tok);
        continue;
      }

      break;
    }
  } catch (ScannerEOF &) { /* EOF signal optimization */
    tok = Token::EndOfFile();
  }

  m.m_eof |= tok.Is(EofF);
  m.m_current = tok;

  return tok;
}

auto IScanner::Peek() -> Token {
  Token tok;
  PImpl &m = *m_impl;

  try {
    while (true) {
      if (m.m_ready.empty()) {
        m.m_ready.push_back(GetNext());
      }

      tok = m.m_ready.front();

      /* Handle comment token buffering */
      if (m.m_skip && tok.Is(Note)) [[unlikely]] {
        m.m_comments.push_back(tok);
        m.m_ready.pop_front();
        continue;
      }

      break;
    }
  } catch (ScannerEOF &) { /* EOF signal optimization */
    tok = Token::EndOfFile();
  }

  m.m_eof |= tok.Is(EofF);
  m.m_current = tok;

  return tok;
}

auto IScanner::Insert(Token tok) -> void {
  PImpl &m = *m_impl;
  m.m_ready.push_front(tok);
  m.m_current = tok;
}

auto IScanner::Start(Token t) -> Location { return t.GetStart().Get(*this); }

auto IScanner::End(Token) -> Location {
  /// TODO: Support relexing to get the end location
  return Location::EndOfFile();
}

auto IScanner::StartLine(Token t) -> uint32_t { return Start(t).GetRow(); }
auto IScanner::StartColumn(Token t) -> uint32_t { return Start(t).GetCol(); }
auto IScanner::EndLine(Token t) -> uint32_t { return End(t).GetRow(); }
auto IScanner::EndColumn(Token t) -> uint32_t { return End(t).GetCol(); }

auto IScanner::GetLocation(LocationID id) -> Location {
  PImpl &m = *m_impl;

  if (id.GetId() < m.m_location_interned.size()) {
    return m.m_location_interned[id.GetId()];
  }

  return GetLocationFallback(LocationID(id.GetId())).value_or(Location::EndOfFile());
}

auto IScanner::SkipCommentsState(bool skip) -> bool {
  PImpl &m = *m_impl;

  auto old = m.m_skip;
  m.m_skip = skip;
  return old;
}

auto IScanner::GetEnvironment() const -> std::shared_ptr<IEnvironment> { return m_env; }

auto IScanner::InternLocation(Location loc) -> LocationID {
  PImpl &m = *m_impl;

  m.m_location_interned.push_back(loc);
  return LocationID(m.m_location_interned.size() - 1);
}
