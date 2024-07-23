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

#include <chrono>
#include <cstdlib>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <quixcc/plugin/EngineAPI.h>
#include <quixcc/preprocessor/QSys.h>
#include <string_view>
#include <thread>
#include <time.h>
#include <unordered_map>
#include <vector>

QSYS_DEFINE(qsys_time, "Get the timestamp local or UTC") {
  QSYS_ARGASSERT(qsys_time, 1);
  QSYS_ARG_INT64(qsys_time, is_local, 0);

  if (is_local) {
    time_t t = time(0);
    struct tm *now = localtime(&t);
    char buf[80] = {0};
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", now);
    return strdup(buf);
  } else {
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::milliseconds ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return strdup(std::to_string(ms.count()).c_str());
  }
}

QSYS_DEFINE(qsys_sleep, "Sleep for a specified number of milliseconds") {
  QSYS_ARGASSERT(qsys_sleep, 1);
  QSYS_ARG_INT64(qsys_sleep, ms, 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  return strdup("");
}

QSYS_DEFINE(qsys_crypto_random, "Generate a secure random number") {
#define MAX_RANDOM_BYTES 4096

  QSYS_ARGASSERT(qsys_crypto_random, 1);
  QSYS_ARG_INT64(qsys_crypto_random, numbytes, 0);

  if (numbytes > MAX_RANDOM_BYTES) {
    quixcc_engine_message(e, QUIXCC_MESSAGE_ERROR,
                          "qsys_crypto_random: numbytes must be less than or equal to %d",
                          MAX_RANDOM_BYTES);
    return strdup("");
  }

  uint8_t buf[MAX_RANDOM_BYTES] = {0};
  if (RAND_bytes(buf, numbytes) != 1) {
    unsigned long err = ERR_get_error();
    char errbuf[256] = {0};
    ERR_error_string_n(err, errbuf, sizeof(errbuf));

    quixcc_engine_message(e, QUIXCC_MESSAGE_ERROR,
                          "qsys_crypto_random: failed to generate random bytes: %s", errbuf);
    return strdup("");
  }

  char *hexbuf = (char *)malloc(numbytes * 2 + 1);
  for (int i = 0; i < numbytes; i++) {
    sprintf(hexbuf + i * 2, "%02x", buf[i]);
  }
  hexbuf[numbytes * 2] = '\0';

  return hexbuf;
}

QSYS_DEFINE(qsys_hash, "Cryptographic hash") {
#define OSSL_ERR() (ERR_error_string_n(err = ERR_get_error(), errbuf, sizeof(errbuf)), errbuf)

  enum class HashAlgo {
    Q384,
  };

  static const std::unordered_map<std::string_view, HashAlgo> algo_map = {
      {"q384", HashAlgo::Q384},
  };

  HashAlgo algo;
  char errbuf[256] = {0};

  QSYS_ARGASSERT(qsys_hash, 2);
  QSYS_ARG_STRING(qsys_hash, data, 0); // Hex-encoded data
  QSYS_ARG_STRING(qsys_hash, algorithm, 1);
  (void)algorithm_len;

  { /* Determine algorithm */
    if (!algo_map.contains(algorithm)) {
      quixcc_engine_message(e, QUIXCC_MESSAGE_ERROR, "qsys_hash: unknown algorithm %s", algorithm);
      return strdup("");
    }

    algo = algo_map.at(algorithm);
  }

  std::vector<uint8_t> message;

  { /* Decode user message data */
    message.resize(data_len / 2);
    for (size_t i = 0; i < data_len / 2; i++) {
      sscanf(data + i * 2, "%02hhx", &message[i]);
    }
  }

  switch (algo) {
  case HashAlgo::Q384: {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned long err;

    { /* EVP INIT */
      if ((mdctx = EVP_MD_CTX_new()) == nullptr) {
        quixcc_engine_message(e, QUIXCC_MESSAGE_ERROR, "qsys_hash: failed to create MD context: %s",
                              OSSL_ERR());
        goto error_ret;
      }

      md = EVP_sha3_512();

      if (md == nullptr) {
        quixcc_engine_message(e, QUIXCC_MESSAGE_ERROR, "qsys_hash: failed to get SHA3-512: %s",
                              OSSL_ERR());
        goto error_ret;
      }

      if (EVP_DigestInit_ex(mdctx, md, nullptr) != 1) {
        quixcc_engine_message(e, QUIXCC_MESSAGE_ERROR, "qsys_hash: failed to initialize digest: %s",
                              OSSL_ERR());
        goto error_ret;
      }
    }

    { /* EVP UPDATE */
      if (EVP_DigestUpdate(mdctx, message.data(), message.size()) != 1) {
        quixcc_engine_message(e, QUIXCC_MESSAGE_ERROR, "qsys_hash: failed to update digest: %s",
                              OSSL_ERR());
        goto error_ret;
      }
    }

    { /* EVP FINAL */
      uint8_t hash[EVP_MAX_MD_SIZE];
      unsigned int hash_len;

      if (EVP_DigestFinal_ex(mdctx, hash, &hash_len) != 1) {
        quixcc_engine_message(e, QUIXCC_MESSAGE_ERROR, "qsys_hash: failed to finalize digest: %s",
                              OSSL_ERR());
        goto error_ret;
      }

      EVP_MD_CTX_free(mdctx);

      // We will fold the last 16 bytes into the first 16 bytes
      for (int i = 0; i < 16; i++) {
        hash[i] ^= hash[i + 48];
      }

      char *hexbuf = (char *)malloc(48 * 2 + 1);
      for (int i = 0; i < 48; i++) {
        sprintf(hexbuf + i * 2, "%02x", hash[i]);
      }
      hexbuf[48 * 2] = '\0';

      return hexbuf;
    }
  }
  }

error_ret:
  return strdup("");
}

QSYS_NOT_IMPLEMENTED(qsys_insmod);
QSYS_NOT_IMPLEMENTED(qsys_rmmod);

QSYS_DEFINE(qsys_ioctl, "Basically everything else that I did not want to name") {
  QSYS_ARGASSERT(qsys_ioctl, 5);
  QSYS_ARG_STRING(qsys_ioctl, feature, 0);
  QSYS_ARG_STRING(qsys_ioctl, method, 1);
  QSYS_ARG_STRING(qsys_ioctl, arg0, 2);
  QSYS_ARG_STRING(qsys_ioctl, arg1, 3);
  QSYS_ARG_STRING(qsys_ioctl, arg2, 4);

  (void)feature_len;
  (void)method_len;
  (void)arg0_len;
  (void)arg1_len;
  (void)arg2_len;

  quixcc_engine_message(e, QUIXCC_MESSAGE_INFO,
                        "qsys_ioctl: feature=%s method=%s arg0=%s arg1=%s arg2=%s", feature, method,
                        arg0, arg1, arg2);

  return strdup("");
}
