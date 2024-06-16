#include <cache/GenericCache.hh>
#include <core/Logger.hh>
#include <iomanip>
#include <sstream>

static std::string base64_encode(const std::string &in) {
  std::string out;

  int val = 0, valb = -6;
  for (unsigned char c : in) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      out.push_back(
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
              [(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6)
    out.push_back(
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
            [((val << 8) >> (valb + 8)) & 0x3F]);
  while (out.size() % 4) out.push_back('=');
  return out;
}

std::string qpkg::cache::keyToString(const qpkg::cache::CacheKey &key,
                                     qpkg::cache::Format format) {
  std::stringstream ss;

  switch (format) {
    case Format::HEX:
      for (const auto &byte : key)
        ss << std::hex << std::setfill('0') << std::setw(2) << (int)byte;
      return ss.str();
    case Format::BASE64:
      return base64_encode(std::string(key.begin(), key.end()));
    default:
      LOG(core::FATAL) << "Invalid format" << std::endl;
  }
}

qpkg::cache::CacheHasher::CacheHasher() { blake3_hasher_init(&m_ctx); }

qpkg::cache::CacheHasher &qpkg::cache::CacheHasher::update(const uint8_t *data,
                                                           size_t size) {
  blake3_hasher_update(&m_ctx, data, size);

  return *this;
}

qpkg::cache::CacheKey qpkg::cache::CacheHasher::finalize() {
  CacheKey key;
  blake3_hasher_finalize(&m_ctx, key.data(), CacheKeySize);

  return key;
}

void qpkg::cache::CacheHasher::reset() { blake3_hasher_init(&m_ctx); }

std::ostream &std::operator<<(std::ostream &os,
                              const qpkg::cache::CacheKey &key) {
  os << qpkg::cache::keyToString(key);
  return os;
}