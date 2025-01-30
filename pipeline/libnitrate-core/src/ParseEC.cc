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

#include <nitrate-core/NewLogger.hh>
#include <nitrate-core/OldLogger.hh>
#include <nlohmann/json.hpp>

using namespace ncc;

static bool ValidateJsonObject(const nlohmann::json &j) {
  if (!j.is_object()) [[unlikely]] {
    return false;
  }

  if (!j.contains("flagname") || !j["flagname"].is_string()) [[unlikely]] {
    return false;
  }

  if (!j.contains("nice_name") || !j["nice_name"].is_string()) [[unlikely]] {
    return false;
  }

  if (!j.contains("details") || !j["details"].is_string()) [[unlikely]] {
    return false;
  }

  if (!j.contains("tags") || !j["tags"].is_array()) [[unlikely]] {
    return false;
  }

  if (!j.contains("fixes") || !j["fixes"].is_array()) [[unlikely]] {
    return false;
  }

  if (!j.contains("examples") || !j["examples"].is_array()) [[unlikely]] {
    return false;
  }

  if (!j.contains("dev_notes") || !j["dev_notes"].is_array()) [[unlikely]] {
    return false;
  }

  if (!j.contains("user_notes") || !j["user_notes"].is_array()) [[unlikely]] {
    return false;
  }

  if (!std::all_of(j["tags"].begin(), j["tags"].end(),
                   [](auto tag) { return tag.is_string(); })) [[unlikely]] {
    return false;
  }

  if (!std::all_of(j["fixes"].begin(), j["fixes"].end(),
                   [](auto fix) { return fix.is_string(); })) [[unlikely]] {
    return false;
  }

  if (!std::all_of(j["examples"].begin(), j["examples"].end(),
                   [](auto example) { return example.is_string(); }))
      [[unlikely]] {
    return false;
  }

  if (!std::all_of(j["dev_notes"].begin(), j["dev_notes"].end(),
                   [](auto dev_note) { return dev_note.is_string(); }))
      [[unlikely]] {
    return false;
  }

  if (!std::all_of(j["user_notes"].begin(), j["user_notes"].end(),
                   [](auto user_note) { return user_note.is_string(); }))
      [[unlikely]] {
    return false;
  }

  return true;
}

static ECDetails JsonObjectToStructure(const nlohmann::json &j) {
  ECDetails details;
  details.m_flagname = j["flagname"].get<std::string>();
  details.m_nice_name = j["nice_name"].get<std::string>();
  details.m_details = j["details"].get<std::string>();

  details.m_tags.reserve(j["tags"].size());
  for (const auto &tag : j["tags"]) {
    details.m_tags.push_back(tag.get<std::string>());
  }

  details.m_fixes.reserve(j["fixes"].size());
  for (const auto &fix : j["fixes"]) {
    details.m_fixes.push_back(fix.get<std::string>());
  }

  details.m_examples.reserve(j["examples"].size());
  for (const auto &example : j["examples"]) {
    details.m_examples.push_back(example.get<std::string>());
  }

  details.m_dev_notes.reserve(j["dev_notes"].size());
  for (const auto &dev_note : j["dev_notes"]) {
    details.m_dev_notes.push_back(dev_note.get<std::string>());
  }

  details.m_notes.reserve(j["user_notes"].size());
  for (const auto &user_note : j["user_notes"]) {
    details.m_notes.push_back(user_note.get<std::string>());
  }

  return details;
};

auto ECBase::ParseJsonECConfig(std::string_view json)
    -> std::optional<ECDetails> {
  auto j = nlohmann::json::parse(json, nullptr, false);
  if (j.is_discarded()) {
    /* No error logging here, as the logger is not initialized yet. */
    return std::nullopt;
  }

  if (!ValidateJsonObject(j)) {
    /* No error logging here, as the logger is not initialized yet. */
    return std::nullopt;
  }

  return JsonObjectToStructure(j);
}
