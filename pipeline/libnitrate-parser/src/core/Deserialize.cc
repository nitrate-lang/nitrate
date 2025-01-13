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

#include <cstddef>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/ASTReader.hh>
#include <nlohmann/json.hpp>
#include <stack>

using namespace ncc::parse;
using namespace nlohmann;

using JsonRange = std::pair<json::const_iterator, json::const_iterator>;

static std::optional<AST_Reader::Value> JsonToValue(json v) {
  switch (v.type()) {
    case json::value_t::null: {
      return AST_Reader::Value(nullptr);
    }

    case json::value_t::object:
    case json::value_t::array: {
      return std::nullopt;
    }

    case json::value_t::string: {
      return AST_Reader::Value(v.get<std::string>());
    }

    case json::value_t::boolean: {
      return AST_Reader::Value(v.get<bool>());
    }

    case json::value_t::number_integer: {
      return std::nullopt;
    }

    case json::value_t::number_unsigned: {
      return AST_Reader::Value(v.get<uint64_t>());
    }

    case json::value_t::number_float: {
      return AST_Reader::Value(v.get<double>());
    }

    case json::value_t::binary: {
      return std::nullopt;
    }

    case json::value_t::discarded: {
      return std::nullopt;
    }
  }
}

static void FlattenJsonObject(
    const json& obj, std::stack<std::variant<JsonRange, json>>& stack) {
  std::vector<std::variant<JsonRange, json>> buf(obj.size() * 2);
  size_t i = 0;

  for (const auto& [key, val] : obj.items()) {
    buf[i++] = key;
    buf[i++] = val;
  }

  for (auto it = buf.rbegin(); it != buf.rend(); ++it) {
    stack.push(*it);
  }
}

static std::optional<AST_Reader::Value> GetNextValueImpl(
    std::stack<std::variant<JsonRange, json>>& stack) {
  /* Flatten the JSON data structure as-if doing a depth-first traversal */
  using Value = AST_Reader::Value;

  while (true) {
    if (stack.empty()) [[unlikely]] {
      return std::nullopt;
    }

    if (std::holds_alternative<json>(stack.top())) {
      const auto value = std::get<json>(stack.top());
      stack.pop();

      return JsonToValue(std::move(value));
    }

    auto& [it, end] = std::get<JsonRange>(stack.top());

    /* End of structured object */
    if (it != end) {
      const auto& value = *it++;

      if (value.is_array()) {
        /* Flatten the array be prefixing it with its size */
        stack.push(JsonRange{value.begin(), value.end()});

        return Value(value.size());
      } else if (!value.is_object()) {
        return JsonToValue(std::move(value));
      }

      FlattenJsonObject(value, stack);
    } else {
      stack.pop();
    }
  }
}

///=============================================================================

struct AST_JsonReader::PImpl {
  json m_json;
  std::stack<std::variant<JsonRange, json>> stack;
};

AST_JsonReader::AST_JsonReader(std::istream& is,
                               ReaderSourceManager source_manager)
    : AST_Reader([&]() { return ReadValue(); }, source_manager), m_is(is) {
  m_pimpl = std::make_unique<PImpl>();
  m_pimpl->m_json = json::parse(is, nullptr, false);

  if (!m_pimpl->m_json.is_discarded() && m_pimpl->m_json.is_object()) {
    FlattenJsonObject(m_pimpl->m_json, m_pimpl->stack);
  }
}

AST_JsonReader::~AST_JsonReader() = default;

std::optional<AST_Reader::Value> AST_JsonReader::ReadValue() {
  return GetNextValueImpl(m_pimpl->stack);
}

///=============================================================================

struct AST_MsgPackReader::PImpl {
  json m_json;
  std::stack<std::variant<JsonRange, json>> stack;
};

AST_MsgPackReader::AST_MsgPackReader(std::istream& is,
                                     ReaderSourceManager source_manager)
    : AST_Reader([&]() { return ReadValue(); }, source_manager), m_is(is) {
  m_pimpl = std::make_unique<PImpl>();
  m_pimpl->m_json = json::from_msgpack(is, true, false);

  if (!m_pimpl->m_json.is_discarded() && m_pimpl->m_json.is_object()) {
    FlattenJsonObject(m_pimpl->m_json, m_pimpl->stack);
  }
}

AST_MsgPackReader::~AST_MsgPackReader() = default;

std::optional<AST_Reader::Value> AST_MsgPackReader::ReadValue() {
  return GetNextValueImpl(m_pimpl->stack);
}
