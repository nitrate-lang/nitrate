#include <argparse.h>
#include <glog/logging.h>
#include <nitrate-core/Macro.h>

#include <boost/assert/source_location.hpp>
#include <boost/throw_exception.hpp>
#include <csignal>
#include <filesystem>
#include <lsp/core/license-data.hh>
#include <lsp/core/server.hh>
#include <memory>
#include <optional>

static constexpr void create_parser(argparse::ArgumentParser& parser) {
  ///=================== BASIC CONFIGURATION ======================

  parser.add_argument("--config")
      .default_value(std::string(""))
      .help("Specify the configuration file");

  ///=================== CONNECTION CONFIGURATION ======================

  auto& group = parser.add_mutually_exclusive_group();

  group.add_argument("--pipe").help("Specify the pipe file to connect to");
  group.add_argument("--port").help("Specify the port to connect to");
  group.add_argument("--stdio").default_value(false).implicit_value(true).help(
      "Use standard I/O");
}

C_EXPORT int nitrated_main(int argc, char** argv) {
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
  create_parser(parser);

  parser.parse_args(args);

  std::unique_ptr<Configuration> config;
  { /* Setup config */
    std::string config_file = parser.get<std::string>("--config");
    if (config_file.empty()) {
      config = std::make_unique<Configuration>(Configuration::defaults());
    } else {
      if (!std::filesystem::exists(config_file)) {
        LOG(ERROR) << "Configuration file does not exist: " << config_file;
        return 1;
      }

      auto config_opt = parse_config(config_file);
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

    if (parser.is_used("--pipe")) {
      connection_type = ConnectionType::Pipe;
      connect_param = parser.get<std::string>("--pipe");
    } else if (parser.is_used("--port")) {
      connection_type = ConnectionType::Port;
      connect_param = parser.get<std::string>("--port");
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

    auto channel_opt = open_connection(connection_type, connect_param);
    if (!channel_opt) {
      LOG(ERROR) << "Failed to open channel";
      return 1;
    }

    channel = std::move(channel_opt.value());
  }

  ServerContext::the().start_server(channel);
}
