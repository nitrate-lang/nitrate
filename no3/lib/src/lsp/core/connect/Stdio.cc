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

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <lsp/core/connect/Connection.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp;

/// TODO: Verify this code

class FileDescriptorPairStream : public std::streambuf {
  int m_in;
  int m_out;
  bool m_close;

public:
  FileDescriptorPairStream(int in, int out, bool close) : m_in(in), m_out(out), m_close(close) {
    setg(nullptr, nullptr, nullptr);
    setp(nullptr, nullptr);
  };

  ~FileDescriptorPairStream() override {
    if (m_close) {
      close(m_in);
      close(m_out);
    }
  }

  int_type underflow() override {
    char c;
    ssize_t n = read(m_in, &c, 1);
    if (n <= 0) {
      return traits_type::eof();
    }
    setg(&c, &c, &c + 1);
    return traits_type::to_int_type(c);
  }

  int_type overflow(int_type c) override {
    if (c != traits_type::eof()) {
      char ch = traits_type::to_char_type(c);
      ssize_t n = write(m_out, &ch, 1);
      if (n <= 0) {
        return traits_type::eof();
      }
    }
    return c;
  }

  int sync() override { return 0; }
};

class IostreamDerivative : public std::iostream {
  std::unique_ptr<FileDescriptorPairStream> m_stream;

public:
  IostreamDerivative(std::unique_ptr<FileDescriptorPairStream> stream)
      : std::iostream(stream.get()), m_stream(std::move(stream)) {}
};

auto core::ConnectToStdio() -> std::optional<DuplexStream> {
  Log << Trace << "Creating stream wrapper for stdin and stdout";

  auto stream_buf = std::make_unique<FileDescriptorPairStream>(STDIN_FILENO, STDOUT_FILENO, false);
  auto io_stream = std::make_unique<IostreamDerivative>(std::move(stream_buf));
  if (!io_stream->good()) {
    Log << "Failed to create stream wrapper for stdin and stdout";
    return std::nullopt;
  }

  Log << Trace << "Connected to stdio";

  return io_stream;
}
