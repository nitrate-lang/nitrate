#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <boost/iostreams/stream.hpp>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <lsp/core/server.hh>
#include <memory>
#include <nitrate-core/Logger.hh>

static auto ConnectUnixSocket(const std::string& path) -> std::optional<int> {
  int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1) {
    LOG(ERROR) << "Failed to create socket: " << GetStrerror();
    return std::nullopt;
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

  if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    LOG(ERROR) << "Failed to connect to socket: " << GetStrerror();
    return std::nullopt;
  }

  return sockfd;
}

static auto ConnectToPipe(const std::string& path)
    -> std::optional<Connection> {
  auto conn = ConnectUnixSocket(path);
  if (!conn) {
    LOG(ERROR) << "Failed to connect to UNIX socket " << path;
    return std::nullopt;
  }

  auto source = boost::iostreams::file_descriptor_source(
      conn.value(), boost::iostreams::close_handle);

  auto in_stream = std::make_unique<
      boost::iostreams::stream<boost::iostreams::file_descriptor_source>>(
      (source));

  auto out_stream = std::make_unique<
      boost::iostreams::stream<boost::iostreams::file_descriptor_sink>>(
      conn.value(), boost::iostreams::close_handle);

  if (!in_stream->is_open() || !out_stream->is_open()) {
    LOG(ERROR) << "Failed to open stdio streams";
    return std::nullopt;
  }

  auto stream = std::make_pair(std::move(in_stream), std::move(out_stream));

  LOG(INFO) << "Connected to UNIX socket " << path;

  return stream;
}

static auto GetTcpClient(const std::string& host,
                         uint16_t port) -> std::optional<int> {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    LOG(ERROR) << "Failed to create socket: " << GetStrerror();
    return std::nullopt;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(host.c_str());

  if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) {
    LOG(ERROR) << "Failed to bind socket: " << GetStrerror();
    return std::nullopt;
  }

  LOG(INFO) << "Bound to TCP port " << port;
  LOG(INFO) << "Waiting for connection";

  if (listen(fd, 1) == -1) {
    LOG(ERROR) << "Failed to listen on socket: " << GetStrerror();
    return std::nullopt;
  }

  int client_fd = accept(fd, nullptr, nullptr);
  if (client_fd == -1) {
    LOG(ERROR) << "Failed to accept connection: " << GetStrerror();
    return std::nullopt;
  }

  LOG(INFO) << "Accepted connection from client";

  if (close(fd) == -1) {
    LOG(ERROR) << "Failed to close listening socket: " << GetStrerror();
    return std::nullopt;
  }

  return client_fd;
}

static auto ConnectToTcpPort(uint16_t tcp_port) -> std::optional<Connection> {
  auto conn = GetTcpClient("127.0.0.1", tcp_port);
  if (!conn) {
    LOG(ERROR) << "Failed to connect to TCP port " << tcp_port;
    return std::nullopt;
  }

  auto source = boost::iostreams::file_descriptor_source(
      conn.value(), boost::iostreams::close_handle);

  auto in_stream = std::make_unique<
      boost::iostreams::stream<boost::iostreams::file_descriptor_source>>(
      (source));

  auto out_stream = std::make_unique<
      boost::iostreams::stream<boost::iostreams::file_descriptor_sink>>(
      conn.value(), boost::iostreams::close_handle);

  if (!in_stream->is_open() || !out_stream->is_open()) {
    LOG(ERROR) << "Failed to open stdio streams";
    return std::nullopt;
  }

  auto stream = std::make_pair(std::move(in_stream), std::move(out_stream));

  LOG(INFO) << "Connected to TCP port " << tcp_port;

  return stream;
}

static auto ConnectToStdio() -> std::optional<Connection> {
  LOG(INFO) << "Connecting to stdio";

  auto in_stream = std::make_unique<
      boost::iostreams::stream<boost::iostreams::file_descriptor>>(
      STDIN_FILENO, boost::iostreams::never_close_handle);

  auto out_stream = std::make_unique<
      boost::iostreams::stream<boost::iostreams::file_descriptor>>(
      STDOUT_FILENO, boost::iostreams::never_close_handle);

  if (!in_stream->is_open() || !out_stream->is_open()) {
    LOG(ERROR) << "Failed to open stdio streams";
    return std::nullopt;
  }

  auto stream = std::make_pair(std::move(in_stream), std::move(out_stream));

  LOG(INFO) << "Connected to stdio";

  return stream;
}

auto OpenConnection(ConnectionType type,
                    const std::string& param) -> std::optional<Connection> {
  switch (type) {
    case ConnectionType::Pipe: {
      return ConnectToPipe(param);
    }

    case ConnectionType::Port: {
      uint16_t port = 0;

      std::from_chars_result res =
          std::from_chars(param.data(), param.data() + param.size(), port);
      if (res.ec != std::errc()) {
        LOG(ERROR) << "Invalid port number: " << param;
        return std::nullopt;
      }

      if (port < 0 || port > UINT16_MAX) {
        LOG(ERROR) << "Port number is out of the range of valid TCP ports";
        return std::nullopt;
      }

      return ConnectToTcpPort(port);
    }

    case ConnectionType::Stdio: {
      return ConnectToStdio();
    }
  }
}
