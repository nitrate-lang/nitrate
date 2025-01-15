#include <glog/logging.h>

#include <argparse.hpp>
#include <boost/assert/source_location.hpp>
#include <boost/throw_exception.hpp>
#include <csignal>
#include <filesystem>
#include <lsp/core/license-data.hh>
#include <lsp/core/server.hh>
#include <memory>
#include <nitrate-core/Macro.hh>
#include <optional>

static constexpr void CreateParser(argparse::ArgumentParser& parser) {
  ///=================== BASIC CONFIGURATION ======================

  parser.AddArgument("--config")
      .DefaultValue(std::string(""))
      .Help("Specify the configuration file");

  ///=================== CONNECTION CONFIGURATION ======================

  auto& group = parser.AddMutuallyExclusiveGroup();

  group.AddArgument("--pipe").Help("Specify the pipe file to connect to");
  group.AddArgument("--port").Help("Specify the port to connect to");
  group.AddArgument("--stdio").DefaultValue(false).ImplicitValue(true).Help(
      "Use standard I/O");
}

extern "C" NCC_EXPORT int NitratedMain(int argc, char** argv) {
  std::vector<std::string> args(argv, argv + argc);

  {
    std::string str;
    for (auto it = args.begin(); it != args.end(); ++it) {
      str += *it;
      if (it + 1 != args.end()) {
        str += " ";
      }
    }

    LOG(INFO) << "Starting nitrated: \"" << str << "\"";
  }

  argparse::ArgumentParser parser("nitrated", "1.0");
  CreateParser(parser);

  parser.ParseArgs(args);

  std::unique_ptr<Configuration> config;
  { /* Setup config */
    std::string config_file = parser.Get<std::string>("--config");
    if (config_file.empty()) {
      config = std::make_unique<Configuration>(Configuration::Defaults());
    } else {
      if (!std::filesystem::exists(config_file)) {
        LOG(ERROR) << "Configuration file does not exist: " << config_file;
        return 1;
      }

      auto config_opt = ParseConfig(config_file);
      if (!config_opt.has_value()) {
        LOG(ERROR) << "Failed to parse configuration file: " << config_file;
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
        LOG(INFO) << "Using pipe: " << connect_param;
        break;
      case ConnectionType::Port:
        LOG(INFO) << "Using port: " << connect_param;
        break;
      case ConnectionType::Stdio:
        LOG(INFO) << "Using standard I/O";
        break;
    }

    auto channel_opt = OpenConnection(connection_type, connect_param);
    if (!channel_opt) {
      LOG(ERROR) << "Failed to open channel";
      return 1;
    }

    channel = std::move(channel_opt.value());
  }

  ServerContext::The().StartServer(channel);
}
