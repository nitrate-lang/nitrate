#include <nitrate-parser/Node.h>
#include <rapidjson/document.h>

#include <cctype>
#include <cstdint>
#include <lsp/core/SyncFS.hh>
#include <lsp/core/server.hh>
#include <lsp/route/RoutesList.hh>
#include <memory>
#include <nitrate-core/Classes.hh>
#include <nitrate-lexer/Classes.hh>
#include <nitrate-parser/Classes.hh>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>

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
public:
  size_t indent = 0;
  bool did_root = false;
  size_t tabSize = 0;
  std::stack<size_t> field_indent_stack;

  std::stringstream S;
  std::stringstream line;

  void flush_line() {
    S << line.str();
    line.str("");
  }

  AutomatonState(size_t theTabSize) {
    tabSize = theTabSize;
    field_indent_stack.push(1);
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

static void escape_string_literal_chunk(AutomatonState& S,
                                        std::string_view str) {
  for (char ch : str) {
    switch (ch) {
      case '\n':
        S.line << "\\n";
        break;
      case '\t':
        S.line << "\\t";
        break;
      case '\r':
        S.line << "\\r";
        break;
      case '\v':
        S.line << "\\v";
        break;
      case '\f':
        S.line << "\\f";
        break;
      case '\b':
        S.line << "\\b";
        break;
      case '\a':
        S.line << "\\a";
        break;
      case '\\':
        S.line << "\\\\";
        break;
      case '"':
        S.line << "\\\"";
        break;
      default:
        S.line << ch;
        break;
    }
  }
}

static void escape_string_literal(AutomatonState& S, std::string_view str) {
  constexpr size_t max_chunk_size = 60;

  if (str.empty()) {
    S.line << "\"\"";
  }

  size_t num_chunks = str.size() / max_chunk_size;
  size_t rem = str.size() % max_chunk_size;

  S.line.seekg(0, std::ios::end);
  size_t line_size = S.line.tellg();

  for (size_t i = 0; i < num_chunks; i++) {
    S.line << "\"";
    escape_string_literal_chunk(S,
                                str.substr(i * max_chunk_size, max_chunk_size));
    S.line << "\"";

    if (rem > 0 || i < num_chunks - 1) {
      S.line << " \\\n";
      S.flush_line();
      if (line_size) {
        S.line << std::string(line_size, ' ');
      }
    }
  }

  if (rem > 0) {
    S.line << "\"";
    escape_string_literal_chunk(S,
                                str.substr(num_chunks * max_chunk_size, rem));
    S.line << "\"";
  }
}

static void write_float_literal_chunk(AutomatonState& S,
                                      std::string_view float_str) {
  constexpr size_t insert_sep_every = 10;

  bool already_write_type_suffix = false;

  for (size_t i = 0; i < float_str.size(); i++) {
    bool underscore = false;

    if (!already_write_type_suffix && i != 0 && (i % (insert_sep_every)) == 0) {
      underscore = true;
    } else if (!already_write_type_suffix && !std::isdigit(float_str[i]) &&
               float_str[i] != '.') {
      already_write_type_suffix = true;
      underscore = true;
    }

    if (underscore) {
      S.line << "_";
    }

    S.line << float_str[i];
  }
}

static void write_float_literal(AutomatonState& S, std::string_view float_str) {
  constexpr size_t max_chunk_size = 50;

  if (float_str.empty()) {
    S.line << "";
  }

  size_t num_chunks = float_str.size() / max_chunk_size;
  size_t rem = float_str.size() % max_chunk_size;

  S.line.seekg(0, std::ios::end);
  size_t line_size = S.line.tellg();

  for (size_t i = 0; i < num_chunks; i++) {
    write_float_literal_chunk(
        S, float_str.substr(i * max_chunk_size, max_chunk_size));

    if (rem > 0 || i < num_chunks - 1) {
      S.line << "_ \\\n";
      S.flush_line();
      if (line_size) {
        S.line << std::string(line_size, ' ');
      }
    }
  }

  if (rem > 0) {
    write_float_literal_chunk(
        S, float_str.substr(num_chunks * max_chunk_size, rem));
  }
}

static void put_indent(AutomatonState& S) {
  if (S.indent) {
    S.line << std::string(S.indent * S.tabSize, ' ');
  }
}

static void recurse(npar_node_t* C, AutomatonState& S);

static void put_type_metadata(npar::Type* N, AutomatonState& S) {
  auto range = N->get_range();

  if (range.first || range.second) {
    S.line << ": [";
    recurse(range.first, S);
    S.line << ":";
    recurse(range.second, S);
    S.line << "]";
  }

  if (N->get_width()) {
    S.line << ": ";
    recurse(N->get_width(), S);
  }
}

template <typename T>
static void put_composite_defintion(T* N, AutomatonState& S) {
  using namespace npar;

  static const std::unordered_map<Vis, std::string_view> vis_str = {
      {Vis::PRIVATE, "sec"},
      {Vis::PUBLIC, "pub"},
      {Vis::PROTECTED, "pro"},
  };

  S.line << " " << N->get_name() << " ";

  bool has_fields = !N->get_fields().empty();
  bool has_methods = !N->get_methods().empty();
  bool has_static_methods = !N->get_static_methods().empty();

  bool empty = !has_fields && !has_methods && !has_static_methods &&
               N->get_tags().empty();
  if (empty) {
    S.line << "{}";
    return;
  }

  S.line << "{\n";
  S.flush_line();
  S.indent++;

  for (auto it = N->get_fields().begin(); it != N->get_fields().end(); it++) {
    put_indent(S);

    S.line << vis_str.at((*it)->get_visibility()) << " ";
    recurse(*it, S);
    S.line << "\n";
    S.flush_line();
  }

  if (has_methods) {
    if (has_fields) {
      S.line << "\n";
      S.flush_line();
    }

    for (auto it = N->get_methods().begin(); it != N->get_methods().end();
         it++) {
      put_indent(S);

      S.line << vis_str.at((*it)->get_visibility()) << " ";
      recurse(*it, S);
      S.line << ";\n";
      S.flush_line();
    }
  }

  if (has_static_methods) {
    if (has_fields || has_methods) {
      S.line << "\n";
      S.flush_line();
    }

    for (auto it = N->get_static_methods().begin();
         it != N->get_static_methods().end(); it++) {
      put_indent(S);

      S.line << vis_str.at((*it)->get_visibility()) << " static ";
      recurse(*it, S);
      S.line << ";\n";
      S.flush_line();
    }
  }

  S.indent--;
  put_indent(S);
  S.line << "}";
}

// static void recurse(npar_node_t* C, AutomatonState& S) {
//   /**
//    * TODO: Resolve the following issues:
//    * - Parentheses are currently lost and the order of sub-expressions is not
//    * preserved;
//    * - Integer literals loose their original base.
//    * - Code comments are lost
//    * - Code with macros is not supported
//    * - Maximum line width is not supported at all (might me okay)
//    */

//   using namespace npar;

//   if (!C) {
//     return;
//   }

//   switch (C->getKind()) {
//     case QAST_NODE_NODE: {
//       break;
//     }

//     case QAST_NODE_BINEXPR: {
//       BinExpr* N = C->as<BinExpr>();
//       S.line << "(";
//       recurse(N->get_lhs(), S);
//       S.line << " " << N->get_op() << " ";
//       recurse(N->get_rhs(), S);
//       S.line << ")";
//       break;
//     }

//     case QAST_NODE_UNEXPR: {
//       UnaryExpr* N = C->as<UnaryExpr>();
//       S.line << "(";
//       S.line << N->get_op();
//       switch (N->get_op()) {
//         case qOpSizeof:
//         case qOpBitsizeof:
//         case qOpAlignof:
//         case qOpTypeof:
//           S.line << "(";
//           recurse(N->get_rhs(), S);
//           S.line << ")";
//           break;
//         default:
//           recurse(N->get_rhs(), S);
//           break;
//       }
//       S.line << ")";
//       break;
//     }

//     case QAST_NODE_TEREXPR: {
//       TernaryExpr* N = C->as<TernaryExpr>();
//       S.line << "(";
//       recurse(N->get_cond(), S);
//       S.line << " ? ";
//       recurse(N->get_lhs(), S);
//       S.line << " : ";
//       recurse(N->get_rhs(), S);
//       S.line << ")";
//       break;
//     }

//     case QAST_NODE_INT: {
//       S.line << C->as<ConstInt>()->get_value();
//       break;
//     }

//     case QAST_NODE_FLOAT: {
//       write_float_literal(S, C->as<ConstFloat>()->get_value());
//       break;
//     }

//     case QAST_NODE_STRING: {
//       escape_string_literal(S, C->as<ConstString>()->get_value());
//       break;
//     }

//     case QAST_NODE_CHAR: {
//       ConstChar* N = C->as<ConstChar>();
//       S.line << escape_char_literal(N->get_value());
//       break;
//     }

//     case QAST_NODE_BOOL: {
//       if (C->as<ConstBool>()->get_value()) {
//         S.line << "true";
//       } else {
//         S.line << "false";
//       }
//       break;
//     }

//     case QAST_NODE_NULL: {
//       S.line << "null";
//       break;
//     }

//     case QAST_NODE_UNDEF: {
//       S.line << "undef";
//       break;
//     }

//     case QAST_NODE_CALL: {
//       Call* N = C->as<Call>();

//       recurse(N->get_func(), S);

//       S.line << "(";

//       S.line.seekg(0, std::ios::end);
//       size_t line_size = S.line.tellg();

//       size_t split_on = -1, i = 0;

//       if (std::any_of(N->get_args().begin(), N->get_args().end(),
//                       [](auto x) { return !std::isdigit(x.first.at(0)); })) {
//         split_on = 1;
//       } else if (N->get_args().size() > 6) {
//         split_on = std::ceil(std::sqrt(N->get_args().size()));
//       }

//       for (auto it = N->get_args().begin(); it != N->get_args().end(); it++)
//       {
//         bool is_call = it->second->getKind() == QAST_NODE_CALL;
//         bool is_other = it->second->getKind() == QAST_NODE_LIST ||
//                         (it->second->getKind() == QAST_NODE_STMT_EXPR &&
//                          (it->second->as<StmtExpr>()->get_stmt()->getKind()
//                          ==
//                           QAST_NODE_FN));

//         S.line.seekg(0, std::ios::end);
//         size_t line_width = S.line.tellg();

//         if (!is_call && !is_other) {
//           S.field_indent_stack.push(line_width);
//         } else if (is_call) {
//           S.field_indent_stack.push(line_width - S.tabSize);
//         } else if (is_other) {
//           S.indent++;
//         }

//         if (!std::isdigit(it->first.at(0))) {
//           S.line << it->first;
//           S.line << ": ";
//         }
//         recurse(it->second, S);
//         i++;

//         if (std::next(it) != N->get_args().end()) {
//           S.line << ",";

//           if (i % split_on == 0) {
//             S.line << "\n";
//             S.flush_line();
//             S.line << std::string(line_size, ' ');
//           } else {
//             S.line << " ";
//           }
//         }
//         if (!is_other) {
//           S.field_indent_stack.pop();
//         } else {
//           S.indent--;
//         }
//       }
//       S.line << ")";

//       break;
//     }

//     case QAST_NODE_LIST: {
//       List* N = C->as<List>();
//       if (N->get_items().empty()) {
//         S.line << "[]";
//         break;
//       }

//       auto ty = N->get_items().front()->getKind();
//       if (N->get_items().size() > 0 &&
//           (ty == QAST_NODE_ASSOC || ty == QAST_NODE_LIST ||
//            ty == QAST_NODE_CALL || ty == QAST_NODE_TEMPL_CALL ||
//            (ty == QAST_NODE_STMT_EXPR &&
//             N->get_items().front()->as<StmtExpr>()->get_stmt()->getKind() ==
//                 QAST_NODE_FN))) {
//         S.line << "[\n";
//         S.flush_line();
//         S.indent++;
//         for (auto it = N->get_items().begin(); it != N->get_items().end();
//              it++) {
//           put_indent(S);
//           recurse(*it, S);
//           S.line << ",\n";
//           S.flush_line();
//         }
//         S.indent--;
//         put_indent(S);
//         S.line << "]";
//       } else {
//         size_t split_on = N->get_items().size() <= 8
//                               ? 8
//                               : std::ceil(std::sqrt(N->get_items().size()));

//         S.line << "[";

//         S.line.seekg(0, std::ios::end);
//         size_t line_size = S.line.tellg();

//         size_t i = 0;
//         for (auto it = N->get_items().begin(); it != N->get_items().end();
//              it++) {
//           recurse(*it, S);

//           i++;
//           if (std::next(it) != N->get_items().end()) {
//             S.line << ",";
//             if (i % split_on == 0) {
//               S.line << "\n";
//               S.flush_line();
//               S.line << std::string(line_size, ' ');
//             } else {
//               S.line << " ";
//             }
//           }
//         }
//         S.line << "]";
//       }
//       break;
//     }

//     case QAST_NODE_ASSOC: {
//       Assoc* N = C->as<Assoc>();
//       S.line << "{";
//       recurse(N->get_key(), S);
//       S.line << ": ";
//       recurse(N->get_value(), S);
//       S.line << "}";
//       break;
//     }

//     case QAST_NODE_FIELD: {
//       Field* N = C->as<Field>();
//       bool break_chain_call = false;

//       if (N->get_base()->getKind() == QAST_NODE_CALL) {
//         break_chain_call = true;
//       }

//       recurse(N->get_base(), S);

//       if (break_chain_call) {
//         S.line << "\n";
//         S.flush_line();
//         qcore_assert(S.field_indent_stack.top() > 0);
//         S.line << std::string(
//             (S.indent * S.tabSize) + S.field_indent_stack.top(), ' ');
//       } else {
//         S.line.seekg(0, std::ios::end);
//         size_t line_width = S.line.tellg();

//         S.field_indent_stack.top() = line_width;
//       }

//       S.line << "." << N->get_field();

//       break;
//     }

//     case QAST_NODE_INDEX: {
//       Index* N = C->as<Index>();
//       recurse(N->get_base(), S);
//       S.line << "[";
//       recurse(N->get_index(), S);
//       S.line << "]";
//       break;
//     }

//     case QAST_NODE_SLICE: {
//       Slice* N = C->as<Slice>();
//       recurse(N->get_base(), S);
//       S.line << "[";
//       recurse(N->get_start(), S);
//       S.line << ":";
//       recurse(N->get_end(), S);
//       S.line << "]";
//       break;
//     }

//     case QAST_NODE_FSTRING: {
//       FString* N = C->as<FString>();
//       S.line << "f\"";
//       for (auto it = N->get_items().begin(); it != N->get_items().end();
//       it++) {
//         if (std::holds_alternative<String>(*it)) {
//           escape_string_literal_chunk(S, std::get<String>(*it));
//         } else {
//           S.line << "{";
//           std::stringstream swap_line, swap_buffer;
//           std::swap(S.line, swap_line);
//           std::swap(S.S, swap_buffer);

//           recurse(std::get<Expr*>(*it), S);

//           std::swap(S.line, swap_line);
//           std::swap(S.S, swap_buffer);

//           swap_buffer << swap_line.str();

//           escape_string_literal_chunk(S, swap_buffer.str());
//           S.line << "}";
//         }
//       }
//       S.line << "\"";
//       break;
//     }

//     case QAST_NODE_IDENT: {
//       S.line << C->as<Ident>()->get_name();
//       break;
//     }

//     case QAST_NODE_SEQ: {
//       SeqPoint* N = C->as<SeqPoint>();

//       S.line << "(";
//       for (auto it = N->get_items().begin(); it != N->get_items().end();
//       it++) {
//         recurse(*it, S);

//         if (std::next(it) != N->get_items().end()) {
//           S.line << ", ";
//         }
//       }
//       S.line << ")";
//       break;
//     }

//     case QAST_NODE_POST_UNEXPR: {
//       PostUnaryExpr* N = C->as<PostUnaryExpr>();
//       S.line << "(";
//       recurse(N->get_lhs(), S);
//       S.line << N->get_op() << ")";
//       break;
//     }

//     case QAST_NODE_STMT_EXPR: {
//       recurse(C->as<StmtExpr>()->get_stmt(), S);
//       break;
//     }

//     case QAST_NODE_TYPE_EXPR: {
//       recurse(C->as<TypeExpr>()->get_type(), S);
//       break;
//     }

//     case QAST_NODE_TEMPL_CALL: {
//       TemplCall* N = C->as<TemplCall>();

//       recurse(N->get_func(), S);

//       S.line << "<";
//       for (auto it = N->get_template_args().begin();
//            it != N->get_template_args().end(); it++) {
//         if (!std::isdigit(it->first.at(0))) {
//           S.line << it->first;
//           S.line << ": ";
//         }
//         recurse(it->second, S);

//         if (std::next(it) != N->get_template_args().end()) {
//           S.line << ", ";
//         }
//       }
//       S.line << ">";

//       S.line << "(";
//       for (auto it = N->get_args().begin(); it != N->get_args().end(); it++)
//       {
//         if (!std::isdigit(it->first.at(0))) {
//           S.line << it->first;
//           S.line << ": ";
//         }
//         recurse(it->second, S);

//         if (std::next(it) != N->get_args().end()) {
//           S.line << ", ";
//         }
//       }
//       S.line << ")";

//       break;
//     }

//     case QAST_NODE_REF_TY: {
//       S.line << "&";
//       recurse(C->as<RefTy>()->get_item(), S);
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_U1_TY: {
//       S.line << "u1";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_U8_TY: {
//       S.line << "u8";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_U16_TY: {
//       S.line << "u16";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_U32_TY: {
//       S.line << "u32";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_U64_TY: {
//       S.line << "u64";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_U128_TY: {
//       S.line << "u128";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_I8_TY: {
//       S.line << "i8";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_I16_TY: {
//       S.line << "i16";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_I32_TY: {
//       S.line << "i32";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_I64_TY: {
//       S.line << "i64";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_I128_TY: {
//       S.line << "i128";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_F16_TY: {
//       S.line << "f16";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_F32_TY: {
//       S.line << "f32";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_F64_TY: {
//       S.line << "f64";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_F128_TY: {
//       S.line << "f128";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_VOID_TY: {
//       S.line << "void";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_PTR_TY: {
//       S.line << "*";
//       recurse(C->as<PtrTy>()->get_item(), S);
//       break;
//     }

//     case QAST_NODE_OPAQUE_TY: {
//       S.line << "opaque(" << C->as<OpaqueTy>()->get_name() << ")";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_STRUCT_TY: {
//       qcore_panic("Unreachable");
//       break;
//     }

//     case QAST_NODE_ARRAY_TY: {
//       ArrayTy* N = C->as<ArrayTy>();
//       S.line << "[";
//       recurse(N->get_item(), S);
//       S.line << "; ";
//       recurse(N->get_size(), S);
//       S.line << "]";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_TUPLE_TY: {
//       TupleTy* N = C->as<TupleTy>();

//       S.line << "(";
//       for (auto it = N->get_items().begin(); it != N->get_items().end();
//       ++it) {
//         recurse(*it, S);

//         if (std::next(it) != N->get_items().end()) {
//           S.line << ", ";
//         }
//       }
//       S.line << ")";

//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_FN_TY: {
//       static const std::unordered_map<FuncPurity, std::string> purity_str = {
//           {FuncPurity::IMPURE_THREAD_UNSAFE, ""},
//           {FuncPurity::IMPURE_THREAD_SAFE, " tsafe"},
//           {FuncPurity::PURE, " pure"},
//           {FuncPurity::QUASIPURE, " quasipure"},
//           {FuncPurity::RETROPURE, " retropure"}};

//       FuncTy* N = C->as<FuncTy>();

//       std::string props;
//       props += purity_str.at(N->get_purity());

//       if (N->is_noexcept()) {
//         props += " noexcept";
//       }

//       if (N->is_foreign()) {
//         props += " foreign";
//       }

//       if (N->is_crashpoint()) {
//         props += " crashpoint";
//       }

//       S.line << "fn";
//       if (!props.empty()) {
//         S.line << std::move(props);
//       }

//       S.line << "(";
//       for (auto it = N->get_params().begin(); it != N->get_params().end();
//            it++) {
//         S.line << std::get<0>(*it);
//         auto param_ty = std::get<1>(*it);
//         if ((param_ty && param_ty->getKind() != QAST_NODE_INFER_TY) ||
//             std::get<2>(*it)) {
//           S.line << ": ";
//           recurse(std::get<1>(*it), S);
//           if (std::get<2>(*it)) {
//             S.line << " = ";
//             recurse(std::get<2>(*it), S);
//           }
//         }

//         if (std::next(it) != N->get_params().end() || N->is_variadic()) {
//           S.line << ", ";
//         }
//       }

//       if (N->is_variadic()) {
//         S.line << "...";
//       }

//       S.line << ")";

//       if (N->get_return_ty() &&
//           N->get_return_ty()->getKind() != QAST_NODE_VOID_TY) {
//         S.line << ": ";
//         recurse(N->get_return_ty(), S);
//       }

//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_UNRES_TY: {
//       S.line << C->as<NamedTy>()->get_name();
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_INFER_TY: {
//       S.line << "?";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_TEMPL_TY: {
//       TemplType* N = C->as<TemplType>();

//       if (N->get_template()->getKind() == QAST_NODE_UNRES_TY) {
//         auto name = N->get_template()->as<NamedTy>()->get_name();

//         if (name == "__builtin_result") {
//           qcore_assert(N->get_args().size() == 1);
//           recurse(N->get_args().front(), S);
//           S.line << "?";

//           put_type_metadata(C->as<npar::Type>(), S);
//           break;
//         } else if (name == "__builtin_vec") {
//           qcore_assert(N->get_args().size() == 1);

//           S.line << "[";
//           recurse(N->get_args().front(), S);
//           S.line << "]";

//           put_type_metadata(C->as<npar::Type>(), S);
//           break;
//         } else if (name == "__builtin_uset") {
//           qcore_assert(N->get_args().size() == 1);

//           S.line << "{";
//           recurse(N->get_args().front(), S);
//           S.line << "}";

//           put_type_metadata(C->as<npar::Type>(), S);
//           break;
//         } else if (name == "__builtin_umap") {
//           qcore_assert(N->get_args().size() == 2);

//           S.line << "[";
//           recurse(N->get_args()[0], S);
//           S.line << "->";
//           recurse(N->get_args()[1], S);
//           S.line << "]";

//           put_type_metadata(C->as<npar::Type>(), S);
//           break;
//         }
//       }

//       recurse(N->get_template(), S);
//       S.line << "<";
//       for (auto it = N->get_args().begin(); it != N->get_args().end(); ++it)
//       {
//         recurse(*it, S);

//         if (std::next(it) != N->get_args().end()) {
//           S.line << ", ";
//         }
//       }
//       S.line << ">";
//       put_type_metadata(C->as<npar::Type>(), S);
//       break;
//     }

//     case QAST_NODE_TYPEDEF: {
//       TypedefDecl* N = C->as<TypedefDecl>();
//       S.line << "type " << N->get_name() << " = ";
//       recurse(N->get_type(), S);
//       break;
//     }

//     case QAST_NODE_FNDECL: {
//       static const std::unordered_map<FuncPurity, std::string> purity_str = {
//           {FuncPurity::IMPURE_THREAD_UNSAFE, ""},
//           {FuncPurity::IMPURE_THREAD_SAFE, " tsafe"},
//           {FuncPurity::PURE, " pure"},
//           {FuncPurity::QUASIPURE, " quasipure"},
//           {FuncPurity::RETROPURE, " retropure"}};

//       FnDecl* N = C->as<FnDecl>();
//       FuncTy* F = N->get_type();
//       if (!F) {
//         break;
//       }

//       std::string props;
//       props += purity_str.at(F->get_purity());

//       if (F->is_noexcept()) {
//         props += " noexcept";
//       }

//       if (F->is_foreign()) {
//         props += " foreign";
//       }

//       if (F->is_crashpoint()) {
//         props += " crashpoint";
//       }

//       if (!N->get_name().empty()) {
//         props += " " + N->get_name();
//       }

//       S.line << "fn";
//       if (!props.empty()) {
//         S.line << std::move(props);
//       }

//       S.line << "(";
//       for (auto it = F->get_params().begin(); it != F->get_params().end();
//            it++) {
//         if (std::get<0>(*it) == "this") {
//           continue;
//         }

//         S.line << std::get<0>(*it);
//         auto param_ty = std::get<1>(*it);
//         if ((param_ty && param_ty->getKind() != QAST_NODE_INFER_TY) ||
//             std::get<2>(*it)) {
//           S.line << ": ";
//           recurse(std::get<1>(*it), S);
//           if (std::get<2>(*it)) {
//             S.line << " = ";
//             recurse(std::get<2>(*it), S);
//           }
//         }

//         if (std::next(it) != F->get_params().end() || F->is_variadic()) {
//           S.line << ", ";
//         }
//       }

//       if (F->is_variadic()) {
//         S.line << "...";
//       }

//       S.line << ")";

//       if (F->get_return_ty() &&
//           F->get_return_ty()->getKind() != QAST_NODE_VOID_TY) {
//         S.line << ": ";
//         recurse(F->get_return_ty(), S);
//       }

//       break;
//     }

//     case QAST_NODE_STRUCT: {
//       S.line << "struct";
//       put_composite_defintion(C->as<StructDef>(), S);
//       break;
//     }

//     case QAST_NODE_ENUM: {
//       EnumDef* N = C->as<EnumDef>();
//       S.line << "enum " << N->get_name();

//       if (N->get_type()) {
//         S.line << ": ";
//         recurse(N->get_type(), S);
//         S.line << " ";
//       } else {
//         S.line << " ";
//       }

//       if (N->get_items().empty()) {
//         S.line << "{}";
//         break;
//       }

//       S.line << "{\n";
//       S.flush_line();
//       S.indent++;

//       for (auto it = N->get_items().begin(); it != N->get_items().end();
//       it++) {
//         put_indent(S);

//         S.line << it->first;
//         if (it->second) {
//           S.line << " = ";
//           recurse(it->second, S);
//         }

//         S.line << ",\n";
//         S.flush_line();
//       }

//       S.indent--;
//       put_indent(S);
//       S.line << "}";

//       break;
//     }

//     case QAST_NODE_FN: {
//       static const std::unordered_map<FuncPurity, std::string> purity_str = {
//           {FuncPurity::IMPURE_THREAD_UNSAFE, ""},
//           {FuncPurity::IMPURE_THREAD_SAFE, " tsafe"},
//           {FuncPurity::PURE, " pure"},
//           {FuncPurity::QUASIPURE, " quasipure"},
//           {FuncPurity::RETROPURE, " retropure"}};

//       FnDef* N = C->as<FnDef>();
//       FuncTy* F = N->get_type();
//       if (!F) {
//         break;
//       }

//       std::string props;
//       props += purity_str.at(F->get_purity());

//       if (F->is_noexcept()) {
//         props += " noexcept";
//       }

//       if (F->is_foreign()) {
//         props += " foreign";
//       }

//       if (F->is_crashpoint()) {
//         props += " crashpoint";
//       }

//       if (!N->get_name().empty()) {
//         props += " " + N->get_name();
//       }

//       S.line << "fn";
//       if (!props.empty()) {
//         S.line << std::move(props);
//       }

//       S.line << "(";
//       for (auto it = F->get_params().begin(); it != F->get_params().end();
//            it++) {
//         if (std::get<0>(*it) == "this") {
//           continue;
//         }

//         S.line << std::get<0>(*it);
//         auto param_ty = std::get<1>(*it);
//         if ((param_ty && param_ty->getKind() != QAST_NODE_INFER_TY) ||
//             std::get<2>(*it)) {
//           S.line << ": ";
//           recurse(std::get<1>(*it), S);
//           if (std::get<2>(*it)) {
//             S.line << " = ";
//             recurse(std::get<2>(*it), S);
//           }
//         }

//         if (std::next(it) != F->get_params().end() || F->is_variadic()) {
//           S.line << ", ";
//         }
//       }

//       if (F->is_variadic()) {
//         S.line << "...";
//       }

//       S.line << ")";

//       if (F->get_return_ty() &&
//           F->get_return_ty()->getKind() != QAST_NODE_VOID_TY) {
//         S.line << ": ";
//         recurse(F->get_return_ty(), S);
//       }
//       S.line << " ";

//       bool arrow_syntax =
//           (N->get_body()->get_items().size() == 1) && !N->get_precond() &&
//           !N->get_postcond() &&
//           (N->get_body()->get_items()[0]->getKind() == QAST_NODE_RETURN ||
//            N->get_body()->get_items()[0]->getKind() == QAST_NODE_CALL);

//       if (arrow_syntax) {
//         S.line << "=> ";
//         recurse(N->get_body()->get_items().front(), S);
//         S.line << ";";
//       } else {
//         recurse(N->get_body(), S);

//         bool promises = N->get_precond() || N->get_postcond();
//         if (promises) {
//           S.line << " promise {\n";
//           S.flush_line();
//           S.indent++;
//           if (N->get_precond()) {
//             put_indent(S);
//             S.line << "in ";
//             recurse(N->get_precond(), S);
//             S.line << ";\n";
//             S.flush_line();
//           }

//           if (N->get_postcond()) {
//             put_indent(S);
//             S.line << "out ";
//             recurse(N->get_postcond(), S);
//             S.line << ";\n";
//             S.flush_line();
//           }
//           S.indent--;
//           S.line << "}";
//         }
//       }

//       break;
//     }

//     case QAST_NODE_SUBSYSTEM: {
//       ScopeDecl* N = C->as<ScopeDecl>();
//       S.line << "scope " << N->get_name();

//       if (!N->get_deps().empty()) {
//         S.line << ": [";
//         for (auto it = N->get_deps().begin(); it != N->get_deps().end();
//         it++) {
//           S.line << *it;
//           if (std::next(it) != N->get_deps().end()) {
//             S.line << ", ";
//           }
//         }
//         S.line << "]";
//       }

//       S.line << " ";

//       recurse(N->get_body(), S);

//       break;
//     }

//     case QAST_NODE_EXPORT: {
//       ExportDecl* N = C->as<ExportDecl>();
//       std::vector<Stmt*> imports, exports;

//       for (auto it = N->get_body()->get_items().begin();
//            it != N->get_body()->get_items().end(); it++) {
//         npar_ty_t ty = (*it)->getKind();

//         if (ty == QAST_NODE_FNDECL) {
//           imports.push_back(*it);
//         } else if (ty == QAST_NODE_FN) {
//           exports.push_back(*it);
//         } else if (ty == QAST_NODE_VAR) {
//           VarDecl* V = (*it)->as<VarDecl>();
//           V->get_value() ? exports.push_back(*it) : imports.push_back(*it);
//         } else if (ty == QAST_NODE_VAR) {
//           VarDecl* V = (*it)->as<VarDecl>();
//           V->get_value() ? exports.push_back(*it) : imports.push_back(*it);
//         } else if (ty == QAST_NODE_CONST) {
//           ConstDecl* V = (*it)->as<ConstDecl>();
//           V->get_value() ? exports.push_back(*it) : imports.push_back(*it);
//         }
//       }

//       if (imports.size() == 1) {
//         S.line << "import ";
//         if (N->get_abi_name().empty()) {
//           S.line << "\"std\"";
//         } else {
//           escape_string_literal(S, N->get_abi_name());
//         }
//         S.line << " ";

//         recurse(imports.front(), S);
//         S.line << ";";
//       } else if (!imports.empty()) {
//         S.line << "import ";
//         if (N->get_abi_name().empty()) {
//           S.line << "\"std\"";
//         } else {
//           escape_string_literal(S, N->get_abi_name());
//         }
//         S.line << " {\n";
//         S.flush_line();
//         S.indent++;

//         for (auto& stmt : imports) {
//           put_indent(S);
//           recurse(stmt, S);
//           S.line << ";\n";
//           S.flush_line();
//         }

//         S.indent--;
//         put_indent(S);

//         S.line << "}";
//       }

//       if (exports.size() == 1) {
//         S.line << "pub ";
//         if (N->get_abi_name().empty()) {
//           S.line << "\"std\"";
//         } else {
//           escape_string_literal(S, N->get_abi_name());
//         }
//         S.line << " ";

//         recurse(exports.front(), S);
//         if (exports.front()->getKind() != QAST_NODE_FN) {
//           S.line << ";";
//         }
//       } else if (!exports.empty()) {
//         S.line << "pub ";
//         if (N->get_abi_name().empty()) {
//           S.line << "\"std\"";
//         } else {
//           escape_string_literal(S, N->get_abi_name());
//         }
//         S.line << " {\n";
//         S.flush_line();
//         S.indent++;

//         for (auto& stmt : exports) {
//           put_indent(S);
//           recurse(stmt, S);
//           S.line << ";\n";
//           S.flush_line();
//         }

//         S.indent--;
//         put_indent(S);

//         S.line << "}";
//       }

//       break;
//     }

//     case QAST_NODE_STRUCT_FIELD: {
//       StructField* N = C->as<StructField>();

//       S.line << N->get_name() << ": ";
//       recurse(N->get_type(), S);

//       if (N->get_value()) {
//         S.line << " = ";
//         recurse(N->get_value(), S);
//       }

//       S.line << ",";

//       break;
//     }

//     case QAST_NODE_BLOCK: {
//       static const std::unordered_set<npar_ty_t> no_has_semicolon = {
//           QAST_NODE_FN,
//           QAST_NODE_EXPORT,
//           QAST_NODE_BLOCK,
//       };

//       static const std::unordered_set<npar_ty_t> double_sep = {
//           QAST_NODE_FNDECL,    QAST_NODE_STRUCT, QAST_NODE_ENUM,
//           QAST_NODE_FN, QAST_NODE_SUBSYSTEM, QAST_NODE_EXPORT,
//           QAST_NODE_BLOCK,
//       };

//       Block* N = C->as<Block>();

//       bool did_root = S.did_root;
//       S.did_root = true;

//       if (did_root && N->get_items().empty() &&
//           N->get_safety() == SafetyMode::Unknown) {
//         S.line << "{}";
//         break;
//       }

//       if (N->get_safety() == SafetyMode::Unsafe) {
//         if (N->get_items().empty()) {
//           break;
//         }

//         S.line << "/* UNS"
//                   "AFE: */ unsafe ";

//         if (N->get_items().size() == 1) {
//           Stmt* stmt = N->get_items()[0];
//           recurse(stmt, S);
//           if (!no_has_semicolon.contains(stmt->getKind())) {
//             S.line << ";";
//           }

//           break;
//         }
//       } else if (N->get_safety() == SafetyMode::Safe) {
//         if (N->get_items().empty()) {
//           break;
//         }

//         S.line << "safe ";

//         if (N->get_items().size() == 1) {
//           Stmt* stmt = N->get_items()[0];
//           recurse(stmt, S);
//           if (!no_has_semicolon.contains(stmt->getKind())) {
//             S.line << ";";
//           }

//           break;
//         }
//       }

//       if (did_root) {
//         S.line << "{\n";
//         S.flush_line();
//         S.indent++;
//       }

//       for (auto it = N->get_items().begin(); it != N->get_items().end();
//       ++it) {
//         npar_ty_t ty = (*it)->getKind();

//         put_indent(S);
//         recurse(*it, S);
//         if (!no_has_semicolon.contains(ty)) {
//           S.line << ";";
//         }

//         bool do_double_line = double_sep.contains((*it)->getKind()) ||
//                               (std::next(it) != N->get_items().end() &&
//                                (*std::next(it))->getKind() != ty);

//         if (std::next(it) != N->get_items().end() && do_double_line) {
//           S.line << "\n\n";
//           S.flush_line();
//         } else {
//           S.line << "\n";
//           S.flush_line();
//         }
//       }

//       if (did_root) {
//         S.indent--;
//         put_indent(S);
//         S.line << "}";
//       } else {
//         S.flush_line();
//       }

//       break;
//     }

//     case QAST_NODE_CONST: {
//       ConstDecl* N = C->as<ConstDecl>();
//       S.line << "const " << N->get_name();
//       if (N->get_type()) {
//         S.line << ": ";
//         recurse(N->get_type(), S);
//       }
//       if (N->get_value()) {
//         S.line << " = ";
//         recurse(N->get_value(), S);
//       }
//       break;
//     }

//     case QAST_NODE_VAR: {
//       VarDecl* N = C->as<VarDecl>();
//       S.line << "var " << N->get_name();
//       if (N->get_type()) {
//         S.line << ": ";
//         recurse(N->get_type(), S);
//       }
//       if (N->get_value()) {
//         S.line << " = ";
//         recurse(N->get_value(), S);
//       }
//       break;
//     }

//     case QAST_NODE_VAR: {
//       VarDecl* N = C->as<VarDecl>();
//       S.line << "let " << N->get_name();
//       if (N->get_type()) {
//         S.line << ": ";
//         recurse(N->get_type(), S);
//       }
//       if (N->get_value()) {
//         S.line << " = ";
//         recurse(N->get_value(), S);
//       }
//       break;
//     }

//     case QAST_NODE_INLINE_ASM: {
//       qcore_panic("Asm format is not implemented");
//       break;
//     }

//     case QAST_NODE_RETURN: {
//       ReturnStmt* N = C->as<ReturnStmt>();

//       S.line << "ret";
//       if (N->get_value()) {
//         S.line << " ";
//         recurse(N->get_value(), S);
//       }

//       break;
//     }

//     case QAST_NODE_RETIF: {
//       ReturnIfStmt* N = C->as<ReturnIfStmt>();

//       S.line << "retif ";
//       recurse(N->get_cond(), S);
//       S.line << ", ";
//       recurse(N->get_value(), S);

//       break;
//     }

//     case QAST_NODE_BREAK: {
//       S.line << "break";
//       break;
//     }

//     case QAST_NODE_CONTINUE: {
//       S.line << "continue";
//       break;
//     }

//     case QAST_NODE_IF: {
//       IfStmt* N = C->as<IfStmt>();

//       S.line << "if ";
//       recurse(N->get_cond(), S);

//       bool no_compress = N->get_else() != nullptr;

//       if (N->get_then()->get_items().size() == 1 && !no_compress) {
//         S.line << " => ";
//         recurse(N->get_then()->get_items().front(), S);
//         if (N->get_else()) {
//           S.line << ";";
//         }
//       } else {
//         S.line << " ";
//         recurse(N->get_then(), S);
//       }

//       if (N->get_else()) {
//         if (N->get_else()->get_items().size() == 1 && !no_compress) {
//           S.line << " else => ";
//           recurse(N->get_else()->get_items().front(), S);
//         } else {
//           S.line << " else ";
//           recurse(N->get_else(), S);
//         }
//       }

//       break;
//     }

//     case QAST_NODE_WHILE: {
//       WhileStmt* N = C->as<WhileStmt>();
//       S.line << "while ";
//       recurse(N->get_cond(), S);
//       S.line << " ";

//       if (N->get_body()->get_items().size() == 1) {
//         S.line << "=> ";
//         recurse(N->get_body()->get_items().front(), S);
//       } else {
//         recurse(N->get_body(), S);
//       }
//       break;
//     }

//     case QAST_NODE_FOR: {
//       ForStmt* N = C->as<ForStmt>();
//       S.line << "for (";

//       recurse(N->get_init(), S);
//       S.line << ";";
//       if (N->get_init()) S.line << " ";

//       recurse(N->get_cond(), S);
//       S.line << ";";
//       if (N->get_cond()) S.line << " ";

//       recurse(N->get_step(), S);

//       S.line << ") ";

//       if (N->get_body()->get_items().size() == 1) {
//         S.line << "=> ";
//         recurse(N->get_body()->get_items().front(), S);
//       } else {
//         recurse(N->get_body(), S);
//       }
//       break;
//     }

//     case QAST_NODE_FOREACH: {
//       ForeachStmt* N = C->as<ForeachStmt>();
//       S.line << "foreach (" << N->get_idx_ident() << ", " <<
//       N->get_val_ident()
//              << " in ";
//       recurse(N->get_expr(), S);

//       S.line << ") ";

//       if (N->get_body()->get_items().size() == 1) {
//         S.line << "=> ";
//         recurse(N->get_body()->get_items().front(), S);
//       } else {
//         recurse(N->get_body(), S);
//       }
//       break;
//     }

//     case QAST_NODE_CASE: {
//       CaseStmt* N = C->as<CaseStmt>();
//       S.line << "case ";
//       recurse(N->get_cond(), S);
//       S.line << ": ";
//       recurse(N->get_body(), S);
//       break;
//     }

//     case QAST_NODE_SWITCH: {
//       SwitchStmt* N = C->as<SwitchStmt>();
//       S.line << "switch ";
//       recurse(N->get_cond(), S);
//       if (N->get_cases().empty() && !N->get_default()) {
//         S.line << " {}";
//       } else {
//         S.line << " {\n";
//         S.flush_line();
//         S.indent++;
//         for (auto& stmt : N->get_cases()) {
//           put_indent(S);
//           recurse(stmt, S);
//           S.line << "\n";
//           S.flush_line();
//         }
//         if (N->get_default()) {
//           put_indent(S);
//           S.line << "default: ";
//           recurse(N->get_default(), S);
//           S.line << "\n";
//           S.flush_line();
//         }
//         S.indent--;
//         put_indent(S);
//         S.line << "}";
//       }
//       break;
//     }

//     case QAST_NODE_EXPR_STMT: {
//       recurse(C->as<ExprStmt>()->get_expr(), S);
//       break;
//     }

//   if (C->is_decl()) {
//     Decl* N = C->as<Decl>();

//     if (!N->get_tags().empty()) {
//       size_t split_on = N->get_tags().size() <= 6
//                             ? 6
//                             : std::ceil(std::sqrt(N->get_tags().size()));

//       S.line << " with [";

//       S.line.seekg(0, std::ios::end);
//       size_t line_size = S.line.tellg(), i = 0;

//       for (auto it = N->get_tags().begin(); it != N->get_tags().end(); it++)
//       {
//         recurse(*it, S);

//         i++;
//         if (std::next(it) != N->get_tags().end()) {
//           S.line << ",";

//           if (i % split_on == 0) {
//             S.line << "\n";
//             S.flush_line();
//             S.line << std::string(line_size, ' ');
//           } else {
//             S.line << " ";
//           }
//         }
//       }
//       S.line << "]";
//     }
//   }
// }

static void recurse(npar_node_t*, AutomatonState&) {
  (void)write_float_literal;
  (void)escape_char_literal;
  (void)escape_string_literal;
  (void)put_indent;
  (void)put_type_metadata;
}

void do_formatting(const lsp::RequestMessage& req, lsp::ResponseMessage& resp) {
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
    resp.error(lsp::ErrorCodes::InvalidParams,
               "textDocument.uri is not a string");
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
    resp.error(lsp::ErrorCodes::InvalidParams,
               "options.tabSize is not an integer");
    return;
  }

  if (req.params()["options"]["tabSize"].GetUint() == 0) {
    resp.error(lsp::ErrorCodes::InvalidParams, "options.tabSize is 0");
    return;
  }

  if (!req.params()["options"].HasMember("insertSpaces")) {
    resp.error(lsp::ErrorCodes::InvalidParams, "Missing options.insertSpaces");
    return;
  }

  if (!req.params()["options"]["insertSpaces"].IsBool()) {
    resp.error(lsp::ErrorCodes::InvalidParams,
               "options.insertSpaces is not a boolean");
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
  nr_syn parser(lexer.get(), env.get());

  npar_node_t* root = nullptr;
  if (!npar_do(parser.get(), &root)) {
    return;
  }

  /// FIXME: Re-enable checking once checking is fixed
  // if (!npar_check(parser.get(), root)) {
  //   return;
  // }

  LOG(INFO) << "Requested document format";

  AutomatonState S(options.tabSize);

  recurse(static_cast<npar_node_t*>(root), S);

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
  std::string new_text = S.S.str();
  edit.AddMember(
      "newText",
      Value(new_text.c_str(), new_text.size(), resp->GetAllocator()).Move(),
      resp->GetAllocator());

  resp->PushBack(edit, resp->GetAllocator());

  ///==========================================================

  return;
}
