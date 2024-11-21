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

#include <argparse.h>
#include <glog/logging.h>
#include <lsp/nitrated.h>
#include <nitrate-core/Error.h>
#include <nitrate-core/Lib.h>
#include <nitrate-emit/Code.h>
#include <nitrate-emit/Lib.h>
#include <nitrate-ir/Lib.h>
#include <nitrate-lexer/Lib.h>
#include <nitrate-parser/Lib.h>
#include <nitrate-seq/Lib.h>

#include <atomic>
#include <clean/Cleanup.hh>
#include <core/ANSI.hh>
#include <core/Config.hh>
#include <core/Logger.hh>
#include <fstream>
#include <init/Package.hh>
#include <install/Install.hh>
#include <iostream>
#include <nitrate-core/Classes.hh>
#include <nitrate-emit/Classes.hh>
#include <nitrate-ir/Classes.hh>
#include <nitrate-parser/Classes.hh>
#include <nitrate-seq/Classes.hh>
#include <string>
#include <string_view>
#include <vector>

using namespace no3;
using argparse::ArgumentParser;

namespace no3::router {
  static int run_init_mode(const ArgumentParser &parser) {
    using namespace init;

    core::SetDebugMode(parser["--verbose"] == true);

    PackageBuilder builder =
        PackageBuilder()
            .output(parser.get<std::string>("--output"))
            .name(parser.get<std::string>("package-name"))
            .license(parser.get<std::string>("--license"))
            .author(parser.get<std::string>("--author"))
            .email(parser.get<std::string>("--email"))
            .url(parser.get<std::string>("--url"))
            .version(parser.get<std::string>("--version"))
            .description(parser.get<std::string>("--description"))
            .verbose(parser["--verbose"] == true)
            .force(parser["--force"] == true);

    if (parser.get<std::string>("--type") == "program")
      builder.type(PackageType::PROGRAM);
    else if (parser.get<std::string>("--type") == "staticlib")
      builder.type(PackageType::STATICLIB);
    else if (parser.get<std::string>("--type") == "sharedlib")
      builder.type(PackageType::SHAREDLIB);

    return builder.build().create() ? 0 : -1;
  }

  static int run_build_mode(const ArgumentParser &parser) {
    (void)parser;

    LOG(ERROR) << "The build subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static int run_clean_mode(const ArgumentParser &parser) {
    core::SetDebugMode(parser["--verbose"] == true);

    if (clean::CleanPackageSource(parser.get<std::string>("package-src"),
                                  parser["--verbose"] == true)) {
      return 0;
    } else {
      return -1;
    }
  }

  static int run_update_mode(const ArgumentParser &parser) {
    (void)parser;

    LOG(ERROR) << "The update subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static int run_install_mode(const ArgumentParser &parser) {
    (void)parser;

    LOG(ERROR) << "The install subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static int run_doc_mode(const ArgumentParser &parser) {
    (void)parser;

    LOG(ERROR) << "The doc subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static int run_format_mode(const ArgumentParser &parser) {
    (void)parser;

    LOG(ERROR) << "The format subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static int run_list_mode(const ArgumentParser &parser) {
    (void)parser;

    LOG(ERROR) << "The list subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static int run_test_mode(const ArgumentParser &parser) {
    (void)parser;

    LOG(ERROR) << "The test subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static int run_lsp_mode(const ArgumentParser &parser) {
    core::SetDebugMode(parser["--verbose"] == true);

    std::vector<std::string> args;
    args.push_back("nitrated");

    if (parser.is_used("--config")) {
      args.push_back("--config");
      args.push_back(parser.get<std::string>("--config"));
    }

    if (parser.is_used("--log")) {
      std::fstream log_file(parser.get<std::string>("--log"));
      if (!log_file.is_open()) {
        LOG(ERROR) << "Failed to open log file";
        return -2;
      }

      qcore_implement();
      /// TODO: Handle log stream redirect
    }

    if (parser.is_used("--pipe")) {
      args.push_back("--pipe");
      args.push_back(parser.get<std::string>("--pipe"));
    } else if (parser.is_used("--port")) {
      args.push_back("--port");
      args.push_back(parser.get<std::string>("--port"));
    } else if (parser["--stdio"] == true) {
      args.push_back("--stdio");
    }

    std::vector<char *> c_args;
    c_args.reserve(args.size());

    std::string inner_command;
    for (size_t i = 0; i < args.size(); i++) {
      c_args.push_back(args[i].data());
      inner_command += args[i];

      if (i != args.size() - 1) {
        inner_command += " ";
      }
    }
    LOG(INFO) << "Invoking LSP: \"" << inner_command << "\"";

    return nitrated_main(args.size(), c_args.data());
  }

