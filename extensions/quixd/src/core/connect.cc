#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <core/server.hh>
#include <cstdint>
#include <cstring>
#include <stdexcept>

using ManagedHandle = std::optional<std::unique_ptr<Connection>>;

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
      try {
        int port_tmp = std::stoi(param);

        if (port_tmp < 0 || port_tmp > UINT16_MAX) {
          throw std::out_of_range("");
        }

        port = port_tmp;
      } catch (std::invalid_argument&) {
        LOG(ERROR) << "Invalid port number: " << param;
        return std::nullopt;
      } catch (std::out_of_range&) {
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
        throw std::runtime_error("Failed to write to stream");
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
        throw std::runtime_error("Failed to write to stream");
      }
      written += n;
    }

    return count;
  }

  virtual int_type underflow() override {
    char c;
    ssize_t res = read(m_fd, &c, 1);
    if (res < 0) {
      throw std::runtime_error("Failed to read from stream");
    }

    if (res == 0) {
      return traits_type::eof();
    }

    setg(&c, &c, &c + 1);

    return traits_type::to_int_type(c);
  }

  virtual std::streamsize xsgetn(char* s, std::streamsize count) override {
    ssize_t res = read(m_fd, s, count);

    if (res < 0) {
      throw std::runtime_error("Failed to read from stream");
    }

    return res;
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

  auto thing = std::make_shared<FdStreamBuf>(conn.value());
  auto stream = std::make_unique<Connection>(thing);

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

  auto thing = std::make_shared<FdStreamBuf>(conn.value());
  auto stream = std::make_unique<Connection>(thing);

  LOG(INFO) << "Connected to TCP port " << tcp_port;

  return stream;
}

static ManagedHandle connect_to_stdio() {
  LOG(INFO) << "Connecting to stdio";

  class MyStreamBuf : public std::streambuf {
    std::ostream& m_output;
    std::istream& m_input;

  public:
    MyStreamBuf(std::ostream& output, std::istream& input) : m_output(output), m_input(input) {}

    virtual int_type overflow(int_type ch) override {
      if (ch != EOF) {
        char c = ch;
        m_output.write(&c, 1);
      }

      return ch;
    }

    virtual std::streamsize xsputn(const char* s, std::streamsize count) override {
      m_output.write(s, count);

      return count;
    }

    virtual int_type underflow() override {
      char c;
      m_input.read(&c, 1);

      if (m_input.eof()) {
        return traits_type::eof();
      }

      setg(&c, &c, &c + 1);

      return traits_type::to_int_type(c);
    }

    virtual std::streamsize xsgetn(char* s, std::streamsize count) override {
      m_input.read(s, count);

      return m_input.gcount();
    }
  };

  auto thing = std::make_shared<MyStreamBuf>(std::cout, std::cin);
  auto stream = std::make_unique<Connection>(thing);

  LOG(INFO) << "Connected to stdio";

  return stream;
}

///==========================================================

Connection::~Connection() {
  LOG(INFO) << "Closing connection";

  flush();
}
