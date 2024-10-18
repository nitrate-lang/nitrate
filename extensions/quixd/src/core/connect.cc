#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <charconv>
#include <core/server.hh>
#include <cstdint>
#include <cstring>

using ManagedHandle = std::optional<Connection>;

static ManagedHandle connect_to_pipe(const std::string& path);
static ManagedHandle connect_to_tcp_port(uint16_t tcp_port);
static ManagedHandle connect_to_stdio();

ManagedHandle open_connection(ConnectionType type, const std::string& param) {
  switch (type) {
    case ConnectionType::Pipe: {
      return connect_to_pipe(param);
    }

    case ConnectionType::Port: {
      uint16_t port = 0;

      std::from_chars_result res = std::from_chars(param.data(), param.data() + param.size(), port);
      if (res.ec != std::errc()) {
        LOG(ERROR) << "Invalid port number: " << param;
        return std::nullopt;
      }

      if (port < 0 || port > UINT16_MAX) {
        LOG(ERROR) << "Port number is out of the range of valid TCP ports";
        return std::nullopt;
      }

      return connect_to_tcp_port(port);
    }

    case ConnectionType::Stdio: {
      return connect_to_stdio();
    }
  }
}

class FdStreamBuf : public std::streambuf {
  int m_fd;

public:
  FdStreamBuf(int fd) : m_fd(fd) {}
  ~FdStreamBuf() {
    if (m_fd != -1) {
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

  virtual std::streamsize xsputn(const char* s, std::streamsize count) override {
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
    char c;
    ssize_t res = read(m_fd, &c, 1);
    if (res < 0) {
      LOG(ERROR) << "Failed to read from stream: " << strerror(errno);
      return traits_type::eof();
    }

    if (res == 0) {
      return traits_type::eof();
    }

    setg(&c, &c, &c + 1);

    return traits_type::to_int_type(c);
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

static std::optional<int> connect_unix_socket(const std::string& path) {
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

static ManagedHandle connect_to_pipe(const std::string& path) {
  auto conn = connect_unix_socket(path);
  if (!conn) {
    LOG(ERROR) << "Failed to connect to UNIX socket " << path;
    return std::nullopt;
  }

  auto in_buf = std::make_shared<FdStreamBuf>(conn.value());
  auto out_buf = std::make_shared<FdStreamBuf>(conn.value());
  auto stream =
      std::make_pair(std::make_unique<BufIStream>(in_buf), std::make_unique<BufOStream>(out_buf));

  LOG(INFO) << "Connected to UNIX socket " << path;

  return stream;
}

static std::optional<int> get_tcp_client(const std::string& host, uint16_t port) {
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
  }

  return client_fd;
}

static ManagedHandle connect_to_tcp_port(uint16_t tcp_port) {
  auto conn = get_tcp_client("127.0.0.1", tcp_port);
  if (!conn) {
    LOG(ERROR) << "Failed to connect to TCP port " << tcp_port;
    return std::nullopt;
  }

  auto in_buf = std::make_shared<FdStreamBuf>(conn.value());
  auto out_buf = std::make_shared<FdStreamBuf>(conn.value());
  auto stream =
      std::make_pair(std::make_unique<BufIStream>(in_buf), std::make_unique<BufOStream>(out_buf));

  LOG(INFO) << "Connected to TCP port " << tcp_port;

  return stream;
}

static ManagedHandle connect_to_stdio() {
  LOG(INFO) << "Connecting to stdio";

  auto in_buf = std::make_shared<FdStreamBuf>(STDIN_FILENO);
  auto out_buf = std::make_shared<FdStreamBuf>(STDOUT_FILENO);
  auto stream =
      std::make_pair(std::make_unique<BufIStream>(in_buf), std::make_unique<BufOStream>(out_buf));

  LOG(INFO) << "Connected to stdio";

  return stream;
}

///==========================================================

BufOStream::~BufOStream() {
  LOG(INFO) << "Closing connection";
  flush();
}
