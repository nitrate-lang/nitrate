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

#ifndef __NO3_INIT_PACKAGE_HH__
#define __NO3_INIT_PACKAGE_HH__

#include <conf/Config.hh>
#include <filesystem>
#include <string>

namespace no3::init {
  enum class PackageType : uint8_t { PROGRAM, STATICLIB, SHAREDLIB };

  class Package {
    std::filesystem::path m_output;
    std::string m_name;
    std::string m_license;
    std::string m_author;
    std::string m_email;
    std::string m_url;
    std::string m_version;
    std::string m_description;
    PackageType m_type;
    bool m_verbose;
    bool m_force;

    auto CreatePackage() -> bool;

    static auto ValidateName(const std::string &name) -> bool;
    static auto ValidateVersion(const std::string &version) -> bool;
    static auto ValidateEmail(const std::string &email) -> bool;
    static auto ValidateUrl(const std::string &url) -> bool;
    static auto ValidateLicense(const std::string &license) -> bool;

    auto WriteGitIgnore() -> bool;
    auto WriteMain() -> bool;
    auto WriteReadme() -> bool;
    auto WriteConfig() -> bool;

  public:
    Package(auto output, auto name, auto license, auto author, auto email, auto url, auto version, auto description,
            PackageType type, auto verbose, auto force)
        : m_output(std::move(output)),
          m_name(std::move(name)),
          m_license(std::move(license)),
          m_author(std::move(author)),
          m_email(std::move(email)),
          m_url(std::move(url)),
          m_version(std::move(version)),
          m_description(std::move(description)),
          m_type(type),
          m_verbose(verbose),
          m_force(force) {
      (void)m_verbose;
    }

    auto Create() -> bool;
  };

  class PackageBuilder {
    std::string m_output;
    std::string m_name;
    std::string m_license;
    std::string m_author;
    std::string m_email;
    std::string m_url;
    std::string m_version;
    std::string m_description;
    PackageType m_type;
    bool m_verbose{};
    bool m_force{};

  public:
    PackageBuilder() = default;

    auto Output(const std::string &output) -> PackageBuilder &;
    auto Name(const std::string &name) -> PackageBuilder &;
    auto License(const std::string &license) -> PackageBuilder &;
    auto Author(const std::string &author) -> PackageBuilder &;
    auto Email(const std::string &email) -> PackageBuilder &;
    auto Url(const std::string &url) -> PackageBuilder &;
    auto Version(const std::string &version) -> PackageBuilder &;
    auto Description(const std::string &description) -> PackageBuilder &;
    auto Type(PackageType type) -> PackageBuilder &;
    auto Verbose(bool verbose) -> PackageBuilder &;
    auto Force(bool force) -> PackageBuilder &;

    auto Build() -> Package;
  };
}  // namespace no3::init

#endif  // __NO3_INIT_PACKAGE_HH__