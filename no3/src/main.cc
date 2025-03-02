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
#include <glog/logging.h>
#include <lsp/nitrated.h>
#include <nitrate-emit/Code.h>
#include <nitrate-emit/Lib.h>

#include <argparse.hpp>
#include <atomic>
#include <clean/Cleanup.hh>
#include <core/ANSI.hh>
#include <core/Config.hh>
#include <core/Logger.hh>
#include <fstream>
#include <init/Package.hh>
#include <install/Install.hh>
#include <iostream>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-emit/Classes.hh>
#include <nitrate-ir/ABI/Name.hh>
#include <nitrate-ir/Init.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-parser/Init.hh>
#include <nitrate-seq/Init.hh>
#include <nitrate-seq/Sequencer.hh>
#include <string>
#include <string_view>
#include <vector>

#include "nitrate-core/Assert.hh"

using namespace no3;
using argparse::ArgumentParser;

static core::MyLogSink GGoogleLogSink;

namespace no3::router {
  static auto RunInitMode(const ArgumentParser &parser) -> int {
    using namespace init;

    core::SetDebugMode(parser["--verbose"] == true);

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

    LOG(ERROR) << "The build subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunCleanMode(const ArgumentParser &parser) -> int {
    core::SetDebugMode(parser["--verbose"] == true);

    if (clean::CleanPackageSource(parser.Get<std::string>("package-src"), parser["--verbose"] == true)) {
      return 0;
    }
    return -1;
  }

  static auto RunUpdateMode(const ArgumentParser &parser) -> int {
    (void)parser;

    LOG(ERROR) << "The update subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunInstallMode(const ArgumentParser &parser) -> int {
    (void)parser;

    LOG(ERROR) << "The install subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunDocMode(const ArgumentParser &parser) -> int {
    (void)parser;

    LOG(ERROR) << "The doc subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunFormatMode(const ArgumentParser &parser) -> int {
    (void)parser;

    LOG(ERROR) << "The format subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunListMode(const ArgumentParser &parser) -> int {
    (void)parser;

    LOG(ERROR) << "The list subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunTestMode(const ArgumentParser &parser) -> int {
    (void)parser;

    LOG(ERROR) << "The test subcommand is not implemented yet";
    /// TODO: Implement

    return 1;
  }

  static auto RunLspMode(const ArgumentParser &parser) -> int {
    core::SetDebugMode(parser["--verbose"] == true);

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

    auto log_file = std::make_unique<std::fstream>(parser.Get<std::string>("--log"), std::ios::app);

    if (!log_file->is_open()) {
      LOG(ERROR) << "Failed to open log file";
      return -2;
    }

    auto old_stream = GGoogleLogSink.RedirectToStream(std::move(log_file));

    /// TODO: Handle log stream redirect

    LOG(INFO) << "Invoking LSP: \"" << inner_command << "\"";

    int ret = NitratedMain(args.size(), c_args.data());

    GGoogleLogSink.RedirectToStream(std::move(old_stream));

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

  core::SetColorMode(ansi::IsUsingColors());
  core::SetDebugMode(false);

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

extern "C" __attribute__((visibility("default"))) auto No3Init() -> bool {
  static std::atomic<bool> is_initialized = false;
  if (is_initialized) {
    return true;
  }

  { /* Configure Google logger */
    FLAGS_stderrthreshold = google::FATAL;
    google::InitGoogleLogging("no3");
    google::InstallFailureSignalHandler();

    google::AddLogSink(&GGoogleLogSink);
  }

  { /* Initialize compiler pipeline libraries */
    if (!ncc::CoreLibrary.InitRC()) {
      LOG(ERROR) << "Failed to initialize NITRATE-CORE library";
      return false;
    }

    ncc::Log.Subscribe([](auto msg, auto sev, const auto &ec) {
      using namespace ncc;

      if (sev > Debug || core::GetDebugMode()) {
        switch (sev) {
          case Trace:
          case Debug:
          case Info:
          case Notice: {
            LOG(INFO) << ec.Format(msg, sev);
            break;
          }

          case Warning: {
            LOG(WARNING) << ec.Format(msg, sev);
            break;
          }

          case Error:
          case Critical:
          case Alert:
          case Emergency: {
            LOG(ERROR) << ec.Format(msg, sev);
            break;
          }
        }
      }
    });

    if (!ncc::lex::LexerLibrary.InitRC()) {
      LOG(ERROR) << "Failed to initialize NITRATE-LEX library";
      return false;
    }

    if (!ncc::seq::SeqLibrary.InitRC()) {
      LOG(ERROR) << "Failed to initialize NITRATE-PREP library";
      return false;
    }

    if (!ncc::parse::ParseLibrary.InitRC()) {
      LOG(ERROR) << "Failed to initialize NITRATE-PARSE library";
      return false;
    }

    if (!ncc::ir::IRLibrary.InitRC()) {
      LOG(ERROR) << "Failed to initialize NITRATE-IR library";
      return false;
    }

    if (!QcodeLibInit()) {
      LOG(ERROR) << "Failed to initialize NITRATE-CODE library";
      return false;
    }
  }

  if (git_libgit2_init() <= 0) {
    LOG(ERROR) << "Failed to initialize libgit2";
    return false;
  }

  is_initialized = true;

  return true;
}

#ifdef NO3_MAIN
int main(int argc, char *argv[]) {
  if (!No3Init()) {
    std::cerr << "Failed to initialize no3\n";
    return 1;
  }

  return No3Command(argc, argv);
}
#endif
