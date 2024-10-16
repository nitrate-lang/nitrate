#include <core/argparse.hh>
#include <core/license-data.hh>
#include <core/server.hh>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>

static constexpr void create_parser(argparse::ArgumentParser& parser) {
  ///=================== BASIC CONFIGURATION ======================

  parser.add_argument("--license")
      .default_value(false)
      .implicit_value(true)
      .help("Print the software license");

  parser.add_argument("--config")
      .default_value(std::string(""))
      .help("Specify the configuration file");

  parser.add_argument("--log").required().help("Specify the log file");

  ///=================== CONNECTION CONFIGURATION ======================

  auto& group = parser.add_mutually_exclusive_group();

  group.add_argument("--pipe").help("Specify the pipe file to connect to");
  group.add_argument("--port").help("Specify the port to connect to");
  group.add_argument("--stdio").default_value(false).implicit_value(true).help("Use standard I/O");
}

void signal_handler(int signal);

static void install_signal_handlers() {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);
  std::signal(SIGSEGV, signal_handler);
  std::signal(SIGABRT, signal_handler);
  std::signal(SIGFPE, signal_handler);
  std::signal(SIGILL, signal_handler);
  std::signal(SIGBUS, signal_handler);
}

static void erase_signals() {
  std::signal(SIGINT, SIG_DFL);
  std::signal(SIGTERM, SIG_DFL);
  std::signal(SIGSEGV, SIG_DFL);
  std::signal(SIGABRT, SIG_DFL);
  std::signal(SIGFPE, SIG_DFL);
  std::signal(SIGILL, SIG_DFL);
  std::signal(SIGBUS, SIG_DFL);
}

void signal_handler(int signal) {
  LOG(INFO) << "Received signal: " << signal;

  switch (signal) {
    case SIGINT:
    case SIGTERM:
      LOG(INFO) << "Shutting down quixd";
      erase_signals();
      exit(0);
      break;
    case SIGSEGV:
      LOG(ERROR) << "Segmentation fault";
      erase_signals();
      exit(1);
      break;
    case SIGABRT:
      LOG(ERROR) << "Aborted";
      erase_signals();
      abort();
      break;
    case SIGFPE:
      LOG(ERROR) << "Floating point exception";
      erase_signals();
      abort();
      break;
    case SIGILL:
      LOG(ERROR) << "Illegal instruction";
      erase_signals();
      abort();
      break;
    case SIGBUS:
      LOG(ERROR) << "Bus error";
      erase_signals();
      abort();
      break;
    default:
      LOG(WARNING) << "Unhandled signal: " << signal;
      break;
  }
}

int main(int argc, char** argv) {
  std::vector<std::string> args(argv, argv + argc);
  argparse::ArgumentParser parser("quixd", "1.0");
  create_parser(parser);

  try {
    parser.parse_args(args);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  if (parser["--license"] == true) {
    std::cout << license_text << std::endl;
    return 0;
  }

  { /* Setup log file */
    std::string log_file = parser.get<std::string>("--log");

    google::InitGoogleLogging(argv[0]);

    static MyLogSink sink(log_file);
    google::AddLogSink(&sink);
  }

  install_signal_handlers();

  LOG(INFO) << "Starting quixd";

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

  std::unique_ptr<Connection> channel;
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

  if (!start_language_server(*channel, *config)) {
    LOG(ERROR) << "An error occured in the language server";
    return 1;
  }

  return 0;
}
