#include <core/server.hh>
#include <iostream>
#include <string>
#include <unordered_map>

std::string MyLogSink::escape_message(std::string_view str) {
  std::string ret;
  ret.reserve(str.size());

  for (char c : str) {
    if (c == '"') {
      ret.push_back('\\');
      ret.push_back(c);
    } else if (c == '\n') {
      ret += "\\n";
    } else if (c == '\r') {
      ret += "\\r";
    } else if (!isprint(c)) {
      std::stringstream ss;
      ss << "\\x" << std::hex << static_cast<int>(c);
      ret += ss.str();
    } else {
      ret.push_back(c);
    }
  }

  return ret;
}

MyLogSink::MyLogSink(const std::string& log_file) {
  m_log_file.open(log_file, std::ios::out | std::ios::app);
  if (!m_log_file.is_open()) {
    std::cerr << "Failed to open log file: " << log_file << std::endl;
    std::exit(1);
  }
}

static std::string_view get_hostname() {
  static std::string hostname;
  if (hostname.empty()) {
    char buf[256];
    gethostname(buf, sizeof(buf));
    hostname = buf;
  }

  return hostname;
}

static int get_pid() {
  static int pid = getpid();
  return pid;
}

void MyLogSink::send(google::LogSeverity severity, const char*, const char* base_filename, int line,
                     const struct tm* tm, const char* message, std::size_t message_len) {
  static const std::unordered_map<google::LogSeverity, std::string_view> lvl_names = {
      {google::GLOG_INFO, "info"},
      {google::GLOG_WARNING, "warn"},
      {google::GLOG_ERROR, "fail"},
      {google::GLOG_FATAL, "crit"}};

  { /* Get timestamp in Syslog format */
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%b %d %H:%M:%S", tm);
    m_log_file << timestamp;
  }

  m_log_file << " " << get_hostname();
  m_log_file << " quixd[" << get_pid() << "]: ";
  m_log_file << "{" << base_filename << ":" << line << "}: ";
  m_log_file << "(" << lvl_names.at(severity) << "): ";
  m_log_file << "\"" << escape_message(std::string_view(message, message_len)) << "\"";
  m_log_file << std::endl;
}
