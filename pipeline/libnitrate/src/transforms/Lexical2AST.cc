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

#include "nitrate-core/Error.h"
#define LIBNITRATE_INTERNAL
#include <nitrate-core/Lib.h>
#include <nitrate/code.h>

#include <core/SerialUtil.hh>
#include <core/Transformer.hh>
#include <cstdint>
#include <functional>
#include <nitrate-core/Classes.hh>
#include <nitrate-lexer/Base.hh>
#include <nitrate-lexer/Classes.hh>
#include <nitrate-parser/Classes.hh>
#include <nitrate-parser/Writer.hh>
#include <nlohmann/json.hpp>
#include <string_view>
#include <unordered_set>

using json = nlohmann::json;

static inline qlex_tok_t err_tok() {
  qlex_tok_t tok{};
  tok.ty = qErro;
  return tok;
}

static inline qlex_tok_t eof_tok() {
  qlex_tok_t tok{};
  tok.ty = qEofF;
  return tok;
}

class DeserializerAdapterLexer final : public qlex_t {
  static constexpr std::array<uint8_t, 256> valid_ty_id_tab = []() {
    std::array<uint8_t, 256> tab = {};
    tab.fill(0);

    tab[qEofF] = 1;
    tab[qErro] = 1;
    tab[qKeyW] = 1;
    tab[qOper] = 1;
    tab[qPunc] = 1;
    tab[qName] = 1;
    tab[qIntL] = 1;
    tab[qNumL] = 1;
    tab[qText] = 1;
    tab[qChar] = 1;
    tab[qMacB] = 1;
    tab[qMacr] = 1;
    tab[qNote] = 1;

    return tab;
  }();

  enum class InMode {
    JSON,
    MsgPack,
    BadCodec,
  } m_mode;

  uint64_t m_ele_count;
  bool m_eof_bit;

  qlex_tok_t next_impl_json() {
    if (m_eof_bit) [[unlikely]] {
      return eof_tok();
    }

    uint32_t ty, a, b, c, d;
    char *str = nullptr;

    { /* Read the token array */
      size_t str_len = 0;

      if (m_file.get() != '[') return err_tok();
      m_file >> ty;
      ty &= 0xff;
      if (m_file.get() != ',') return err_tok();

      if (!read_json_string(m_file, &str, str_len)) [[unlikely]] {
        return err_tok();
      }

      if (m_file.get() != ',') return err_tok();
      m_file >> a;
      if (m_file.get() != ',') return err_tok();
      m_file >> b;
      if (m_file.get() != ',') return err_tok();
      m_file >> c;
      if (m_file.get() != ',') return err_tok();
      m_file >> d;
      if (m_file.get() != ']') return err_tok();
    }

    { /* Check the delimiter */
      char delim = m_file.get();

      if (delim == ']') [[unlikely]] {
        m_eof_bit = true;
        free(str);
        return eof_tok();
      } else if (delim != ',') [[unlikely]] {
        free(str);
        return err_tok();
      }
    }

    /* Validate the token type */
    if (valid_ty_id_tab[ty]) [[likely]] {
      qlex_tok_t T;

      qlex_tok_fromstr(this, static_cast<qlex_ty_t>(ty), str, &T);

      T.start = save_loc(a, b, 0);
      T.end = save_loc(c, d, 0);

      free(str);
      return T;
    }

    free(str);
    return err_tok();
  }

  qlex_tok_t next_impl_msgpack() {
    if (m_eof_bit || !m_ele_count) [[unlikely]] {
      return eof_tok();
    }

    uint64_t ty, a, b, c, d;
    char *str = nullptr;

    { /* Read the token array */
      // Array start byte for 6 elements
      if (m_file.get() != 0x96) {
        return err_tok();
      }

      size_t str_len;

      if (!msgpack_read_uint(m_file, ty)) [[unlikely]] {
        return err_tok();
      }
      ty &= 0xff;

      if (!msgpack_read_str(m_file, &str, str_len)) [[unlikely]] {
        return err_tok();
      }

      if (!msgpack_read_uint(m_file, a) || !msgpack_read_uint(m_file, b) ||
          !msgpack_read_uint(m_file, c) || !msgpack_read_uint(m_file, d))
          [[unlikely]] {
        free(str);
        return err_tok();
      }
    }

    m_ele_count--;

    /* Validate the token type */
    if (valid_ty_id_tab[ty]) [[likely]] {
      qlex_tok_t T;

      qlex_tok_fromstr(this, static_cast<qlex_ty_t>(ty), str, &T);

      T.start = save_loc(a, b, 0);
      T.end = save_loc(c, d, 0);

      free(str);
      return T;
    }

    free(str);
    return err_tok();
  }

