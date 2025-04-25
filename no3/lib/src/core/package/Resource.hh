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

#pragma once

#include <core/package/LazyResource.hh>
#include <memory>

namespace no3::package {
  class ResourceReader : public LazyResourceReader {
    class PImpl;
    std::unique_ptr<PImpl> m_impl;

  public:
    ResourceReader();
    ResourceReader(std::istream& istream);
    ResourceReader(const ResourceReader&) = delete;
    ResourceReader(ResourceReader&&) = default;
    ResourceReader& operator=(const ResourceReader&) = delete;
    ResourceReader& operator=(ResourceReader&&) = default;
    ~ResourceReader();

    [[nodiscard]] auto Resolve() const -> ResourceReader&;
    [[nodiscard]] auto GetReader() const -> std::istream&;
    [[nodiscard]] auto GetSize() const -> std::size_t;
  };

  class ResourceWriter : public LazyResourceWriter {
    class PImpl;
    std::unique_ptr<PImpl> m_impl;

  public:
    ResourceWriter(std::ostream& ostream);
    ResourceWriter(const ResourceWriter&) = delete;
    ResourceWriter(ResourceWriter&&) = default;
    ResourceWriter& operator=(const ResourceWriter&) = delete;
    ResourceWriter& operator=(ResourceWriter&&) = default;
    ~ResourceWriter();

    [[nodiscard]] auto Resolve() const -> ResourceWriter&;
    [[nodiscard]] auto GetWriter() const -> std::ostream&;
  };

  class Resource : public ResourceReader, public ResourceWriter {
    class PImpl;
    std::unique_ptr<PImpl> m_impl;

  public:
    Resource(std::istream& istream, std::ostream& ostream);
    Resource(const Resource&) = delete;
    Resource(Resource&&) = default;
    Resource& operator=(const Resource&) = delete;
    Resource& operator=(Resource&&) = default;
    ~Resource() = default;

    [[nodiscard]] auto Resolve() const -> Resource&;
    [[nodiscard]] auto Desolve() const -> LazyResource;
    [[nodiscard]] auto GetStream() const -> std::iostream&;
  };
}  // namespace no3::package
