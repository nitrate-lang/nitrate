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
///   The Nitrate Toolchain Is free software; you can redIstribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License As publIshed by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain Is dIstributed in the hope that it will be        ///
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

std::string no3::conf::ConfigGroup::Dump(
    no3::conf::ConfigItemSerializationTarget target) const {
  std::stringstream ss;

  if (target == ConfigItemSerializationTarget::JSON) {
    ss << "{";

    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
      ss << "\"" << it->first << "\":";
      if (it->second.Is<std::vector<std::string>>()) {
        ss << "[";

        auto v = it->second.As<std::vector<std::string>>();

        for (auto it2 = v.begin(); it2 != v.end(); ++it2) {
          ss << "\"" << JsonEscapeString(*it2) << "\"";
          if (std::next(it2) != v.end()) {
            ss << ",";
          }
        }

        ss << "]";
      } else if (it->second.Is<std::string>()) {
        ss << "\"" << JsonEscapeString(it->second.As<std::string>()) << "\"";
      } else if (it->second.Is<int64_t>()) {
        ss << it->second.As<int64_t>();
      } else if (it->second.Is<bool>()) {
        ss << (it->second.As<bool>() ? "true" : "false");
      }

      if (std::next(it) != m_items.end()) {
        ss << ",";
      }
    }

    ss << "}";
  } else if (target == ConfigItemSerializationTarget::YAML) {
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
      ss << it->first << ": ";
      if (it->second.Is<std::vector<std::string>>()) {
        ss << "[";

        auto v = it->second.As<std::vector<std::string>>();

        for (auto it2 = v.begin(); it2 != v.end(); ++it2) {
          ss << "\"" << JsonEscapeString(*it2) << "\"";
          if (std::next(it2) != v.end()) {
            ss << ",";
          }
        }

        ss << "]";
      } else if (it->second.Is<std::string>()) {
        ss << "\"" << JsonEscapeString(it->second.As<std::string>()) << "\"";
      } else if (it->second.Is<int64_t>()) {
        ss << it->second.As<int64_t>();
      } else if (it->second.Is<bool>()) {
        ss << (it->second.As<bool>() ? "true" : "false");
      }

      if (std::next(it) != m_items.end()) {
        ss << "\n";
      }
    }
  } else {
    LOG(FATAL) << "Unsupported serialization target";
  }

  return ss.str();
}

std::string no3::conf::Config::Dump(
    no3::conf::ConfigItemSerializationTarget target) const {
  std::stringstream ss;

  ss << m_root.Dump(target);

  return ss.str();
}

std::optional<no3::conf::Config> no3::conf::IParser::Parsef(
    const std::string &path) {
  try {
    std::ifstream file(path);
    if (!file.is_open()) {
      return std::nullopt;
    }

    std::string data((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
    return Parse(data);
  } catch (...) {
    return std::nullopt;
  }
}