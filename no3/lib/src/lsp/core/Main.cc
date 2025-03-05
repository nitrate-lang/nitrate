#include <boost/assert/source_location.hpp>
#include <boost/throw_exception.hpp>
#include <core/argparse.hpp>
#include <csignal>
#include <filesystem>
#include <lsp/core/Server.hh>
#include <memory>
#include <nitrate-core/LogOStream.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <optional>

using namespace ncc;
using namespace no3::lsp;
using namespace no3::lsp::srv;

static constexpr void CreateParser(argparse::ArgumentParser& parser) {
  ///=================== BASIC CONFIGURATION ======================

  parser.AddArgument("--config").DefaultValue(std::string("")).Help("Specify the configuration file");

  ///=================== CONNECTION CONFIGURATION ======================

  auto& group = parser.AddMutuallyExclusiveGroup();

  group.AddArgument("--pipe").Help("Specify the pipe file to connect to");
  group.AddArgument("--port").Help("Specify the port to connect to");
  group.AddArgument("--stdio").DefaultValue(false).ImplicitValue(true).Help("Use standard I/O");
}

extern "C" auto NitratedMain(int argc, char** argv) -> int {
  std::vector<std::string> args(argv, argv + argc);

  {
    std::string str;
    for (auto it = args.begin(); it != args.end(); ++it) {
      str += *it;
      if (it + 1 != args.end()) {
        str += " ";
      }
    }

    Log << Info << "Starting nitrated: \"" << str << "\"";
  }

  argparse::ArgumentParser parser(ncc::clog, "nitrated", "1.0");
  CreateParser(parser);

  parser.ParseArgs(args);

  std::unique_ptr<Configuration> config;
  { /* Setup config */
    auto config_file = parser.Get<std::string>("--config");
    if (config_file.empty()) {
      config = std::make_unique<Configuration>(Configuration::Defaults());
    } else {
      if (!std::filesystem::exists(config_file)) {
        Log << "Configuration file does not exist: " << config_file;
        return 1;
      }

      auto config_opt = ParseConfig(config_file);
      if (!config_opt.has_value()) {
        Log << "Failed to parse configuration file: " << config_file;
        return 1;
      }

      config = std::make_unique<Configuration>(config_opt.value());
    }
  }

  Connection channel;
  { /* Setup connection */
    std::string connect_param;
    ConnectionType connection_type;

    if (parser.IsUsed("--pipe")) {
      connection_type = ConnectionType::Pipe;
      connect_param = parser.Get<std::string>("--pipe");
    } else if (parser.IsUsed("--port")) {
      connection_type = ConnectionType::Port;
      connect_param = parser.Get<std::string>("--port");
    } else {
      connection_type = ConnectionType::Stdio;
    }

    switch (connection_type) {
      case ConnectionType::Pipe:
        Log << Info << "Using pipe: " << connect_param;
        break;
      case ConnectionType::Port:
        Log << Info << "Using port: " << connect_param;
        break;
      case ConnectionType::Stdio:
        Log << Info << "Using standard I/O";
        break;
    }

    auto channel_opt = OpenConnection(connection_type, String(connect_param));
    if (!channel_opt) {
      Log << "Failed to open channel";
      return 1;
    }

    channel = std::move(channel_opt.value());
  }

  ServerContext::The().StartServer(channel);
}
