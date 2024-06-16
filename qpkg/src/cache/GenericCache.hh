#ifndef __QPKG_CACHE_GENERICCACHE_HH__
#define __QPKG_CACHE_GENERICCACHE_HH__

#include <cache/blake3/blake3.h>

#include <array>
#include <chrono>
#include <set>
#include <string>
#include <vector>

namespace qpkg {
namespace cache {
const size_t CacheKeySize = 16;
typedef std::array<uint8_t, CacheKeySize> CacheKey;

enum class Format { HEX, BASE64 };

std::string keyToString(const CacheKey &key, Format format = Format::HEX);

class CacheHasher {
  blake3_hasher m_ctx;

 public:
  CacheHasher();

  inline CacheHasher &update(const std::vector<uint8_t> &data) {
    return update(data.data(), data.size());
  }
  CacheHasher &update(const uint8_t *data, size_t size);
  template <size_t T>
  CacheHasher &update(const std::array<uint8_t, T> &data) {
    update(data.data(), T);
    return *this;
  }

  CacheKey finalize();
  void reset();

  static inline CacheKey digest(const std::vector<uint8_t> &data) {
    CacheHasher digest;
    digest.update(data);
    return digest.finalize();
  }

  static inline CacheKey digest(const uint8_t *data, size_t size) {
    CacheHasher digest;
    digest.update(data, size);
    return digest.finalize();
  }

  static inline CacheKey digest(const std::string &data) {
    CacheHasher digest;
    digest.update((const uint8_t *)data.c_str(), data.size());
    return digest.finalize();
  }

  template <size_t T>
  static inline CacheKey digest(const std::array<uint8_t, T> &data) {
    CacheHasher digest;
    digest.update(data);
    return digest.finalize();
  }
};

class ICache {
 public:
  virtual ~ICache() = default;

  virtual void acquire_lock() = 0;
  virtual void release_lock() = 0;

  /**
   * @brief Setup the cache instance
   *
   * @param location The location of the cache
   * @param init Whether to initialize the cache if it doesn't exist
   * @return true if the cache was successfully setup, false otherwise
   */
  virtual bool setup(const std::string &location, bool init = false) = 0;

  /**
   * @brief Check if the cache contains a value for the given key
   *
   * @param key The key to check
   * @return true if the cache contains a value for the given key, false
   * otherwise
   */
  virtual bool contains(const CacheKey &key) = 0;

  /**
   * @brief Get a set of all keys in the cache
   *
   * @return A set of all keys in the cache
   */
  virtual std::set<CacheKey> keys() = 0;

  /**
   * @brief Load the cached value for the given key
   *
   * @param key The key to load
   * @throws std::out_of_range if the key is not in the cache
   */
  virtual void loadb(const CacheKey &key, std::vector<uint8_t> &value) = 0;

  /**
   * @brief Get a filepath that contains the raw binary data for the given key
   *
   * @param key The key to get the filepath for
   * @return The filepath that contains the raw binary data for the given key
   * @throws std::out_of_range if the key is not in the cache
   */
  virtual std::string loadf(const CacheKey &key) = 0;

  /**
   * @brief Get last modified/created timestamp of the cache entry
   *
   * @param key The key to get the timestamp for
   * @return The last modified/created timestamp of the cache entry
   * @throws std::out_of_range if the key is not in the cache
   */
  virtual std::chrono::system_clock::time_point timestamp(
      const CacheKey &key) = 0;

  /**
   * @brief Store the value for the given key in the cache
   *
   * @param key The key to store the value for
   * @param value The value to store
   */
  virtual void storeb(const CacheKey &key,
                      const std::vector<uint8_t> &value) = 0;

  /**
   * @brief Store the value for the given key in the cache
   *
   * @param key The key to store the value for
   * @param value A filepath that contains the raw binary data
   */
  virtual void storef(const CacheKey &key, const std::string &filepath) = 0;

  /**
   * @brief Remove the value for the given key from the cache
   *
   * @param key The key to remove the value for
   */
  virtual void remove(const CacheKey &key) = 0;

  /**
   * @brief Clear the cache
   */
  virtual void clear() = 0;

  /**
   * @brief Get the location of the cache
   *
   * @return The location of the cache
   */
  virtual std::string location() = 0;

  /**
   * @brief Get the number of items in the cache
   *
   * @return The number of items in the cache
   */
  virtual size_t count() = 0;

  /**
   * @brief Get the size of the cache in bytes
   *
   * @return The size of the cache
   */
  virtual size_t size() = 0;

  /**
   * @brief Sync the cache to disk
   */
  virtual void sync() = 0;
};
}  // namespace cache
}  // namespace qpkg

namespace std {
std::ostream &operator<<(std::ostream &os, const qpkg::cache::CacheKey &key);
}

#endif  // __QPKG_CACHE_GENERICCACHE_HH__
