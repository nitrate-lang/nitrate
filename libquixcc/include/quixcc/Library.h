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

#ifndef __QUIXCC_LIBRARY_H__
#define __QUIXCC_LIBRARY_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <quixcc/Compile.h>
#include <quixcc/interface/All.h>
#include <quixcc/plugin/All.h>

///=============================================================================
/// BEGIN: LIBRARY RESOURCE MANAGEMENT
///=============================================================================

/**
 * @brief Initialize the QUIX compiler library.
 *
 * Initializes the QUIX compiler library. This function must be called
 * before any other APIs in the library are called.
 *
 * @return true if the library was initialized successfully.
 * @note This function is thread-safe.
 * @note Initialization more than once is a no-op.
 */
bool quixcc_lib_init();

/**
 * @brief Dispose of the QUIX compiler library.
 *
 * Will dispose of the QUIX compiler library and free all resources.
 * This function must be called prior to unloading the library.
 * This function will invalidate all jobs and cache entries.
 *
 * @return true if the library was disposed successfully. If false, the
 * library is not safe to unload.
 * @note This function is thread-safe.
 * @note Disposal more than once is a no-op.
 */
bool quixcc_lib_deinit();

/**
 * @brief Get the version of the QUIX compiler library.
 *
 * @return The version string of the QUIX compiler library.
 * @warning Don't free the returned string.
 * @note This function is thread-safe.
 * @note This function is safe to call irrespective of the library state dictated
 * by quixcc_lib_init and quixcc_lib_deinit.
 */
const char *quixcc_lib_version();

/**
 * @brief Check if the QUIX compiler library supports a specific language version.
 *
 * @param major The major version number.
 * @param minor The minor version number.
 *
 * @return true if the library supports the version, false otherwise.
 * @note This function is thread-safe.
 */
bool quixcc_has_version(uint32_t major, uint32_t minor);

/**
 * @brief Reset and free the internal cache memory
 *
 * @brief This function is thread-safe.
 * @return true if the cache was reset successfully. false otherwise.
 *
 * @note This function requires all jobs to be disposed before calling.
 * @warning Although this will decrease memory usage, it may also
 *          decrease pipeline performance significantly.
 * @note This function will return false if any jobs are still active.
 */
bool quixcc_cache_reset();

typedef struct {
  uint8_t key[20];
} __attribute__((packed)) quixcc_cache_key_t;

/**
 * @brief Cache check signature
 *
 * @param key The cache key
 *
 * @return The payload_size of the cached object in bytes, or -1 if the object
 * is not cached.
 */
typedef ssize_t (*quixcc_cache_has_t)(const quixcc_cache_key_t *key);

/**
 * @brief Cache read signature
 *
 * @param key The cache key
 * @param payload The buffer to read the cached object into
 * @param payload_size The maximum number of bytes to read
 *
 * @note The data consumer is not required to read the entire object.
 * @note The provider is required to be able to provide the entire object
 *       (if sufficient space is provided) in a single call.
 * @note Data integrity is the responsibility of the cache provider.
 *
 * @return true if the object was read successfully, false otherwise.
 */
typedef bool (*quixcc_cache_read_t)(const quixcc_cache_key_t *key, void *payload,
                                    size_t payload_size);

/**
 * @brief Cache write signature
 *
 * @param key The cache key
 * @param payload The data to write to the cache
 * @param payload_size The length of the data to write
 *
 * @note The writer must write the entire object in a single call.
 * @note Data integrity is the responsibility of the cache provider.
 *
 * @return true if the object was written successfully, false otherwise.
 */
typedef bool (*quixcc_cache_write_t)(const quixcc_cache_key_t *key, const void *payload,
                                     size_t payload_size);

/**
 * @brief Bind a cache provider to the QUIX compiler library.
 *
 * @param has The cache check function.
 * @param read The cache read function.
 * @param write The cache write function.
 *
 * @return true if the provider was bound successfully. false otherwise.
 *
 * @note This function is thread-safe.
 * @note This function will return false if a provider is already bound.
 */
bool quixcc_cache_bind(quixcc_cache_has_t has, quixcc_cache_read_t read,
                       quixcc_cache_write_t write);

/**
 * @brief Unbind the current cache provider from the QUIX compiler library.
 *
 * @note This function is thread-safe.
 */
void quixcc_cache_unbind();

/**
 * @brief Check if an object is currently cached.
 *
 * @param key The cache key.
 *
 * @return The payload_size of the cached object in bytes, or -1 if the object
 * is not cached.
 *
 * @note This function is thread-safe.
 * @warning This function does not guarantee that the object will be available
 *          when read. Ensure to protect against race conditions.
 */
ssize_t quixcc_cache_has(const quixcc_cache_key_t *key);

/**
 * @brief Read a cached object into a buffer.
 *
 * @param key The cache key.
 * @param payload The buffer to read the cached object into.
 * @param payload_size The maximum number of bytes to read.
 *
 * @return true if the object was read successfully, false otherwise.
 *
 * @note This function is thread-safe.
 * @note The data consumer is not required to read the entire object.
 * @note The provider is required to be able to provide the entire object
 *       (if sufficient space is provided) in a single call.
 * @note Data integrity is the responsibility of the cache provider.
 */
bool quixcc_cache_read(const quixcc_cache_key_t *key, void *payload, size_t payload_size);

/**
 * @brief Write an object to the cache.
 *
 * @param key The cache key.
 * @param payload The data to write to the cache.
 * @param payload_size The length of the data to write.
 *
 * @return true if the object was written successfully, false otherwise.
 *
 * @note This function is thread-safe.
 * @note The writer must write the entire object in a single call.
 * @note Data integrity is the responsibility of the cache provider.
 */
bool quixcc_cache_write(const quixcc_cache_key_t *key, const void *payload, size_t payload_size);

///=============================================================================
/// END: LIBRARY RESOURCE MANAGEMENT
///=============================================================================

#ifdef __cplusplus
}
#endif

#endif  // __QUIXCC_LIBRARY_H__
