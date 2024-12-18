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

#ifndef __NITRATE_NR_READER_H__
#define __NITRATE_NR_READER_H__

#include <cstdint>
#include <istream>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/Visitor.hh>
#include <optional>
#include <stack>

namespace nr {
  class CPP_EXPORT NR_Reader {
    enum class State {
      ObjStart,
      ObjEnd,
    };

    std::stack<State> m_state;
    std::stack<Expr*> m_parse;

    void handle_state();

    /// TODO: Implement state

  protected:
    void str(std::string_view str);
    void uint(uint64_t val);
    void dbl(double val);
    void boolean(bool val);
    void null();
    void begin_obj();
    void end_obj();
    void begin_arr(size_t max_size);
    void end_arr();

  public:
    NR_Reader() { m_state.push(State::ObjStart); }
    virtual ~NR_Reader() = default;

    std::optional<Expr*> get() {
      if (m_parse.empty() || m_parse.top() == nullptr) {
        return std::nullopt;
      }

      return m_parse.top();
    }
  };

  class CPP_EXPORT NR_JsonReader final : public NR_Reader {
    void parse_stream(std::istream& is);

  public:
    NR_JsonReader(std::istream& is) { parse_stream(is); }
    virtual ~NR_JsonReader() = default;
  };

  class CPP_EXPORT NR_MsgPackReader final : public NR_Reader {
    void parse_stream(std::istream& is);

  public:
    NR_MsgPackReader(std::istream& is) { parse_stream(is); }
    virtual ~NR_MsgPackReader() = default;
  };
}  // namespace nr

#endif
