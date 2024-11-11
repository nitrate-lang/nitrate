////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///           ░▒▓██████▓▒░░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░                  ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓███████▓▒░░▒▓███████▓▒░░▒▓█▓▒▒▓███▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///           ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///             ░▒▓█▓▒░                                                      ///
///              ░▒▓██▓▒░                                                    ///
///                                                                          ///
///   * NITRATE PACKAGE MANAGER - The official app for the Nitrate language. ///
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

std::optional<no3::conf::Config> no3::conf::YamlConfigParser::parse(const std::string &content) {
  YAML::Node config;

  try {
    config = YAML::Load(content);
  } catch (YAML::ParserException &e) {
    LOG(ERROR) << "Failed to parse YAML configuration: " << e.what() << std::endl;
    return std::nullopt;
  }

  if (!config.IsMap()) {
    LOG(ERROR) << "Invalid YAML configuration: root element must be a map" << std::endl;
    return std::nullopt;
  }

  ConfigGroup grp;

  for (auto it = config.begin(); it != config.end(); ++it) {
    if (it->second.IsScalar()) {
      try {
        int64_t i = it->second.as<int64_t>();
        grp.set(it->first.as<std::string>(), i);
      } catch (YAML::TypedBadConversion<int64_t> &e) {
        try {
          bool b = it->second.as<bool>();
          grp.set(it->first.as<std::string>(), b);
        } catch (YAML::TypedBadConversion<bool> &e) {
          grp.set(it->first.as<std::string>(), it->second.as<std::string>());
        }
      }
    } else if (it->second.IsSequence()) {
      std::vector<std::string> v;

      for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
        if (it2->IsScalar())
          v.push_back(it2->as<std::string>());
        else {
          LOG(ERROR) << "Invalid YAML configuration: unsupported value type" << std::endl;
          return std::nullopt;
        }
      }

      grp.set(it->first.as<std::string>(), v);
    } else {
      LOG(ERROR) << "Invalid YAML configuration: unsupported value type" << std::endl;
      return std::nullopt;
    }
  }

  if (!grp.has<int64_t>("version")) {
    LOG(ERROR) << "Invalid YAML configuration: missing 'version' key" << std::endl;
    return std::nullopt;
  }

  return Config(grp, grp["version"].as<int64_t>());
}
