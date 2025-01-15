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
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/String.hh>
#include <sparsehash/dense_hash_map>

using namespace ncc;

static NCC_FORCE_INLINE constexpr std::vector<char> FromStr(
    std::string_view str) {
  std::vector<char> vec(str.size());
  std::copy(str.begin(), str.end(), vec.begin());
  return vec;
}

static NCC_FORCE_INLINE constexpr auto FromVec(const std::vector<char>& vec) {
  return std::string_view(vec.data(), vec.size());
}

struct Storage {
  std::vector<std::vector<char>> m_data;
  google::dense_hash_map<std::string_view, uint64_t> m_map;

  Storage() {
    constexpr size_t kInitSize = 4096;

    m_map.set_empty_key("");
    m_data.reserve(kInitSize);
  }
};

static Storage GLobalStorage;
static std::mutex GLobalStorageLock;

NCC_EXPORT std::string_view String::Get() const {
  if (m_id == 0) {
    return "";
  }

  bool sync = EnableSync;
  if (sync) {
    GLobalStorageLock.lock();
  }

  std::string_view str;

  if (m_id < GLobalStorage.m_data.size()) [[likely]] {
    str = FromVec(GLobalStorage.m_data[m_id]);
  } else {
    str = "";
  }

  if (sync) {
    GLobalStorageLock.unlock();
  }

  return str;
}

NCC_EXPORT uint64_t StringMemory::FromString(std::string_view str) {
  assert(!str.empty());

  bool sync = EnableSync;

  if (sync) {
    GLobalStorageLock.lock();
  }

  uint64_t id;
  if (auto it = GLobalStorage.m_map.find(str);
      it != GLobalStorage.m_map.end()) {
    id = it->second;
  } else {
    id = GLobalStorage.m_data.size();
    GLobalStorage.m_data.emplace_back(FromStr(str));
    GLobalStorage.m_map[FromVec(GLobalStorage.m_data.back())] = id;
  }

  if (sync) {
    GLobalStorageLock.unlock();
  }

  return id;
}

NCC_EXPORT uint64_t StringMemory::FromString(std::string&& str) {
  assert(!str.empty());

  bool sync = EnableSync;

  if (sync) {
    GLobalStorageLock.lock();
  }

  uint64_t id;
  if (auto it = GLobalStorage.m_map.find(str);
      it != GLobalStorage.m_map.end()) {
    id = it->second;
  } else {
    id = GLobalStorage.m_data.size();
    GLobalStorage.m_data.emplace_back(FromStr(std::move(str)));
    GLobalStorage.m_map[FromVec(GLobalStorage.m_data.back())] = id;
  }

  if (sync) {
    GLobalStorageLock.unlock();
  }

  return id;
}

NCC_EXPORT void StringMemory::Reset() {
  bool sync = EnableSync;

  if (sync) {
    GLobalStorageLock.lock();
  }

  GLobalStorage.m_map.clear();
  GLobalStorage.m_data.clear();

  if (sync) {
    GLobalStorageLock.unlock();
  }
}
