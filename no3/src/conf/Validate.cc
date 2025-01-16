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

#include <conf/SPDX.hh>
#include <conf/Validate.hh>
#include <core/Logger.hh>

enum class KeyName : uint8_t {
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

enum class ValueType : uint8_t { STRING, INTEGER, BOOLEAN, STRING_ARRAY };

static std::unordered_map<std::string, KeyName> KeyMap = {
    {"name", KeyName::NAME},
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

static std::set<std::string> RequiredKeys = {
    "name",
    "version",
    "description",
    "target",
};

static std::set<std::string> TargetValidValues = {"sharedlib", "staticlib",
                                                  "executable"};

/// https://stackoverflow.com/questions/1031645/how-to-detect-utf-8-in-plain-c
auto IsUtf8(const char *string) -> bool {
  if (string == nullptr) {
    return false;
  }

  const auto *bytes = (const unsigned char *)string;
  while (*bytes != 0U) {
    if ((  // ASCII
           // use bytes[0] <= 0x7F to allow ASCII control characters
            bytes[0] == 0x09 || bytes[0] == 0x0A || bytes[0] == 0x0D ||
            (0x20 <= bytes[0] && bytes[0] <= 0x7E))) {
      bytes += 1;
      continue;
    }

    if ((  // non-overlong 2-byte
            (0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF))) {
      bytes += 2;
      continue;
    }

    if ((  // excluding overlongs
            bytes[0] == 0xE0 && (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF)) ||
        (  // straight 3-byte
            ((0xE1 <= bytes[0] && bytes[0] <= 0xEC) || bytes[0] == 0xEE ||
             bytes[0] == 0xEF) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF)) ||
        (  // excluding surrogates
            bytes[0] == 0xED && (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF))) {
      bytes += 3;
      continue;
    }

    if ((  // planes 1-3
            bytes[0] == 0xF0 && (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
            (0x80 <= bytes[3] && bytes[3] <= 0xBF)) ||
        (  // planes 4-15
            (0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
            (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
            (0x80 <= bytes[3] && bytes[3] <= 0xBF)) ||
        (  // plane 16
            bytes[0] == 0xF4 && (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
            (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
            (0x80 <= bytes[3] && bytes[3] <= 0xBF))) {
      bytes += 4;
      continue;
    }

    return false;
  }

  return true;
}

auto no3::conf::ValidateConfig(const no3::conf::Config &config,
                               const std::filesystem::path &base) -> bool {
  using namespace no3::core;

  auto keys = config.Keys();

  for (const auto &key : keys) {
    if (!KeyMap.contains(key)) {
      LOG(ERROR) << "Invalid key in configuration: " << key;
      return false;
    }

    switch (KeyMap[key]) {
      case KeyName::NAME:
        if (!config[key].Is<std::string>()) {
          LOG(ERROR) << "Invalid value type for key 'name' in configuration";
          return false;
        }
        if (!IsUtf8(config[key].As<std::string>().c_str())) {
          LOG(ERROR)
              << "Invalid value for key 'name' in configuration: must be UTF-8";
          return false;
        }
        break;
      case KeyName::VERSION:
        if (!config[key].Is<int64_t>()) {
          LOG(ERROR) << "Invalid value type for key 'version' in configuration";
          return false;
        }
        if (config[key].As<int64_t>() != 1) {
          LOG(ERROR)
              << "This version of the NO3 system only supports version 1";
          return false;
        }
        break;
      case KeyName::DESCRIPTION:
        if (!config[key].Is<std::string>()) {
          LOG(ERROR)
              << "Invalid value type for key 'description' in configuration";
          return false;
        }
        if (!IsUtf8(config[key].As<std::string>().c_str())) {
          LOG(ERROR) << "Invalid value for key 'description' in "
                        "configuration: must be UTF-8";
          return false;
        }
        break;
      case KeyName::AUTHORS:
        if (!config[key].Is<std::vector<std::string>>()) {
          LOG(ERROR) << "Invalid value type for key 'authors' in configuration";
          return false;
        }
        for (const auto &author : config[key].As<std::vector<std::string>>()) {
          if (!IsUtf8(author.c_str())) {
            LOG(ERROR) << "Invalid value for key 'authors' in "
                          "configuration: must be UTF-8";
            return false;
          }
        }
        break;
      case KeyName::EMAILS:
        if (!config[key].Is<std::vector<std::string>>()) {
          LOG(ERROR) << "Invalid value type for key 'emails' in configuration";
          return false;
        }
        for (const auto &email : config[key].As<std::vector<std::string>>()) {
          if (!IsUtf8(email.c_str())) {
            LOG(ERROR) << "Invalid value for key 'emails' in "
                          "configuration: must be UTF-8";
            return false;
          }
        }
        break;
      case KeyName::URL:
        if (!config[key].Is<std::string>()) {
          LOG(ERROR) << "Invalid value type for key 'url' in configuration";
          return false;
        }
        if (!IsUtf8(config[key].As<std::string>().c_str())) {
          LOG(ERROR)
              << "Invalid value for key 'url' in configuration: must be UTF-8";
          return false;
        }
        break;
      case KeyName::LICENSES:
        if (!config[key].Is<std::vector<std::string>>()) {
          LOG(ERROR)
              << "Invalid value type for key 'licenses' in configuration";
          return false;
        }
        for (const auto &license : config[key].As<std::vector<std::string>>()) {
          if (!no3::conf::SPDX_IDENTIFIERS.contains(license)) {
            LOG(ERROR) << "Invalid license in configuration: " << license;
            return false;
          }
        }
        break;
      case KeyName::SOURCES:
        if (!config[key].Is<std::vector<std::string>>()) {
          LOG(ERROR) << "Invalid value type for key 'sources' in configuration";
          return false;
        }
        for (const auto &source : config[key].As<std::vector<std::string>>()) {
          if (!std::filesystem::exists(base / source)) {
            LOG(ERROR) << "Source does not exist: " << source;
            return false;
          }
        }
        break;
      case KeyName::TARGET:
        if (!config[key].Is<std::string>()) {
          LOG(ERROR) << "Invalid value type for key 'target' in configuration";
          return false;
        }
        if (!TargetValidValues.contains(config[key].As<std::string>())) {
          LOG(ERROR)
              << "Invalid value for key 'target' in configuration: must be one "
                 "of 'sharedlib', 'staticlib', or 'executable'";
          return false;
        }
        break;
      case KeyName::TRIPLE:
        if (!config[key].Is<std::string>()) {
          LOG(ERROR) << "Invalid value type for key 'triple' in configuration";
          return false;
        }
        if (!IsUtf8(config[key].As<std::string>().c_str())) {
          LOG(ERROR) << "Invalid value for key 'triple' in "
                        "configuration: must be UTF-8";
          return false;
        }
        break;
      case KeyName::CPU:
        if (!config[key].Is<std::string>()) {
          LOG(ERROR) << "Invalid value type for key 'cpu' in configuration";
          return false;
        }
        if (!IsUtf8(config[key].As<std::string>().c_str())) {
          LOG(ERROR)
              << "Invalid value for key 'cpu' in configuration: must be UTF-8";
          return false;
        }
        break;
      case KeyName::CFLAGS:
        if (!config[key].Is<std::vector<std::string>>()) {
          LOG(ERROR) << "Invalid value type for key 'cflags' in configuration";
          return false;
        }
        break;
      case KeyName::LFLAGS:
        if (!config[key].Is<std::vector<std::string>>()) {
          LOG(ERROR) << "Invalid value type for key 'lflags' in configuration";
          return false;
        }
        break;
      case KeyName::NOLINK:
        if (!config[key].Is<bool>()) {
          LOG(ERROR) << "Invalid value type for key 'nolink' in configuration";
          return false;
        }
        break;
      case KeyName::PACKAGES:
        if (!config[key].Is<std::vector<std::string>>()) {
          LOG(ERROR)
              << "Invalid value type for key 'packages' in configuration";
          return false;
        }
        for (const auto &source : config[key].As<std::vector<std::string>>()) {
          if (!std::filesystem::exists(base / source) ||
              !std::filesystem::is_directory(base / source)) {
            LOG(ERROR) << "Dependency does not exist: " << source;
            return false;
          }
        }
        break;
      default:
        LOG(ERROR) << "Invalid key in configuration: " << key;
        return false;
    }
  }

  if (!std::all_of(RequiredKeys.begin(), RequiredKeys.end(),
                   [&keys](const auto &key) { return keys.contains(key); })) {
    LOG(ERROR) << "Missing required key in configuration";
    return false;
  }

  return true;
}

void no3::conf::PopulateConfig(no3::conf::Config &config) {
  if (!config.m_root.Has<std::vector<std::string>>("authors")) {
    config.m_root.Set("authors", std::vector<std::string>());
  }

  if (!config.m_root.Has<std::vector<std::string>>("licenses")) {
    config.m_root.Set("licenses", std::vector<std::string>());
  }

  if (!config.m_root.Has<std::vector<std::string>>("depends")) {
    config.m_root.Set("depends", std::vector<std::string>());
  }

  if (!config.m_root.Has<std::vector<std::string>>("sources")) {
    config.m_root.Set("sources", std::vector<std::string>());
  }

  if (!config.m_root.Has<std::vector<std::string>>("packages")) {
    config.m_root.Set("packages", std::vector<std::string>());
  }

  if (!config.m_root.Has<std::string>("triple")) {
    config.m_root.Set("triple", "");
  }

  if (!config.m_root.Has<std::string>("cpu")) {
    config.m_root.Set("cpu", "");
  }

  if (!config.m_root.Has<std::vector<std::string>>("cflags")) {
    config.m_root.Set("cflags", std::vector<std::string>());
  }

  if (!config.m_root.Has<std::vector<std::string>>("lflags")) {
    config.m_root.Set("lflags", std::vector<std::string>());
  }

  if (!config.m_root.Has<bool>("nolink")) {
    config.m_root.Set("nolink", false);
  }
}