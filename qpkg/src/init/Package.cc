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
#include <core/Logger.hh>
#include <fstream>
#include <init/Package.hh>
#include <regex>

static bool touch(const std::filesystem::path &path) {
  using namespace qpkg;

  try {
    std::ofstream file(path.string());
    if (!file.is_open()) {
      LOG(core::ERROR) << "Failed to create file: " << path << std::endl;
      return false;
    }

    return true;
  } catch (const std::exception &e) {
    LOG(core::ERROR) << "Failed to create file: " << path << " (" << e.what()
                     << ")" << std::endl;
    return false;
  }
}

bool qpkg::init::Package::validateName(const std::string &name) {
  static std::regex regex("^[a-zA-Z0-9_]+$");
  return std::regex_match(name, regex);
}

bool qpkg::init::Package::validateVersion(const std::string &version) {
  static std::regex regex("^[0-9]+\\.[0-9]+\\.[0-9]+$");
  return std::regex_match(version, regex);
}

bool qpkg::init::Package::validateEmail(const std::string &email) {
  static std::regex regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
  return std::regex_match(email, regex);
}

bool qpkg::init::Package::validateUrl(const std::string &url) {
  static std::regex regex("^(http|https)://.*$");
  return std::regex_match(url, regex);
}

bool qpkg::init::Package::validateLicense(const std::string &license) {
  static std::regex regex("^[a-zA-Z0-9_]+$");
  return std::regex_match(license, regex);
}

void qpkg::init::Package::writeGitIgnore() {
  std::ofstream gitignore((m_output / m_name / ".gitignore").string());
  if (!gitignore.is_open()) {
    LOG(core::ERROR) << "Failed to create .gitignore file" << std::endl;
    return;
  }

  gitignore << ".qpkg/\n";
  gitignore << m_name << "\n";
  gitignore << m_name << ".exe\n";
  gitignore << "lib" << m_name << ".a\n";
  gitignore << "lib" << m_name << ".so\n";
}

void qpkg::init::Package::writeLicense() {
  std::ofstream license((m_output / m_name / "LICENSE").string());

  if (!license.is_open()) {
    LOG(core::ERROR) << "Failed to create LICENSE file" << std::endl;
    return;
  }

  if (m_license.empty()) {
    LOG(core::WARN) << "No license specified" << std::endl;
    return;
  }

  if (!conf::valid_licenses.contains(m_license)) {
    LOG(core::WARN) << "License is not a recognized SPDX license: " << m_license
                    << std::endl;
    return;
  }

  license << conf::valid_licenses.at(m_license) << std::endl;
}

void qpkg::init::Package::writeMain() {
  std::ofstream main((m_output / m_name / "src/main.q").string());
  if (!main.is_open()) {
    LOG(core::ERROR) << "Failed to create main.q file" << std::endl;
    return;
  }

  main << R"(@use "v1.0";

@import std;

fn main() {
    printn("Hello, World!");
}
)";
}

bool qpkg::init::Package::createPackage() {
  switch (m_type) {
    case PackageType::PROGRAM:
      LOG(core::DEBUG) << "Creating program package" << std::endl;
      break;
    case PackageType::STATICLIB:
      LOG(core::DEBUG) << "Creating static library package" << std::endl;
      break;
    case PackageType::SHAREDLIB:
      LOG(core::DEBUG) << "Creating shared library package" << std::endl;
      break;
  }

  try {
    if (!std::filesystem::create_directories(m_output / m_name / "src") ||
        !std::filesystem::create_directories(m_output / m_name / "include") ||
        !std::filesystem::create_directories(m_output / m_name / "test") ||
        !std::filesystem::create_directories(m_output / m_name / "doc")) {
      LOG(core::ERROR) << "Failed to create package directories" << std::endl;
      return false;
    }

    if (!touch(m_output / m_name / "include/.gitkeep") ||
        !touch(m_output / m_name / "test/.gitkeep") ||
        !touch(m_output / m_name / "doc/.gitkeep")) {
      LOG(core::ERROR) << "Failed to create package directories" << std::endl;
      return false;
    }

    if (!touch(m_output / m_name / "README.md")) {
      LOG(core::ERROR) << "Failed to create package files" << std::endl;
      return false;
    }

    conf::ConfigGroup grp;

    grp.set("version", 1);
    grp.set("name", m_name);
    grp.set("description", m_description);

    if (!m_author.empty())
      grp.set("authors", std::vector<std::string>({m_author}));

    if (!m_email.empty())
      grp.set("emails", std::vector<std::string>({m_email}));

    if (!m_url.empty()) grp.set("url", m_url);

    if (!m_license.empty())
      grp.set("licenses", std::vector<std::string>({m_license}));
    else
      grp.set("licenses", std::vector<std::string>());

    grp.set("sources", std::vector<std::string>({"src"}));
    grp.set("headers", std::vector<std::string>({"include"}));

    switch (m_type) {
      case PackageType::PROGRAM:
        grp.set("target", "executable");
        break;
      case PackageType::STATICLIB:
        grp.set("target", "staticlib");
        break;
      case PackageType::SHAREDLIB:
        grp.set("target", "sharedlib");
        break;
    }

    conf::Config config(grp, 0);

    std::ofstream config_file((m_output / m_name / "qpkg.yaml").string());
    if (!config_file.is_open()) {
      LOG(core::ERROR) << "Failed to create package configuration file"
                       << std::endl;
      return false;
    }

    config_file << config.dump(conf::ConfigItemSerializationTarget::YAML);

    writeGitIgnore();
    writeLicense();
    writeMain();

    setenv("QPKG_GIT_INJECT_DEST", (m_output / m_name).string().c_str(), 1);
    if (system("git init $QPKG_GIT_INJECT_DEST") != 0) {
      LOG(core::ERROR) << "Failed to initialize git repository" << std::endl;
      return false;
    }

    switch (m_type) {
      case PackageType::PROGRAM:
        LOG(core::DEBUG) << "Program package created" << std::endl;
        break;
      case PackageType::STATICLIB:
        LOG(core::DEBUG) << "Static library package created" << std::endl;
        break;
      case PackageType::SHAREDLIB:
        LOG(core::DEBUG) << "Shared library package created" << std::endl;
        break;
    }

    return true;
  } catch (const std::exception &e) {
    LOG(core::ERROR) << "Failed to create program package: " << e.what()
                     << std::endl;
    return false;
  }
}

