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

#ifndef __QPKG_INIT_PACKAGE_HH__
#define __QPKG_INIT_PACKAGE_HH__

#include <conf/Config.hh>
#include <filesystem>
#include <optional>
#include <set>
#include <string>

namespace qpkg {
  namespace init {
    enum class PackageType { PROGRAM, STATICLIB, SHAREDLIB };

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

      bool createPackage();

      static bool validateName(const std::string &name);
      static bool validateVersion(const std::string &version);
      static bool validateEmail(const std::string &email);
      static bool validateUrl(const std::string &url);
      static bool validateLicense(const std::string &license);

      void writeGitIgnore();
      void writeLicense();
      void writeMain();

    public:
      Package(std::filesystem::path output, std::string name, std::string license,
              std::string author, std::string email, std::string url, std::string version,
              std::string description, PackageType type, bool verbose, bool force)
          : m_output(output),
            m_name(name),
            m_license(license),
            m_author(author),
            m_email(email),
            m_url(url),
            m_version(version),
            m_description(description),
            m_type(type),
            m_verbose(verbose),
            m_force(force) {
        (void)m_verbose;
      }

      bool create();
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
      bool m_verbose;
      bool m_force;

    public:
      PackageBuilder() : m_verbose(false), m_force(false) {}

      PackageBuilder &output(const std::string &output);
      PackageBuilder &name(const std::string &name);
      PackageBuilder &license(const std::string &license);
      PackageBuilder &author(const std::string &author);
      PackageBuilder &email(const std::string &email);
      PackageBuilder &url(const std::string &url);
      PackageBuilder &version(const std::string &version);
      PackageBuilder &description(const std::string &description);
      PackageBuilder &type(PackageType type);
      PackageBuilder &verbose(bool verbose);
      PackageBuilder &force(bool force);

      Package build();
    };
  }  // namespace init
}  // namespace qpkg

#endif  // __QPKG_INIT_PACKAGE_HH__