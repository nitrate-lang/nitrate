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
///   useful, but WITHOUT ANY WARRANTY; without even the fnied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <stddef.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t _nlr_alloc_impl(uint64_t size);
void _nlr_dealloc_impl(uint64_t ptr);
uint64_t _nlr_csprng_impl(uint64_t buf_ptr, uint64_t size);

///============================================================================

static void _nlr_panic_impl(uint64_t code) {
  (void)code;
  __builtin_trap();
}

static uint64_t _nlr_nmem_impl(uint64_t req_size) {
  (void)req_size;  // Nothing to do here
  return 0;
}

///============================================================================

typedef uint64_t (*nlr_alloc_fn)(uint64_t size);
typedef void (*nlr_dealloc_fn)(uint64_t ptr);
typedef uint64_t (*nlr_nmem_fn)(uint64_t req_size);
typedef void (*nlr_panic_fn)(uint64_t code);
typedef uint64_t (*nlr_csprng_fn)(uint64_t buf_ptr, uint64_t size);

extern void nlr_initialize(nlr_alloc_fn alloc, nlr_dealloc_fn dealloc,
                           nlr_nmem_fn nmem, nlr_panic_fn panic,
                           nlr_csprng_fn csprng);

extern int nlr_start(int argc, char** argv, char** envp);

void _start(int argc, char** argv, char** envp) {
  nlr_initialize(_nlr_alloc_impl, _nlr_dealloc_impl, _nlr_nmem_impl,
                 _nlr_panic_impl, _nlr_csprng_impl);

  int rc = nlr_start(argc, argv, envp);

  syscall(SYS_exit_group, rc);
}
