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

#ifndef __QPKG_BUILD_ENGINE_BUILDER_HH__
#define __QPKG_BUILD_ENGINE_BUILDER_HH__

#include <build/Engine.hh>
#include <set>
#include <string>

namespace qpkg {
  namespace build {
    class EngineBuilder {
      std::string m_package_src;
      std::string m_output;
      std::set<std::string> m_trustkeys;
      std::set<std::string> m_trustkeyfiles;
      std::pair<std::string, std::string> m_certify;
      int m_jobs;
      bool m_disable_cache;
      bool m_verbose;
      bool m_optimize;
      bool m_optimize_size;
      bool m_debug;
      bool m_disable_sigcheck;

    public:
      EngineBuilder()
          : m_jobs(0),
            m_disable_cache(false),
            m_verbose(false),
            m_optimize(false),
            m_optimize_size(false),
            m_debug(false),
            m_disable_sigcheck(false) {}

      EngineBuilder &set_package_src(const std::string &directory);
      EngineBuilder &set_output(const std::string &output);
      EngineBuilder &disable_cache();
      EngineBuilder &jobs(int jobs);
      EngineBuilder &verbose();
      EngineBuilder &optimize();
      EngineBuilder &optimize_size();
      EngineBuilder &debug();
      EngineBuilder &certify(const std::string &pkcs12);
      EngineBuilder &certify_password(const std::string &password);
      EngineBuilder &disable_sigcheck();
      EngineBuilder &trustkey(const std::string &key);
      EngineBuilder &trustkeys(const std::string &keyfile);

      std::optional<Engine> build();
    };
  }  // namespace build
}  // namespace qpkg

#endif  // __QPKG_BUILD_ENGINE_BUILDER_HH__