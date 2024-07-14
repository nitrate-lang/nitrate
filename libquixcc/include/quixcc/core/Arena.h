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
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_CORE_ARENA_H__
#define __QUIXCC_CORE_ARENA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

struct quixcc_arena_impl_t;

/**
 * @brief Quixcc general-purpose arena allocator.
 */
typedef struct quixcc_arena_t {
  struct quixcc_arena_impl_t *m_impl;
} quixcc_arena_t;

/**
 * @brief Initializes an arena allocator.
 *
 * @param arena The arena to initialize.
 *
 * @return The initialized arena.
 *
 * @note If the arena is currently open, it will be reset and all of its memory
 * will be LEAKED.
 * @note This function is thread-safe.
 */
quixcc_arena_t *quixcc_arena_open(quixcc_arena_t *arena);

/**
 * @brief Copy an arena allocator.
 *
 * @param dst The arena to copy to.
 * @param src The arena to copy from.
 *
 * @return The copied arena.
 *
 * @warning This is a shallow byte-by-byte copy.
 */
quixcc_arena_t *quixcc_arena_copy(quixcc_arena_t *dst, const quixcc_arena_t *src);

/**
 * @brief Allocates memory from the arena.
 *
 * @param arena The arena to allocate memory from.
 * @param size The size of the memory to allocate.
 *
 * @return A pointer to the allocated memory.
 *
 * @note If the arena is not valid the behavior is undefined.
 * @note The arena is resized automatically when needed.
 * @note Out-of-memory errors will call `quixcc_panic`.
 * @note The pointer is never NULL.
 * @note The returned memory is uninitialized.
 * @warning DO NOT free the returned memory manually.
 * @note This function is thread-safe.
 *
 * @warning All pointers are owned by the arena and will be freed
 * when the arena is closed.
 */
void *quixcc_arena_alloc(quixcc_arena_t *arena, size_t size);

/**
 * @brief Allocates memory from the arena with a specific alignment.
 *
 * @param arena The arena to allocate memory from.
 * @param size The size of the memory to allocate.
 * @param align The alignment of the memory to allocate.
 *
 * @return A pointer to the allocated memory.
 *
 * @note If the arena is not valid the behavior is undefined.
 * @note The arena is resized automatically when needed.
 * @note Out-of-memory errors will call `quixcc_panic`.
 * @note The pointer is never NULL.
 * @note The returned memory is uninitialized.
 * @warning DO NOT free the returned memory manually.
 * @note This function is thread-safe.
 *
 * @warning All pointers are owned by the arena and will be freed
 * when the arena is closed.
 */
void *quixcc_arena_alloc_ex(quixcc_arena_t *arena, size_t size, size_t align);

/**
 * @brief Returns the total size of the arena.
 *
 * @param arena The arena to get the size of.
 *
 * @return The total size of the arena.
 *
 * @note If the arena is not valid the behavior is undefined.
 * @note This function is thread-safe.
 */
size_t quixcc_arena_capacity(quixcc_arena_t *arena);

/**
 * @brief Returns the amount of memory used in the arena.
 *
 * @param arena The arena to get the used memory of.
 *
 * @return The amount of memory used in the arena.
 *
 * @note If the arena is not valid the behavior is undefined.
 * @note This function is thread-safe.
 */
size_t quixcc_arena_used(quixcc_arena_t *arena);

/**
 * @brief Closes an arena allocator.
 *
 * @param arena The arena to close.
 *
 * @note If the arena is already closed, this is a no-op.
 * @note Calling this function on an invalid arena is undefined behavior.
 * @note This will free all memory allocated from the arena.
 * @note This function is thread-safe.
 */
void quixcc_arena_close(quixcc_arena_t *arena);

#ifdef __cplusplus
}
#endif

#endif // __QUIXCC_CORE_ARENA_H__
