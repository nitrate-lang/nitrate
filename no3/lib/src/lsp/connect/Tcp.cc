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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <lsp/connect/Connection.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp;

static auto AcceptTcpClientConnection(const char* srv_host, uint16_t srv_port) -> std::optional<int> {
  union SockAddrUnion {
    struct sockaddr_in m_in;
    struct sockaddr m_addr;
  };

  std::array<char, 256> err_buffer;
  int fd = -1;
  int client_fd = -1;

  {
    Log << Trace << "Creating TCP AF_INET socket";

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
      auto* error = strerror_r(errno, err_buffer.data(), err_buffer.size());
      Log << "Failed to create socket: " << error;
      return std::nullopt;
    }

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
      close(fd);

      auto* error = strerror_r(errno, err_buffer.data(), err_buffer.size());
      Log << "Failed to set socket options: " << error;
      return std::nullopt;
    }

    Log << Trace << "TCP socket created";
  }

  {
    SockAddrUnion addr = {
        .m_in =
            {
                .sin_family = AF_INET,
                .sin_port = htons(srv_port),
                .sin_addr =
                    {
                        .s_addr = inet_addr(srv_host),
                    },
                .sin_zero = {0},
            },
    };

    Log << Trace << "Binding to TCP socket";

    if (bind(fd, &addr.m_addr, sizeof(addr.m_addr)) == -1) {
      close(fd);

      auto* error = strerror_r(errno, err_buffer.data(), err_buffer.size());
      Log << "Failed to bind socket: " << error;
      return std::nullopt;
    }

    Log << Trace << "Socket bound to " << srv_host << ":" << srv_port;
  }

  {
    if (listen(fd, 1) == -1) {
      close(fd);

      auto* error = strerror_r(errno, err_buffer.data(), err_buffer.size());
      Log << "Failed to listen on socket: " << error;
      return std::nullopt;
    }

    Log << Trace << "Listening on TCP socket";
  }

  {
    Log << Info << "Waiting for TCP connection on: " << srv_host << ":" << srv_port;

    client_fd = accept(fd, nullptr, nullptr);
    if (client_fd == -1) {
      close(fd);

      auto* error = strerror_r(errno, err_buffer.data(), err_buffer.size());
      Log << "Failed to accept connection: " << error;
      return std::nullopt;
    }

    Log << Info << "Accepted connection from client";
  }

  {
    Log << Trace << "Closing listening socket";

    if (close(fd) == -1) {
      close(client_fd);

      auto* error = strerror_r(errno, err_buffer.data(), err_buffer.size());
      Log << "Failed to close listening socket: " << error;
      return std::nullopt;
    }

    Log << Trace << "Listening socket closed";
  }

  Log << Trace << "Returning client socket: " << client_fd;

  return client_fd;
}

auto core::ConnectToTcpPort(uint16_t tcp_port) -> std::optional<DuplexStream> {
  Log << Trace << "Creating temporary TCP server on port " << tcp_port;
  auto conn = AcceptTcpClientConnection("0.0.0.0", tcp_port);
  if (!conn) {
    Log << "Failed to accept a TCP client connection";
    return std::nullopt;
  }

  Log << Trace << "Creating boost::iostreams::stream from raw file descriptor";
  auto source = boost::iostreams::file_descriptor(conn.value(), boost::iostreams::close_handle);
  auto io_stream = std::make_unique<boost::iostreams::stream<boost::iostreams::file_descriptor>>((source));

  if (!io_stream->is_open()) {
    Log << "Failed to open TCP iostreams";
    return std::nullopt;
  }

  Log << Trace << "Connected to a TCP client";

  return io_stream;
}
