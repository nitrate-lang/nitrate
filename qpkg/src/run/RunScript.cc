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

#include <iostream>
#include <quixcc/Quix.hpp>
#include <run/RunScript.hh>

#if __linux__
#include <sys/wait.h>
#endif

#define QPKG_ERROR 10

qpkg::run::RunScript::RunScript(const std::string &scriptfile) {
  m_scriptfile = scriptfile;
}

bool qpkg::run::RunScript::is_okay() const { return true; }

int qpkg::run::RunScript::run(const std::vector<std::string> &args) {
  if (!std::filesystem::exists(m_scriptfile)) {
    LOG(core::ERROR) << "Script file does not exist: " << m_scriptfile
                     << std::endl;
    return QPKG_ERROR;
  }

  /*
   * 1. compile the script into temp file
   * 2. run the binary with program arguments `args`
   * 3. return the exit code of the subprocess
   **/

  char tempfile[] = "/tmp/qpkg-script-XXXXXX";
  if (mktemp(tempfile) == nullptr) {
    LOG(core::ERROR) << "Failed to create temporary file" << std::endl;
    return QPKG_ERROR;
  }

  int exit_code = QPKG_ERROR;
  try {
    quixcc::CompilerBuilder builder;
    builder.set_output(tempfile);
    builder.set_verbosity(quixcc::Verbosity::NORMAL);
    builder.set_optimization(quixcc::OptimizationLevel::SPEED_4);
    builder.add_source(m_scriptfile);
    builder.opt("-c");
    if (!builder.build().run(1).puts().ok()) return QPKG_ERROR;

    std::string linker_cmd =
        "qld " + std::string(tempfile) + " -o " + tempfile + ".tmp";
    if (system(linker_cmd.c_str()) != 0) {
      LOG(core::ERROR) << "Failed to link the binary" << std::endl;
      return QPKG_ERROR;
    }

    std::string run_cmd = std::string(tempfile) + ".tmp";
    for (const auto &arg : args) {
      run_cmd += " " + arg;
    }

    exit_code = system(run_cmd.c_str());

#if __linux__
    exit_code = WEXITSTATUS(exit_code);
#endif

  } catch (const std::exception &e) {
    LOG(core::ERROR) << "Failed to compile the script: " << e.what()
                     << std::endl;
  }

  std::filesystem::remove(tempfile);
  std::filesystem::remove(std::string(tempfile) + ".tmp");
  return exit_code;
}