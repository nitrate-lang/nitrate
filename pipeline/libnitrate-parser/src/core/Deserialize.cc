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

#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/ASTReader.hh>
#include <nlohmann/json.hpp>
#include <queue>

using namespace ncc::parse;
using namespace nlohmann;

static std::optional<AST_Reader::Value> JsonToValue(ordered_json v) {
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

static void FlattenJson(const ordered_json& value,
                        std::queue<ordered_json>& queue) {
  if (value.is_array()) {
    for (const auto& val : value) {
      FlattenJson(val, queue);
    }
  } else if (value.is_object()) {
    for (const auto& [key, val] : value.items()) {
      queue.push(key);          // Push key first
      FlattenJson(val, queue);  // Then recursively process the value
    }
  } else {
    queue.push(value);  // Push primitive values directly
  }
}

///=============================================================================

struct AST_JsonReader::PImpl {
  ordered_json m_json;
  std::queue<ordered_json> queue;
};

AST_JsonReader::AST_JsonReader(std::istream& is,
                               ReaderSourceManager source_manager)
    : AST_Reader([&]() { return ReadValue(); }, source_manager), m_is(is) {
  m_pimpl = std::make_unique<PImpl>();
  m_pimpl->m_json = ordered_json::parse(is, nullptr, false);

  if (!m_pimpl->m_json.is_discarded() && m_pimpl->m_json.is_object()) {
    FlattenJson(m_pimpl->m_json, m_pimpl->queue);
  }
}

AST_JsonReader::~AST_JsonReader() = default;

std::optional<AST_Reader::Value> AST_JsonReader::ReadValue() {
  auto& queue = m_pimpl->queue;
  if (queue.empty()) [[unlikely]] {
    return std::nullopt;
  } else {
    auto value = queue.front();
    queue.pop();

    return JsonToValue(std::move(value));
  }
}

///=============================================================================

struct AST_MsgPackReader::PImpl {
  ordered_json m_json;
  std::queue<ordered_json> queue;
};

AST_MsgPackReader::AST_MsgPackReader(std::istream& is,
                                     ReaderSourceManager source_manager)
    : AST_Reader([&]() { return ReadValue(); }, source_manager), m_is(is) {
  m_pimpl = std::make_unique<PImpl>();
  m_pimpl->m_json = ordered_json::from_msgpack(is, true, false);

  if (!m_pimpl->m_json.is_discarded() && m_pimpl->m_json.is_object()) {
    FlattenJson(m_pimpl->m_json, m_pimpl->queue);
  }
}

AST_MsgPackReader::~AST_MsgPackReader() = default;

std::optional<AST_Reader::Value> AST_MsgPackReader::ReadValue() {
  auto& queue = m_pimpl->queue;
  if (queue.empty()) [[unlikely]] {
    return std::nullopt;
  } else {
    auto value = queue.front();
    queue.pop();

    return JsonToValue(std::move(value));
  }
}
