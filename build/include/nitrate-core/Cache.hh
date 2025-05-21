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

#ifndef __NITRATE_CORE_CACHE_H__
#define __NITRATE_CORE_CACHE_H__

#include <array>
#include <cstdint>
#include <functional>
#include <mutex>
#include <nitrate-core/Init.hh>
#include <nitrate-core/SmartLock.hh>
#include <string>

namespace ncc {
  constexpr size_t kResourceKeySize = 20;
  using ResourceKey = std::array<uint8_t, kResourceKeySize>;

  template <typename Value>
  class IResourceCache {
  public:
    virtual ~IResourceCache() = default;

    virtual auto Has(const ResourceKey &key) -> bool = 0;
    virtual auto Read(const ResourceKey &key, Value &value) -> bool = 0;
    virtual auto Write(const ResourceKey &key, const Value &value) -> bool = 0;
  };

  template <typename Value>
  class ExternalResourceCache final : public IResourceCache<Value> {
    using has_t = std::function<bool(const ResourceKey &)>;
    using read_t = std::function<bool(const ResourceKey &, Value &)>;
    using write_t = std::function<bool(const ResourceKey &, Value)>;

  public:
    ExternalResourceCache()
        : m_has([](const ResourceKey &) { return false; }),
          m_read([](const ResourceKey &, Value &) { return false; }),
          m_write([](const ResourceKey &, Value) { return false; }) {}

    auto Has(const ResourceKey &key) -> bool override {
      SmartLock lock(m_mutex);

      return m_has(key);
    }

    auto Read(const ResourceKey &key, Value &value) -> bool override {
      SmartLock lock(m_mutex);

      return m_read(key, value);
    }

    auto Write(const ResourceKey &key, const Value &value) -> bool override {
      SmartLock lock(m_mutex);

      return m_write(key, value);
    }

    void Bind(has_t has, read_t read, write_t write) {
      SmartLock lock(m_mutex);

      m_has = std::move(has);
      m_read = std::move(read);
      m_write = std::move(write);
    }

  private:
    has_t m_has;
    read_t m_read;
    write_t m_write;
    std::recursive_mutex m_mutex;
  };

  template <typename Value>
  class MockArtifactCache final : public IResourceCache<Value> {
  public:
    [[nodiscard]] auto Has(const ResourceKey &) const -> bool override { return false; }
    auto Read(const ResourceKey &, Value &) const -> bool override { return false; }
    auto Write(const ResourceKey &, const Value &) -> bool override { return false; }
  };

  using TheCache = ExternalResourceCache<std::string>;

  auto GetCache() -> TheCache &;
}  // namespace ncc

#endif  // __NITRATE_CORE_CACHE_H__
