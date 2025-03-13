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

#define DEFAULT_INTERNING 1

#include <mutex>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/SmartLock.hh>
#include <nitrate-core/String.hh>
#include <sparsehash/dense_hash_map>

using namespace ncc;

/** Optimized for very fast append. NonLinearContainer preserves reference integery unlike std::vector */
template <typename T, auto BufferSize>
class NonLinearContainer final {
  static_assert(BufferSize > 0);

  std::vector<std::vector<T>> m_data;
  size_t m_back_size = 0;

  void AddChunk() { m_data.emplace_back().resize(BufferSize); }

public:
  NonLinearContainer() {
    m_data.reserve(BufferSize);
    AddChunk();
  }

  auto Append(T&& value) -> T& {
    if (m_back_size != BufferSize) [[likely]] {
      return m_data.back()[m_back_size++] = std::move(value);
    }

    AddChunk();
    m_back_size = 1;
    return m_data.back().front() = std::move(value);
  }

  void Clear() {
    m_data.clear();
    m_data.shrink_to_fit();

    AddChunk();
    m_back_size = 0;
  }
};

#if MEMORY_OVER_SPEED == 1 || DEFAULT_INTERNING == 1
#define __IS_INTERNING
#endif

static std::mutex StringLock;
static NonLinearContainer<std::string, 1024> Strings;

#ifdef __IS_INTERNING
static google::dense_hash_map<std::string_view, const std::string*> FastMap = [] {
  google::dense_hash_map<std::string_view, const std::string*> map;
  map.set_empty_key("");
  return map;
}();
#endif

std::string ncc::string::DefaultEmptyString;

auto String::CreateInstance(std::string_view str) -> const std::string& {
  if (str.empty()) {
    return DefaultEmptyString;
  }

  SmartLock lock(StringLock);

#ifdef __IS_INTERNING
  /* Search for existing key, thereby deduplicating elements */
  if (auto it = FastMap.find(str); it != FastMap.end()) {
    return *it->second;
  }
#endif

  const std::string* ptr = &Strings.Append(std::string(str));

#ifdef __IS_INTERNING
  FastMap[std::string_view(*ptr)] = ptr;
#endif

  qcore_assert(ptr != nullptr);

  return *ptr;
}

auto String::CreateInstance(std::string&& str) -> const std::string& {
  if (str.empty()) {
    return DefaultEmptyString;
  }

  SmartLock lock(StringLock);

#ifdef __IS_INTERNING
  /* Search for existing key, thereby deduplicating elements */
  if (auto it = FastMap.find(str); it != FastMap.end()) {
    return *it->second;
  }
#endif

  const std::string* ptr = &Strings.Append(std::move(str));

#ifdef __IS_INTERNING
  FastMap[std::string_view(*ptr)] = ptr;
#endif

  qcore_assert(ptr != nullptr);

  return *ptr;
}

void String::ResetInstances() {
  SmartLock lock(StringLock);

  Strings.Clear();

#ifdef __IS_INTERNING
  FastMap.clear();
#endif
}
