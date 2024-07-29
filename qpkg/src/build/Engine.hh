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

#ifndef __QPKG_BUILD_ENGINE_HH__
#define __QPKG_BUILD_ENGINE_HH__

#include <cache/CacheDirectory.hh>
#include <conf/Config.hh>
#include <core/Logger.hh>
#include <filesystem>
#include <optional>
#include <set>
#include <string>

namespace qpkg {
  namespace build {
    enum class Optimization { OPTIMIZE_DEFAULT, OPTIMIZE_SIZE, OPTIMIZE_SPEED };

    struct BuildSecurityConfig {
      std::set<std::string> trustkeys;
      std::optional<std::pair<std::string, std::string>> certify;
      bool disable_sigcheck;

      BuildSecurityConfig() : disable_sigcheck(false) {}

      BuildSecurityConfig(std::set<std::string> trustkeys,
                          std::optional<std::pair<std::string, std::string>> certify,
                          bool disable_sigcheck)
          : trustkeys(trustkeys), certify(certify), disable_sigcheck(disable_sigcheck) {}
    };

    class Engine {
      enum class BuildType { SHAREDLIB, STATICLIB, EXECUTABLE };

      std::string m_package_src;
      std::string m_package_name;
      std::unique_ptr<cache::ICache> m_cache;
      conf::Config m_config;
      std::string m_output;
      int m_jobs;
      BuildSecurityConfig m_security;
      Optimization m_optimization;
      bool m_debug;
      bool m_verbose;
      BuildType m_build_type;

      std::optional<qpkg::conf::Config> load_config(const std::filesystem::path &base);
      std::vector<std::string> get_source_files(const std::filesystem::path &base);
      bool run_threads(const std::filesystem::path &base,
                       const std::vector<std::string> &source_files,
                       const std::filesystem::path &build_dir, qpkg::core::Process &log) const;
      bool build_package(const std::filesystem::path &base,
                         const std::vector<std::string> &source_files,
                         const std::filesystem::path &build_dir);
      bool build_source_file(const std::filesystem::path &base,
                             const std::filesystem::path &build_dir,
                             const std::filesystem::path &file) const;
      bool link_objects(const std::vector<std::filesystem::path> &objects) const;

    public:
      Engine(const std::string &package_src, const std::string &output, int jobs,
             std::unique_ptr<cache::ICache> cache, BuildSecurityConfig security,
             Optimization optimization, bool debug, bool verbose);

      bool run();
    };
  }  // namespace build
}  // namespace qpkg

#endif  // __QPKG_BUILD_ENGINE_HH__