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

#include <yaml-cpp/yaml.h>

#include <conf/Parser.hh>
#include <core/Logger.hh>

auto no3::conf::YamlConfigParser::Parse(
    const std::string &content) -> std::optional<no3::conf::Config> {
  YAML::Node config;

  try {
    config = YAML::Load(content);
  } catch (YAML::ParserException &e) {
    LOG(ERROR) << "Failed to parse YAML configuration: " << e.what();
    return std::nullopt;
  }

  if (!config.IsMap()) {
    LOG(ERROR) << "Invalid YAML configuration: root element must be a map";
    return std::nullopt;
  }

  ConfigGroup grp;

  for (auto it = config.begin(); it != config.end(); ++it) {
    if (it->second.IsScalar()) {
      try {
        auto i = it->second.as<int64_t>();
        grp.Set(it->first.as<std::string>(), i);
      } catch (YAML::TypedBadConversion<int64_t> &e) {
        try {
          bool b = it->second.as<bool>();
          grp.Set(it->first.as<std::string>(), b);
        } catch (YAML::TypedBadConversion<bool> &e) {
          grp.Set(it->first.as<std::string>(), it->second.as<std::string>());
        }
      }
    } else if (it->second.IsSequence()) {
      std::vector<std::string> v;

      for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
        if (it2->IsScalar()) {
          v.push_back(it2->as<std::string>());
        } else {
          LOG(ERROR) << "Invalid YAML configuration: unsupported value type";
          return std::nullopt;
        }
      }

      grp.Set(it->first.as<std::string>(), v);
    } else {
      LOG(ERROR) << "Invalid YAML configuration: unsupported value type";
      return std::nullopt;
    }
  }

  if (!grp.Has<int64_t>("version")) {
    LOG(ERROR) << "Invalid YAML configuration: missing 'version' key";
    return std::nullopt;
  }

  return Config(grp, grp["version"].As<int64_t>());
}
