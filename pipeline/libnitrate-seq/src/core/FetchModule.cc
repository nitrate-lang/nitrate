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

#include <algorithm>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <core/EC.hh>
#include <core/PImpl.hh>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nitrate-core/IEnvironment.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/Algorithm.hh>
#include <nitrate-parser/CodeWriter.hh>
#include <nitrate-parser/Context.hh>
#include <nitrate-seq/Sequencer.hh>
#include <sstream>

using namespace ncc;
using namespace ncc::seq;
using namespace ncc::seq::ec;
using namespace std::literals;
using namespace std::filesystem;

static std::string ReadEnvironmentVariable(const char *name) {
  const char *value = std::getenv(name); /* NOLINT(concurrency-mt-unsafe) */
  if (value == nullptr) {
    return "";
  }

  return value;
}

static std::vector<path> GetResourceSearchPaths() {
  std::string env_path(ReadEnvironmentVariable("NCC_SEARCH_PATH"));

  std::vector<path> paths;
  std::string the_path;
  size_t start = 0;
  size_t end = env_path.find(':');
  while (end != std::string::npos) {
    the_path = env_path.substr(start, end - start);
    paths.emplace_back(exists(the_path) ? absolute(the_path) : path(the_path));
    start = end + 1;
    end = env_path.find(':', start);
  }

  the_path = env_path.substr(start, end);
  paths.emplace_back(exists(the_path) ? absolute(the_path) : path(the_path));

  std::remove_if(paths.begin(), paths.end(), [](const auto &path) { return !exists(path) || !is_directory(path); });

  return paths;
}

static std::optional<path> FindResource(const std::string &resource_name, const std::vector<path> &search_paths) {
  for (const auto &path : search_paths) {
    Log << SeqLog << Debug << "Searching for resource '" << resource_name << "' in " << path;

    auto alleged_path = path / resource_name;
    if (exists(alleged_path) && is_regular_file(alleged_path)) {
      Log << SeqLog << Debug << "Found resource '" << resource_name << "' at " << alleged_path;
      return alleged_path;
    }

    Log << SeqLog << Debug << "Resource '" << resource_name << "' not found at " << alleged_path;
  }

  return std::nullopt;
}

NCC_EXPORT auto ncc::seq::FileSystemFetchModule(std::string_view path) -> std::optional<std::string> {
  const auto expected_prefix = "file:///package/"sv;
  if (!path.starts_with(expected_prefix)) {
    return std::nullopt;
  }

  path.remove_prefix(expected_prefix.size());
  if (path.size() < 37) {
    return std::nullopt;
  }

  // const auto job_uuid = path.substr(0, 36);
  path.remove_prefix(37);

  const auto resource_search_paths = GetResourceSearchPaths();
  if (const auto the_path = FindResource(std::string(path), resource_search_paths)) {
    if (auto file = std::fstream(the_path->string(), std::ios::in); file.is_open()) {
      return std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
    }
  }

  Log << SeqLog << "Resource not found: '" << path << "'";

  return std::nullopt;
}

auto Sequencer::RenderTranslationUnitSource(Sequencer &self, std::string_view source) -> std::optional<std::string> {
  std::stringstream output;

  {
    using namespace ncc::parse;

    auto istream = boost::iostreams::stream<boost::iostreams::array_source>(source.data(), source.size());
    auto sub_scanner = CreateChild(self, istream);
    if (!sub_scanner) [[unlikely]] { /* Default to returning the whole TU */
      Log << SeqLog << "Failed to create child scanner";
      return std::nullopt;
    }

    const auto ast_root = GeneralParser::Create(*sub_scanner, self.m_env)->Parse();
    if (!ast_root.Check()) [[unlikely]] {
      Log << SeqLog << "Failed to parse translation unit";
      return std::nullopt;
    }

    /// TODO: Replace definitions with declarations

    for_each<Function>(ast_root.Get(), [](auto node) {
      auto function = node.template As<Function>();
      function->SetBody(nullptr);
    });

    /// FIXME: Fix this
    for_each<Variable>(ast_root.Get(), [](auto node) {
      auto variable = node.template As<Variable>();
      variable->SetInitializer(nullptr);
    });

    auto writer = CodeWriterFactory::Create(output);
    ast_root.Get()->Accept(*writer);
  }

  return output.str();
}

auto Sequencer::FetchModuleData(Sequencer &self, std::string_view raw_module_name) -> std::optional<std::string> {
  auto module_name = std::string(raw_module_name);

  const auto get_fetch_uri = [](const std::string &module_name, const std::string &jobid) {
    return "file:///package/" + jobid + "/" + module_name;
  };

  /* Translate module names into their actual names */
  if (const auto actual_name = self.m_env->Get("map." + module_name)) {
    module_name = actual_name.value();
  }

  const auto jobid = std::string(self.m_env->Get("this.job").value());
  const auto module_uri = get_fetch_uri(module_name, jobid);

  Log << SeqLog << Debug << "Importing module: '" << module_name << "'...";

  if (const auto module_content = self.m_shared->m_fetch_module(module_uri)) {
    return RenderTranslationUnitSource(self, module_content.value());
  }

  Log << SeqLog << "Import not found: '" << module_name << "'";

  return std::nullopt;
}
