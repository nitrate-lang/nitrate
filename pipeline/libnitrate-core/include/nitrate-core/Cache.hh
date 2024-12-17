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

#ifndef __NITRATE_CORE_CACHE_H__
#define __NITRATE_CORE_CACHE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t key[20];
} __attribute__((packed)) qcore_cache_key_t;

/**
 * @brief Cache check signature
 *
 * @param key The cache key
 *
 * @return The payload_size of the cached object in bytes, or -1 if the object
 * is not cached.
 */
typedef int64_t (*qcore_cache_has_t)(const qcore_cache_key_t *key);

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
typedef bool (*qcore_cache_read_t)(const qcore_cache_key_t *key, void *payload,
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
typedef bool (*qcore_cache_write_t)(const qcore_cache_key_t *key,
                                    const void *payload, size_t payload_size);

/**
 * @brief Bind a cache provider.
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
bool qcore_cache_bind(qcore_cache_has_t has, qcore_cache_read_t read,
                      qcore_cache_write_t write);

/**
 * @brief Unbind the current cache provider.
 *
 * @note This function is thread-safe.
 */
void qcore_cache_unbind();

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
int64_t qcore_cache_has(const qcore_cache_key_t *key);

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
bool qcore_cache_read(const qcore_cache_key_t *key, void *payload,
                      size_t payload_size);

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
bool qcore_cache_write(const qcore_cache_key_t *key, const void *payload,
                       size_t payload_size);

#endif  // __NITRATE_CORE_CACHE_H__