  int run_dev_mode(
      const ArgumentParser &parser,
      const std::unordered_map<std::string_view,
                               std::unique_ptr<ArgumentParser>> &subparsers);
}  // namespace no3::router

namespace no3::argparse_setup {
  extern ArgumentParser &program;
  extern ArgumentParser init_parser;
  extern ArgumentParser build_parser;
  extern ArgumentParser clean_parser;
  extern ArgumentParser update_parser;
  extern ArgumentParser install_parser;
  extern ArgumentParser doc_parser;
  extern ArgumentParser format_parser;
  extern ArgumentParser list_parser;
  extern ArgumentParser test_parser;
  extern ArgumentParser lsp_parser;
  extern ArgumentParser dev_parser;
  extern std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>>
      dev_subparsers;
}  // namespace no3::argparse_setup

extern "C" __attribute__((visibility("default"))) int no3_command(int argc,
                                                                  char **argv) {
  using namespace argparse_setup;

  core::SetColorMode(ansi::IsUsingColors());
  core::SetDebugMode(false);

  { /* Parse arguments */
    std::vector<std::string> args(argv, argv + argc);

    try {
      program.parse_args(args);
    } catch (const std::runtime_error &e) {
      std::cerr << e.what() << std::endl;
      std::cerr << program;
      return 1;
    }
  }

  { /* Subcommand routing */
    if (program["--license"] == true) {
      std::cout << R"(This file is part of Nitrate Compiler Suite.
Copyright (C) 2024 Wesley C. Jones

The Nitrate Compiler Suite is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.)"
                << std::endl;
      return 0;
    } else if (program.is_subcommand_used("init")) {
      return router::run_init_mode(init_parser);
    } else if (program.is_subcommand_used("build")) {
      return router::run_build_mode(build_parser);
    } else if (program.is_subcommand_used("clean")) {
      return router::run_clean_mode(clean_parser);
    } else if (program.is_subcommand_used("update")) {
      return router::run_update_mode(update_parser);
    } else if (program.is_subcommand_used("install")) {
      return router::run_install_mode(install_parser);
    } else if (program.is_subcommand_used("doc")) {
      return router::run_doc_mode(doc_parser);
    } else if (program.is_subcommand_used("format")) {
      return router::run_format_mode(format_parser);
    } else if (program.is_subcommand_used("list")) {
      return router::run_list_mode(list_parser);
    } else if (program.is_subcommand_used("test")) {
      return router::run_test_mode(test_parser);
    } else if (program.is_subcommand_used("lsp")) {
      return router::run_lsp_mode(lsp_parser);
    } else if (program.is_subcommand_used("dev")) {
      return router::run_dev_mode(dev_parser, dev_subparsers);
    } else {
      std::cerr << "No command specified" << std::endl;
      std::cerr << program;
      return 1;
    }
  }
}

extern "C" __attribute__((visibility("default"))) bool no3_init() {
  static std::atomic<bool> is_initialized = false;
  if (is_initialized) {
    return true;
  }

  { /* Configure Google logger */
    FLAGS_stderrthreshold = google::FATAL;
    google::InitGoogleLogging("no3");
    google::InstallFailureSignalHandler();

    static core::MyLogSink sink;
    google::AddLogSink(&sink);
  }

  { /* Initialize libraries */
    if (!qcore_lib_init()) {
      LOG(ERROR) << "Failed to initialize NITRATE-CORE library" << std::endl;
      return false;
    }

    if (!qlex_lib_init()) {
      LOG(ERROR) << "Failed to initialize NITRATE-LEX library" << std::endl;
      return false;
    }

    if (!qprep_lib_init()) {
      LOG(ERROR) << "Failed to initialize NITRATE-PREP library" << std::endl;
      return false;
    }

    if (!qparse_lib_init()) {
      LOG(ERROR) << "Failed to initialize NITRATE-PARSE library" << std::endl;
      return false;
    }

    if (!nr_lib_init()) {
      LOG(ERROR) << "Failed to initialize NITRATE-IR library" << std::endl;
      return false;
    }

    if (!qcode_lib_init()) {
      LOG(ERROR) << "Failed to initialize NITRATE-CODE library" << std::endl;
      return false;
    }
  }

  is_initialized = true;

  return true;
}

int main(int argc, char *argv[]) {
  if (!no3_init()) {
    std::cerr << "Failed to initialize no3\n";
    return 1;
  }

  return no3_command(argc, argv);
}
