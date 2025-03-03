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

#include <git2.h>
#include <git2/config.h>
#include <git2/repository.h>

#include <array>
#include <conf/SPDX.hh>
#include <fstream>
#include <init/Package.hh>
#include <nitrate-core/Logger.hh>
#include <optional>
#include <random>
#include <regex>
#include <sstream>

using namespace ncc;
using namespace no3::init;

class GitConfig final {
  git_config *m_config;

public:
  GitConfig(git_config *conf) : m_config(conf) {}

  ~GitConfig() { git_config_free(m_config); }

  auto SetString(const char *key, const char *value) -> bool {
    return git_config_set_string(m_config, key, value) == 0;
  }

  operator git_config *() const { return m_config; }
};

class GitRepository final {
  git_repository *m_repo;

public:
  GitRepository(const char *path) {
    if (git_repository_init(&m_repo, path, 0) != 0) {
      m_repo = nullptr;
    }
  }

  ~GitRepository() { git_repository_free(m_repo); }

  [[nodiscard]] auto DidInit() const -> bool { return m_repo != nullptr; }

  auto GetConfig() -> std::optional<GitConfig> {
    git_config *config;
    if (git_repository_config(&config, m_repo) != 0) {
      return std::nullopt;
    }

    return config;
  }
};

static auto GenerateUUIDv4() -> std::string {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 15);
  static constexpr std::array<char, 16> kHexChars = {'0', '1', '2', '3', '4', '5', '6', '7',
                                                     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  std::stringstream ss;
  for (int i = 0; i < 36; i++) {
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      ss << "-";
    } else if (i == 14) {
      ss << "4";
    } else if (i == 19) {
      ss << kHexChars[dis(gen) | 0x8];
    } else {
      ss << kHexChars[dis(gen)];
    }
  }

  return ss.str();
}

static auto CreatePackageRepository(const char *path, PackageType type) -> bool {
  static constexpr std::array kPackageTypes = {"application", "staticlib", "sharedlib"};

  GitRepository repo(path);
  if (!repo.DidInit()) {
    Log << "Failed to initialize git repository";
    return false;
  }

  auto config = repo.GetConfig();
  if (!config) {
    Log << "Failed to retrieve git configuration";
    return false;
  }

  bool config_ok = config->SetString("no3.package", GenerateUUIDv4().c_str()) &&
                   config->SetString("no3.type", kPackageTypes[static_cast<int>(type)]);

  if (!config_ok) {
    Log << "Failed to set git configuration";
    return false;
  }

  return true;
}

auto no3::init::Package::ValidateName(const std::string &name) -> bool {
  static std::regex regex("^[a-zA-Z0-9_-]+$");
  return std::regex_match(name, regex);
}

auto no3::init::Package::ValidateVersion(const std::string &version) -> bool {
  static std::regex regex("^[0-9]+\\.[0-9]+\\.[0-9]+$");
  return std::regex_match(version, regex);
}

auto no3::init::Package::ValidateEmail(const std::string &email) -> bool {
  static std::regex regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
  return std::regex_match(email, regex);
}

auto no3::init::Package::ValidateUrl(const std::string &url) -> bool {
  static std::regex regex("^(http|https)://.*$");
  return std::regex_match(url, regex);
}

auto no3::init::Package::ValidateLicense(const std::string &license) -> bool {
  return conf::SPDX_IDENTIFIERS.contains(license);
}

auto no3::init::Package::WriteGitIgnore() -> bool {
  std::ofstream gitignore((m_output / m_name / ".gitignore").string());
  if (!gitignore.is_open()) {
    Log << "Failed to create .gitignore file";
    return false;
  }

  gitignore << ".no3/\n";
  gitignore << m_name << "\n";
  gitignore << m_name << ".exe\n";
  gitignore << "lib" << m_name << ".a\n";
  gitignore << "lib" << m_name << ".so\n";

  return true;
}

auto no3::init::Package::WriteMain() -> bool {
  if (!std::filesystem::create_directories(m_output / m_name / "src")) {
    Log << "Failed to create package directories";
    return false;
  }

  std::ofstream main((m_output / m_name / "src/main.n").string());
  if (!main.is_open()) {
    Log << "Failed to create main.q file";
    return false;
  }

  main << R"(@use "v1.0";

pub "std" fn main(args: [str]): i32 {
  printn("Whatcha doing, Stepbro?");

  ret 0;
}
)";

  return true;
}

auto no3::init::Package::WriteReadme() -> bool {
  std::ofstream readme((m_output / m_name / "README.md").string());
  if (!readme.is_open()) {
    Log << "Failed to create README file";
    return false;
  }

  std::string capitalized_name = m_name;
  if (!capitalized_name.empty()) {
    capitalized_name[0] = std::toupper(capitalized_name[0]);
  }

  readme << "# " << capitalized_name << " Project\n\n";

  if (!m_description.empty()) {
    readme << "## Overview\n\n";
    readme << m_description << "\n\n";
  }

  if (!m_url.empty()) {
    readme << "## Project URL\n\n";
    readme << m_url << "\n\n";
  }

  if (!m_license.empty()) {
    readme << "## License\n\n";
    readme << m_license << "\n\n";
  }

  if (!m_version.empty()) {
    readme << "## Version\n\n";
    readme << m_version << "\n\n";
  }

  if (!m_email.empty()) {
    readme << "## Contact\n\n";
    readme << m_email << "\n\n";
  }

  if (!m_author.empty()) {
    readme << "## Authors\n\n";
    readme << m_author << "\n\n";
  }

  return true;
}

