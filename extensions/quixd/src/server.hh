#pragma once

#include <glog/logging.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <optional>

struct MyLogSink : google::LogSink {
private:
  std::ofstream m_log_file;

  static std::string escape_message(std::string_view str);

public:
  MyLogSink(const std::string& log_file);

  void send(google::LogSeverity severity, const char*, const char* base_filename, int line,
            const struct tm* tm, const char* message, std::size_t message_len) override;
};

struct Configuration {
private:
  Configuration() = default;

public:
  static Configuration defaults() {
    Configuration config;
    return config;
  }
};

std::optional<Configuration> parse_config(const std::string& path);

enum class ConnectionType { Pipe, Port, Stdio };

class Connection : public std::iostream {
  std::shared_ptr<std::streambuf> m_buf;

public:
  Connection(std::shared_ptr<std::streambuf> buf) : std::iostream(buf.get()), m_buf(buf) {}
  ~Connection();
};

std::optional<std::unique_ptr<Connection>> open_connection(ConnectionType type,
                                                           const std::string& param);
bool start_language_server(std::iostream& channel, const Configuration& config);
