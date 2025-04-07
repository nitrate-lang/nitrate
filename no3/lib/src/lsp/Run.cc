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

#include <core/cli/GetOpt.hh>
#include <core/cli/Interpreter.hh>
#include <fstream>
#include <lsp/connect/Connection.hh>
#include <lsp/server/Server.hh>
#include <memory>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>

using namespace ncc;
using namespace no3::core;
using namespace no3::lsp;
using namespace no3::lsp::core;

class LspCommandArgumentParser {
  struct Options {
    ConnectionType m_connection_mode;
    std::string& m_connect_arg;
    std::filesystem::path& m_log_file;

    Options(std::filesystem::path& log_file, std::string& connect_arg, ConnectionType connection_mode)
        : m_connection_mode(connection_mode), m_connect_arg(connect_arg), m_log_file(log_file) {}
  };

  bool m_help = false;
  size_t m_stdio = 0;
  size_t m_port = 0;
  std::string m_connect_arg;
  ConnectionType m_connection_mode = ConnectionType::Stdio;
  std::filesystem::path m_log_file;
  bool m_too_many_args = false;

  void DisplayHelp() {
    std::string_view help =
        R"(Usage: lsp [--help] [[--port VAR]|[--stdio]] [--log VAR]

Optional arguments:
  -h, --help          shows this help message and exits
  -s, --stdio         instruct LSP server to connect via stdin/stdout
  -p, --port          instruct LSP server to listen on a TCP port
  -o, --log           log output file [default: "nitrate-lsp.log"]
)";

    Log << Raw << help;
  }

  void DoParse(std::vector<std::string> args) {
    constexpr const char* kShortOptions = "hsp:o:";
    constexpr std::array kLongOptions = {
        option{"help", no_argument, nullptr, 'h'},
        option{"stdio", no_argument, nullptr, 's'},
        option{"port", required_argument, nullptr, 'p'},
        option{"log", required_argument, nullptr, 'o'},
        option{nullptr, 0, nullptr, 0},
    };

    std::vector<char*> argv(args.size());
    std::transform(args.begin(), args.end(), argv.begin(), [](auto& str) { return str.data(); });

    {  // Lock the mutex to prevent multiple threads from calling getopt_long at the same time.
      std::lock_guard lock(GET_OPT);

      Log << Trace << "Starting to parse command line arguments";

      int c;
      int option_index = 0;

      opterr = 0;
      while ((c = GET_OPT.getopt_long(args.size(), argv.data(), kShortOptions, kLongOptions.data(), &option_index)) !=
             -1) {
        switch (c) {
          case 'h': {
            Log << Trace << "Parsing command line argument: --help";
            m_help = true;
            break;
          }

          case 's': {
            Log << Trace << "Parsing command line argument: --stdio, -s";

            m_connection_mode = ConnectionType::Stdio;
            if (m_stdio++ > 0) {
              Log << "The -s, --stdio argument was provided more than once.";
              m_too_many_args = true;
            }

            break;
          }

          case 'p': {
            Log << Trace << "Parsing command line argument: --port, -p";

            m_connection_mode = ConnectionType::Port;
            m_connect_arg = optarg;
            if (m_port++ > 0) {
              Log << "The -p, --port argument was provided more than once.";
              m_too_many_args = true;
            }

            break;
          }

          case 'o': {
            Log << Trace << "Parsing command line argument: --log, -o";
            if (!m_log_file.empty()) {
              Log << "The -o, --output argument was provided more than once.";
              m_too_many_args = true;
              break;
            }

            m_log_file = optarg;
            break;
          }

          case '?': {
            Log << "Unknown command line argument: -" << (char)optopt;
            m_too_many_args = true;
            break;
          }

          default: {
            Log << "Unknown command line argument: -" << (char)c;
            m_too_many_args = true;
            break;
          }
        }
      }

      if ((size_t)optind < args.size()) {
        m_too_many_args = true;
      }

      if (m_log_file.empty()) {
        Log << Trace << "No log file path provided. Setting it to \"nitrate-lsp.log\"";
        m_log_file = "nitrate-lsp.log";
      }

      Log << Trace << "Finished parsing command line arguments";
    }
  }

  [[nodiscard]] auto Check() const -> bool {
    bool okay = true;

    if (m_too_many_args) {
      Log << "Too many arguments provided.";
      okay = false;
    } else if (m_stdio + m_port > 1) {
      Log << "Only one of --stdio or --port can be specified.";
      okay = false;
    }

    return okay;
  }

