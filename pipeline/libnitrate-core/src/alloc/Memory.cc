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

#define QCORE_NDEBUG

#include <nitrate-core/Macro.h>

#include <alloc/Collection.hh>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Logger.hh>

CPP_EXPORT qcore_arena_t *qcore_arena_open_ex(qcore_arena_t *A,
                                              qcore_alloc_mode_t mode,
                                              bool is_thread_safe) {
  qcore_assert(A != nullptr, "qcore_arena_open_ex: invalid arena");

  mem::qcore_arena_t *X;

  switch (mode) {
    case QCORE_GBA_V0:
      X = new mem::gba_v0_t();
      break;
    case QCORE_RIBA_V0:
      X = new mem::riba_v0_t();
      break;
    case QCORE_AUTO: {
#if MEMORY_OVER_SPEED == 1
      X = new mem::riba_v0_t();
#else
      X = new mem::gba_v0_t();
#endif
      break;
    }

    default: {
      qcore_panicf("qcore_arena_open_ex: invalid mode %d", mode);
    }
  }

  X->open(is_thread_safe);

  *A = reinterpret_cast<qcore_arena_t>(X);

  qcore_debugf("TRACE: qcore_arena_open_ex(%p, %d, %d)\t-> %p\n", A, mode,
               is_thread_safe, X);

  return A;
}

CPP_EXPORT void *qcore_arena_alloc_ex(qcore_arena_t *A, size_t size,
                                      size_t align) {
  void *ptr;

  qcore_assert(A != nullptr, "qcore_arena_alloc_ex: invalid arena");

  ptr = reinterpret_cast<mem::qcore_arena_t *>(*A)->alloc(size, align);

  qcore_debugf("TRACE: qcore_arena_alloc_ex(%p, %zu, %zu)\t-> %p\n", A, size,
               align, ptr);

  return ptr;
}

CPP_EXPORT void qcore_arena_close(qcore_arena_t *A) {
  qcore_assert(A != nullptr, "qcore_arena_close: invalid arena");

  mem::qcore_arena_t *X = reinterpret_cast<mem::qcore_arena_t *>(*A);
  size_t total_used = X->close();
  (void)total_used;

  qcore_debugf("TRACE: qcore_arena_close(%p)\t-> %zu\n", A, total_used);

  delete X;
}

///=============================================================================

ncc::core::dyn_arena::dyn_arena() {}

ncc::core::dyn_arena::~dyn_arena() {}

void *ncc::core::dyn_arena::alloc(size_t size, size_t align) {
  return qcore_arena_alloc_ex(m_arena.get(), size, align);
}
