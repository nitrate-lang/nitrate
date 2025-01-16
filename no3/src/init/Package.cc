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

#include <conf/SPDX.hh>
#include <core/Logger.hh>
#include <fstream>
#include <init/Package.hh>
#include <random>
#include <regex>
#include <sstream>

using namespace no3::init;

class GitConfig final {
  git_config *m_config;

public:
  GitConfig(git_config *conf) : m_config(conf) {}

  ~GitConfig() { git_config_free(m_config); }

  bool SetString(const char *key, const char *value) {
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

  [[nodiscard]] bool DidInit() const { return m_repo != nullptr; }

  std::optional<GitConfig> GetConfig() {
    git_config *config;
    if (git_repository_config(&config, m_repo) != 0) {
      return std::nullopt;
    }

    return config;
  }
};

static std::string GenerateUUIDv4() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 15);
  static constexpr std::array<char, 16> kHexChars = {
      '0', '1', '2', '3', '4', '5', '6', '7',
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

static bool CreatePackageRepository(const char *path, PackageType type) {
  static constexpr std::array kPackageTypes = {"application", "staticlib",
                                               "sharedlib"};

  GitRepository repo(path);
  if (!repo.DidInit()) {
    LOG(ERROR) << "Failed to initialize git repository" << std::endl;
    return false;
  }

  auto config = repo.GetConfig();
  if (!config) {
    LOG(ERROR) << "Failed to retrieve git configuration" << std::endl;
    return false;
  }

  bool config_ok =
      config->SetString("no3.package", GenerateUUIDv4().c_str()) &&
      config->SetString("no3.type", kPackageTypes[static_cast<int>(type)]);

  if (!config_ok) {
    LOG(ERROR) << "Failed to set git configuration" << std::endl;
    return false;
  }

  return true;
}

bool no3::init::Package::ValidateName(const std::string &name) {
  static std::regex regex("^[a-zA-Z0-9_-]+$");
  return std::regex_match(name, regex);
}

bool no3::init::Package::ValidateVersion(const std::string &version) {
  static std::regex regex("^[0-9]+\\.[0-9]+\\.[0-9]+$");
  return std::regex_match(version, regex);
}

bool no3::init::Package::ValidateEmail(const std::string &email) {
  static std::regex regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
  return std::regex_match(email, regex);
}

bool no3::init::Package::ValidateUrl(const std::string &url) {
  static std::regex regex("^(http|https)://.*$");
  return std::regex_match(url, regex);
}

bool no3::init::Package::ValidateLicense(const std::string &license) {
  return conf::SPDX_IDENTIFIERS.contains(license);
}

bool no3::init::Package::WriteGitIgnore() {
  std::ofstream gitignore((m_output / m_name / ".gitignore").string());
  if (!gitignore.is_open()) {
    LOG(ERROR) << "Failed to create .gitignore file" << std::endl;
    return false;
  }

  gitignore << ".no3/\n";
  gitignore << m_name << "\n";
  gitignore << m_name << ".exe\n";
  gitignore << "lib" << m_name << ".a\n";
  gitignore << "lib" << m_name << ".so\n";

  return true;
}

bool no3::init::Package::WriteMain() {
  if (!std::filesystem::create_directories(m_output / m_name / "src")) {
    LOG(ERROR) << "Failed to create package directories" << std::endl;
    return false;
  }

  std::ofstream main((m_output / m_name / "src/main.n").string());
  if (!main.is_open()) {
    LOG(ERROR) << "Failed to create main.q file" << std::endl;
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

bool no3::init::Package::WriteReadme() {
  std::ofstream readme((m_output / m_name / "README.md").string());
  if (!readme.is_open()) {
    LOG(ERROR) << "Failed to create README file" << std::endl;
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

bool no3::init::Package::WriteConfig() {
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
    LOG(ERROR) << "Failed to create package configuration file" << std::endl;
    return false;
  }

  config_file << config.Dump(conf::ConfigItemSerializationTarget::YAML);

  return true;
}

bool no3::init::Package::CreatePackage() {
  switch (m_type) {
    case PackageType::PROGRAM:
      LOG(INFO) << "Creating program package" << std::endl;
      break;
    case PackageType::STATICLIB:
      LOG(INFO) << "Creating static library package" << std::endl;
      break;
    case PackageType::SHAREDLIB:
      LOG(INFO) << "Creating shared library package" << std::endl;
      break;
  }

  try {
    if (!WriteGitIgnore() || !WriteMain() || !WriteReadme() || !WriteConfig()) {
      return false;
    }

    if (!CreatePackageRepository((m_output / m_name).c_str(), m_type)) {
      LOG(ERROR) << "Failed to create git repository" << std::endl;
      return false;
    }

    switch (m_type) {
      case PackageType::PROGRAM:
        LOG(INFO) << "Program package created" << std::endl;
        break;
      case PackageType::STATICLIB:
        LOG(INFO) << "Static library package created" << std::endl;
        break;
      case PackageType::SHAREDLIB:
        LOG(INFO) << "Shared library package created" << std::endl;
        break;
    }

    return true;
  } catch (const std::exception &e) {
    LOG(ERROR) << "Failed to create program package: " << e.what() << std::endl;
    return false;
  }
}

bool no3::init::Package::Create() {
  if (!ValidateName(m_name)) {
    LOG(ERROR) << "Invalid package name: " << m_name << std::endl;
    return false;
  }

  if (!ValidateVersion(m_version)) {
    LOG(ERROR) << "Invalid package version: " << m_version << std::endl;
    return false;
  }

  if (!m_email.empty() && !ValidateEmail(m_email)) {
    LOG(ERROR) << "Invalid package email: " << m_email << std::endl;
    return false;
  }

  if (!m_url.empty() && !ValidateUrl(m_url)) {
    LOG(ERROR) << "Invalid package url: " << m_url << std::endl;
    return false;
  }

  if (!m_license.empty() && !ValidateLicense(m_license)) {
    LOG(ERROR) << "Invalid package SPDX license identifier: " << m_license
               << std::endl;
    return false;
  }

  try {
    if (std::filesystem::exists(m_output / m_name)) {
      if (m_force) {
        std::filesystem::remove_all(m_output / m_name);
      } else {
        LOG(ERROR) << "Package already exists: " << m_output / m_name
                   << std::endl;
        return false;
      }
    }

    if (!std::filesystem::create_directories(m_output / m_name)) {
      LOG(ERROR) << "Failed to create package directory: " << m_output / m_name
                 << std::endl;
      return false;
    }
  } catch (const std::exception &e) {
    LOG(ERROR) << "Failed to check package existence: " << e.what()
               << std::endl;
    return false;
  }

  return CreatePackage();
}

no3::init::PackageBuilder &no3::init::PackageBuilder::Output(
    const std::string &output) {
  m_output = output;
  return *this;
}

no3::init::PackageBuilder &no3::init::PackageBuilder::Name(
    const std::string &name) {
  m_name = name;
  return *this;
}

no3::init::PackageBuilder &no3::init::PackageBuilder::License(
    const std::string &license) {
  m_license = license;
  return *this;
}

no3::init::PackageBuilder &no3::init::PackageBuilder::Author(
    const std::string &author) {
  m_author = author;
  return *this;
}

no3::init::PackageBuilder &no3::init::PackageBuilder::Email(
    const std::string &email) {
  m_email = email;
  return *this;
}

no3::init::PackageBuilder &no3::init::PackageBuilder::Url(
    const std::string &url) {
  m_url = url;
  return *this;
}

no3::init::PackageBuilder &no3::init::PackageBuilder::Version(
    const std::string &version) {
  m_version = version;
  return *this;
}

no3::init::PackageBuilder &no3::init::PackageBuilder::Description(
    const std::string &description) {
  m_description = description;
  return *this;
}

no3::init::PackageBuilder &no3::init::PackageBuilder::Type(
    no3::init::PackageType type) {
  m_type = type;
  return *this;
}

no3::init::PackageBuilder &no3::init::PackageBuilder::Verbose(bool verbose) {
  m_verbose = verbose;
  return *this;
}

no3::init::PackageBuilder &no3::init::PackageBuilder::Force(bool force) {
  m_force = force;
  return *this;
}

no3::init::Package no3::init::PackageBuilder::Build() {
  return {m_output,  m_name,        m_license, m_author,  m_email, m_url,
          m_version, m_description, m_type,    m_verbose, m_force};
}