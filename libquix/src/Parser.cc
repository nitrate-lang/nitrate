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
#include <nlohmann/json.hpp>
#include <quix-core/Classes.hh>
#include <quix-lexer/Base.hh>
#include <quix-lexer/Classes.hh>
#include <quix-parser/Classes.hh>
#include <string_view>
#include <unordered_set>

using json = nlohmann::json;

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

bool to_json_recurse(qparse::Node *N, json &x);

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

  json o = json::array();

  if (!to_json_recurse(static_cast<qparse::Node *>(root.value()), o)) {
    return false;
  }

  switch (out_mode) {
    case OutMode::JSON: {
      auto str = o.dump();
      ok = (size_t)fprintf(output, "%s", str.c_str()) == str.size();
      break;
    }
    case OutMode::MsgPack: {
      auto bytes = json::to_msgpack(o);
      ok = fwrite(bytes.data(), 1, bytes.size(), output) == bytes.size();
      break;
    }
  }

  return ok;
}

///============================================================================///

using namespace qparse;

bool to_json_recurse(Node *N, json &x) {
  /**
   * WARNING: TODO: Test this code thoughroughly.
   */

  if (!N) {
    x = nullptr;
    return true;
  }

  qparse_ty_t id = N->this_typeid();

  x[0] = id;

  switch (id) {
    case QAST_NODE_CEXPR: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      auto &y = x[1] = json::array();

      if (!to_json_recurse(N->as<ConstExpr>()->get_value(), y)) {
        return false;
      }

      break;
    }

    case QAST_NODE_BINEXPR: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      x[1] = N->as<BinExpr>()->get_op();

      auto &y = x[2] = json::array();
      auto &z = x[3] = json::array();

      if (!to_json_recurse(N->as<BinExpr>()->get_lhs(), y)) {
        return false;
      }

      if (!to_json_recurse(N->as<BinExpr>()->get_rhs(), z)) {
        return false;
      }

      break;
    }

    case QAST_NODE_UNEXPR: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      x[1] = N->as<UnaryExpr>()->get_op();

      auto &y = x[2] = json::array();

      if (!to_json_recurse(N->as<UnaryExpr>()->get_rhs(), y)) {
        return false;
      }

      break;
    }

    case QAST_NODE_TEREXPR: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      auto &y = x[1] = json::array();

      if (!to_json_recurse(N->as<TernaryExpr>()->get_cond(), y)) {
        return false;
      }

      auto &z = x[2] = json::array();

      if (!to_json_recurse(N->as<TernaryExpr>()->get_lhs(), z)) {
        return false;
      }

      auto &w = x[3] = json::array();

      if (!to_json_recurse(N->as<TernaryExpr>()->get_rhs(), w)) {
        return false;
      }

      break;
    }

    case QAST_NODE_INT: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      x[1] = N->as<ConstInt>()->get_value().c_str();

      break;
    }

    case QAST_NODE_FLOAT: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      x[1] = N->as<ConstFloat>()->get_value().c_str();

      break;
    }

    case QAST_NODE_STRING: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      x[1] = json::array();

      for (auto &c : N->as<ConstString>()->get_value()) {
        x[1].push_back((uint8_t)c);
      }

      break;
    }

    case QAST_NODE_CHAR: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      x[1] = (uint32_t)N->as<ConstChar>()->get_value();

      break;
    }

    case QAST_NODE_BOOL: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      x[1] = N->as<ConstBool>()->get_value();

      break;
    }

    case QAST_NODE_NULL: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_UNDEF: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_CALL: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      Call *W = N->as<Call>();

      if (!to_json_recurse(W->get_func(), x[1])) {
        return false;
      }

      auto &y = x[2] = json::object();

      for (auto &[key, val] : W->get_args()) {
        json z;
        if (!to_json_recurse(val, z)) {
          return false;
        }

        y[key.c_str()] = std::move(z);
      }

      break;
    }

    case QAST_NODE_LIST: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      auto &y = x[1] = json::array();

      for (auto &Z : N->as<List>()->get_items()) {
        json z;
        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_ASSOC: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<Assoc>()->get_key(), x[1])) {
        return false;
      }

      if (!to_json_recurse(N->as<Assoc>()->get_value(), x[2])) {
        return false;
      }

      break;
    }

    case QAST_NODE_FIELD: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      Field *W = N->as<Field>();

      if (!to_json_recurse(W->get_base(), x[1])) {
        return false;
      }

      x[2] = W->get_field().c_str();

      break;
    }

    case QAST_NODE_INDEX: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      Index *W = N->as<Index>();

      if (!to_json_recurse(W->get_base(), x[1])) {
        return false;
      }

      if (!to_json_recurse(W->get_index(), x[2])) {
        return false;
      }

      break;
    }

    case QAST_NODE_SLICE: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      Slice *W = N->as<Slice>();

      if (!to_json_recurse(W->get_base(), x[1])) {
        return false;
      }

      if (!to_json_recurse(W->get_start(), x[2])) {
        return false;
      }

      if (!to_json_recurse(W->get_end(), x[3])) {
        return false;
      }

      break;
    }

    case QAST_NODE_FSTRING: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      FString *W = N->as<FString>();

      auto &y = x[1] = json::array();

      for (auto &Z : W->get_items()) {
        json z;
        if (std::holds_alternative<String>(Z)) {
          z = std::get<String>(Z).c_str();
        } else {
          if (!to_json_recurse(std::get<Expr *>(Z), z)) {
            return false;
          }
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_IDENT: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      x[1] = N->as<Ident>()->get_name().c_str();

      break;
    }

    case QAST_NODE_SEQ_POINT: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      auto &y = x[1] = json::array();

      for (auto &Z : N->as<SeqPoint>()->get_items()) {
        json z;
        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_POST_UNEXPR: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      auto &y = x[1] = json::array();

      if (!to_json_recurse(N->as<PostUnaryExpr>()->get_lhs(), y)) {
        return false;
      }

      x[2] = N->as<PostUnaryExpr>()->get_op();

      break;
    }

    case QAST_NODE_STMT_EXPR: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<StmtExpr>()->get_stmt(), x[1])) {
        return false;
      }

      break;
    }

    case QAST_NODE_TYPE_EXPR: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<TypeExpr>()->get_type(), x[1])) {
        return false;
      }

      break;
    }

    case QAST_NODE_TEMPL_CALL: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      TemplCall *W = N->as<TemplCall>();

      if (!to_json_recurse(W->get_func(), x[1])) {
        return false;
      }

      auto &y = x[2] = json::object();

      for (auto &[key, val] : W->get_template_args()) {
        json z;
        if (!to_json_recurse(val, z)) {
          return false;
        }

        y[key.c_str()] = std::move(z);
      }

      auto &z = x[3] = json::object();

      for (auto &[key, val] : W->get_args()) {
        json w;
        if (!to_json_recurse(val, w)) {
          return false;
        }

        z[key.c_str()] = std::move(w);
      }

      break;
    }

    case QAST_NODE_REF_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<RefTy>()->get_item(), x[1])) {
        return false;
      }

      break;
    }

    case QAST_NODE_U1_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_U8_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_U16_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_U32_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_U64_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_U128_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_I8_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_I16_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_I32_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_I64_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_I128_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_F16_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_F32_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_F64_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_F128_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_VOID_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_PTR_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<PtrTy>()->get_item(), x[1])) {
        return false;
      }

      break;
    }

    case QAST_NODE_OPAQUE_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      x[1] = N->as<OpaqueTy>()->get_name().c_str();

      break;
    }

    case QAST_NODE_STRING_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_ENUM_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      EnumTy *W = N->as<EnumTy>();

      x[1] = W->get_name().c_str();

      if (!to_json_recurse(W->get_memtype(), x[2])) {
        return false;
      }

      break;
    }

    case QAST_NODE_STRUCT_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      auto &y = x[1] = json::array();

      for (auto &Z : N->as<StructTy>()->get_items()) {
        json z;
        z[0] = Z.first.c_str();
        z[1] = json::array();

        if (!to_json_recurse(Z.second, z[1])) {
          return false;
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_GROUP_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      auto &y = x[1] = json::array();

      for (auto &Z : N->as<GroupTy>()->get_items()) {
        json z;

        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_REGION_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      auto &y = x[1] = json::array();

      for (auto &Z : N->as<RegionTy>()->get_items()) {
        json z;

        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_UNION_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      auto &y = x[1] = json::array();

      for (auto &Z : N->as<UnionTy>()->get_items()) {
        json z;

        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_ARRAY_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<ArrayTy>()->get_item(), x[1])) {
        return false;
      }

      if (!to_json_recurse(N->as<ArrayTy>()->get_size(), x[2])) {
        return false;
      }

      break;
    }

    case QAST_NODE_VECTOR_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<VectorTy>()->get_item(), x[1])) {
        return false;
      }

      break;
    }

    case QAST_NODE_MAP_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<MapTy>()->get_key(), x[1])) {
        return false;
      }

      if (!to_json_recurse(N->as<MapTy>()->get_value(), x[2])) {
        return false;
      }

      break;
    }

    case QAST_NODE_TUPLE_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      auto &y = x[1] = json::array();

      for (auto &Z : N->as<TupleTy>()->get_items()) {
        json z;

        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_SET_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<SetTy>()->get_item(), x[1])) {
        return false;
      }

      break;
    }

    case QAST_NODE_RESULT_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<OptionalTy>()->get_item(), x[1])) {
        return false;
      }

      break;
    }

    case QAST_NODE_FN_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      FuncTy *W = N->as<FuncTy>();

      if (!to_json_recurse(W->get_return_ty(), x[1])) {
        return false;
      }

      x[2] = W->get_purity();

      auto &y = x[3] = json::array();

      for (auto &Z : W->get_params()) {
        json z;

        z[0] = std::get<0>(Z).c_str();

        if (!to_json_recurse(std::get<1>(Z), z[1])) {
          return false;
        }

        if (!to_json_recurse(std::get<2>(Z), z[2])) {
          return false;
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_UNRES_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      x[1] = N->as<UnresolvedType>()->get_name().c_str();

      break;
    }

    case QAST_NODE_INFER_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_TEMPL_TY: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      TemplType *W = N->as<TemplType>();

      if (!to_json_recurse(W->get_template(), x[1])) {
        return false;
      }

      auto &y = x[2] = json::array();

      for (auto &Z : W->get_args()) {
        json z;

        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_TYPEDEF: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      TypedefDecl *W = N->as<TypedefDecl>();

      x[1] = W->get_name().c_str();

      if (!to_json_recurse(W->get_type(), x[2])) {
        return false;
      }

      break;
    }

    case QAST_NODE_FNDECL: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      FnDecl *W = N->as<FnDecl>();

      x[1] = W->get_name().c_str();

      if (!to_json_recurse(W->get_type(), x[2])) {
        return false;
      }

      break;
    }

    case QAST_NODE_STRUCT: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      StructDef *W = N->as<StructDef>();

      x[1] = W->get_name().c_str();

      auto &y = x[2] = json::array();

      for (auto &Z : W->get_fields()) {
        json z;
        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      auto &z = x[3] = json::array();

      for (auto &Z : W->get_methods()) {
        json w;
        if (!to_json_recurse(Z, w)) {
          return false;
        }

        z.push_back(std::move(w));
      }

      auto &w = x[4] = json::array();

      for (auto &Z : W->get_static_methods()) {
        json v;
        if (!to_json_recurse(Z, v)) {
          return false;
        }

        w.push_back(std::move(v));
      }

      break;
    }

    case QAST_NODE_REGION: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      RegionDef *W = N->as<RegionDef>();

      x[1] = W->get_name().c_str();

      auto &y = x[2] = json::array();

      for (auto &Z : W->get_fields()) {
        json z;
        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      auto &z = x[3] = json::array();

      for (auto &Z : W->get_methods()) {
        json w;
        if (!to_json_recurse(Z, w)) {
          return false;
        }

        z.push_back(std::move(w));
      }

      auto &w = x[4] = json::array();

      for (auto &Z : W->get_static_methods()) {
        json v;
        if (!to_json_recurse(Z, v)) {
          return false;
        }

        w.push_back(std::move(v));
      }

      break;
    }

    case QAST_NODE_GROUP: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      GroupDef *W = N->as<GroupDef>();

      x[1] = W->get_name().c_str();

      auto &y = x[2] = json::array();

      for (auto &Z : W->get_fields()) {
        json z;
        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      auto &z = x[3] = json::array();

      for (auto &Z : W->get_methods()) {
        json w;
        if (!to_json_recurse(Z, w)) {
          return false;
        }

        z.push_back(std::move(w));
      }

      auto &w = x[4] = json::array();

      for (auto &Z : W->get_static_methods()) {
        json v;
        if (!to_json_recurse(Z, v)) {
          return false;
        }

        w.push_back(std::move(v));
      }

      break;
    }

    case QAST_NODE_UNION: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      UnionDef *W = N->as<UnionDef>();

      x[1] = W->get_name().c_str();

      auto &y = x[2] = json::array();

      for (auto &Z : W->get_fields()) {
        json z;
        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      auto &z = x[3] = json::array();

      for (auto &Z : W->get_methods()) {
        json w;
        if (!to_json_recurse(Z, w)) {
          return false;
        }

        z.push_back(std::move(w));
      }

      auto &w = x[4] = json::array();

      for (auto &Z : W->get_static_methods()) {
        json v;
        if (!to_json_recurse(Z, v)) {
          return false;
        }

        w.push_back(std::move(v));
      }

      break;
    }

    case QAST_NODE_ENUM: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      EnumDef *W = N->as<EnumDef>();

      x[1] = W->get_name().c_str();

      auto &y = x[2] = json::array();

      for (auto &Z : W->get_items()) {
        json z;
        z[0] = Z.first.c_str();
        z[1] = json::array();

        if (!to_json_recurse(Z.second, z[1])) {
          return false;
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_FN: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      FnDef *W = N->as<FnDef>();

      x[1] = W->get_name().c_str();

      if (!to_json_recurse(W->get_type(), x[2])) {
        return false;
      }

      auto &y = x[3] = json::array();

      for (auto &Z : W->get_captures()) {
        json z;
        z[0] = Z.first.c_str();
        z[1] = Z.second;

        y.push_back(std::move(z));
      }

      if (!to_json_recurse(W->get_precond(), x[4])) {
        return false;
      }

      if (!to_json_recurse(W->get_postcond(), x[5])) {
        return false;
      }

      if (!to_json_recurse(W->get_body(), x[6])) {
        return false;
      }

      break;
    }

    case QAST_NODE_SUBSYSTEM: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      SubsystemDecl *W = N->as<SubsystemDecl>();

      x[1] = W->get_name().c_str();

      auto &y = x[2] = json::array();

      for (auto &Z : W->get_deps()) {
        y.push_back(Z.c_str());
      }

      if (!to_json_recurse(W->get_body(), x[3])) {
        return false;
      }

      break;
    }

    case QAST_NODE_EXPORT: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      ExportDecl *W = N->as<ExportDecl>();

      x[1] = W->get_name().c_str();
      x[2] = W->get_abi_name().c_str();

      if (!to_json_recurse(W->get_body(), x[3])) {
        return false;
      }

      break;
    }

    case QAST_NODE_COMPOSITE_FIELD: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      CompositeField *W = N->as<CompositeField>();

      x[1] = W->get_name().c_str();

      if (!to_json_recurse(W->get_type(), x[2])) {
        return false;
      }

      if (!to_json_recurse(W->get_value(), x[3])) {
        return false;
      }

      break;
    }

    case QAST_NODE_BLOCK: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      Block *W = N->as<Block>();

      x[1] = W->is_unsafe();

      json &y = x[2] = json::array();

      for (auto &Z : W->get_items()) {
        json z;
        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_CONST: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      ConstDecl *W = N->as<ConstDecl>();

      x[1] = W->get_name().c_str();

      if (!to_json_recurse(W->get_type(), x[2])) {
        return false;
      }

      if (!to_json_recurse(W->get_value(), x[3])) {
        return false;
      }

      break;
    }

    case QAST_NODE_VAR: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      VarDecl *W = N->as<VarDecl>();

      x[1] = W->get_name().c_str();

      if (!to_json_recurse(W->get_type(), x[2])) {
        return false;
      }

      if (!to_json_recurse(W->get_value(), x[3])) {
        return false;
      }

      break;
    }

    case QAST_NODE_LET: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      LetDecl *W = N->as<LetDecl>();

      x[1] = W->get_name().c_str();

      if (!to_json_recurse(W->get_type(), x[2])) {
        return false;
      }

      if (!to_json_recurse(W->get_value(), x[3])) {
        return false;
      }

      break;
    }

    case QAST_NODE_INLINE_ASM: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      qcore_implement("ASM node serialization is not supported yet.");
      break;
    }

    case QAST_NODE_RETURN: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<ReturnStmt>()->get_value(), x[1])) {
        return false;
      }

      break;
    }

    case QAST_NODE_RETIF: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<ReturnIfStmt>()->get_cond(), x[1])) {
        return false;
      }

      if (!to_json_recurse(N->as<ReturnIfStmt>()->get_value(), x[2])) {
        return false;
      }

      break;
    }

    case QAST_NODE_RETZ: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<RetZStmt>()->get_cond(), x[1])) {
        return false;
      }

      if (!to_json_recurse(N->as<RetZStmt>()->get_value(), x[2])) {
        return false;
      }

      break;
    }

    case QAST_NODE_RETV: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<RetVStmt>()->get_cond(), x[1])) {
        return false;
      }

      break;
    }

    case QAST_NODE_BREAK: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_CONTINUE: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      break;
    }

    case QAST_NODE_IF: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      IfStmt *W = N->as<IfStmt>();

      if (!to_json_recurse(W->get_cond(), x[1])) {
        return false;
      }

      if (!to_json_recurse(W->get_then(), x[2])) {
        return false;
      }

      if (!to_json_recurse(W->get_else(), x[3])) {
        return false;
      }

      break;
    }

    case QAST_NODE_WHILE: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      WhileStmt *W = N->as<WhileStmt>();

      if (!to_json_recurse(W->get_cond(), x[1])) {
        return false;
      }

      if (!to_json_recurse(W->get_body(), x[2])) {
        return false;
      }

      break;
    }

    case QAST_NODE_FOR: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      ForStmt *W = N->as<ForStmt>();

      if (!to_json_recurse(W->get_init(), x[1])) {
        return false;
      }

      if (!to_json_recurse(W->get_cond(), x[2])) {
        return false;
      }

      if (!to_json_recurse(W->get_step(), x[3])) {
        return false;
      }

      if (!to_json_recurse(W->get_body(), x[4])) {
        return false;
      }

      break;
    }

    case QAST_NODE_FORM: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      FormStmt *W = N->as<FormStmt>();

      x[1] = W->get_idx_ident().c_str();
      x[2] = W->get_val_ident().c_str();

      if (!to_json_recurse(W->get_maxjobs(), x[3])) {
        return false;
      }

      if (!to_json_recurse(W->get_expr(), x[4])) {
        return false;
      }

      if (!to_json_recurse(W->get_body(), x[4])) {
        return false;
      }

      break;
    }

    case QAST_NODE_FOREACH: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      ForeachStmt *W = N->as<ForeachStmt>();

      x[1] = W->get_idx_ident().c_str();
      x[2] = W->get_val_ident().c_str();

      if (!to_json_recurse(W->get_expr(), x[3])) {
        return false;
      }

      if (!to_json_recurse(W->get_body(), x[4])) {
        return false;
      }

      break;
    }

    case QAST_NODE_CASE: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      CaseStmt *W = N->as<CaseStmt>();

      if (!to_json_recurse(W->get_cond(), x[1])) {
        return false;
      }

      if (!to_json_recurse(W->get_body(), x[2])) {
        return false;
      }

      break;
    }

    case QAST_NODE_SWITCH: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<SwitchStmt>()->get_cond(), x[1])) {
        return false;
      }

      if (!to_json_recurse(N->as<SwitchStmt>()->get_default(), x[2])) {
        return false;
      }

      auto &y = x[3] = json::array();

      for (auto &Z : N->as<SwitchStmt>()->get_cases()) {
        json z;
        if (!to_json_recurse(Z, z)) {
          return false;
        }

        y.push_back(std::move(z));
      }

      break;
    }

    case QAST_NODE_EXPR_STMT: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      auto &y = x[1] = json::array();
      if (!to_json_recurse(N->as<ExprStmt>()->get_expr(), y)) {
        return false;
      }

      break;
    }

    case QAST_NODE_VOLSTMT: {
      /**
       * @brief [Brief Description]
       * @note [Developer Notes]
       */

      if (!to_json_recurse(N->as<VolStmt>()->get_stmt(), x[1])) {
        return false;
      }

      break;
    }

    default: {
      qcore_panicf("Unexpected node type: %d", id);
    }
  }

  if (N->is_decl()) {
    x[x.size()] = N->as<Decl>()->get_visibility();

    auto &y = x[x.size()] = json::array();

    for (auto &Z : N->as<Decl>()->get_tags()) {
      y.push_back(Z.c_str());
    }
  }

  return true;
}
