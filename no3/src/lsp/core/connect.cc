#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <charconv>
#include <cstdint>
#include <cstring>
#include <lsp/core/server.hh>

using ManagedHandle = std::optional<Connection>;

static ManagedHandle ConnectToPipe(const std::string& path);
static ManagedHandle ConnectToTcpPort(uint16_t tcp_port);
static ManagedHandle ConnectToStdio();

ManagedHandle OpenConnection(ConnectionType type, const std::string& param) {
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

class FdStreamBuf : public std::streambuf {
  int m_fd;
  bool m_close;
  char m_c = 0;

public:
  FdStreamBuf(int fd, bool close) : m_fd(fd), m_close(close) { errno = 0; }
  ~FdStreamBuf() {
    LOG(INFO) << "Closing file descriptor";

    if (m_close) {
      if (close(m_fd) == -1) {
        LOG(ERROR) << "Failed to close file descriptor: " << strerror(errno);
      }
    }
  }

  virtual int_type overflow(int_type ch) override {
    if (ch != EOF) {
      char c = ch;
      if (write(m_fd, &c, 1) != 1) {
        LOG(ERROR) << "Failed to write to stream: " << strerror(errno);
        return traits_type::eof();
      }
    }

    return ch;
  }

  virtual std::streamsize xsputn(const char* s,
                                 std::streamsize count) override {
    std::streamsize written = 0;
    while (written < count) {
      ssize_t n = write(m_fd, s + written, count - written);
      if (n == -1) {
        LOG(ERROR) << "Failed to write to stream: " << strerror(errno);
        return written;
      }
      written += n;
    }

    return count;
  }

  virtual int_type underflow() override {
    ssize_t res = read(m_fd, &m_c, 1);
    if (res < 0) {
      LOG(ERROR) << "Failed to read from stream: " << strerror(errno);
      return traits_type::eof();
    }

    if (res == 0) {
      return traits_type::eof();
    }

    setg(&m_c, &m_c, &m_c + 1);

    return traits_type::to_int_type(m_c);
  }

  virtual std::streamsize xsgetn(char* s, std::streamsize count) override {
    std::streamsize bytes_read = 0;

    while (bytes_read < count) {
      ssize_t n = read(m_fd, s + bytes_read, count - bytes_read);
      if (n == -1) {
        LOG(ERROR) << "Failed to read from stream: " << strerror(errno);
        return bytes_read;
      }

      if (n == 0) {
        break;
      }

      bytes_read += n;
    }

    return bytes_read;
  }
};

static std::optional<int> ConnectUnixSocket(const std::string& path) {
  int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1) {
    LOG(ERROR) << "Failed to create socket: " << strerror(errno);
    return std::nullopt;
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

  if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    LOG(ERROR) << "Failed to connect to socket: " << strerror(errno);
    return std::nullopt;
  }

  return sockfd;
}

static ManagedHandle ConnectToPipe(const std::string& path) {
  auto conn = ConnectUnixSocket(path);
  if (!conn) {
    LOG(ERROR) << "Failed to connect to UNIX socket " << path;
    return std::nullopt;
  }

  auto in_buf = std::make_shared<FdStreamBuf>(conn.value(), true);
  auto out_buf = std::make_shared<FdStreamBuf>(conn.value(), true);
  auto stream = std::make_pair(std::make_unique<BufIStream>(in_buf),
                               std::make_unique<BufOStream>(out_buf));

  LOG(INFO) << "Connected to UNIX socket " << path;

  return stream;
}

static std::optional<int> GetTcpClient(const std::string& host,
                                         uint16_t port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    LOG(ERROR) << "Failed to create socket: " << strerror(errno);
    return std::nullopt;
  }

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(host.c_str());

  if (bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) {
    LOG(ERROR) << "Failed to bind socket: " << strerror(errno);
    return std::nullopt;
  }

  LOG(INFO) << "Bound to TCP port " << port;
  LOG(INFO) << "Waiting for connection";

  if (listen(fd, 1) == -1) {
    LOG(ERROR) << "Failed to listen on socket: " << strerror(errno);
    return std::nullopt;
  }

  int client_fd = accept(fd, nullptr, nullptr);
  if (client_fd == -1) {
    LOG(ERROR) << "Failed to accept connection: " << strerror(errno);
    return std::nullopt;
  }

  LOG(INFO) << "Accepted connection from client";

  if (close(fd) == -1) {
    LOG(ERROR) << "Failed to close listening socket: " << strerror(errno);
    return std::nullopt;
  }

  return client_fd;
}

static ManagedHandle ConnectToTcpPort(uint16_t tcp_port) {
  auto conn = GetTcpClient("127.0.0.1", tcp_port);
  if (!conn) {
    LOG(ERROR) << "Failed to connect to TCP port " << tcp_port;
    return std::nullopt;
  }

  auto in_buf = std::make_shared<FdStreamBuf>(conn.value(), true);
  auto out_buf = std::make_shared<FdStreamBuf>(conn.value(), true);
  auto stream = std::make_pair(std::make_unique<BufIStream>(in_buf),
                               std::make_unique<BufOStream>(out_buf));

  LOG(INFO) << "Connected to TCP port " << tcp_port;

  return stream;
}

static ManagedHandle ConnectToStdio() {
  LOG(INFO) << "Connecting to stdio";

  auto in_buf = std::make_shared<FdStreamBuf>(STDIN_FILENO, false);
  auto out_buf = std::make_shared<FdStreamBuf>(STDOUT_FILENO, false);
  auto stream = std::make_pair(std::make_unique<BufIStream>(in_buf),
                               std::make_unique<BufOStream>(out_buf));

  LOG(INFO) << "Connected to stdio";

  return stream;
}

///==========================================================

BufOStream::~BufOStream() {
  LOG(INFO) << "Closing connection";
  flush();
}
