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

#include <core/EC.hh>
#include <core/PImpl.hh>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <nitrate-seq/Sequencer.hh>

using namespace ncc::seq;
using namespace ncc::seq::ec;
using namespace std::literals;

NCC_EXPORT auto ncc::seq::FileSystemFetchModule(std::string_view path) -> std::optional<std::string> {
  const auto expected_prefix = "file:///package/"sv;
  if (!path.starts_with(expected_prefix)) {
    return std::nullopt;
  }

  path.remove_prefix(expected_prefix.size());
  if (path.size() < 37) {
    return std::nullopt;
  }

  const auto job_uuid = path.substr(0, 36);
  path.remove_prefix(37);

  Log << SeqLog << Debug << "Opening file '" << path << "' on behalf of job '" << job_uuid << "'";

  /// TODO: Get the base directory of the project

  auto file = std::fstream(std::string(path), std::ios::in);
  if (!file.is_open()) {
    return std::nullopt;
  }

  return std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
}

auto Sequencer::FetchModuleData(Sequencer &self, std::string_view raw_module_name) -> std::optional<std::string> {
  const auto get_fetch_uri = [](const std::string &module_name, const std::string &jobid) {
    return "file:///package/" + jobid + "/" + module_name;
  };

  const auto canonicalize_module_name = [](std::string module_name) {
    size_t pos = 0;
    while ((pos = module_name.find("::", pos)) != std::string::npos) {
      module_name.replace(pos, 2, ".");
    }

    std::replace(module_name.begin(), module_name.end(), '/', '.');

    return module_name;
  };

  auto module_name = canonicalize_module_name(std::string(raw_module_name));

  /* Translate module names into their actual names */
  if (const auto actual_name = self.m_env->Get("map." + module_name)) {
    module_name = actual_name.value();
  }

  const auto jobid = std::string(self.m_env->Get("this.job").value());
  const auto module_uri = get_fetch_uri(module_name, jobid);

  Log << SeqLog << Debug << "Importing module: '" << module_name << "'...";

  if (const auto module_content = self.m_shared->m_fetch_module(module_uri)) {
    return module_content;
  }

  Log << SeqLog << "Import not found: '" << module_name << "'";

  return std::nullopt;
}
