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

using namespace ncc;
using namespace no3::lsp;

/// TODO: Verify this code

static auto ConnectUnixSocket(const std::filesystem::path& path) -> std::optional<int> {
  std::array<char, 256> err_buffer;

  auto sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1) {
    Log << "Failed to create socket: " << strerror_r(errno, err_buffer.data(), err_buffer.size());
    return std::nullopt;
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

  if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    Log << "Failed to connect to socket: " << strerror_r(errno, err_buffer.data(), err_buffer.size());
    close(sockfd);
    return std::nullopt;
  }

  return sockfd;
}

auto core::ConnectToPipe(const std::filesystem::path& path) -> std::optional<DuplexStream> {
  Log << Trace << "Connecting to UNIX socket " << path;

  auto conn = ConnectUnixSocket(path);
  if (!conn) {
    Log << "Failed to connect to UNIX socket " << path;
    return std::nullopt;
  }

  Log << Trace << "Creating boost::iostreams::stream from raw file descriptor";
  auto stream = boost::iostreams::file_descriptor(conn.value(), boost::iostreams::close_handle);
  auto io_stream = std::make_unique<boost::iostreams::stream<boost::iostreams::file_descriptor>>(stream);

  if (!io_stream->is_open()) {
    Log << "Boost::iostreams::stream failed to open";
    return std::nullopt;
  }

  Log << Trace << "Successfully connected to UNIX socket " << path;

  return io_stream;
}
