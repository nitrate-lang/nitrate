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
///   * QUIX PACKAGE MANAGER - The official tool for the Quix language.      ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <conf/SPDX.hh>
#include <conf/Validate.hh>
#include <core/Logger.hh>

enum class KeyName {
  NAME,
  VERSION,
  DESCRIPTION,
  AUTHORS,
  EMAILS,
  URL,
  LICENSES,
  SOURCES,
  TARGET,
  TRIPLE,
  CPU,
  CFLAGS,
  LFLAGS,
  NOLINK,
  PACKAGES,
};

enum class ValueType { STRING, INTEGER, BOOLEAN, STRING_ARRAY };

static std::unordered_map<std::string, KeyName> key_map = {{"name", KeyName::NAME},
                                                           {"version", KeyName::VERSION},
                                                           {"description", KeyName::DESCRIPTION},
                                                           {"authors", KeyName::AUTHORS},
                                                           {"emails", KeyName::EMAILS},
                                                           {"url", KeyName::URL},
                                                           {"licenses", KeyName::LICENSES},
                                                           {"sources", KeyName::SOURCES},
                                                           {"target", KeyName::TARGET},
                                                           {"triple", KeyName::TRIPLE},
                                                           {"cpu", KeyName::CPU},
                                                           {"cflags", KeyName::CFLAGS},
                                                           {"lflags", KeyName::LFLAGS},
                                                           {"nolink", KeyName::NOLINK},
                                                           {"packages", KeyName::PACKAGES}};

static std::set<std::string> required_keys = {
    "name",
    "version",
    "description",
    "target",
};

static std::set<std::string> target_valid_values = {"sharedlib", "staticlib", "executable"};

/// https://stackoverflow.com/questions/1031645/how-to-detect-utf-8-in-plain-c
bool is_utf8(const char *string) {
  if (!string) return 0;

  const unsigned char *bytes = (const unsigned char *)string;
  while (*bytes) {
    if ((  // ASCII
           // use bytes[0] <= 0x7F to allow ASCII control characters
            bytes[0] == 0x09 || bytes[0] == 0x0A || bytes[0] == 0x0D ||
            (0x20 <= bytes[0] && bytes[0] <= 0x7E))) {
      bytes += 1;
      continue;
    }

    if ((  // non-overlong 2-byte
            (0xC2 <= bytes[0] && bytes[0] <= 0xDF) && (0x80 <= bytes[1] && bytes[1] <= 0xBF))) {
      bytes += 2;
      continue;
    }

    if ((  // excluding overlongs
            bytes[0] == 0xE0 && (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF)) ||
        (  // straight 3-byte
            ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) || bytes[0] == 0xEE || bytes[0] == 0xEF) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF) && (0x80 <= bytes[2] && bytes[2] <= 0xBF)) ||
        (  // excluding surrogates
            bytes[0] == 0xED && (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF))) {
      bytes += 3;
      continue;
    }

    if ((  // planes 1-3
            bytes[0] == 0xF0 && (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) && (0x80 <= bytes[3] && bytes[3] <= 0xBF)) ||
        (  // planes 4-15
            (0xF1 <= bytes[0] && bytes[0] <= 0xF3) && (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) && (0x80 <= bytes[3] && bytes[3] <= 0xBF)) ||
        (  // plane 16
            bytes[0] == 0xF4 && (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) && (0x80 <= bytes[3] && bytes[3] <= 0xBF))) {
      bytes += 4;
      continue;
    }

    return 0;
  }

  return 1;
}

