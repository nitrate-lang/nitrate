#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <lsp/core/Server.hh>
#include <memory>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp;

static auto ConnectUnixSocket(const auto& path) -> std::optional<int> {
  int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1) {
    Log << "Failed to create socket: " << GetStrerror();
    return std::nullopt;
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, std::string(path).c_str(), sizeof(addr.sun_path) - 1);

  if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    Log << "Failed to connect to socket: " << GetStrerror();
    return std::nullopt;
  }

  return sockfd;
}

static auto ConnectToPipe(const auto& path) -> std::optional<srv::Connection> {
  auto conn = ConnectUnixSocket(path);
  if (!conn) {
    Log << "Failed to connect to UNIX socket " << path;
    return std::nullopt;
  }

  auto source = boost::iostreams::file_descriptor_source(conn.value(), boost::iostreams::close_handle);

  auto in_stream = std::make_unique<boost::iostreams::stream<boost::iostreams::file_descriptor_source>>((source));

  auto out_stream = std::make_unique<boost::iostreams::stream<boost::iostreams::file_descriptor_sink>>(
      conn.value(), boost::iostreams::close_handle);

  if (!in_stream->is_open() || !out_stream->is_open()) {
    Log << "Failed to open stdio streams";
    return std::nullopt;
  }

  auto stream = std::make_pair(std::move(in_stream), std::move(out_stream));

  Log << Info << "Connected to UNIX socket " << path;

  return stream;
}

static auto GetTcpClient(const auto& host, uint16_t port) -> std::optional<int> {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    Log << "Failed to create socket: " << GetStrerror();
    return std::nullopt;
  }

  union {
    struct sockaddr_in m_in;
    struct sockaddr m_addr;
  } addr;

  addr.m_in.sin_family = AF_INET;
  addr.m_in.sin_port = htons(port);
  addr.m_in.sin_addr.s_addr = inet_addr(std::string(host).c_str());

  if (bind(fd, &addr.m_addr, sizeof(addr.m_addr)) == -1) {
    Log << "Failed to bind socket: " << GetStrerror();
    return std::nullopt;
  }

  Log << Info << "Bound to TCP port " << port;
  Log << Info << "Waiting for connection";

  if (listen(fd, 1) == -1) {
    Log << "Failed to listen on socket: " << GetStrerror();
    return std::nullopt;
  }

  int client_fd = accept(fd, nullptr, nullptr);
  if (client_fd == -1) {
    Log << "Failed to accept connection: " << GetStrerror();
    return std::nullopt;
  }

  Log << Info << "Accepted connection from client";

  if (close(fd) == -1) {
    Log << "Failed to close listening socket: " << GetStrerror();
    return std::nullopt;
  }

  return client_fd;
}

static auto ConnectToTcpPort(uint16_t tcp_port) -> std::optional<srv::Connection> {
  auto conn = GetTcpClient("127.0.0.1", tcp_port);
  if (!conn) {
    Log << "Failed to connect to TCP port " << tcp_port;
    return std::nullopt;
  }

  auto source = boost::iostreams::file_descriptor_source(conn.value(), boost::iostreams::close_handle);

  auto in_stream = std::make_unique<boost::iostreams::stream<boost::iostreams::file_descriptor_source>>((source));

  auto out_stream = std::make_unique<boost::iostreams::stream<boost::iostreams::file_descriptor_sink>>(
      conn.value(), boost::iostreams::close_handle);

  if (!in_stream->is_open() || !out_stream->is_open()) {
    Log << "Failed to open stdio streams";
    return std::nullopt;
  }

  auto stream = std::make_pair(std::move(in_stream), std::move(out_stream));

  Log << Info << "Connected to TCP port " << tcp_port;

  return stream;
}

static auto ConnectToStdio() -> std::optional<srv::Connection> {
  Log << Info << "Connecting to stdio";

  auto in_stream = std::make_unique<boost::iostreams::stream<boost::iostreams::file_descriptor>>(
      STDIN_FILENO, boost::iostreams::never_close_handle);

  auto out_stream = std::make_unique<boost::iostreams::stream<boost::iostreams::file_descriptor>>(
      STDOUT_FILENO, boost::iostreams::never_close_handle);

  if (!in_stream->is_open() || !out_stream->is_open()) {
    Log << "Failed to open stdio streams";
    return std::nullopt;
  }

  auto stream = std::make_pair(std::move(in_stream), std::move(out_stream));

  Log << Info << "Connected to stdio";

  return stream;
}

auto srv::OpenConnection(srv::ConnectionType type, const String& target) -> std::optional<srv::Connection> {
  switch (type) {
    case ConnectionType::Pipe: {
      return ConnectToPipe(target);
    }

    case ConnectionType::Port: {
      uint16_t port = 0;

      std::from_chars_result res = std::from_chars(target->c_str(), target->c_str() + target->size(), port);
      if (res.ec != std::errc()) {
        Log << "Invalid port number: " << target;
        return std::nullopt;
      }

      if (port < 0 || port > UINT16_MAX) {
        Log << "Port number is out of the range of valid TCP ports";
        return std::nullopt;
      }

      return ConnectToTcpPort(port);
    }

    case ConnectionType::Stdio: {
      return ConnectToStdio();
    }
  }
}
