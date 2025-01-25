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

#include <boost/unordered_map.hpp>
#include <mutex>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/String.hh>
#include <sparsehash/dense_hash_map>

using namespace ncc;

class IStorage {
public:
  virtual ~IStorage() = default;

  [[nodiscard]] virtual auto Get(uint64_t id) -> CStringView = 0;
  virtual auto FromString(std::string_view str) -> uint64_t = 0;
  virtual auto FromString(std::string&& str) -> uint64_t = 0;
  [[nodiscard]] virtual auto CompareEq(uint64_t a, uint64_t b) -> bool = 0;
  [[nodiscard]] virtual auto CompareLt(uint64_t a, uint64_t b) -> bool = 0;
  virtual void Reset() = 0;
};

template <typename T>
class ConditionalLockGuard {
  T& m_mutex;
  bool m_enabled;

public:
  ConditionalLockGuard(T& mutex) : m_mutex(mutex) {
    m_enabled = EnableSync;

    if (m_enabled) {
      m_mutex.lock();
    }
  }

  ~ConditionalLockGuard() {
    if (m_enabled) {
      m_mutex.unlock();
    }
  }
};

class MemoryConservedStorage final : public IStorage {
  std::vector<std::vector<char>> m_data;
  google::dense_hash_map<std::string_view, uint64_t> m_map;
  std::mutex m_lock;

  static NCC_FORCE_INLINE constexpr auto FromStr(std::string_view str)
      -> std::vector<char> {
    std::vector<char> vec(str.size() + 1);
    std::copy(str.begin(), str.end(), vec.begin());
    vec.back() = '\0';
    return vec;
  }

  static NCC_FORCE_INLINE constexpr auto FromVec(const std::vector<char>& vec) {
    return CStringView(vec.data(), vec.size());
  }

public:
  MemoryConservedStorage() {
    constexpr size_t kInitSize = 4096;

    m_map.set_empty_key("");
    m_data.reserve(kInitSize);

    // Insert the empty string for index 0
    m_data.emplace_back(FromStr(""));
  }

  ~MemoryConservedStorage() override { Reset(); }

  [[nodiscard]] auto Get(uint64_t id) -> CStringView override {
    ConditionalLockGuard lock(m_lock);

    return id < m_data.size() ? FromVec(m_data[id]) : CStringView();
  }

  auto FromString(std::string_view str) -> uint64_t override {
    if (str.empty()) {
      return 0;
    }

    ConditionalLockGuard lock(m_lock);

    uint64_t id;
    if (auto it = m_map.find(str); it != m_map.end()) {
      id = it->second;
    } else {
      id = m_data.size();
      m_data.emplace_back(FromStr(str));
      m_map[FromVec(m_data.back())] = id;
    }

    return id;
  }

  auto FromString(std::string&& str) -> uint64_t override {
    if (str.empty()) {
      return 0;
    }

    ConditionalLockGuard lock(m_lock);

    uint64_t id;
    if (auto it = m_map.find(str); it != m_map.end()) {
      id = it->second;
    } else {
      id = m_data.size();
      m_data.emplace_back(FromStr(std::move(str)));
      m_map[FromVec(m_data.back())] = id;
    }

    return id;
  }

  [[nodiscard]] auto CompareEq(uint64_t a, uint64_t b) -> bool override {
    return a == b;
  }

  [[nodiscard]] auto CompareLt(uint64_t a, uint64_t b) -> bool override {
    return a < b;
  }

  void Reset() override {
    ConditionalLockGuard lock(m_lock);

    m_map.clear();
    m_data.clear();
  }
};

class FastStorage final : public IStorage {
  std::vector<std::vector<char>> m_data;
  std::vector<std::string> m_buffered;
  std::mutex m_lock;

  [[nodiscard]] auto NCC_FORCE_INLINE IsValidId(uint64_t id) const -> bool {
    return id < m_data.size() + m_buffered.size();
  }

  NCC_FORCE_INLINE auto GetUnchecked(uint64_t id) -> CStringView {
    assert(id < m_data.size() + m_buffered.size());

    if (id >= m_data.size()) {
      FlushBuffered();
    }

    const auto& vec = m_data[id];
    return {vec.data(), vec.size()};
  }

  void FlushBuffered() {
    for (const auto& str : m_buffered) {
      std::vector<char> vec(str.size() + 1);
      std::copy(str.begin(), str.end(), vec.begin());
      vec.back() = '\0';
      m_data.emplace_back(std::move(vec));
    }

    m_buffered.clear();
  }

public:
  FastStorage() {
    constexpr size_t kInitSize = 4096;

    m_data.reserve(kInitSize);
    m_buffered.reserve(kInitSize);

    // Insert the empty string for index 0
    m_data.emplace_back();
  }

  ~FastStorage() override { Reset(); }

  [[nodiscard]] auto Get(uint64_t id) -> CStringView override {
    ConditionalLockGuard lock(m_lock);

    if (!IsValidId(id)) [[unlikely]] {
      return {};
    }

    return GetUnchecked(id);
  }

  auto FromString(std::string_view str) -> uint64_t override {
    ConditionalLockGuard lock(m_lock);
    m_buffered.emplace_back(str);

    return m_data.size() + m_buffered.size() - 1;
  }

  auto FromString(std::string&& str) -> uint64_t override {
    ConditionalLockGuard lock(m_lock);
    m_buffered.emplace_back(std::move(str));

    return m_data.size() + m_buffered.size() - 1;
  }

  [[nodiscard]] auto CompareEq(uint64_t a, uint64_t b) -> bool override {
    ConditionalLockGuard lock(m_lock);

    if (!IsValidId(a) || !IsValidId(b)) [[unlikely]] {
      return false;
    }

    return GetUnchecked(a) == GetUnchecked(b);
  }

  [[nodiscard]] auto CompareLt(uint64_t a, uint64_t b) -> bool override {
    ConditionalLockGuard lock(m_lock);

    if (!IsValidId(a) || !IsValidId(b)) [[unlikely]] {
      return false;
    }

    return GetUnchecked(a) < GetUnchecked(b);
  }

  void Reset() override {
    ConditionalLockGuard lock(m_lock);

    m_data.clear();
    m_buffered.clear();
  }
};

#if MEMORY_OVER_SPEED == 1
static MemoryConservedStorage GlobalStorage;
#else
static FastStorage GlobalStorage;
#endif

///=============================================================================

auto String::Get() const -> CStringView {
  if (m_id == 0) {
    return {};
  }

  return GlobalStorage.Get(m_id);
}

auto String::operator==(const String& o) const -> bool {
  if (m_id == o.m_id) {
    return true;
  }

  return GlobalStorage.CompareEq(m_id, o.m_id);
}

auto String::operator<(const String& o) const -> bool {
  if (m_id == o.m_id) {
    return false;
  }

  return GlobalStorage.CompareLt(m_id, o.m_id);
}

auto StringMemory::FromString(std::string_view str) -> uint64_t {
  return GlobalStorage.FromString(str);
}

auto StringMemory::FromString(std::string&& str) -> uint64_t {
  return GlobalStorage.FromString(std::move(str));
}

void StringMemory::Reset() { GlobalStorage.Reset(); }