bool qpkg::init::Package::create() {
  if (!validateName(m_name)) {
    LOG(core::ERROR) << "Invalid package name: " << m_name << std::endl;
    return false;
  }

  if (!validateVersion(m_version)) {
    LOG(core::ERROR) << "Invalid package version: " << m_version << std::endl;
    return false;
  }

  if (!m_email.empty() && !validateEmail(m_email)) {
    LOG(core::ERROR) << "Invalid package email: " << m_email << std::endl;
    return false;
  }

  if (!m_url.empty() && !validateUrl(m_url)) {
    LOG(core::ERROR) << "Invalid package url: " << m_url << std::endl;
    return false;
  }

  if (!m_license.empty() && !validateLicense(m_license)) {
    LOG(core::ERROR) << "Invalid package license: " << m_license << std::endl;
    return false;
  }

  try {
    if (std::filesystem::exists(m_output / m_name)) {
      if (m_force) {
        std::filesystem::remove_all(m_output / m_name);
      } else {
        LOG(core::ERROR) << "Package already exists: " << m_output / m_name
                         << std::endl;
        return false;
      }
    }

    if (!std::filesystem::create_directories(m_output / m_name)) {
      LOG(core::ERROR) << "Failed to create package directory: "
                       << m_output / m_name << std::endl;
      return false;
    }
  } catch (const std::exception &e) {
    LOG(core::ERROR) << "Failed to check package existence: " << e.what()
                     << std::endl;
    return false;
  }

  return createPackage();
}

qpkg::init::PackageBuilder &qpkg::init::PackageBuilder::output(
    const std::string &output) {
  m_output = output;
  return *this;
}

qpkg::init::PackageBuilder &qpkg::init::PackageBuilder::name(
    const std::string &name) {
  m_name = name;
  return *this;
}

qpkg::init::PackageBuilder &qpkg::init::PackageBuilder::license(
    const std::string &license) {
  m_license = license;
  return *this;
}

qpkg::init::PackageBuilder &qpkg::init::PackageBuilder::author(
    const std::string &author) {
  m_author = author;
  return *this;
}

qpkg::init::PackageBuilder &qpkg::init::PackageBuilder::email(
    const std::string &email) {
  m_email = email;
  return *this;
}

qpkg::init::PackageBuilder &qpkg::init::PackageBuilder::url(
    const std::string &url) {
  m_url = url;
  return *this;
}

qpkg::init::PackageBuilder &qpkg::init::PackageBuilder::version(
    const std::string &version) {
  m_version = version;
  return *this;
}

qpkg::init::PackageBuilder &qpkg::init::PackageBuilder::description(const std::string &description)
{
  m_description = description;
  return *this;

}

qpkg::init::PackageBuilder &qpkg::init::PackageBuilder::type(
    qpkg::init::PackageType type) {
  m_type = type;
  return *this;
}

qpkg::init::PackageBuilder &qpkg::init::PackageBuilder::verbose(bool verbose) {
  m_verbose = verbose;
  return *this;
}

qpkg::init::PackageBuilder &qpkg::init::PackageBuilder::force(bool force) {
  m_force = force;
  return *this;
}

qpkg::init::Package qpkg::init::PackageBuilder::build() {
  return qpkg::init::Package(m_output, m_name, m_license, m_author, m_email,
                             m_url, m_version, m_description, m_type, m_verbose, m_force);
}