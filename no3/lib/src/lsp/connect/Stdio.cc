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
#include <lsp/connect/Connection.hh>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp;

class FileDescriptorPairStream : public std::streambuf {
  char m_tmp;
  int m_in;
  int m_out;
  bool m_close;

public:
  FileDescriptorPairStream(int in, int out, bool close) : m_in(in), m_out(out), m_close(close){};
  FileDescriptorPairStream(const FileDescriptorPairStream &) = delete;
  FileDescriptorPairStream(FileDescriptorPairStream &&) = delete;
  ~FileDescriptorPairStream() override {
    if (m_close) {
      if (m_in >= 0) {
        close(m_in);
      }
      if (m_out >= 0) {
        close(m_out);
      }
    }
  }

  int_type underflow() override {
    /// FIXME: Verify this is correct.

    auto bytes_read = ::read(m_in, &m_tmp, 1);
    if (bytes_read <= 0) {
      return traits_type::eof();
    }
    setg(&m_tmp, &m_tmp, &m_tmp + 1);
    return traits_type::to_int_type(m_tmp);
  }

  int_type overflow(int_type c) override {
    /// FIXME: Verify this is correct.

    auto ch = traits_type::to_char_type(c);
    return ::write(m_out, &ch, 1) <= 0 ? traits_type::eof() : traits_type::not_eof(c);
  }
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
