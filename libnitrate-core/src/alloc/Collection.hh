////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
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

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

namespace mem {
  class qcore_arena_t {
  public:
    virtual ~qcore_arena_t() = default;
    virtual void open(bool thread_safe) = 0;
    virtual void *alloc(size_t size, size_t align) = 0;
    virtual size_t close() = 0;
  };

  class gba_v0_t final : public qcore_arena_t {
    struct region_t {
      uintptr_t base = 0;
      uintptr_t offset = 0;
      size_t size = 0;
    };
    std::vector<region_t> m_bases;
    std::mutex m_mutex;
    bool m_thread_safe;

    void alloc_region(size_t size) {
      uintptr_t base = (uintptr_t) new uint8_t[size];
      m_bases.push_back({base, base, size});
    }

  public:
    virtual ~gba_v0_t() = default;
    void open(bool thread_safe) override;
    void *alloc(size_t size, size_t align) override;
    size_t close() override;
  };

  class riba_v0_t final : public qcore_arena_t {
    struct region_t {
      uintptr_t base = 0;
      uintptr_t offset = 0;
      size_t size = 0;
    };
    std::vector<region_t> m_bases;
    std::mutex m_mutex;
    bool m_thread_safe;

    void alloc_region(size_t size) {
      uintptr_t base = (uintptr_t) new uint8_t[size];
      m_bases.push_back({base, base, size});
    }

  public:
    virtual ~riba_v0_t() = default;
    void open(bool thread_safe) override;
    void *alloc(size_t size, size_t align) override;
    size_t close() override;
  };
}  // namespace mem
