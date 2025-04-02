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
#include <lsp/core/resource/File.hh>
#include <nitrate-core/Assert.hh>

#include "lsp/core/protocol/Base.hh"

using namespace no3::lsp::core;

class ConstFile::PImpl {
public:
  FlyPath m_path;
  FlyString m_raw;
  FileRevision m_revision;

  PImpl(FlyPath path, FileRevision revision, FlyString raw)
      : m_path(std::move(path)), m_raw(std::move(raw)), m_revision(revision) {}
  PImpl(const PImpl &) = delete;
};

ConstFile::ConstFile(FlyPath path, FileRevision revision, FlyString raw)
    : m_impl(std::make_unique<PImpl>(std::move(path), revision, std::move(raw))) {}

ConstFile::~ConstFile() = default;

auto ConstFile::GetRevision() const -> FileRevision {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_revision;
}

auto ConstFile::GetPath() const -> FlyPath {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_path;
}

auto ConstFile::GetFileName() const -> FlyPath {
  qcore_assert(m_impl != nullptr);
  return FlyPath(m_impl->m_path->filename());
}

auto ConstFile::GetFileNameWithoutExtension() const -> FlyString {
  qcore_assert(m_impl != nullptr);
  return FlyString(m_impl->m_path->stem());
}

auto ConstFile::GetFileExtension() const -> FlyString {
  qcore_assert(m_impl != nullptr);
  return FlyString(m_impl->m_path->extension());
}

auto ConstFile::GetFileSizeInBytes() const -> std::streamsize {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_raw->size();
}

auto ConstFile::GetFileSizeInKiloBytes() const -> std::streamsize { return GetFileSizeInBytes() / 1000; }
auto ConstFile::GetFileSizeInMegaBytes() const -> std::streamsize { return GetFileSizeInKiloBytes() / 1000; }
auto ConstFile::GetFileSizeInGigaBytes() const -> std::streamsize { return GetFileSizeInMegaBytes() / 1000; }

auto ConstFile::ReadAll() const -> FlyString {
  qcore_assert(m_impl != nullptr);
  return m_impl->m_raw;
}

class SourceReferencingStream : public boost::iostreams::stream<boost::iostreams::array_source> {
  FlyString m_source;

public:
  SourceReferencingStream(const char *data, std::size_t size, FlyString source)
      : boost::iostreams::stream<boost::iostreams::array_source>(data, size), m_source(std::move(source)){};
};

auto ConstFile::GetReader() const -> std::unique_ptr<std::istream> {
  qcore_assert(m_impl != nullptr);

  const auto &data = m_impl->m_raw;
  return std::make_unique<SourceReferencingStream>(data->data(), data->size(), data);
}