  virtual qlex_tok_t next_impl() override {
    switch (m_mode) {
      case InMode::JSON: {
        return next_impl_json();
      }
      case InMode::MsgPack: {
        return next_impl_msgpack();
      }
      case InMode::BadCodec: {
        return eof_tok();
      }
    }
  }

public:
  DeserializerAdapterLexer(std::istream &file, const char *filename,
                           qcore_env_t env)
      : qlex_t(file, filename, env) {
    int ch = file.get();

    m_mode = InMode::BadCodec;
    m_eof_bit = false;

    if (ch == EOF) {
      return;
    } else if (ch == '[') {
      m_mode = InMode::JSON;
      return;
    } else if (ch == 0xdd) {
      m_ele_count = 0;

      if ((ch = file.get()) == EOF) return;
      m_ele_count |= ch << 24;
      if ((ch = file.get()) == EOF) return;
      m_ele_count |= ch << 16;
      if ((ch = file.get()) == EOF) return;
      m_ele_count |= ch << 8;
      if ((ch = file.get()) == EOF) return;
      m_ele_count |= ch;

      m_mode = InMode::MsgPack;
    } else if (ch == 0xdc) {
      m_ele_count = 0;

      if ((ch = file.get()) == EOF) return;
      m_ele_count |= ch << 8;
      if ((ch = file.get()) == EOF) return;
      m_ele_count |= ch;

      m_mode = InMode::MsgPack;
    } else if ((ch & 0b10010000) == 0b10010000) {
      m_mode = InMode::MsgPack;
      m_ele_count = ch & 0b00001111;
    }
  }

  virtual ~DeserializerAdapterLexer() override = default;
};

static std::optional<npar_node_t *> parse_tokens(
    npar_t *L, std::function<void(const char *)> diag_cb) {
  npar_node_t *root = nullptr;
  bool ok = npar_do(L, &root);

  ///============================================================================///
  /// Some dangerous code here, be careful! ///
  npar_dumps(
      L, false,
      [](const char *msg, size_t, uintptr_t dat) {
        std::function<void(const char *)> &stack_tmp =
            *(std::function<void(const char *)> *)dat;
        stack_tmp(msg);
      },
      (uintptr_t)&diag_cb);
  ///============================================================================///

  if (!ok) {
    return std::nullopt;
  }

  return root;
}

bool nit::parser(std::istream &source, std::ostream &output,
                 std::function<void(const char *)> diag_cb,
                 const std::unordered_set<std::string_view> &opts) {
  enum class OutMode {
    JSON,
    MsgPack,
  } out_mode = OutMode::JSON;

  if (opts.contains("-fuse-json") && opts.contains("-fuse-msgpack")) {
    qcore_print(QCORE_ERROR, "Cannot use both JSON and MsgPack output.");
    return false;
  }

  if (opts.contains("-fuse-msgpack")) {
    out_mode = OutMode::MsgPack;
  }

  DeserializerAdapterLexer lex(source, nullptr, qcore_env_current());
  nr_syn par(&lex, qcore_env_current());

  if (let root = parse_tokens(par.get(), diag_cb)) {
    switch (out_mode) {
      case OutMode::JSON: {
        auto writter = npar::AST_JsonWriter(output);
        root.value()->accept(writter);
        return true;
      }
      case OutMode::MsgPack: {
        auto writter = npar::AST_MsgPackWriter(output);
        root.value()->accept(writter);
        return true;
      }
      default: {
        return false;
      }
    }
  } else {
    return false;
  }
}