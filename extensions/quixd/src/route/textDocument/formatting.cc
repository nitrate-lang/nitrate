
#include <quix-parser/Node.h>
#include <rapidjson/document.h>

#include <cctype>
#include <core/SyncFS.hh>
#include <core/server.hh>
#include <cstdint>
#include <quix-core/Classes.hh>
#include <quix-lexer/Classes.hh>
#include <quix-parser/Classes.hh>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace rapidjson;

struct Position {
  size_t line = 0;
  size_t character = 0;
};

struct Range {
  Position start;
  Position end;
};

struct FormattingOptions {
  size_t tabSize = 0;
  bool insertSpaces = false;
};

struct AutomatonState {
  qlex_tok_t last;
  size_t brk_depth /* Square bracket */, par_depth /* Parantheis */, bra_depth /* Curly bracket */;
  bool lword;
  bool eof;
  bool did_root;

  AutomatonState() {
    last = {};
    brk_depth = par_depth = bra_depth = 0;
    lword = false;
    eof = false;
    did_root = false;
  }
};

static std::string escape_char_literal(char ch) {
  if (!std::isspace(ch) && !std::isprint(ch)) {
    const char* tab = "0123456789abcdef";
    uint8_t uch = ch;
    char enc[6] = {'\'', '\\', 'x', 0, 0, '\''};
    enc[3] = tab[uch >> 4];
    enc[4] = tab[uch & 0xF];
    return std::string(enc, 6);
  }

  switch (ch) {
    case '\n':
      return "'\\n'";
    case '\t':
      return "'\\t'";
    case '\r':
      return "'\\r'";
    case '\v':
      return "'\\v'";
    case '\f':
      return "'\\f'";
    case '\b':
      return "'\\b'";
    case '\a':
      return "'\\a'";
    case '\\':
      return "'\\\\'";
    case '\'':
      return "'\\''";
    default:
      return "'" + std::string(1, ch) + "'";
  }
}

static std::string escape_string_literal_chunk(AutomatonState&, std::string_view str) {
  std::string res = "";
  res.reserve(str.size());

  for (char ch : str) {
    switch (ch) {
      case '\n':
        res += "\\n";
        break;
      case '\t':
        res += "\\t";
        break;
      case '\r':
        res += "\\r";
        break;
      case '\v':
        res += "\\v";
        break;
      case '\f':
        res += "\\f";
        break;
      case '\b':
        res += "\\b";
        break;
      case '\a':
        res += "\\a";
        break;
      case '\\':
        res += "\\\\";
        break;
      case '"':
        res += "\\\"";
        break;
      default:
        res += ch;
        break;
    }
  }

  return res;
}

static std::string escape_string_literal(AutomatonState& S, std::string_view str) {
  constexpr size_t max_chunk_size = 60;

  if (str.empty()) {
    return "\"\"";
  }

  size_t num_chunks = str.size() / max_chunk_size;
  size_t rem = str.size() % max_chunk_size;

  std::string res;

  for (size_t i = 0; i < num_chunks; i++) {
    res += "\"" + escape_string_literal_chunk(S, str.substr(i * max_chunk_size, max_chunk_size)) +
           "\"";

    if (rem > 0 || i < num_chunks - 1) {
      res += " \\\n";
    }
  }

  if (rem > 0) {
    res +=
        "\"" + escape_string_literal_chunk(S, str.substr(num_chunks * max_chunk_size, rem)) + "\"";
  }

  return res;
}

static void put_indent(AutomatonState& S, std::ostream& O) {
  if (S.bra_depth) {
    O << std::string(S.bra_depth * 2, ' ');
  }
  if (S.brk_depth) {
    O << std::string(S.brk_depth * 2, ' ');
  }
}

static void automaton_recurse(qparse::Node* C, AutomatonState& S, std::ostream& O);

static void put_type_metadata(qparse::Type* N, AutomatonState& S, std::ostream& O) {
  auto range = N->get_range();

  if (range.first || range.second) {
    O << ": [";
    automaton_recurse(range.first, S, O);
    O << ":";
    automaton_recurse(range.second, S, O);
    O << "]";
  }

  if (N->get_width()) {
    O << ": ";
    automaton_recurse(N->get_width(), S, O);
  }
}

