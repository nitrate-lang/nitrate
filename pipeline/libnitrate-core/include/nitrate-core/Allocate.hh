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

#ifndef __NITRATE_CORE_MEMORY_H__
#define __NITRATE_CORE_MEMORY_H__

#include <cstddef>
#include <utility>

namespace ncc::core {
  class IMemory {
  public:
    virtual ~IMemory() = default;

    virtual void *alloc(size_t size, size_t align = DEFAULT_ALIGNMENT) = 0;

    static constexpr size_t DEFAULT_ALIGNMENT = 16;
  };

  class dyn_arena final : public IMemory {
    class PImpl;

    PImpl *m_arena;
    bool m_owned;

    dyn_arena(const dyn_arena &) = delete;

  public:
    dyn_arena();

    virtual ~dyn_arena() override;

    dyn_arena(dyn_arena &&o) {
      m_arena = std::move(o.m_arena);
      o.m_owned = false;
      m_owned = true;
    }

    dyn_arena &operator=(dyn_arena &&o) {
      m_arena = std::move(o.m_arena);
      o.m_owned = false;
      m_owned = true;
      return *this;
    }

    void *alloc(size_t size, size_t align = DEFAULT_ALIGNMENT) override;
  };
}  // namespace ncc::core

#endif