bool qpkg::conf::ValidateConfig(const qpkg::conf::Config &config,
                                const std::filesystem::path &base) {
  using namespace qpkg::core;

  auto keys = config.keys();

  for (const auto &key : keys) {
    if (!key_map.contains(key)) {
      LOG(ERROR) << "Invalid key in configuration: " << key << std::endl;
      return false;
    }

    switch (key_map[key]) {
      case KeyName::NAME:
        if (!config[key].is<std::string>()) {
          LOG(ERROR) << "Invalid value type for key 'name' in configuration" << std::endl;
          return false;
        }
        if (!is_utf8(config[key].as<std::string>().c_str())) {
          LOG(ERROR) << "Invalid value for key 'name' in configuration: must be UTF-8" << std::endl;
          return false;
        }
        break;
      case KeyName::VERSION:
        if (!config[key].is<int64_t>()) {
          LOG(ERROR) << "Invalid value type for key 'version' in configuration" << std::endl;
          return false;
        }
        if (config[key].as<int64_t>() != 1) {
          LOG(ERROR) << "This version of the QPKG system only supports version 1" << std::endl;
          return false;
        }
        break;
      case KeyName::DESCRIPTION:
        if (!config[key].is<std::string>()) {
          LOG(ERROR) << "Invalid value type for key 'description' in configuration" << std::endl;
          return false;
        }
        if (!is_utf8(config[key].as<std::string>().c_str())) {
          LOG(ERROR) << "Invalid value for key 'description' in "
                        "configuration: must be UTF-8"
                     << std::endl;
          return false;
        }
        break;
      case KeyName::AUTHORS:
        if (!config[key].is<std::vector<std::string>>()) {
          LOG(ERROR) << "Invalid value type for key 'authors' in configuration" << std::endl;
          return false;
        }
        for (const auto &author : config[key].as<std::vector<std::string>>()) {
          if (!is_utf8(author.c_str())) {
            LOG(ERROR) << "Invalid value for key 'authors' in "
                          "configuration: must be UTF-8"
                       << std::endl;
            return false;
          }
        }
        break;
      case KeyName::EMAILS:
        if (!config[key].is<std::vector<std::string>>()) {
          LOG(ERROR) << "Invalid value type for key 'emails' in configuration" << std::endl;
          return false;
        }
        for (const auto &email : config[key].as<std::vector<std::string>>()) {
          if (!is_utf8(email.c_str())) {
            LOG(ERROR) << "Invalid value for key 'emails' in "
                          "configuration: must be UTF-8"
                       << std::endl;
            return false;
          }
        }
        break;
      case KeyName::URL:
        if (!config[key].is<std::string>()) {
          LOG(ERROR) << "Invalid value type for key 'url' in configuration" << std::endl;
          return false;
        }
        if (!is_utf8(config[key].as<std::string>().c_str())) {
          LOG(ERROR) << "Invalid value for key 'url' in configuration: must be UTF-8" << std::endl;
          return false;
        }
        break;
      case KeyName::LICENSES:
        if (!config[key].is<std::vector<std::string>>()) {
          LOG(ERROR) << "Invalid value type for key 'licenses' in configuration" << std::endl;
          return false;
        }
        for (const auto &license : config[key].as<std::vector<std::string>>()) {
          if (!qpkg::conf::valid_licenses.contains(license)) {
            LOG(ERROR) << "Invalid license in configuration: " << license << std::endl;
            return false;
          }
        }
        break;
      case KeyName::SOURCES:
        if (!config[key].is<std::vector<std::string>>()) {
          LOG(ERROR) << "Invalid value type for key 'sources' in configuration" << std::endl;
          return false;
        }
        for (const auto &source : config[key].as<std::vector<std::string>>()) {
          if (!std::filesystem::exists(base / source)) {
            LOG(ERROR) << "Source does not exist: " << source << std::endl;
            return false;
          }
        }
        break;
      case KeyName::TARGET:
        if (!config[key].is<std::string>()) {
          LOG(ERROR) << "Invalid value type for key 'target' in configuration" << std::endl;
          return false;
        }
        if (!target_valid_values.contains(config[key].as<std::string>())) {
          LOG(ERROR) << "Invalid value for key 'target' in configuration: must be one "
                        "of 'sharedlib', 'staticlib', or 'executable'"
                     << std::endl;
          return false;
        }
        break;
      case KeyName::TRIPLE:
        if (!config[key].is<std::string>()) {
          LOG(ERROR) << "Invalid value type for key 'triple' in configuration" << std::endl;
          return false;
        }
        if (!is_utf8(config[key].as<std::string>().c_str())) {
          LOG(ERROR) << "Invalid value for key 'triple' in "
                        "configuration: must be UTF-8"
                     << std::endl;
          return false;
        }
        break;
      case KeyName::CPU:
        if (!config[key].is<std::string>()) {
          LOG(ERROR) << "Invalid value type for key 'cpu' in configuration" << std::endl;
          return false;
        }
        if (!is_utf8(config[key].as<std::string>().c_str())) {
          LOG(ERROR) << "Invalid value for key 'cpu' in configuration: must be UTF-8" << std::endl;
          return false;
        }
        break;
      case KeyName::CFLAGS:
        if (!config[key].is<std::vector<std::string>>()) {
          LOG(ERROR) << "Invalid value type for key 'cflags' in configuration" << std::endl;
          return false;
        }
        break;
      case KeyName::LFLAGS:
        if (!config[key].is<std::vector<std::string>>()) {
          LOG(ERROR) << "Invalid value type for key 'lflags' in configuration" << std::endl;
          return false;
        }
        break;
      case KeyName::NOLINK:
        if (!config[key].is<bool>()) {
          LOG(ERROR) << "Invalid value type for key 'nolink' in configuration" << std::endl;
          return false;
        }
        break;
      case KeyName::PACKAGES:
        if (!config[key].is<std::vector<std::string>>()) {
          LOG(ERROR) << "Invalid value type for key 'packages' in configuration" << std::endl;
          return false;
        }
        for (const auto &source : config[key].as<std::vector<std::string>>()) {
          if (!std::filesystem::exists(base / source) ||
              !std::filesystem::is_directory(base / source)) {
            LOG(ERROR) << "Dependency does not exist: " << source << std::endl;
            return false;
          }
        }
        break;
      default:
        LOG(ERROR) << "Invalid key in configuration: " << key << std::endl;
        return false;
    }
  }

  for (const auto &key : required_keys) {
    if (!keys.contains(key)) {
      LOG(ERROR) << "Missing required key in configuration: " << key << std::endl;
      return false;
    }
  }

  return true;
}

void qpkg::conf::PopulateConfig(qpkg::conf::Config &config) {
  if (!config.m_root.has<std::vector<std::string>>("authors"))
    config.m_root.set("authors", std::vector<std::string>());

  if (!config.m_root.has<std::vector<std::string>>("licenses"))
    config.m_root.set("licenses", std::vector<std::string>());

  if (!config.m_root.has<std::vector<std::string>>("depends"))
    config.m_root.set("depends", std::vector<std::string>());

  if (!config.m_root.has<std::vector<std::string>>("sources"))
    config.m_root.set("sources", std::vector<std::string>());

  if (!config.m_root.has<std::vector<std::string>>("packages"))
    config.m_root.set("packages", std::vector<std::string>());

  if (!config.m_root.has<std::string>("triple")) config.m_root.set("triple", "");

  if (!config.m_root.has<std::string>("cpu")) config.m_root.set("cpu", "");

  if (!config.m_root.has<std::vector<std::string>>("cflags"))
    config.m_root.set("cflags", std::vector<std::string>());

  if (!config.m_root.has<std::vector<std::string>>("lflags"))
    config.m_root.set("lflags", std::vector<std::string>());

  if (!config.m_root.has<bool>("nolink")) config.m_root.set("nolink", false);
}