template <typename T>
static void put_composite_defintion(T* N, AutomatonState& S, std::ostream& O) {
  using namespace qparse;

  static const std::unordered_map<Visibility, std::string_view> vis_str = {
      {Visibility::PRIVATE, "sec"},
      {Visibility::PUBLIC, "pub"},
      {Visibility::PROTECTED, "pro"},
  };

  O << " " << N->get_name() << " ";

  bool has_fields = !N->get_fields().empty();
  bool has_methods = !N->get_methods().empty();
  bool has_static_methods = !N->get_static_methods().empty();

  bool empty = !has_fields && !has_methods && !has_static_methods && N->get_tags().empty();
  if (empty) {
    O << "{}";
    return;
  }

  O << "{\n";
  S.bra_depth++;

  for (auto it = N->get_fields().begin(); it != N->get_fields().end(); it++) {
    put_indent(S, O);

    O << vis_str.at((*it)->get_visibility()) << " ";
    automaton_recurse(*it, S, O);
    O << "\n";
  }

  if (has_methods) {
    if (has_fields) {
      O << "\n";
    }

    for (auto it = N->get_methods().begin(); it != N->get_methods().end(); it++) {
      put_indent(S, O);

      O << vis_str.at((*it)->get_visibility()) << " ";
      automaton_recurse(*it, S, O);
      O << ";\n";
    }
  }

  if (has_static_methods) {
    if (has_fields || has_methods) {
      O << "\n";
    }

    for (auto it = N->get_static_methods().begin(); it != N->get_static_methods().end(); it++) {
      put_indent(S, O);

      O << vis_str.at((*it)->get_visibility()) << " static ";
      automaton_recurse(*it, S, O);
      O << ";\n";
    }
  }

  S.bra_depth--;
  put_indent(S, O);
  O << "}";

  if (!N->get_tags().empty()) {
    O << " with [";
    for (auto it = N->get_tags().begin(); it != N->get_tags().end(); it++) {
      O << *it;
      if (std::next(it) != N->get_tags().end()) {
        O << ", ";
      }
    }
    O << "]";
  }
}

