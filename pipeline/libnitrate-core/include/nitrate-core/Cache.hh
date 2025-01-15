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
#include <string>

namespace ncc {
  using ResourceKey = std::array<uint8_t, 20>;

  template <typename Value>
  class IResourceCache {
  public:
    virtual ~IResourceCache() = default;

    virtual bool has(const ResourceKey &key) = 0;
    virtual bool read(const ResourceKey &key, Value &value) = 0;
    virtual bool write(const ResourceKey &key, const Value &value) = 0;
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

    bool has(const ResourceKey &key) override {
      bool sync = EnableSync;

      if (sync) {
        m_mutex.lock();
      }

      auto r = m_has(key);

      if (sync) {
        m_mutex.unlock();
      }

      return r;
    }

    bool read(const ResourceKey &key, Value &value) override {
      bool sync = EnableSync;

      if (sync) {
        m_mutex.lock();
      }

      auto r = m_read(key, value);

      if (sync) {
        m_mutex.unlock();
      }

      return r;
    }

    bool write(const ResourceKey &key, const Value &value) override {
      bool sync = EnableSync;

      if (sync) {
        m_mutex.lock();
      }

      auto r = m_write(key, value);

      if (sync) {
        m_mutex.unlock();
      }

      return r;
    }

    void bind(has_t has, read_t read, write_t write) {
      bool sync = EnableSync;

      if (sync) {
        m_mutex.lock();
      }

      m_has = std::move(has);
      m_read = std::move(read);
      m_write = std::move(write);

      if (sync) {
        m_mutex.unlock();
      }
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
    bool has(const ResourceKey &) const override { return false; }
    bool read(const ResourceKey &, Value &) const override { return false; }
    bool write(const ResourceKey &, const Value &) override { return false; }
  };

  using TheCache = ExternalResourceCache<std::string>;

  TheCache &get_cache();
}  // namespace ncc

#endif  // __NITRATE_CORE_CACHE_H__
