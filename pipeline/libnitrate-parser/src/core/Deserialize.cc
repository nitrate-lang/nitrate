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

static auto JsonToValue(const ordered_json& v) -> std::optional<AstReader::Value> {
  switch (v.type()) {
    case json::value_t::null: {
      return AstReader::Value(nullptr);
    }

    case json::value_t::object:
    case json::value_t::array: {
      return std::nullopt;
    }

    case json::value_t::string: {
      return AstReader::Value(v.get<std::string>());
    }

    case json::value_t::boolean: {
      return AstReader::Value(v.get<bool>());
    }

    case json::value_t::number_integer: {
      return std::nullopt;
    }

    case json::value_t::number_unsigned: {
      return AstReader::Value(v.get<uint64_t>());
    }

    case json::value_t::number_float: {
      return AstReader::Value(v.get<double>());
    }

    case json::value_t::binary: {
      return std::nullopt;
    }

    case json::value_t::discarded: {
      return std::nullopt;
    }

    default: {
      return std::nullopt;
    }
  }
}

static void FlattenJson(const ordered_json& value,
                        std::queue<ordered_json>& queue) {
  if (value.is_array()) {
    queue.emplace(value.size());
    for (const auto& val : value) {
      FlattenJson(val, queue);
    }
  } else if (value.is_object()) {
    for (const auto& [key, val] : value.items()) {
      queue.emplace(key);
      FlattenJson(val, queue);
    }
  } else {
    queue.push(value);
  }
}

///=============================================================================

struct AstJsonReader::PImpl {
  ordered_json m_json;
  std::queue<ordered_json> m_queue;
};

AstJsonReader::AstJsonReader(std::istream& is,
                             ReaderSourceManager source_manager)
    : AstReader([&]() { return ReadValue(); }, source_manager),
      m_is(is),
      m_pimpl(std::make_unique<PImpl>()) {
  m_pimpl->m_json = ordered_json::parse(is, nullptr, false);

  if (!m_pimpl->m_json.is_discarded() && m_pimpl->m_json.is_object()) {
    FlattenJson(m_pimpl->m_json, m_pimpl->m_queue);
  }
}

AstJsonReader::~AstJsonReader() = default;

auto AstJsonReader::ReadValue() -> std::optional<AstReader::Value> {
  auto& queue = m_pimpl->m_queue;
  if (queue.empty()) [[unlikely]] {
    return std::nullopt;
  } else {
    auto value = queue.front();
    queue.pop();

    return JsonToValue(value);
  }
}

///=============================================================================

struct AstMsgPackReader::PImpl {
  ordered_json m_json;
  std::queue<ordered_json> m_queue;
};

AstMsgPackReader::AstMsgPackReader(std::istream& is,
                                   ReaderSourceManager source_manager)
    : AstReader([&]() { return ReadValue(); }, source_manager),
      m_is(is),
      m_pimpl(std::make_unique<PImpl>()) {
  m_pimpl->m_json = ordered_json::from_msgpack(is, true, false);

  if (!m_pimpl->m_json.is_discarded() && m_pimpl->m_json.is_object()) {
    FlattenJson(m_pimpl->m_json, m_pimpl->m_queue);
  }
}

AstMsgPackReader::~AstMsgPackReader() = default;

auto AstMsgPackReader::ReadValue() -> std::optional<AstReader::Value> {
  auto& queue = m_pimpl->m_queue;
  if (queue.empty()) [[unlikely]] {
    return std::nullopt;
  } else {
    auto value = queue.front();
    queue.pop();

    return JsonToValue(value);
  }
}
