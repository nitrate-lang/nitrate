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

#define LIBQUIX_INTERNAL

#include <quix-core/Lib.h>
#include <quix/code.h>

#include <SerialUtil.hh>
#include <cstdint>
#include <functional>
#include <quix-core/Classes.hh>
#include <quix-lexer/Base.hh>
#include <quix-lexer/Classes.hh>
#include <quix-parser/Classes.hh>
#include <string_view>
#include <unordered_set>

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
      return {.ty = qEofF};
    }

    uint32_t ty, a, b, c, d;
    char *str = nullptr;

    { /* Read the token array */
      size_t str_len;
      if (fscanf(m_file, "[%u,", &ty) != 1) [[unlikely]] {
        return {.ty = qErro};
      }

      ty &= 0xff;

      if (!read_json_string(m_file, &str, str_len)) [[unlikely]] {
        return {.ty = qErro};
      }

      if (fscanf(m_file, ",%u,%u,%u,%u]", &a, &b, &c, &d) != 4) [[unlikely]] {
        free(str);
        return {.ty = qErro};
      }
    }

    { /* Check the delimiter */
      char delim = fgetc(m_file);

      if (delim == ']') [[unlikely]] {
        m_eof_bit = true;
        free(str);
        return {.ty = qEofF};
      } else if (delim != ',') [[unlikely]] {
        free(str);
        return {.ty = qErro};
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
    return {.ty = qErro};
  }

  qlex_tok_t next_impl_msgpack() {
    if (m_eof_bit || !m_ele_count) [[unlikely]] {
      return {.ty = qEofF};
    }

    uint64_t ty, a, b, c, d;
    char *str = nullptr;

    { /* Read the token array */
      // Array start byte for 6 elements
      if (fgetc(m_file) != 0x96) {
        return {.ty = qErro};
      }

      size_t str_len;

      if (!msgpack_read_uint(m_file, ty)) [[unlikely]] {
        return {.ty = qErro};
      }
      ty &= 0xff;

      if (!msgpack_read_str(m_file, &str, str_len)) [[unlikely]] {
        return {.ty = qErro};
      }

      if (!msgpack_read_uint(m_file, a) || !msgpack_read_uint(m_file, b) ||
          !msgpack_read_uint(m_file, c) || !msgpack_read_uint(m_file, d)) [[unlikely]] {
        free(str);
        return {.ty = qErro};
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
    return {.ty = qErro};
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
        return {.ty = qErro};
      }
    }
  }

public:
  DeserializerAdapterLexer(FILE *file, const char *filename, qcore_env_t env)
      : qlex_t(file, filename, false, env) {
    int ch = fgetc(file);

    m_mode = InMode::BadCodec;
    m_eof_bit = false;

    if (ch == '[') {
      m_mode = InMode::JSON;
      return;
    } else if (ch == 0xdd) {
      m_ele_count = 0;

      if ((ch = fgetc(file)) == EOF) return;
      m_ele_count |= ch << 24;
      if ((ch = fgetc(file)) == EOF) return;
      m_ele_count |= ch << 16;
      if ((ch = fgetc(file)) == EOF) return;
      m_ele_count |= ch << 8;
      if ((ch = fgetc(file)) == EOF) return;
      m_ele_count |= ch;

      m_mode = InMode::MsgPack;
    } else if (ch == 0xdc) {
      m_ele_count = 0;

      if ((ch = fgetc(file)) == EOF) return;
      m_ele_count |= ch << 8;
      if ((ch = fgetc(file)) == EOF) return;
      m_ele_count |= ch;

      m_mode = InMode::MsgPack;
    } else if ((ch & 0b10010000) == 0b10010000) {
      m_mode = InMode::MsgPack;
      m_ele_count = ch & 0b00001111;
    }
  }

  virtual ~DeserializerAdapterLexer() override = default;
};

static std::optional<qparse_node_t *> parse_tokens(qparse_t *L,
                                                   std::function<void(const char *)> diag_cb) {
  qparse_node_t *root = nullptr;
  bool ok = qparse_do(L, &root);

  ///============================================================================///
  /// Some dangerous code here, be careful!                                      ///
  qparse_dumps(
      L, false,
      [](const char *msg, size_t, uintptr_t dat) {
        std::function<void(const char *)> &stack_tmp = *(std::function<void(const char *)> *)dat;
        stack_tmp(msg);
      },
      (uintptr_t)&diag_cb);
  ///============================================================================///

  if (!ok) {
    return std::nullopt;
  }

  return root;
}

static bool impl_use_json(qparse_node_t *R, FILE *O) {
  /// TODO: Do correct JSON serialization
  size_t buf_sz = 0;

  // Buf is allocated in the arena
  char *buf = qparse_repr(R, false, 2, &buf_sz);
  if (!buf) {
    return false;
  }

  fwrite(buf, 1, buf_sz, O);

  return true;
}

static bool impl_use_msgpack(qparse_node_t *R, FILE *O) {
  /// TODO: Do correct MsgPack serialization
  return impl_use_json(R, O);
}

bool impl_subsys_parser(FILE *source, FILE *output, std::function<void(const char *)> diag_cb,
                        const std::unordered_set<std::string_view> &opts) {
  enum class OutMode {
    JSON,
    MsgPack,
  } out_mode = OutMode::JSON;

  if (opts.contains("-fuse-json") && opts.contains("-fuse-msgpack")) {
    qcore_print(QCORE_ERROR, "Cannot use both JSON and MsgPack output.");
    return false;
  } else if (opts.contains("-fuse-msgpack")) {
    out_mode = OutMode::MsgPack;
  }

  qparse_conf conf;

  { /* Should the parser use the crashguard signal handler? */
    if (opts.contains("-fparse-crashguard=off")) {
      qparse_conf_setopt(conf.get(), QPK_CRASHGUARD, QPV_OFF);
    } else if (opts.contains("-fparse-crashguard=on")) {
      qparse_conf_setopt(conf.get(), QPK_CRASHGUARD, QPV_ON);
    }
  }

  { /* Should we implement the default construct attributes? */
    if (opts.contains("-fparse-autoimpl-func=off")) {
      qparse_conf_setopt(conf.get(), QPK_NO_AUTO_IMPL, QPV_FUNCTION);
    } else if (opts.contains("-fparse-autoimpl-func=on")) {
      // ignore; its the default
    }

    if (opts.contains("-fparse-autoimpl-struct=off")) {
      qparse_conf_setopt(conf.get(), QPK_NO_AUTO_IMPL, QPV_STRUCT);
    } else if (opts.contains("-fparse-autoimpl-struct=on")) {
      // ignore; its the default
    }

    if (opts.contains("-fparse-autoimpl-union=off")) {
      qparse_conf_setopt(conf.get(), QPK_NO_AUTO_IMPL, QPV_UNION);
    } else if (opts.contains("-fparse-autoimpl-union=on")) {
      // ignore; its the default
    }
  }

  DeserializerAdapterLexer lex(source, nullptr, qcore_env_current());
  qparser par(&lex, conf.get(), qcore_env_current());

  auto root = parse_tokens(par.get(), diag_cb);
  if (!root.has_value()) {
    return false;
  }

  bool ok = false;

  switch (out_mode) {
    case OutMode::JSON:
      ok = impl_use_json(root.value(), output);
      break;
    case OutMode::MsgPack:
      ok = impl_use_msgpack(root.value(), output);
      break;
  }

  return ok;
}
