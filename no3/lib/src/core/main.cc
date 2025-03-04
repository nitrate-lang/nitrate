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

#include <git2/global.h>
#include <lsp/nitrated.h>
#include <nitrate-emit/Code.h>
#include <nitrate-emit/Lib.h>

#include <atomic>
#include <clean/Cleanup.hh>
#include <core/Config.hh>
#include <core/argparse.hpp>
#include <core/termcolor.hh>
#include <fstream>
#include <init/Package.hh>
#include <install/Install.hh>
#include <iostream>
#include <memory>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-emit/Classes.hh>
#include <nitrate-ir/ABI/Name.hh>
#include <nitrate-ir/Init.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-parser/Init.hh>
#include <nitrate-seq/Init.hh>
#include <nitrate-seq/Sequencer.hh>
#include <no3/Interpreter.hh>
#include <string>
#include <string_view>
#include <vector>

using namespace ncc;
using namespace no3;
using argparse::ArgumentParser;

NCC_EXPORT std::unique_ptr<std::ostream> no3::GlobalOutputStream = std::make_unique<std::ostream>(std::cerr.rdbuf());

namespace no3::router {
  static auto RunInitMode(const ArgumentParser &parser) -> int {
    using namespace init;

    PackageBuilder builder = PackageBuilder()
                                 .Output(parser.Get<std::string>("--output"))
                                 .Name(parser.Get<std::string>("package-name"))
                                 .License(parser.Get<std::string>("--license"))
                                 .Author(parser.Get<std::string>("--author"))
                                 .Email(parser.Get<std::string>("--email"))
                                 .Url(parser.Get<std::string>("--url"))
                                 .Version(parser.Get<std::string>("--version"))
                                 .Description(parser.Get<std::string>("--description"))
                                 .Verbose(parser["--verbose"] == true)
                                 .Force(parser["--force"] == true);

    if (parser.Get<std::string>("--type") == "program") {
      builder.Type(PackageType::PROGRAM);
    } else if (parser.Get<std::string>("--type") == "staticlib") {
      builder.Type(PackageType::STATICLIB);
    } else if (parser.Get<std::string>("--type") == "sharedlib") {
      builder.Type(PackageType::SHAREDLIB);
    }

    return builder.Build().Create() ? 0 : -1;
  }

  static auto RunBuildMode(const ArgumentParser &parser) -> int {
    (void)parser;

    Log << "The build subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunCleanMode(const ArgumentParser &parser) -> int {
    if (clean::CleanPackageSource(parser.Get<std::string>("package-src"), parser["--verbose"] == true)) {
      return 0;
    }
    return -1;
  }

  static auto RunUpdateMode(const ArgumentParser &parser) -> int {
    (void)parser;

    Log << "The update subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunInstallMode(const ArgumentParser &parser) -> int {
    (void)parser;

    Log << "The install subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunDocMode(const ArgumentParser &parser) -> int {
    (void)parser;

    Log << "The doc subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunFormatMode(const ArgumentParser &parser) -> int {
    (void)parser;

    Log << "The format subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunListMode(const ArgumentParser &parser) -> int {
    (void)parser;

    Log << "The list subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunTestMode(const ArgumentParser &parser) -> int {
    (void)parser;

    Log << "The test subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunLspMode(const ArgumentParser &parser) -> int {
    std::vector<std::string> args;
    args.emplace_back("nitrated");

    if (parser.IsUsed("--config")) {
      args.emplace_back("--config");
      args.push_back(parser.Get<std::string>("--config"));
    }

    if (parser.IsUsed("--pipe")) {
      args.emplace_back("--pipe");
      args.push_back(parser.Get<std::string>("--pipe"));
    } else if (parser.IsUsed("--port")) {
      args.emplace_back("--port");
      args.push_back(parser.Get<std::string>("--port"));
    } else if (parser["--stdio"] == true) {
      args.emplace_back("--stdio");
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

    const auto log_filepath = parser.Get<std::string>("--log");
    auto log_file = std::make_unique<std::fstream>(log_filepath, std::ios::app);
    if (!log_file->is_open()) {
      Log << "Failed to open log file";
      return -2;
    }

    std::unique_ptr<std::ostream> log_stream = std::make_unique<std::ostream>(log_file->rdbuf());

    std::swap(GlobalOutputStream, log_stream);

    Log << Info << "Invoking LSP: \"" << inner_command << "\"";
    int ret = NitratedMain(args.size(), c_args.data());

    std::swap(GlobalOutputStream, log_stream);

    return ret;
  }

  auto RunDevMode(const ArgumentParser &parser,
                  const std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>> &subparsers) -> int;
}  // namespace no3::router

namespace no3::argparse_setup {
  extern ArgumentParser &Program;
  extern ArgumentParser InitParser;
  extern ArgumentParser BuildParser;
  extern ArgumentParser CleanParser;
  extern ArgumentParser UpdateParser;
  extern ArgumentParser InstallParser;
  extern ArgumentParser DocParser;
  extern ArgumentParser FormatParser;
  extern ArgumentParser ListParser;
  extern ArgumentParser TestParser;
  extern ArgumentParser LspParser;
  extern ArgumentParser DevParser;
  extern std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>> DevSubparsers;
}  // namespace no3::argparse_setup

extern "C" __attribute__((visibility("default"))) auto No3Command(int argc, char **argv) -> int {
  using namespace argparse_setup;

  { /* Parse arguments */
    std::vector<std::string> args(argv, argv + argc);

    try {
      Program.ParseArgs(args);
    } catch (const std::runtime_error &e) {
      std::cerr << e.what() << std::endl;
      std::cerr << Program;
      return 1;
    }
  }

  { /* Subcommand routing */
    if (Program["--license"] == true) {
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
    }
    if (Program.IsSubcommandUsed("init")) {
      return router::RunInitMode(InitParser);
    }
    if (Program.IsSubcommandUsed("build")) {
      return router::RunBuildMode(BuildParser);
    }
    if (Program.IsSubcommandUsed("clean")) {
      return router::RunCleanMode(CleanParser);
    }
    if (Program.IsSubcommandUsed("update")) {
      return router::RunUpdateMode(UpdateParser);
    }
    if (Program.IsSubcommandUsed("install")) {
      return router::RunInstallMode(InstallParser);
    }
    if (Program.IsSubcommandUsed("doc")) {
      return router::RunDocMode(DocParser);
    }
    if (Program.IsSubcommandUsed("format")) {
      return router::RunFormatMode(FormatParser);
    }
    if (Program.IsSubcommandUsed("list")) {
      return router::RunListMode(ListParser);
    }
    if (Program.IsSubcommandUsed("test")) {
      return router::RunTestMode(TestParser);
    }
    if (Program.IsSubcommandUsed("lsp")) {
      return router::RunLspMode(LspParser);
    }
    if (Program.IsSubcommandUsed("dev")) {
      return router::RunDevMode(DevParser, DevSubparsers);
    }

    std::cerr << "No command specified" << std::endl;
    std::cerr << Program;
    return 1;
  }
}
