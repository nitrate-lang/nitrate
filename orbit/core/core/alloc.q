///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///    ░▒▓██████▓▒░  ░▒▓███████▓▒░  ░▒▓███████▓▒░  ░▒▓█▓▒░ ░▒▓████████▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░    ░▒▓█▓▒░       ///
///   ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░    ░▒▓█▓▒░       ///
///   ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓███████▓▒░  ░▒▓███████▓▒░  ░▒▓█▓▒░    ░▒▓█▓▒░       ///
///   ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░    ░▒▓█▓▒░       ///
///   ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░    ░▒▓█▓▒░       ///
///    ░▒▓██████▓▒░  ░▒▓█▓▒░░▒▓█▓▒░ ░▒▓███████▓▒░  ░▒▓█▓▒░    ░▒▓█▓▒░       ///
///                                                                         ///
///     * ORBIT STANDARD LIBRARY - The official stdlib for QUIX.            ///
///     * Copyright (c) 2024, Wesley C. Jones.                              ///
///     * License terms may be found in the LICENSE file.                   ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

@use "v1.0"
@language "en"

// @import core::panic;

subsystem core::mem: [core::panic, 
                      ex::cstdlib::malloc, 
                      ex::cstdlib::free] {
  unsafe {
    /// @brief Allocates a region of memory.
    /// @param size The size of the region to allocate.
    /// @return A pointer to the allocated region.
    /// @SECURITY This is just an alias for C's stdlib malloc function.
    pub fn tsafe unsafe_allocate(size: usize): *u8 {
      import "C" fn malloc(size: usize): *u8;

      let ptr = malloc(size);

      if ptr == null {
        core::panic("Out of memory");
      }

      ret ptr;
    }

    /// @brief Deallocates a region of memory.
    /// @param ptr A pointer to the region to deallocate.
    /// @param size The size of the region to deallocate.
    /// @SECURITY This is just an alias for C's stdlib `free` function.
    pub fn tsafe unsafe_deallocate(ptr: *u8, size: usize) {
      import "C" fn free(ptr: *u8);

      free(ptr);
    }
  }
}
