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

#include <conf/Parser.hh>
#include <core/Logger.hh>
#include <fstream>

static std::string JsonEscapeString(const std::string &str) {
  std::stringstream ss;

  for (char c : str) {
    switch (c) {
      case '"':
        ss << "\\\"";
        break;
      case '\\':
        ss << "\\\\";
        break;
      case '/':
        ss << "\\/";
        break;
      case '\b':
        ss << "\\b";
        break;
      case '\f':
        ss << "\\f";
        break;
      case '\n':
        ss << "\\n";
        break;
      case '\r':
        ss << "\\r";
        break;
      case '\t':
        ss << "\\t";
        break;
      default:
        ss << c;
        break;
    }
  }

  return ss.str();
}

std::string no3::conf::ConfigGroup::dump(
    no3::conf::ConfigItemSerializationTarget target) const {
  std::stringstream ss;

  if (target == ConfigItemSerializationTarget::JSON) {
    ss << "{";

    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
      ss << "\"" << it->first << "\":";
      if (it->second.is<std::vector<std::string>>()) {
        ss << "[";

        auto v = it->second.as<std::vector<std::string>>();

        for (auto it2 = v.begin(); it2 != v.end(); ++it2) {
          ss << "\"" << JsonEscapeString(*it2) << "\"";
          if (std::next(it2) != v.end()) ss << ",";
        }

        ss << "]";
      } else if (it->second.is<std::string>()) {
        ss << "\"" << JsonEscapeString(it->second.as<std::string>()) << "\"";
      } else if (it->second.is<int64_t>()) {
        ss << it->second.as<int64_t>();
      } else if (it->second.is<bool>()) {
        ss << (it->second.as<bool>() ? "true" : "false");
      }

      if (std::next(it) != m_items.end()) ss << ",";
    }

    ss << "}";
  } else if (target == ConfigItemSerializationTarget::YAML) {
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
      ss << it->first << ": ";
      if (it->second.is<std::vector<std::string>>()) {
        ss << "[";

        auto v = it->second.as<std::vector<std::string>>();

        for (auto it2 = v.begin(); it2 != v.end(); ++it2) {
          ss << "\"" << JsonEscapeString(*it2) << "\"";
          if (std::next(it2) != v.end()) ss << ",";
        }

        ss << "]";
      } else if (it->second.is<std::string>()) {
        ss << "\"" << JsonEscapeString(it->second.as<std::string>()) << "\"";
      } else if (it->second.is<int64_t>()) {
        ss << it->second.as<int64_t>();
      } else if (it->second.is<bool>()) {
        ss << (it->second.as<bool>() ? "true" : "false");
      }

      if (std::next(it) != m_items.end()) ss << std::endl;
    }
  } else {
    LOG(FATAL) << "Unsupported serialization target" << std::endl;
  }

  return ss.str();
}

std::string no3::conf::Config::dump(
    no3::conf::ConfigItemSerializationTarget target) const {
  std::stringstream ss;

  ss << m_root.dump(target);

  return ss.str();
}

std::optional<no3::conf::Config> no3::conf::IParser::parsef(
    const std::string &path) {
  try {
    std::ifstream file(path);
    if (!file.is_open()) return std::nullopt;

    std::string data((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    return parse(data);
  } catch (...) {
    return std::nullopt;
  }
}