auto no3::init::Package::WriteConfig() -> bool {
  conf::ConfigGroup grp;

  grp.Set("version", 1);
  grp.Set("name", m_name);
  grp.Set("description", m_description);

  if (!m_author.empty()) {
    grp.Set("authors", std::vector<std::string>({m_author}));
  }

  if (!m_email.empty()) {
    grp.Set("emails", std::vector<std::string>({m_email}));
  }

  if (!m_url.empty()) {
    grp.Set("url", m_url);
  }

  if (!m_license.empty()) {
    grp.Set("licenses", std::vector<std::string>({m_license}));
  } else {
    grp.Set("licenses", std::vector<std::string>());
  }

  grp.Set("sources", std::vector<std::string>({"src"}));

  switch (m_type) {
    case PackageType::PROGRAM:
      grp.Set("target", "executable");
      break;
    case PackageType::STATICLIB:
      grp.Set("target", "staticlib");
      break;
    case PackageType::SHAREDLIB:
      grp.Set("target", "sharedlib");
      break;
  }

  conf::Config config(grp, 0);

  std::ofstream config_file((m_output / m_name / "no3.yaml").string());
  if (!config_file.is_open()) {
    Log << "Failed to create package configuration file";
    return false;
  }

  config_file << config.Dump(conf::ConfigItemSerializationTarget::YAML);

  return true;
}

auto no3::init::Package::CreatePackage() -> bool {
  switch (m_type) {
    case PackageType::PROGRAM:
      Log << Info << "Creating program package";
      break;
    case PackageType::STATICLIB:
      Log << Info << "Creating static library package";
      break;
    case PackageType::SHAREDLIB:
      Log << Info << "Creating shared library package";
      break;
  }

  try {
    if (!WriteGitIgnore() || !WriteMain() || !WriteReadme() || !WriteConfig()) {
      return false;
    }

    if (!CreatePackageRepository((m_output / m_name).c_str(), m_type)) {
      Log << "Failed to create git repository";
      return false;
    }

    switch (m_type) {
      case PackageType::PROGRAM:
        Log << Info << "Program package created";
        break;
      case PackageType::STATICLIB:
        Log << Info << "Static library package created";
        break;
      case PackageType::SHAREDLIB:
        Log << Info << "Shared library package created";
        break;
    }

    return true;
  } catch (const std::exception &e) {
    Log << "Failed to create program package: " << e.what();
    return false;
  }
}

auto no3::init::Package::Create() -> bool {
  if (!ValidateName(m_name)) {
    Log << "Invalid package name: " << m_name;
    return false;
  }

  if (!ValidateVersion(m_version)) {
    Log << "Invalid package version: " << m_version;
    return false;
  }

  if (!m_email.empty() && !ValidateEmail(m_email)) {
    Log << "Invalid package email: " << m_email;
    return false;
  }

  if (!m_url.empty() && !ValidateUrl(m_url)) {
    Log << "Invalid package url: " << m_url;
    return false;
  }

  if (!m_license.empty() && !ValidateLicense(m_license)) {
    Log << "Invalid package SPDX license identifier: " << m_license;
    return false;
  }

  try {
    if (std::filesystem::exists(m_output / m_name)) {
      if (m_force) {
        std::filesystem::remove_all(m_output / m_name);
      } else {
        Log << "Package already exists: " << m_output / m_name;
        return false;
      }
    }

    if (!std::filesystem::create_directories(m_output / m_name)) {
      Log << "Failed to create package directory: " << m_output / m_name;
      return false;
    }
  } catch (const std::exception &e) {
    Log << "Failed to check package existence: " << e.what();
    return false;
  }

  return CreatePackage();
}

auto no3::init::PackageBuilder::Output(const std::string &output) -> no3::init::PackageBuilder & {
  m_output = output;
  return *this;
}

auto no3::init::PackageBuilder::Name(const std::string &name) -> no3::init::PackageBuilder & {
  m_name = name;
  return *this;
}

auto no3::init::PackageBuilder::License(const std::string &license) -> no3::init::PackageBuilder & {
  m_license = license;
  return *this;
}

auto no3::init::PackageBuilder::Author(const std::string &author) -> no3::init::PackageBuilder & {
  m_author = author;
  return *this;
}

auto no3::init::PackageBuilder::Email(const std::string &email) -> no3::init::PackageBuilder & {
  m_email = email;
  return *this;
}

auto no3::init::PackageBuilder::Url(const std::string &url) -> no3::init::PackageBuilder & {
  m_url = url;
  return *this;
}

auto no3::init::PackageBuilder::Version(const std::string &version) -> no3::init::PackageBuilder & {
  m_version = version;
  return *this;
}

auto no3::init::PackageBuilder::Description(const std::string &description) -> no3::init::PackageBuilder & {
  m_description = description;
  return *this;
}

auto no3::init::PackageBuilder::Type(no3::init::PackageType type) -> no3::init::PackageBuilder & {
  m_type = type;
  return *this;
}

auto no3::init::PackageBuilder::Verbose(bool verbose) -> no3::init::PackageBuilder & {
  m_verbose = verbose;
  return *this;
}

auto no3::init::PackageBuilder::Force(bool force) -> no3::init::PackageBuilder & {
  m_force = force;
  return *this;
}

auto no3::init::PackageBuilder::Build() -> no3::init::Package {
  return {m_output, m_name, m_license, m_author, m_email, m_url, m_version, m_description, m_type, m_verbose, m_force};
}