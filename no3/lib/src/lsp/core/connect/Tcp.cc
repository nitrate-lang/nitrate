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
#include <lsp/core/connect/Connection.hh>
#include <nitrate-core/Logger.hh>

/// TODO: Verify this code

using namespace ncc;
using namespace no3::lsp;

static auto GetTcpClient(const auto& host, uint16_t port) -> std::optional<int> {
  std::array<char, 256> err_buffer;

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    Log << "Failed to create socket: " << strerror_r(errno, err_buffer.data(), err_buffer.size());
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
    Log << "Failed to bind socket: " << strerror_r(errno, err_buffer.data(), err_buffer.size());
    return std::nullopt;
  }

  Log << Info << "Bound to TCP port " << port;
  Log << Info << "Waiting for connection";

  if (listen(fd, 1) == -1) {
    Log << "Failed to listen on socket: " << strerror_r(errno, err_buffer.data(), err_buffer.size());
    return std::nullopt;
  }

  int client_fd = accept(fd, nullptr, nullptr);
  if (client_fd == -1) {
    Log << "Failed to accept connection: " << strerror_r(errno, err_buffer.data(), err_buffer.size());
    return std::nullopt;
  }

  Log << Info << "Accepted connection from client";

  if (close(fd) == -1) {
    Log << "Failed to close listening socket: " << strerror_r(errno, err_buffer.data(), err_buffer.size());
    return std::nullopt;
  }

  return client_fd;
}

auto core::ConnectToTcpPort(uint16_t tcp_port) -> std::optional<DuplexStream> {
  auto conn = GetTcpClient("127.0.0.1", tcp_port);
  if (!conn) {
    Log << "Failed to connect to TCP port " << tcp_port;
    return std::nullopt;
  }

  auto source = boost::iostreams::file_descriptor(conn.value(), boost::iostreams::close_handle);
  auto io_stream = std::make_unique<boost::iostreams::stream<boost::iostreams::file_descriptor>>((source));

  if (!io_stream->is_open()) {
    Log << "Failed to open stdio streams";
    return std::nullopt;
  }

  Log << Info << "Connected to TCP port " << tcp_port;

  return io_stream;
}