static void automaton_recurse(qparse::Node* C, AutomatonState& S, std::ostream& O) {
  (void)escape_char_literal;

  using namespace qparse;

  if (!C) {
    return;
  }

  switch (C->this_typeid()) {
    case QAST_NODE_STMT:
    case QAST_NODE_TYPE:
    case QAST_NODE_DECL:
    case QAST_NODE_EXPR: {
      break;
    }

    case QAST_NODE_CEXPR: {
      automaton_recurse(C->as<ConstExpr>()->get_value(), S, O);
      break;
    }

    case QAST_NODE_BINEXPR: {
      BinExpr* N = C->as<BinExpr>();
      automaton_recurse(N->get_lhs(), S, O);
      O << " " << N->get_op() << " ";
      automaton_recurse(N->get_rhs(), S, O);
      break;
    }

    case QAST_NODE_UNEXPR: {
      UnaryExpr* N = C->as<UnaryExpr>();
      O << N->get_op();
      switch (N->get_op()) {
        case qOpSizeof:
        case qOpBitsizeof:
        case qOpAlignof:
        case qOpTypeof:
          O << "(";
          automaton_recurse(N->get_rhs(), S, O);
          O << ")";
          break;
        default:
          automaton_recurse(N->get_rhs(), S, O);
          break;
      }
      break;
    }

    case QAST_NODE_TEREXPR: {
      TernaryExpr* N = C->as<TernaryExpr>();
      automaton_recurse(N->get_cond(), S, O);
      O << " ? ";
      automaton_recurse(N->get_lhs(), S, O);
      O << " : ";
      automaton_recurse(N->get_rhs(), S, O);
      break;
    }

    case QAST_NODE_INT: {
      O << C->as<ConstInt>()->get_value();
      break;
    }

    case QAST_NODE_FLOAT: {
      O << C->as<ConstFloat>()->get_value();
      break;
    }

    case QAST_NODE_STRING: {
      O << escape_string_literal(S, C->as<ConstString>()->get_value());
      break;
    }

    case QAST_NODE_CHAR: {
      ConstChar* N = C->as<ConstChar>();
      qcore_assert(N->get_value() >= 0 && N->get_value() <= 255);
      O << escape_char_literal(N->get_value());
      break;
    }

    case QAST_NODE_BOOL: {
      if (C->as<ConstBool>()->get_value()) {
        O << "true";
      } else {
        O << "false";
      }
      break;
    }

    case QAST_NODE_NULL: {
      O << "null";
      break;
    }

    case QAST_NODE_UNDEF: {
      O << "undef";
      break;
    }

    case QAST_NODE_CALL: {
      Call* N = C->as<Call>();

      automaton_recurse(N->get_func(), S, O);

      O << "(";
      for (auto it = N->get_args().begin(); it != N->get_args().end(); it++) {
        if (!std::isdigit(it->first.at(0))) {
          O << it->first;
          O << ": ";
        }
        automaton_recurse(it->second, S, O);

        if (std::next(it) != N->get_args().end()) {
          O << ", ";
        }
      }
      O << ")";
      break;
    }

    case QAST_NODE_LIST: {
      List* N = C->as<List>();
      auto ty = N->get_items().front()->this_typeid();
      if (N->get_items().size() > 0 && (ty == QAST_NODE_ASSOC || ty == QAST_NODE_LIST)) {
        O << "[\n";
        S.brk_depth++;
        for (auto it = N->get_items().begin(); it != N->get_items().end(); it++) {
          put_indent(S, O);
          automaton_recurse(*it, S, O);
          O << ",\n";
        }
        S.brk_depth--;
        put_indent(S, O);
        O << "]";
      } else {
        size_t split_on =
            N->get_items().size() <= 8 ? 8 : std::ceil(std::sqrt(N->get_items().size()));

        O << "[";
        size_t i = 0;
        for (auto it = N->get_items().begin(); it != N->get_items().end(); it++) {
          automaton_recurse(*it, S, O);

          i++;
          if (std::next(it) != N->get_items().end()) {
            O << ",";
            if (i % split_on == 0) {
              O << "\n ";
              put_indent(S, O);
            } else {
              O << " ";
            }
          }
        }
        O << "]";
      }
      break;
    }

    case QAST_NODE_ASSOC: {
      Assoc* N = C->as<Assoc>();
      O << "{";
      automaton_recurse(N->get_key(), S, O);
      O << ": ";
      automaton_recurse(N->get_value(), S, O);
      O << "}";
      break;
    }

    case QAST_NODE_FIELD: {
      Field* N = C->as<Field>();
      automaton_recurse(N->get_base(), S, O);
      O << "." << N->get_field();
      break;
    }

    case QAST_NODE_INDEX: {
      Index* N = C->as<Index>();
      automaton_recurse(N->get_base(), S, O);
      O << "[";
      automaton_recurse(N->get_index(), S, O);
      O << "]";
      break;
    }

    case QAST_NODE_SLICE: {
      Slice* N = C->as<Slice>();
      automaton_recurse(N->get_base(), S, O);
      O << "[";
      automaton_recurse(N->get_start(), S, O);
      O << ":";
      automaton_recurse(N->get_end(), S, O);
      O << "]";
      break;
    }

    case QAST_NODE_FSTRING: {
      FString* N = C->as<FString>();
      O << "f\"";
      for (auto it = N->get_items().begin(); it != N->get_items().end(); it++) {
        if (std::holds_alternative<String>(*it)) {
          O << escape_string_literal_chunk(S, std::get<String>(*it));
        } else {
          O << "{";
          std::stringstream ss;
          automaton_recurse(std::get<Expr*>(*it), S, ss);
          O << escape_string_literal_chunk(S, ss.str());
          O << "}";
        }
      }
      O << "\"";
      break;
    }

    case QAST_NODE_IDENT: {
      O << C->as<Ident>()->get_name();
      break;
    }

    case QAST_NODE_SEQ_POINT: {
      SeqPoint* N = C->as<SeqPoint>();

      O << "(";
      for (auto it = N->get_items().begin(); it != N->get_items().end(); it++) {
        automaton_recurse(*it, S, O);

        if (std::next(it) != N->get_items().end()) {
          O << ", ";
        }
      }
      O << ")";
      break;
    }

    case QAST_NODE_POST_UNEXPR: {
      PostUnaryExpr* N = C->as<PostUnaryExpr>();
      automaton_recurse(N->get_lhs(), S, O);
      O << N->get_op();
      break;
    }

    case QAST_NODE_STMT_EXPR: {
      automaton_recurse(C->as<StmtExpr>()->get_stmt(), S, O);
      break;
    }

    case QAST_NODE_TYPE_EXPR: {
      automaton_recurse(C->as<TypeExpr>()->get_type(), S, O);
      break;
    }

    case QAST_NODE_TEMPL_CALL: {
      /// FIXME: Verify this one is correct
      TemplCall* N = C->as<TemplCall>();

      automaton_recurse(N->get_func(), S, O);

      O << "<";
      for (auto it = N->get_template_args().begin(); it != N->get_template_args().end(); it++) {
        if (!std::isdigit(it->first.at(0))) {
          O << it->first;
          O << ": ";
        }
        automaton_recurse(it->second, S, O);

        if (std::next(it) != N->get_template_args().end()) {
          O << ", ";
        }
      }
      O << ">";

      O << "(";
      for (auto it = N->get_args().begin(); it != N->get_args().end(); it++) {
        if (!std::isdigit(it->first.at(0))) {
          O << it->first;
          O << ": ";
        }
        automaton_recurse(it->second, S, O);

        if (std::next(it) != N->get_args().end()) {
          O << ", ";
        }
      }
      O << ")";

      break;
    }

    case QAST_NODE_REF_TY: {
      O << "&";
      automaton_recurse(C->as<RefTy>()->get_item(), S, O);
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_U1_TY: {
      O << "u1";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_U8_TY: {
      O << "u8";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_U16_TY: {
      O << "u16";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_U32_TY: {
      O << "u32";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_U64_TY: {
      O << "u64";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_U128_TY: {
      O << "u128";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_I8_TY: {
      O << "i8";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_I16_TY: {
      O << "i16";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_I32_TY: {
      O << "i32";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_I64_TY: {
      O << "i64";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_I128_TY: {
      O << "i128";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_F16_TY: {
      O << "f16";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_F32_TY: {
      O << "f32";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_F64_TY: {
      O << "f64";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_F128_TY: {
      O << "f128";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_VOID_TY: {
      O << "void";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_PTR_TY: {
      O << "*";
      automaton_recurse(C->as<PtrTy>()->get_item(), S, O);
      break;
    }

    case QAST_NODE_OPAQUE_TY: {
      O << "opaque(" << C->as<OpaqueTy>()->get_name() << ")";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_STRUCT_TY: {
      qcore_panic("Unreachable");
      break;
    }

    case QAST_NODE_GROUP_TY: {
      qcore_panic("Unreachable");
      break;
    }

    case QAST_NODE_REGION_TY: {
      qcore_panic("Unreachable");
      break;
    }

    case QAST_NODE_UNION_TY: {
      qcore_panic("Unreachable");
      break;
    }

    case QAST_NODE_ARRAY_TY: {
      ArrayTy* N = C->as<ArrayTy>();
      O << "[";
      automaton_recurse(N->get_item(), S, O);
      O << "; ";
      automaton_recurse(N->get_size(), S, O);
      O << "]";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_TUPLE_TY: {
      TupleTy* N = C->as<TupleTy>();

      O << "(";
      for (auto it = N->get_items().begin(); it != N->get_items().end(); ++it) {
        automaton_recurse(*it, S, O);

        if (std::next(it) != N->get_items().end()) {
          O << ", ";
        }
      }
      O << ")";

      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_FN_TY: {
      static const std::unordered_map<FuncPurity, std::string> purity_str = {
          {FuncPurity::IMPURE_THREAD_UNSAFE, ""},
          {FuncPurity::IMPURE_THREAD_SAFE, " tsafe"},
          {FuncPurity::PURE, " pure"},
          {FuncPurity::QUASIPURE, " quasipure"},
          {FuncPurity::RETROPURE, " retropure"}};

      FuncTy* N = C->as<FuncTy>();

      std::string props;
      props += purity_str.at(N->get_purity());

      if (N->is_noexcept()) {
        props += " noexcept";
      }

      if (N->is_foreign()) {
        props += " foreign";
      }

      if (N->is_crashpoint()) {
        props += " crashpoint";
      }

      O << "fn";
      if (!props.empty()) {
        O << std::move(props);
      }

      O << "(";
      for (auto it = N->get_params().begin(); it != N->get_params().end(); it++) {
        O << std::get<0>(*it);
        auto param_ty = std::get<1>(*it);
        if ((param_ty && param_ty->this_typeid() != QAST_NODE_INFER_TY) || std::get<2>(*it)) {
          O << ": ";
          automaton_recurse(std::get<1>(*it), S, O);
          if (std::get<2>(*it)) {
            O << " = ";
            automaton_recurse(std::get<2>(*it), S, O);
          }
        }

        if (std::next(it) != N->get_params().end() || N->is_variadic()) {
          O << ", ";
        }
      }

      if (N->is_variadic()) {
        O << "...";
      }

      O << ")";

      if (N->get_return_ty() && N->get_return_ty()->this_typeid() != QAST_NODE_VOID_TY) {
        O << ": ";
        automaton_recurse(N->get_return_ty(), S, O);
      }

      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_UNRES_TY: {
      O << C->as<UnresolvedType>()->get_name();
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_INFER_TY: {
      O << "?";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_TEMPL_TY: {
      TemplType* N = C->as<TemplType>();

      if (N->get_template()->this_typeid() == QAST_NODE_UNRES_TY) {
        auto name = N->get_template()->as<UnresolvedType>()->get_name();

        if (name == "__builtin_result") {
          qcore_assert(N->get_args().size() == 1);
          automaton_recurse(N->get_args().front(), S, O);
          O << "?";

          put_type_metadata(C->as<qparse::Type>(), S, O);
          break;
        } else if (name == "__builtin_vec") {
          qcore_assert(N->get_args().size() == 1);

          O << "[";
          automaton_recurse(N->get_args().front(), S, O);
          O << "]";

          put_type_metadata(C->as<qparse::Type>(), S, O);
          break;
        } else if (name == "__builtin_uset") {
          qcore_assert(N->get_args().size() == 1);

          O << "{";
          automaton_recurse(N->get_args().front(), S, O);
          O << "}";

          put_type_metadata(C->as<qparse::Type>(), S, O);
          break;
        } else if (name == "__builtin_umap") {
          qcore_assert(N->get_args().size() == 2);

          O << "[";
          automaton_recurse(N->get_args()[0], S, O);
          O << "->";
          automaton_recurse(N->get_args()[1], S, O);
          O << "]";

          put_type_metadata(C->as<qparse::Type>(), S, O);
          break;
        }
      }

      automaton_recurse(N->get_template(), S, O);
      O << "<";
      for (auto it = N->get_args().begin(); it != N->get_args().end(); ++it) {
        automaton_recurse(*it, S, O);

        if (std::next(it) != N->get_args().end()) {
          O << ", ";
        }
      }
      O << ">";
      put_type_metadata(C->as<qparse::Type>(), S, O);
      break;
    }

    case QAST_NODE_TYPEDEF: {
      TypedefDecl* N = C->as<TypedefDecl>();
      O << "type " << N->get_name() << " = ";
      automaton_recurse(N->get_type(), S, O);
      break;
    }

    case QAST_NODE_FNDECL: {
      static const std::unordered_map<FuncPurity, std::string> purity_str = {
          {FuncPurity::IMPURE_THREAD_UNSAFE, ""},
          {FuncPurity::IMPURE_THREAD_SAFE, " tsafe"},
          {FuncPurity::PURE, " pure"},
          {FuncPurity::QUASIPURE, " quasipure"},
          {FuncPurity::RETROPURE, " retropure"}};

      FnDecl* N = C->as<FnDecl>();
      FuncTy* F = N->get_type();
      if (!F) {
        break;
      }

      std::string props;
      props += purity_str.at(F->get_purity());

      if (F->is_noexcept()) {
        props += " noexcept";
      }

      if (F->is_foreign()) {
        props += " foreign";
      }

      if (F->is_crashpoint()) {
        props += " crashpoint";
      }

      if (!N->get_name().empty()) {
        props += " " + N->get_name();
      }

      O << "fn";
      if (!props.empty()) {
        O << std::move(props);
      }

      O << "(";
      for (auto it = F->get_params().begin(); it != F->get_params().end(); it++) {
        if (std::get<0>(*it) == "this") {
          continue;
        }

        O << std::get<0>(*it);
        auto param_ty = std::get<1>(*it);
        if ((param_ty && param_ty->this_typeid() != QAST_NODE_INFER_TY) || std::get<2>(*it)) {
          O << ": ";
          automaton_recurse(std::get<1>(*it), S, O);
          if (std::get<2>(*it)) {
            O << " = ";
            automaton_recurse(std::get<2>(*it), S, O);
          }
        }

        if (std::next(it) != F->get_params().end() || F->is_variadic()) {
          O << ", ";
        }
      }

      if (F->is_variadic()) {
        O << "...";
      }

      O << ")";

      if (F->get_return_ty() && F->get_return_ty()->this_typeid() != QAST_NODE_VOID_TY) {
        O << ": ";
        automaton_recurse(F->get_return_ty(), S, O);
      }

      break;
    }

    case QAST_NODE_STRUCT: {
      O << "struct";
      put_composite_defintion(C->as<StructDef>(), S, O);
      break;
    }

    case QAST_NODE_REGION: {
      O << "region";
      put_composite_defintion(C->as<RegionDef>(), S, O);
      break;
    }

    case QAST_NODE_GROUP: {
      O << "group";
      put_composite_defintion(C->as<GroupDef>(), S, O);
      break;
    }

    case QAST_NODE_UNION: {
      O << "union";
      put_composite_defintion(C->as<UnionDef>(), S, O);
      break;
    }

    case QAST_NODE_ENUM: {
      /// TODO:
      EnumDef* N = C->as<EnumDef>();
      O << "enum " << N->get_name();

      if (N->get_type()) {
        O << ": ";
        automaton_recurse(N->get_type(), S, O);
        O << " ";
      } else {
        O << " ";
      }

      if (N->get_items().empty()) {
        O << "{}";
        break;
      }

      O << "{\n";
      S.bra_depth++;

      for (auto it = N->get_items().begin(); it != N->get_items().end(); it++) {
        put_indent(S, O);

        O << it->first;
        if (it->second) {
          O << " = ";
          automaton_recurse(it->second, S, O);
        }

        O << ",\n";
      }

      S.bra_depth--;
      put_indent(S, O);
      O << "}";

      break;
    }

    case QAST_NODE_FN: {
      static const std::unordered_map<FuncPurity, std::string> purity_str = {
          {FuncPurity::IMPURE_THREAD_UNSAFE, ""},
          {FuncPurity::IMPURE_THREAD_SAFE, " tsafe"},
          {FuncPurity::PURE, " pure"},
          {FuncPurity::QUASIPURE, " quasipure"},
          {FuncPurity::RETROPURE, " retropure"}};

      FnDef* N = C->as<FnDef>();
      FuncTy* F = N->get_type();
      if (!F) {
        break;
      }

      std::string props;
      props += purity_str.at(F->get_purity());

      if (F->is_noexcept()) {
        props += " noexcept";
      }

      if (F->is_foreign()) {
        props += " foreign";
      }

      if (F->is_crashpoint()) {
        props += " crashpoint";
      }

      if (!N->get_name().empty()) {
        props += " " + N->get_name();
      }

      O << "fn";
      if (!props.empty()) {
        O << std::move(props);
      }

      O << "(";
      for (auto it = F->get_params().begin(); it != F->get_params().end(); it++) {
        if (std::get<0>(*it) == "this") {
          continue;
        }

        O << std::get<0>(*it);
        auto param_ty = std::get<1>(*it);
        if ((param_ty && param_ty->this_typeid() != QAST_NODE_INFER_TY) || std::get<2>(*it)) {
          O << ": ";
          automaton_recurse(std::get<1>(*it), S, O);
          if (std::get<2>(*it)) {
            O << " = ";
            automaton_recurse(std::get<2>(*it), S, O);
          }
        }

        if (std::next(it) != F->get_params().end() || F->is_variadic()) {
          O << ", ";
        }
      }

      if (F->is_variadic()) {
        O << "...";
      }

      O << ")";

      if (F->get_return_ty() && F->get_return_ty()->this_typeid() != QAST_NODE_VOID_TY) {
        O << ": ";
        automaton_recurse(F->get_return_ty(), S, O);
      }
      O << " ";

      bool arrow_syntax =
          (N->get_body()->get_items().size() == 1) && !N->get_precond() && !N->get_postcond();

      if (arrow_syntax) {
        O << "=> ";
        automaton_recurse(N->get_body()->get_items().front(), S, O);
        O << ";";
      } else {
        automaton_recurse(N->get_body(), S, O);

        bool promises = N->get_precond() || N->get_postcond();
        if (promises) {
          O << " promise {\n";
          S.bra_depth++;
          if (N->get_precond()) {
            put_indent(S, O);
            O << "in ";
            automaton_recurse(N->get_precond(), S, O);
            O << ";\n";
          }

          if (N->get_postcond()) {
            put_indent(S, O);
            O << "out ";
            automaton_recurse(N->get_postcond(), S, O);
            O << ";\n";
          }
          S.bra_depth--;
          O << "}";
        }
      }

      break;
    }

    case QAST_NODE_SUBSYSTEM: {
      SubsystemDecl* N = C->as<SubsystemDecl>();
      O << "subsystem " << N->get_name();

      if (!N->get_deps().empty()) {
        O << ": [";
        for (auto it = N->get_deps().begin(); it != N->get_deps().end(); it++) {
          O << *it;
          if (std::next(it) != N->get_deps().end()) {
            O << ", ";
          }
        }
        O << "]";
      }

      O << " ";

      automaton_recurse(N->get_body(), S, O);

      if (!N->get_tags().empty()) {
        O << " with [";
        for (auto it = N->get_tags().begin(); it != N->get_tags().end(); it++) {
          O << *it;
          if (std::next(it) != N->get_tags().end()) {
            O << ", ";
          }
        }
        O << "]";
      }
      break;
    }

    case QAST_NODE_EXPORT: {
      ExportDecl* N = C->as<ExportDecl>();
      std::vector<Stmt*> imports, exports;

      for (auto it = N->get_body()->get_items().begin(); it != N->get_body()->get_items().end();
           it++) {
        qparse_ty_t ty = (*it)->this_typeid();

        if (ty == QAST_NODE_FNDECL) {
          imports.push_back(*it);
        } else if (ty == QAST_NODE_FN) {
          exports.push_back(*it);
        } else if (ty == QAST_NODE_LET) {
          LetDecl* V = (*it)->as<LetDecl>();
          V->get_value() ? exports.push_back(*it) : imports.push_back(*it);
        } else if (ty == QAST_NODE_VAR) {
          VarDecl* V = (*it)->as<VarDecl>();
          V->get_value() ? exports.push_back(*it) : imports.push_back(*it);
        } else if (ty == QAST_NODE_CONST) {
          ConstDecl* V = (*it)->as<ConstDecl>();
          V->get_value() ? exports.push_back(*it) : imports.push_back(*it);
        }
      }

      if (imports.size() == 1) {
        O << "import ";
        if (N->get_abi_name().empty()) {
          O << "\"std\"";
        } else {
          O << escape_string_literal(S, N->get_abi_name());
        }
        O << " ";

        automaton_recurse(imports.front(), S, O);
      } else if (!imports.empty()) {
        O << "import ";
        if (N->get_abi_name().empty()) {
          O << "\"std\"";
        } else {
          O << escape_string_literal(S, N->get_abi_name());
        }
        O << " {\n";
        S.bra_depth++;

        for (auto& stmt : imports) {
          put_indent(S, O);
          automaton_recurse(stmt, S, O);
          O << ";\n";
        }

        S.bra_depth--;
        put_indent(S, O);

        O << "}";
      }

      if (exports.size() == 1) {
        O << "pub ";
        if (N->get_abi_name().empty()) {
          O << "\"std\"";
        } else {
          O << escape_string_literal(S, N->get_abi_name());
        }
        O << " ";

        automaton_recurse(exports.front(), S, O);
      } else if (!exports.empty()) {
        O << "pub ";
        if (N->get_abi_name().empty()) {
          O << "\"std\"";
        } else {
          O << escape_string_literal(S, N->get_abi_name());
        }
        O << " {\n";
        S.bra_depth++;

        for (auto& stmt : exports) {
          put_indent(S, O);
          automaton_recurse(stmt, S, O);
          O << ";\n";
        }

        S.bra_depth--;
        put_indent(S, O);

        O << "}";
      }

      break;
    }

    case QAST_NODE_COMPOSITE_FIELD: {
      CompositeField* N = C->as<CompositeField>();

      O << N->get_name() << ": ";
      automaton_recurse(N->get_type(), S, O);

      if (N->get_value()) {
        O << " = ";
        automaton_recurse(N->get_value(), S, O);
      }

      O << ",";

      break;
    }

    case QAST_NODE_BLOCK: {
      static const std::unordered_set<qparse_ty_t> no_has_semicolon = {
          QAST_NODE_FN,
      };

      static const std::unordered_set<qparse_ty_t> double_sep = {
          QAST_NODE_FNDECL, QAST_NODE_STRUCT,    QAST_NODE_REGION,
          QAST_NODE_GROUP,  QAST_NODE_UNION,     QAST_NODE_ENUM,
          QAST_NODE_FN,     QAST_NODE_SUBSYSTEM, QAST_NODE_EXPORT,
      };

      Block* N = C->as<Block>();

      bool did_root = S.did_root;
      S.did_root = true;

      if (did_root && N->get_items().empty()) {
        O << "{}";
        break;
      }

      if (did_root) {
        O << "{\n";
        S.bra_depth++;
      }

      for (auto it = N->get_items().begin(); it != N->get_items().end(); ++it) {
        qparse_ty_t ty = (*it)->this_typeid();

        put_indent(S, O);
        automaton_recurse(*it, S, O);
        if (!no_has_semicolon.contains(ty)) {
          O << ";";
        }

        bool do_double_line =
            double_sep.contains((*it)->this_typeid()) ||
            (std::next(it) != N->get_items().end() && (*std::next(it))->this_typeid() != ty);

        if (std::next(it) != N->get_items().end() && do_double_line) {
          O << "\n\n";
        } else {
          O << "\n";
        }
      }

      if (did_root) {
        S.bra_depth--;
        put_indent(S, O);
        O << "}";
      }

      break;
    }

    case QAST_NODE_CONST: {
      ConstDecl* N = C->as<ConstDecl>();
      O << "const " << N->get_name();
      if (N->get_type()) {
        O << ": ";
        automaton_recurse(N->get_type(), S, O);
      }
      if (N->get_value()) {
        O << " = ";
        automaton_recurse(N->get_value(), S, O);
      }
      break;
    }

    case QAST_NODE_VAR: {
      VarDecl* N = C->as<VarDecl>();
      O << "var " << N->get_name();
      if (N->get_type()) {
        O << ": ";
        automaton_recurse(N->get_type(), S, O);
      }
      if (N->get_value()) {
        O << " = ";
        automaton_recurse(N->get_value(), S, O);
      }
      break;
    }

    case QAST_NODE_LET: {
      LetDecl* N = C->as<LetDecl>();
      O << "let " << N->get_name();
      if (N->get_type()) {
        O << ": ";
        automaton_recurse(N->get_type(), S, O);
      }
      if (N->get_value()) {
        O << " = ";
        automaton_recurse(N->get_value(), S, O);
      }
      break;
    }

    case QAST_NODE_INLINE_ASM: {
      qcore_panic("Asm format is not implemented");
      break;
    }

    case QAST_NODE_RETURN: {
      ReturnStmt* N = C->as<ReturnStmt>();

      O << "ret";
      if (N->get_value()) {
        O << " ";
        automaton_recurse(N->get_value(), S, O);
      }

      break;
    }

    case QAST_NODE_RETIF: {
      ReturnIfStmt* N = C->as<ReturnIfStmt>();

      O << "retif";
      automaton_recurse(N->get_cond(), S, O);
      O << ", ";
      automaton_recurse(N->get_value(), S, O);

      break;
    }

    case QAST_NODE_RETZ: {
      RetZStmt* N = C->as<RetZStmt>();

      O << "retz";
      automaton_recurse(N->get_cond(), S, O);
      O << ", ";
      automaton_recurse(N->get_value(), S, O);

      break;
    }

    case QAST_NODE_RETV: {
      RetVStmt* N = C->as<RetVStmt>();

      O << "retv";
      automaton_recurse(N->get_cond(), S, O);

      break;
    }

    case QAST_NODE_BREAK: {
      O << "break";
      break;
    }

    case QAST_NODE_CONTINUE: {
      O << "continue";
      break;
    }

    case QAST_NODE_IF: {
      IfStmt* N = C->as<IfStmt>();

      O << "if ";
      automaton_recurse(N->get_cond(), S, O);

      if (N->get_then()->get_items().size() == 1) {
        O << " => ";
        automaton_recurse(N->get_then()->get_items().front(), S, O);
        if (N->get_else()) {
          O << ";";
        }
      } else {
        O << " ";
        automaton_recurse(N->get_then(), S, O);
      }

      if (N->get_else()) {
        if (N->get_else()->get_items().size() == 1) {
          O << " else => ";
          automaton_recurse(N->get_else()->get_items().front(), S, O);
        } else {
          O << " else ";
          automaton_recurse(N->get_else(), S, O);
        }
      }

      break;
    }

    case QAST_NODE_WHILE: {
      WhileStmt* N = C->as<WhileStmt>();
      O << "while ";
      automaton_recurse(N->get_cond(), S, O);
      O << " ";

      if (N->get_body()->get_items().size() == 1) {
        O << "=> ";
        automaton_recurse(N->get_body()->get_items().front(), S, O);
      } else {
        automaton_recurse(N->get_body(), S, O);
      }
      break;
    }

    case QAST_NODE_FOR: {
      ForStmt* N = C->as<ForStmt>();
      O << "for (";

      automaton_recurse(N->get_init(), S, O);
      O << ";";
      if (N->get_init()) O << " ";

      automaton_recurse(N->get_cond(), S, O);
      O << ";";
      if (N->get_cond()) O << " ";

      automaton_recurse(N->get_step(), S, O);

      O << ") ";

      if (N->get_body()->get_items().size() == 1) {
        O << "=> ";
        automaton_recurse(N->get_body()->get_items().front(), S, O);
      } else {
        automaton_recurse(N->get_body(), S, O);
      }
      break;
    }

    case QAST_NODE_FORM: {
      FormStmt* N = C->as<FormStmt>();
      O << "form (";
      automaton_recurse(N->get_maxjobs(), S, O);
      O << ") (" << N->get_idx_ident() << ", " << N->get_val_ident() << " in ";
      automaton_recurse(N->get_expr(), S, O);

      O << ") ";

      if (N->get_body()->get_items().size() == 1) {
        O << "=> ";
        automaton_recurse(N->get_body()->get_items().front(), S, O);
      } else {
        automaton_recurse(N->get_body(), S, O);
      }
      break;
    }

    case QAST_NODE_FOREACH: {
      ForeachStmt* N = C->as<ForeachStmt>();
      O << "foreach (" << N->get_idx_ident() << ", " << N->get_val_ident() << " in ";
      automaton_recurse(N->get_expr(), S, O);

      O << ") ";

      if (N->get_body()->get_items().size() == 1) {
        O << "=> ";
        automaton_recurse(N->get_body()->get_items().front(), S, O);
      } else {
        automaton_recurse(N->get_body(), S, O);
      }
      break;
    }

    case QAST_NODE_CASE: {
      CaseStmt* N = C->as<CaseStmt>();
      O << "case ";
      automaton_recurse(N->get_cond(), S, O);
      O << ": ";
      automaton_recurse(N->get_body(), S, O);
      break;
    }

    case QAST_NODE_SWITCH: {
      SwitchStmt* N = C->as<SwitchStmt>();
      O << "switch ";
      automaton_recurse(N->get_cond(), S, O);
      if (N->get_cases().empty() && !N->get_default()) {
        O << " {}";
      } else {
        O << " {\n";
        S.bra_depth++;
        for (auto& stmt : N->get_cases()) {
          put_indent(S, O);
          automaton_recurse(stmt, S, O);
          O << "\n";
        }
        if (N->get_default()) {
          put_indent(S, O);
          O << "default: ";
          automaton_recurse(N->get_default(), S, O);
          O << "\n";
        }
        S.bra_depth--;
        put_indent(S, O);
        O << "}";
      }
      break;
    }

    case QAST_NODE_EXPR_STMT: {
      automaton_recurse(C->as<ExprStmt>()->get_expr(), S, O);
      break;
    }

    case QAST_NODE_VOLSTMT: {
      VolStmt* N = C->as<VolStmt>();
      O << "volatile ";
      automaton_recurse(N->get_stmt(), S, O);
      break;
    }
  }
}

static void do_formatting(const lsp::RequestMessage& req, lsp::ResponseMessage& resp) {
  if (!req.params().HasMember("textDocument")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing textDocument");
    return;
  }

  if (!req.params()["textDocument"].IsObject()) {
    resp.error(lsp::ErrorCodes::InvalidParams, "textDocument is not an object");
    return;
  }

  if (!req.params()["textDocument"].HasMember("uri")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing textDocument.uri");
    return;
  }

  if (!req.params()["textDocument"]["uri"].IsString()) {
    resp.error(lsp::ErrorCodes::InvalidParams, "textDocument.uri is not a string");
    return;
  }

  if (!req.params().HasMember("options")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing options");
    return;
  }

  if (!req.params()["options"].IsObject()) {
    resp.error(lsp::ErrorCodes::InvalidParams, "options is not an object");
    return;
  }

  if (!req.params()["options"].HasMember("tabSize")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing options.tabSize");
    return;
  }

  if (!req.params()["options"]["tabSize"].IsInt()) {
    resp.error(lsp::ErrorCodes::InvalidParams, "options.tabSize is not an integer");
    return;
  }

  if (!req.params()["options"].HasMember("insertSpaces")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing options.insertSpaces");
    return;
  }

  if (!req.params()["options"]["insertSpaces"].IsBool()) {
    resp.error(lsp::ErrorCodes::InvalidParams, "options.insertSpaces is not a boolean");
    return;
  }

  FormattingOptions options;
  options.tabSize = req.params()["options"]["tabSize"].GetInt();
  options.insertSpaces = req.params()["options"]["insertSpaces"].GetBool();

  std::string uri = req.params()["textDocument"]["uri"].GetString();
  SyncFS::the().select_uri(uri);
  SyncFS::the().wait_for_open();

  std::string text_content;
  if (!SyncFS::the().read_current(text_content)) {
    resp.error(lsp::ErrorCodes::InternalError, "Failed to read file");
    return;
  }

  auto ss = std::make_shared<std::stringstream>(std::move(text_content));

  qcore_env env;
  qlex lexer(ss, uri.c_str(), env.get());
  qparse_conf conf;
  qparser parser(lexer.get(), conf.get(), env.get());

  qparse_node_t* root = nullptr;
  if (!qparse_do(parser.get(), &root)) {
    return;
  }

  /// FIXME: Re-enable checking once checking is fixed
  // if (!qparse_check(parser.get(), root)) {
  //   return;
  // }

  LOG(INFO) << "Requested document format";

  std::stringstream formatted_text;
  AutomatonState S;

  automaton_recurse(static_cast<qparse::Node*>(root), S, formatted_text);

  ///==========================================================
  /// Send the whole new file contents

  resp->SetArray();
  Value edit(kObjectType);
  edit.AddMember("range", Value(kObjectType), resp->GetAllocator());
  edit["range"].AddMember("start", Value(kObjectType), resp->GetAllocator());
  edit["range"]["start"].AddMember("line", 0, resp->GetAllocator());
  edit["range"]["start"].AddMember("character", 0, resp->GetAllocator());
  edit["range"].AddMember("end", Value(kObjectType), resp->GetAllocator());
  edit["range"]["end"].AddMember("line", SIZE_MAX, resp->GetAllocator());
  edit["range"]["end"].AddMember("character", SIZE_MAX, resp->GetAllocator());
  std::string new_text = formatted_text.str();
  edit.AddMember("newText", Value(new_text.c_str(), new_text.size(), resp->GetAllocator()).Move(),
                 resp->GetAllocator());

  resp->PushBack(edit, resp->GetAllocator());

  ///==========================================================

  return;
}

ADD_REQUEST_HANDLER("textDocument/formatting", do_formatting);
