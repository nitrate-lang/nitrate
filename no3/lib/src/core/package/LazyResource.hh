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

#include <future>
#include <iostream>
#include <optional>

namespace no3::package {
  class ResourceReader;
  class ResourceWriter;
  class Resource;

  class LazyResourceReader {
  public:
    using LazyReader = std::shared_future<std::optional<std::istream>>;

    LazyResourceReader() : m_reader(AlwaysFail()) {}
    LazyResourceReader(LazyReader lazy_reader) : m_reader(std::move(lazy_reader)) {}
    LazyResourceReader(const LazyResourceReader&) = delete;
    LazyResourceReader(LazyResourceReader&&) = default;
    LazyResourceReader& operator=(const LazyResourceReader&) = delete;
    LazyResourceReader& operator=(LazyResourceReader&&) = default;
    ~LazyResourceReader() = default;

    [[nodiscard]] auto GetReader() const -> const std::optional<std::istream>&;
    [[nodiscard]] auto Resolve() const -> std::optional<ResourceReader>;

  private:
    LazyReader m_reader;

    static auto AlwaysFail() -> LazyReader {
      return std::async(std::launch::deferred, []() -> std::optional<std::istream> { return std::nullopt; });
    }
  };

  class LazyResourceWriter {
  public:
    using LazyWriter = std::shared_future<std::optional<std::ostream>>;

    LazyResourceWriter();
    LazyResourceWriter(LazyWriter lazy_writer) : m_writer(std::move(lazy_writer)) {}
    LazyResourceWriter(const LazyResourceWriter&) = delete;
    LazyResourceWriter(LazyResourceWriter&&) = default;
    LazyResourceWriter& operator=(const LazyResourceWriter&) = delete;
    LazyResourceWriter& operator=(LazyResourceWriter&&) = default;
    ~LazyResourceWriter() = default;

    [[nodiscard]] auto GetWriter() const -> const std::optional<std::ostream>&;
    [[nodiscard]] auto Resolve() const -> std::optional<ResourceWriter>;

  private:
    LazyWriter m_writer;

    static auto AlwaysFail() -> LazyWriter {
      return std::async(std::launch::deferred, []() -> std::optional<std::ostream> { return std::nullopt; });
    }
  };

  class LazyResource : public LazyResourceReader, public LazyResourceWriter {
  public:
    LazyResource() = default;
    LazyResource(LazyResourceReader::LazyReader lazy_reader, LazyResourceWriter::LazyWriter lazy_writer)
        : LazyResourceReader(std::move(lazy_reader)), LazyResourceWriter(std::move(lazy_writer)) {}
    LazyResource(const LazyResource&) = delete;
    LazyResource(LazyResource&&) = default;
    LazyResource& operator=(const LazyResource&) = delete;
    LazyResource& operator=(LazyResource&&) = default;
    ~LazyResource() = default;

    [[nodiscard]] auto Resolve() const -> std::optional<Resource>;
  };
}  // namespace no3::package
