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

#include <stdint.h>

#if defined(__linux__)
#include <sys/syscall.h>
#include <unistd.h>

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define MAP_ANONYMOUS 0x20
#define MAP_PRIVATE 0x2
#define MAP_FAILED ((void*)-1)

static void* the_alloc(uint64_t size) {
  // Allocate some extra space to store the size of the region and padding
  uint8_t* map =
      (uint8_t*)syscall(SYS_mmap, NULL, size + 16, PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (map == MAP_FAILED) {
    return NULL;
  }

  // Store size of region at the front
  ((uint64_t*)map)[0] = size + 16;

  // Return offset which is useable with padding
  return map + 16;
}

static void the_dealloc(uint8_t* ptr) {
  // Subtract to get to the actual base
  ptr -= 16;

  // We stored the size in the_alloc()
  uint64_t mmap_size = ((uint64_t*)ptr)[0];

  // This should always succeed unless the inputs are invalid/corrupted
  if (syscall(SYS_munmap, ptr, mmap_size) != 0) {
    __builtin_trap();
  }
}
#else
#error "Architecture not supported by this runtime
#endif

uint64_t _nlr_alloc_impl(uint64_t size) { return (uint64_t)the_alloc(size); }
void _nlr_dealloc_impl(uint64_t ptr) {
  if (ptr != 0) {
    the_dealloc((uint8_t*)ptr);
  }
}
