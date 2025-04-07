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

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <istream>
#include <lsp/protocol/Base.hh>
#include <lsp/resource/File.hh>
#include <nitrate-core/Assert.hh>

using namespace no3::lsp::core;

class ConstFile::PImpl {
public:
  FlyString m_file_uri;
  FlyByteString m_raw;
  FileVersion m_version;

  PImpl(FlyString file_uri, FileVersion version, FlyByteString raw)
      : m_file_uri(std::move(file_uri)), m_raw(std::move(raw)), m_version(version) {}
  PImpl(const PImpl &) = delete;
};

ConstFile::ConstFile(FlyString file_uri, FileVersion version, FlyByteString raw)
    : m_impl(std::make_unique<PImpl>(std::move(file_uri), version, std::move(raw))) {}

ConstFile::~ConstFile() = default;

auto ConstFile::GetVersion() const -> FileVersion {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_version;
}

auto ConstFile::GetURI() const -> FlyString {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_file_uri;
}

auto ConstFile::GetFileSizeInBytes() const -> std::streamsize {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_raw->size();
}

auto ConstFile::GetFileSizeInKiloBytes() const -> std::streamsize { return GetFileSizeInBytes() / 1000; }
auto ConstFile::GetFileSizeInMegaBytes() const -> std::streamsize { return GetFileSizeInKiloBytes() / 1000; }
auto ConstFile::GetFileSizeInGigaBytes() const -> std::streamsize { return GetFileSizeInMegaBytes() / 1000; }

auto ConstFile::ReadAll() const -> FlyByteString {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_raw;
}

class SourceReferencingStream : public boost::iostreams::stream<boost::iostreams::basic_array_source<uint8_t>> {
  FlyByteString m_source;

public:
  SourceReferencingStream(const uint8_t *data, std::size_t size, FlyByteString source)
      : boost::iostreams::stream<boost::iostreams::basic_array_source<uint8_t>>(data, size),
        m_source(std::move(source)){};
};

auto ConstFile::GetReader() const -> std::unique_ptr<std::basic_istream<uint8_t>> {
  qcore_assert(m_impl != nullptr);

  const auto &data = m_impl->m_raw;
  return std::make_unique<SourceReferencingStream>(data->data(), data->size(), data);
}
