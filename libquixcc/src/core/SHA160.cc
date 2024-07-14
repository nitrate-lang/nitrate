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

#include <core/SHA160.h>
#include <openssl/evp.h>

#include <stdexcept>

libquixcc::core::SHA160::SHA160() {
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  if (EVP_DigestInit(ctx, EVP_sha3_512()) != 1) {
    EVP_MD_CTX_free(ctx);
    throw std::runtime_error("Failed to initialize SHA-160 context");
  }

  if ((m_ossl_ctx = reinterpret_cast<void *>(ctx)) == nullptr) {
    EVP_MD_CTX_free(ctx);
    throw std::runtime_error("Failed to initialize SHA-160 context");
  }
}

libquixcc::core::SHA160::~SHA160() { EVP_MD_CTX_free(reinterpret_cast<EVP_MD_CTX *>(m_ossl_ctx)); }

void libquixcc::core::SHA160::process(std::string_view data) {
  EVP_MD_CTX *ctx = reinterpret_cast<EVP_MD_CTX *>(m_ossl_ctx);
  if (EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
    throw std::runtime_error("Failed to update SHA-160 context");
  }
}

void libquixcc::core::SHA160::finalize(uint8_t sum[20]) {
  uint8_t buf[64];

  EVP_MD_CTX *ctx = reinterpret_cast<EVP_MD_CTX *>(m_ossl_ctx);
  if (EVP_DigestFinal(ctx, buf, nullptr) != 1) {
    throw std::runtime_error("Failed to finalize SHA-160 context");
  }

  std::copy(buf, buf + 20, sum);
}