public:
  LspCommandArgumentParser(const std::vector<std::string>& args, bool& is_valid, bool& performed_action) {
    is_valid = false;
    performed_action = false;

    DoParse(args);

    if (m_help) {
      DisplayHelp();
      is_valid = true;
      performed_action = true;
      return;
    }

    is_valid = Check();
  }

  [[nodiscard]] auto GetOptions() -> Options { return {m_log_file, m_connect_arg, m_connection_mode}; }
};

static bool StartServer(const std::filesystem::path& log_file, const ConnectionType& connection_mode,
                        const std::string& connection_arg) {
  Log << Trace << "Opening connection for the LSP server IO";
  auto lsp_io = OpenConnection(connection_mode, connection_arg);
  if (!lsp_io.has_value()) {
    Log << "Failed to open connection for LSP server.";
    return false;
  }

  Log << Trace << "Connection opened successfully";

  bool lsp_status = false;
  std::unique_ptr<std::fstream> log_stream;

  { /* Open log output file */
    Log << Trace << "Opening log file: " << log_file;
    log_stream = std::make_unique<std::fstream>(log_file, std::ios::out | std::ios::binary | std::ios::app);
    if (!log_stream->is_open()) {
      Log << "Failed to open log file: " << log_file;
      return false;
    }
    Log << Trace << "Log file opened successfully";
  }

  auto file_logger = [&log_stream](const LogMessage& msg) {
    auto message = msg.m_by.Format(msg.m_message, msg.m_sev);
    log_stream->write(message.data(), message.size());
    log_stream->write("\n", 1);
    log_stream->flush();
  };

  { /* Run the LSP server */
    if (connection_mode == ConnectionType::Stdio) {
      Log << Info << "Starting LSP server with " << connection_mode << " connection";

      // Save a copy of all current log subscriber callbacks
      std::vector<LogSubscriberID> old_active_subscribers;
      old_active_subscribers.reserve(Log->SubscribersList().size());
      for (const auto& sub : Log->SubscribersList()) {
        if (!sub.IsSuspended()) {
          old_active_subscribers.push_back(sub.ID());
        }
      }

      // Suspend all log subscribers to ensure nothing interferes with the LSP server stdout
      // and stderr streams.
      Log->SuspendAll();

      {
        auto file_logger_id = Log->Subscribe(file_logger);

        Log << Info << "Starting LSP server with " << connection_mode << " connection";
        lsp_status = Server(*lsp_io.value()).Start();
        Log << Info << "LSP server exited";
        Log->Unsubscribe(file_logger_id);
      }

      for (const auto& sub : old_active_subscribers) {
        Log->Resume(sub);
      }

      Log << Info << "LSP server exited";
    } else {
      const auto file_logger_id = Log->Subscribe(file_logger);
      Log << Info << "Starting LSP server with " << connection_mode << " connection";
      lsp_status = Server(*lsp_io.value()).Start();
      Log << Info << "LSP server exited";
      Log->Unsubscribe(file_logger_id);
    }

    log_stream = nullptr;
  }

  if (!lsp_status) {
    Log << "Failed to start LSP server.";
    return false;
  }

  return true;
}

auto no3::Interpreter::PImpl::CommandLSP(ConstArguments, const MutArguments& argv) -> bool {
  bool is_valid = false;
  bool performed_action = false;

  LspCommandArgumentParser state(argv, is_valid, performed_action);

  if (!is_valid) {
    Log << Trace << "Failed to parse command line arguments.";
    return false;
  }

  if (performed_action) {
    Log << Trace << "Performed built-in action.";
    return true;
  }

  auto [connection_mode, connection_arg, log_file] = state.GetOptions();

  Log << Trace << R"(options["connect_mode"] = ")" << connection_mode << "\"";
  Log << Trace << "options[\"connect_arg\"] = " << connection_arg;
  Log << Trace << "options[\"log\"] = " << log_file;

  return StartServer(log_file, connection_mode, connection_arg);
}
