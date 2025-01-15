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

#ifndef __NITRATE_IR_READER_H__
#define __NITRATE_IR_READER_H__

#include <cstdint>
#include <istream>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR/Visitor.hh>
#include <optional>
#include <stack>

namespace ncc::ir {
  class NCC_EXPORT IRReader {
    enum class State {
      ObjStart,
      ObjEnd,
    };

    std::stack<State> m_state;
    std::stack<Expr*> m_parse;

    void HandleState();

  protected:
    void Str(std::string_view str);
    void Uint(uint64_t val);
    void Dbl(double val);
    void Boolean(bool val);
    void Null();
    void BeginObj();
    void EndObj();
    void BeginArr(size_t max_size);
    void EndArr();

  public:
    IRReader() { m_state.push(State::ObjStart); }
    virtual ~IRReader() = default;

    std::optional<Expr*> Get() {
      if (m_parse.empty() || m_parse.top() == nullptr) {
        return std::nullopt;
      }

      return m_parse.top();
    }
  };

  class NCC_EXPORT IRJsonReader final : public IRReader {
    void ParseStream(std::istream& is);

  public:
    IRJsonReader(std::istream& is) { ParseStream(is); }
    virtual ~IRJsonReader() = default;
  };

  class NCC_EXPORT IRMsgPackReader final : public IRReader {
    void ParseStream(std::istream& is);

  public:
    IRMsgPackReader(std::istream& is) { ParseStream(is); }
    virtual ~IRMsgPackReader() = default;
  };
}  // namespace ncc::ir

#endif
