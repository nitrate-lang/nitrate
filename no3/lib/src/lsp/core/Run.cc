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

#include <boost/assert/source_location.hpp>
#include <boost/throw_exception.hpp>
#include <core/InterpreterImpl.hh>
#include <csignal>
#include <lsp/core/Server.hh>
#include <nitrate-core/LogOStream.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>

using namespace ncc;
using namespace no3;
using namespace no3::lsp;
using namespace no3::lsp::core;

// static constexpr void CreateParser(argparse::ArgumentParser& parser) {
//   ///=================== BASIC CONFIGURATION ======================

//   parser.AddArgument("--config").DefaultValue(std::string("")).Help("Specify the configuration file");

//   ///=================== CONNECTION CONFIGURATION ======================

//   auto& group = parser.AddMutuallyExclusiveGroup();

//   group.AddArgument("--pipe").Help("Specify the pipe file to connect to");
//   group.AddArgument("--port").Help("Specify the port to connect to");
//   group.AddArgument("--stdio").DefaultValue(false).ImplicitValue(true).Help("Use standard I/O");
// }

auto NitratedMain(int argc, char** argv) -> int {
  /// TODO: Implement Nitrate LSP
  (void)argc;
  (void)argv;
  Log << "Nitrate LSP is not implemented yet";
  return 1;

  // std::vector<std::string> args(argv, argv + argc);

  // {
  //   std::string str;
  //   for (auto it = args.begin(); it != args.end(); ++it) {
  //     str += *it;
  //     if (it + 1 != args.end()) {
  //       str += " ";
  //     }
  //   }

  //   Log << Info << "Starting nitrated: \"" << str << "\"";
  // }

  // bool did_default = false;
  // argparse::ArgumentParser parser(ncc::clog, did_default, "nitrated", "1.0");
  // CreateParser(parser);

  // parser.ParseArgs(args);

  // if (did_default) {
  //   return 0;
  // }

  // DuplexStream channel;
  // { /* Setup connection */
  //   std::string connect_param;
  //   ConnectionType connection_type;

  //   if (parser.IsUsed("--pipe")) {
  //     connection_type = ConnectionType::Pipe;
  //     connect_param = parser.Get<std::string>("--pipe");
  //   } else if (parser.IsUsed("--port")) {
  //     connection_type = ConnectionType::Port;
  //     connect_param = parser.Get<std::string>("--port");
  //   } else {
  //     connection_type = ConnectionType::Stdio;
  //   }

  //   switch (connection_type) {
  //     case ConnectionType::Pipe:
  //       Log << Info << "Using pipe: " << connect_param;
  //       break;
  //     case ConnectionType::Port:
  //       Log << Info << "Using port: " << connect_param;
  //       break;
  //     case ConnectionType::Stdio:
  //       Log << Info << "Using standard I/O";
  //       break;
  //   }

  //   auto channel_opt = OpenDuplexStream(connection_type, String(connect_param));
  //   if (!channel_opt) {
  //     Log << "Failed to open channel";
  //     return 1;
  //   }

  //   channel = std::move(channel_opt.value());
  // }

  // ServerContext::The().StartServer(channel);
}

auto Interpreter::PImpl::CommandLSP(ConstArguments, const MutArguments& argv) -> bool {
  (void)argv;

  Log << "Not implemented";
  return false;
}
