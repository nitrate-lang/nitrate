#include <lsp/lang/CambrianStyleFormatter.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTCommon.hh>
#include <sstream>
#include <unordered_set>

using namespace lsp::fmt;

using namespace ncc;
using namespace ncc::parse;
using namespace ncc::lex;

CambrianFormatter::LineStreamWritter&
CambrianFormatter::LineStreamWritter::operator<<(
    std::ostream& (*func)(std::ostream&)) {
  qcore_assert(func ==
               static_cast<std::ostream& (*)(std::ostream&)>(std::endl));

  m_file << m_line_buffer.str() << "\n";
  reset();

  return *this;
}

CambrianFormatter::LineStreamWritter&
CambrianFormatter::LineStreamWritter::operator<<(Operator op) {
  m_line_buffer << op;
  return *this;
}

CambrianFormatter::LineStreamWritter&
CambrianFormatter::LineStreamWritter::operator<<(Vis v) {
  switch (v) {
    case Vis::Sec: {
      m_line_buffer << "sec";
      break;
    }

    case Vis::Pro: {
      m_line_buffer << "pro";
      break;
    }

    case Vis::Pub: {
      m_line_buffer << "pub";
      break;
    }
  }
  return *this;
}

std::string CambrianFormatter::escape_char_literal(char ch) const {
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

std::string CambrianFormatter::escape_string_literal_chunk(
    std::string_view str) const {
  std::stringstream ss;

  for (char ch : str) {
    switch (ch) {
      case '\n':
        ss << "\\n";
        break;
      case '\t':
        ss << "\\t";
        break;
      case '\r':
        ss << "\\r";
        break;
      case '\v':
        ss << "\\v";
        break;
      case '\f':
        ss << "\\f";
        break;
      case '\b':
        ss << "\\b";
        break;
      case '\a':
        ss << "\\a";
        break;
      case '\\':
        ss << "\\\\";
        break;
      case '"':
        ss << "\\\"";
        break;
      default:
        ss << ch;
        break;
    }
  }

  return ss.str();
}

void CambrianFormatter::wrap_stmt_body(FlowPtr<parse::Stmt> n,
                                       size_t size_threshold,
                                       bool use_arrow_if_wrapped) {
  if (n->is(QAST_BLOCK)) {
    auto block = n.as<Block>();
    bool single_stmt = block->get_items().size() == 1;
    bool few_children =
        single_stmt && block->count_children() <= size_threshold;

    if (single_stmt && few_children) {
      if (use_arrow_if_wrapped) {
        line << "=> ";
      }

      block->get_items().front().accept(*this);
      return;
    }
  }

  n.accept(*this);
}

void CambrianFormatter::print_line_comments(FlowPtr<parse::Base> n) {
  auto comments = n->comments();
  auto line_size = line.length();

  if (!comments.empty()) {
    for (auto comment : comments) {
      line << "#";
      line << comment.as_string() << std::endl;

      if (line_size) {
        line << std::string(line_size, ' ');
      }
    }
  }
}

void CambrianFormatter::print_multiline_comments(FlowPtr<parse::Base> n) {
  auto comments = n->comments();
  if (!comments.empty()) {
    for (auto comment : comments) {
      line << "/*";
      line << comment.as_string();
      line << "*/ ";
    }
  }
}

void CambrianFormatter::escape_string_literal(std::string_view str,
                                              bool put_quotes) {
  constexpr size_t max_chunk_size = 60;

  if (str.empty()) {
    if (put_quotes) {
      line << "\"\"";
    }
    return;
  }

  auto chunks_n = str.size() / max_chunk_size;
  auto rem = str.size() % max_chunk_size;
  auto line_size = line.length();

  if (chunks_n) {
    std::vector<std::string> chunks(chunks_n);

    for (size_t i = 0; i < chunks_n; i++) {
      chunks[i] = "\"" +
                  escape_string_literal_chunk(
                      str.substr(i * max_chunk_size, max_chunk_size)) +
                  "\"";
    }

    auto max_segment_size =
        std::max_element(chunks.begin(), chunks.end(), [](auto a, auto b) {
          return a.size() < b.size();
        })->size();

    for (size_t i = 0; i < chunks.size(); ++i) {
      if (i != 0 && line_size) {
        line << std::string(line_size, ' ');
      }

      line << chunks[i];

      auto rpad = (max_segment_size - chunks[i].size());
      if (rpad) {
        line << std::string(rpad, ' ');
      }

      if (rem > 0 || i < chunks_n - 1) {
        line << " \\" << std::endl;
      }
    }
  }

  if (rem > 0) {
    if (line_size && chunks_n > 0) {
      line << std::string(line_size, ' ');
    }

    if (chunks_n > 0 || put_quotes) {
      line << "\"";
    }

    line << escape_string_literal_chunk(
        str.substr(chunks_n * max_chunk_size, rem));

    if (chunks_n > 0 || put_quotes) {
      line << "\"";
    }
  }
}

void CambrianFormatter::write_float_literal_chunk(std::string_view float_str) {
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
      line << "_";
    }

    line << float_str[i];
  }
}

void CambrianFormatter::write_float_literal(std::string_view float_str) {
  constexpr size_t max_chunk_size = 50;

  if (float_str.empty()) {
    line << "";
  }

  size_t chunks_n = float_str.size() / max_chunk_size;
  size_t rem = float_str.size() % max_chunk_size;

  size_t line_size = line.length();

  for (size_t i = 0; i < chunks_n; i++) {
    write_float_literal_chunk(
        float_str.substr(i * max_chunk_size, max_chunk_size));

    if (rem > 0 || i < chunks_n - 1) {
      line << "_ \\" << std::endl;
      if (line_size) {
        line << std::string(line_size, ' ');
      }
    }
  }

  if (rem > 0) {
    write_float_literal_chunk(float_str.substr(chunks_n * max_chunk_size, rem));
  }
}

void CambrianFormatter::format_type_metadata(FlowPtr<parse::Type> n) {
  auto range_start = n->get_range_begin();
  auto range_end = n->get_range_end();

  if (range_start || range_end) {
    line << ": [";
    if (range_start) range_start.value().accept(*this);
    line << ":";
    if (range_end) range_end.value().accept(*this);
    line << "]";
  }

  if (n->get_width()) {
    line << ": ";
    n->get_width().value().accept(*this);
  }
}

void CambrianFormatter::visit(FlowPtr<Base> n) {
  print_multiline_comments(n);

  /** This node symbolizes a placeholder value in the event of an error. */
  failed = true;

  line << "/* !!! */";
}

void CambrianFormatter::visit(FlowPtr<ExprStmt> n) {
  print_line_comments(n);

  n->get_expr().accept(*this);
  line << ";";
}

void CambrianFormatter::visit(FlowPtr<StmtExpr> n) {
  print_multiline_comments(n);

  n->get_stmt().accept(*this);
}

void CambrianFormatter::visit(FlowPtr<TypeExpr> n) {
  print_multiline_comments(n);

  line << "type ";
  n->get_type().accept(*this);
}

void CambrianFormatter::visit(FlowPtr<NamedTy> n) {
  print_multiline_comments(n);

  line << n->get_name();
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<InferTy> n) {
  print_multiline_comments(n);

  line << "?";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<TemplType> n) {
  print_multiline_comments(n);

  bool is_optional =
      n->get_template()->getKind() == QAST_NAMED &&
      n->get_template()->as<NamedTy>()->get_name() == "__builtin_result";

  bool is_vector =
      n->get_template()->getKind() == QAST_NAMED &&
      n->get_template()->as<NamedTy>()->get_name() == "__builtin_vec";

  bool is_map =
      n->get_template()->getKind() == QAST_NAMED &&
      n->get_template()->as<NamedTy>()->get_name() == "__builtin_umap";

  bool is_set =
      n->get_template()->getKind() == QAST_NAMED &&
      n->get_template()->as<NamedTy>()->get_name() == "__builtin_uset";

  bool is_comptime =
      n->get_template()->getKind() == QAST_NAMED &&
      n->get_template()->as<NamedTy>()->get_name() == "__builtin_meta" &&
      n->get_args().size() == 1 &&
      n->get_args().front().second->is(QAST_UNEXPR) &&
      n->get_args().front().second.template as<UnaryExpr>()->get_op() ==
          OpComptime;

  const auto print_without_type_keyword = [&](auto node) {
    if (node->is(QAST_TEXPR)) {
      node->template as<TypeExpr>()->get_type().accept(*this);
    } else {
      node->accept(*this);
    }
  };

  size_t argc = n->get_args().size();
  if (is_optional && argc == 1) {
    print_without_type_keyword(n->get_args().front().second);
    line << "?";
  } else if (is_vector && argc == 1) {
    line << "[";
    print_without_type_keyword(n->get_args().front().second);
    line << "]";
  } else if (is_map && argc == 2) {
    line << "[";
    print_without_type_keyword(n->get_args().front().second);
    line << "->";
    print_without_type_keyword(n->get_args().back().second);
    line << "]";
  } else if (is_set && argc == 1) {
    line << "{";
    print_without_type_keyword(n->get_args().front().second);
    line << "}";
  } else if (is_comptime) {
    line << "comptime(";
    n->get_args().front().second.template as<UnaryExpr>()->get_rhs().accept(
        *this);
    line << ")";
  } else {
    n->get_template().accept(*this);

    line << "<";
    iterate_except_last(
        n->get_args().begin(), n->get_args().end(),
        [&](auto arg, size_t) {
          if (!std::isdigit(arg.first->at(0))) {
            line << arg.first << ": ";
          }

          print_without_type_keyword(arg.second);
        },
        [&](let) { line << ", "; });
    line << ">";
  }

  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<U1> n) {
  print_multiline_comments(n);

  line << "u1";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<U8> n) {
  print_multiline_comments(n);

  line << "u8";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<U16> n) {
  print_multiline_comments(n);

  line << "u16";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<U32> n) {
  print_multiline_comments(n);

  line << "u32";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<U64> n) {
  print_multiline_comments(n);

  line << "u64";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<U128> n) {
  print_multiline_comments(n);

  line << "u128";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<I8> n) {
  print_multiline_comments(n);

  line << "i8";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<I16> n) {
  print_multiline_comments(n);

  line << "i16";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<I32> n) {
  print_multiline_comments(n);

  line << "i32";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<I64> n) {
  print_multiline_comments(n);

  line << "i64";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<I128> n) {
  print_multiline_comments(n);

  line << "i128";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<F16> n) {
  print_multiline_comments(n);

  line << "f16";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<F32> n) {
  print_multiline_comments(n);

  line << "f32";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<F64> n) {
  print_multiline_comments(n);

  line << "f64";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<F128> n) {
  print_multiline_comments(n);

  line << "f128";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<VoidTy> n) {
  print_multiline_comments(n);

  line << "void";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<PtrTy> n) {
  print_multiline_comments(n);

  line << "*";
  n->get_item().accept(*this);

  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<OpaqueTy> n) {
  print_multiline_comments(n);

  line << "opaque(" << n->get_name() << ")";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<TupleTy> n) {
  /* If the number of fields exceeds the threshold, arange fields into a
   * matrix of row size ceil(sqrt(n)). */
  print_multiline_comments(n);

  auto wrap_threshold = 8ULL;

  line << "(";

  auto items = n->get_items();
  auto line_size = line.length();
  auto break_at = items.size() <= wrap_threshold
                      ? wrap_threshold
                      : static_cast<size_t>(std::ceil(std::sqrt(items.size())));

  for (size_t i = 0; i < items.size(); i++) {
    if (i != 0 && i % break_at == 0) {
      line << std::endl << std::string(line_size, ' ');
    }

    auto item = items[i];
    item.accept(*this);

    if (i != items.size() - 1) {
      line << ", ";
    }
  }
  line << ")";

  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<ArrayTy> n) {
  print_multiline_comments(n);

  line << "[";
  n->get_item().accept(*this);
  line << "; ";
  n->get_size().accept(*this);
  line << "]";

  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<RefTy> n) {
  print_multiline_comments(n);

  line << "&";
  n->get_item().accept(*this);

  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<FuncTy> n) {
  print_multiline_comments(n);

  line << "fn";

  if (!n->get_attributes().empty()) {
    line << "[";
    iterate_except_last(
        n->get_attributes().begin(), n->get_attributes().end(),
        [&](auto attr, size_t) { attr.accept(*this); },
        [&](let) { line << ", "; });
    line << "] ";
  }

  switch (n->get_purity()) {
    case Purity::Impure: {
      break;
    }

    case Purity::Impure_TSafe: {
      line << " tsafe";
      break;
    }

    case Purity::Pure: {
      line << " pure";
      break;
    }

    case Purity::Quasi: {
      line << " quasi";
      break;
    }

    case Purity::Retro: {
      line << " retro";
      break;
    }
  }

  line << "(";
  iterate_except_last(
      n->get_params().begin(), n->get_params().end(),
      [&](auto param, size_t) {
        line << std::get<0>(param);

        if (auto type = std::get<1>(param); type->getKind() != QAST_INFER) {
          line << ": ";
          type.accept(*this);
        }

        if (auto def = std::get<2>(param)) {
          line << " = ";
          def.value().accept(*this);
        }
      },
      [&](let) { line << ", "; });
  if (n->is_variadic()) {
    if (!n->get_params().empty()) {
      line << ", ";
    }
    line << "...";
  }
  line << ")";

  line << ": ";
  n->get_return().accept(*this);
}

void CambrianFormatter::visit(FlowPtr<UnaryExpr> n) {
  static const std::unordered_set<Operator> WordOps = {
      OpAs,        OpBitcastAs, OpIn,     OpOut,     OpSizeof,
      OpBitsizeof, OpAlignof,   OpTypeof, OpComptime};

  print_multiline_comments(n);

  line << "(" << n->get_op();
  if (WordOps.contains(n->get_op())) {
    line << " ";
  }
  n->get_rhs().accept(*this);
  line << ")";
}

void CambrianFormatter::visit(FlowPtr<BinExpr> n) {
  print_multiline_comments(n);

  if (n->get_op() == OpDot) {
    n->get_lhs().accept(*this);
    line << ".";
    n->get_rhs().accept(*this);
  } else {
    line << "(";
    n->get_lhs().accept(*this);
    line << " " << n->get_op() << " ";
    n->get_rhs().accept(*this);
    line << ")";
  }
}

void CambrianFormatter::visit(FlowPtr<PostUnaryExpr> n) {
  print_multiline_comments(n);

  line << "(";
  n->get_lhs().accept(*this);
  line << n->get_op() << ")";
}

void CambrianFormatter::visit(FlowPtr<TernaryExpr> n) {
  print_multiline_comments(n);

  line << "(";
  n->get_cond().accept(*this);
  line << " ? ";
  n->get_lhs().accept(*this);
  line << " : ";
  n->get_rhs().accept(*this);
  line << ")";
}

void CambrianFormatter::visit(FlowPtr<ConstInt> n) {
  print_multiline_comments(n);

  write_float_literal(n->get_value());
}

void CambrianFormatter::visit(FlowPtr<ConstFloat> n) {
  print_multiline_comments(n);

  write_float_literal(n->get_value());
}

void CambrianFormatter::visit(FlowPtr<ConstBool> n) {
  print_multiline_comments(n);

  if (n->get_value()) {
    line << "true";
  } else {
    line << "false";
  }
}

void CambrianFormatter::visit(FlowPtr<ConstString> n) {
  print_multiline_comments(n);

  escape_string_literal(n->get_value());
}

void CambrianFormatter::visit(FlowPtr<ConstChar> n) {
  print_multiline_comments(n);

  line << escape_char_literal(n->get_value());
}

void CambrianFormatter::visit(FlowPtr<ConstNull> n) {
  print_multiline_comments(n);

  line << "null";
}

void CambrianFormatter::visit(FlowPtr<ConstUndef> n) {
  print_multiline_comments(n);

  line << "undef";
}

void CambrianFormatter::visit(FlowPtr<Call> n) {
  print_multiline_comments(n);

  auto wrap_threshold = 8ULL;

  n->get_func().accept(*this);

  size_t argc = n->get_args().size();

  bool any_named =
      std::any_of(n->get_args().begin(), n->get_args().end(), [](CallArg arg) {
        auto name = arg.first;
        return !std::isdigit(name->at(0));
      });

  bool any_lambdas = std::any_of(
      n->get_args().begin(), n->get_args().end(),
      [](auto arg) { return std::get<1>(arg)->is_stmt_expr(QAST_FUNCTION); });

  bool is_wrapping = argc >= wrap_threshold || any_named || any_lambdas;

  if (is_wrapping) {
    line << "(";
    size_t line_size = line.length();
    std::swap(indent, line_size);

    for (auto it = n->get_args().begin(); it != n->get_args().end(); ++it) {
      auto arg = *it;
      auto name = std::get<0>(arg);
      auto value = std::get<1>(arg);

      if (!std::isdigit(name->at(0))) {
        line << name << ": ";
      }

      value.accept(*this);

      if (it != n->get_args().end() - 1) {
        line << ", ";
      }

      if (it != n->get_args().end() - 1) {
        line << std::endl << get_indent();
      }
    }

    std::swap(indent, line_size);
    line << ")";
  } else {
    line << "(";
    iterate_except_last(
        n->get_args().begin(), n->get_args().end(),
        [&](auto arg, size_t) {
          auto name = std::get<0>(arg);
          auto value = std::get<1>(arg);

          if (!std::isdigit(name->at(0))) {
            line << name << ": ";
          }

          value.accept(*this);
        },
        [&](let) { line << ", "; });
    line << ")";
  }
}

void CambrianFormatter::visit(FlowPtr<TemplCall> n) {
  print_multiline_comments(n);

  n->get_func().accept(*this);

  line << "{";
  iterate_except_last(
      n->get_template_args().begin(), n->get_template_args().end(),
      [&](auto arg, size_t) {
        auto name = std::get<0>(arg);
        auto value = std::get<1>(arg);
        bool should_print_name = !std::isdigit(name->at(0));

        if (should_print_name) {
          line << name << ": ";
        }

        if (value->is(QAST_TEXPR)) {
          value->template as<TypeExpr>()->get_type().accept(*this);
        } else {
          value.accept(*this);
        }
      },
      [&](let) { line << ", "; });
  line << "}";

  line << "(";
  iterate_except_last(
      n->get_args().begin(), n->get_args().end(),
      [&](auto arg, size_t) {
        auto name = std::get<0>(arg);
        auto value = std::get<1>(arg);

        if (!std::isdigit(name->at(0))) {
          line << name << ": ";
        }

        value.accept(*this);
      },
      [&](let) { line << ", "; });
  line << ")";
}

void CambrianFormatter::visit(FlowPtr<List> n) {
  print_multiline_comments(n);

  auto wrap_threshold = 8ULL;

  if (n->get_items().empty()) {
    line << "[]";
    return;
  }

  auto argc = n->get_items().size();
  bool is_compressing =
      argc >= wrap_threshold &&
      std::all_of(n->get_items().begin(), n->get_items().end(),
                  [&](auto x) { return x->isSame(n->get_items().front()); });

  if (is_compressing) {
    line << "[";
    n->get_items().front().accept(*this);
    line << "; " << argc << "]";
  } else {
    static const std::unordered_set<npar_ty_t> extra_seperation = {
        QAST_TEREXPR, QAST_CALL, QAST_LIST,
        QAST_ASSOC,   QAST_SEQ,  QAST_TEMPL_CALL,
    };

    bool special_case =
        std::any_of(n->get_items().begin(), n->get_items().end(), [&](auto x) {
          return extra_seperation.contains(x->getKind()) ||
                 x->is_stmt_expr(QAST_FUNCTION);
        });

    size_t break_at{};

    if (special_case) {
      break_at = 1;
    } else {
      break_at = argc <= wrap_threshold
                     ? wrap_threshold
                     : static_cast<size_t>(std::ceil(std::sqrt(argc)));
    }

    if (break_at == 1) {
      line << "[";

      line << std::endl;

      { /* Write list items */
        size_t the_indent = indent + tabSize;
        std::swap(indent, the_indent);

        for (size_t i = 0; i < n->get_items().size(); i++) {
          line << get_indent();
          auto item = n->get_items()[i];
          item.accept(*this);

          bool is_last = i == n->get_items().size() - 1;
          if (!is_last) {
            line << ",";
          }

          line << std::endl;
        }

        std::swap(indent, the_indent);
      }

      line << get_indent() << "]";
    } else {
      line << "[";

      bool is_assoc_map =
          std::all_of(n->get_items().begin(), n->get_items().end(),
                      [](auto x) { return x->is(QAST_ASSOC); });

      { /* Write list items */
        size_t the_indent = is_assoc_map ? indent + tabSize : line.length();
        std::swap(indent, the_indent);

        for (size_t i = 0; i < n->get_items().size(); i++) {
          auto item = n->get_items()[i];
          item.accept(*this);

          bool is_last = i == n->get_items().size() - 1;
          if (!is_last) {
            line << ",";
          }

          bool is_break = !is_last && i != 0 && (i + 1) % break_at == 0;

          if (is_break) {
            line << std::endl << get_indent();
          } else if (!is_last) {
            line << " ";
          }
        }

        std::swap(indent, the_indent);
      }

      line << "]";
    }
  }
}

void CambrianFormatter::visit(FlowPtr<Assoc> node) {
  print_multiline_comments(node);

  const std::function<void(FlowPtr<Assoc>, bool)> format =
      [&](FlowPtr<Assoc> n, bool use_braces) {
        bool is_value_map = false;
        if (n->get_value()->is(QAST_LIST)) {
          auto list = n->get_value()->as<List>();
          is_value_map =
              list->get_items().empty() ||
              std::all_of(list->get_items().begin(), list->get_items().end(),
                          [](auto x) { return x->is(QAST_ASSOC); });
        }

        if (use_braces) {
          line << "{" << std::endl;
          indent += tabSize;
          line << get_indent();
        }

        n->get_key().accept(*this);
        line << ": ";

        if (is_value_map) {
          auto list = n->get_value()->as<List>();

          if (list->get_items().empty()) {
            line << "{}";
          } else {
            line << "{" << std::endl;
            indent += tabSize;

            for (auto it = list->get_items().begin();
                 it != list->get_items().end(); ++it) {
              line << get_indent();

              format(it->as<Assoc>(), false);

              if (it != list->get_items().end() - 1) {
                line << ",";
              }

              line << std::endl;
            }

            indent -= tabSize;
            line << get_indent() << "}";
          }
        } else {
          n->get_value().accept(*this);
        }

        if (use_braces) {
          indent -= tabSize;
          line << std::endl << get_indent() << "}";
        }
      };

  format(node, true);
}

void CambrianFormatter::visit(FlowPtr<Index> n) {
  print_multiline_comments(n);

  n->get_base().accept(*this);
  line << "[";
  n->get_index().accept(*this);
  line << "]";
}

void CambrianFormatter::visit(FlowPtr<Slice> n) {
  print_multiline_comments(n);

  n->get_base().accept(*this);
  line << "[";
  if (n->get_start()) {
    n->get_start().accept(*this);
  }
  line << ":";
  if (n->get_end()) {
    n->get_end().accept(*this);
  }
  line << "]";
}

void CambrianFormatter::visit(FlowPtr<FString> n) {
  print_multiline_comments(n);

  line << "f\"";
  for (auto part : n->get_items()) {
    if (std::holds_alternative<ncc::string>(part)) {
      escape_string_literal(*std::get<ncc::string>(part), false);
    } else {
      line << "{";
      std::get<FlowPtr<Expr>>(part).accept(*this);
      line << "}";
    }
  }
  line << "\"";
}

void CambrianFormatter::visit(FlowPtr<Ident> n) {
  print_multiline_comments(n);

  line << n->get_name();
}

void CambrianFormatter::visit(FlowPtr<SeqPoint> n) {
  print_multiline_comments(n);

  line << "(";
  iterate_except_last(
      n->get_items().begin(), n->get_items().end(),
      [&](auto item, size_t) { item.accept(*this); },
      [&](let) { line << ", "; });
  line << ")";
}

void CambrianFormatter::visit(FlowPtr<Block> n) {
  print_line_comments(n);

  bool isRootBlock = !did_root;
  did_root = true;

  switch (n->get_safety()) {
    case SafetyMode::Safe: {
      line << "safe ";
      break;
    }

    case SafetyMode::Unsafe: {
      line << "unsafe ";
      break;
    }

    case SafetyMode::Unknown: {
      break;
    }
  }

  static const std::unordered_set<npar_ty_t> extra_seperation = {
      QAST_STRUCT,     QAST_ENUM,    QAST_FUNCTION,
      QAST_SCOPE,      QAST_EXPORT,  QAST_BLOCK,

      QAST_INLINE_ASM, QAST_IF,      QAST_WHILE,
      QAST_FOR,        QAST_FOREACH, QAST_SWITCH,
  };

  if (!isRootBlock && n->get_items().empty()) {
    line << "{}";
    return;
  }

  if (!isRootBlock) {
    line << "{" << std::endl;
    indent += tabSize;
  }

  auto items = n->get_items();

  for (auto it = items.begin(); it != items.end(); ++it) {
    auto item = *it;

    line << get_indent();
    item.accept(*this);
    line << std::endl;

    bool is_last_item = it == items.end() - 1;

    bool is_next_item_different =
        (it + 1 != items.end() &&
         (*std::next(it))->getKind() != item->getKind());

    bool extra_newline =
        !is_last_item &&
        (is_next_item_different || extra_seperation.contains(item->getKind()));

    if (extra_newline) {
      line << std::endl;
    }
  }

  if (!isRootBlock) {
    indent -= tabSize;
    line << get_indent() << "}";
  }
}

void CambrianFormatter::visit(FlowPtr<VarDecl> n) {
  print_line_comments(n);

  switch (n->get_decl_type()) {
    case VarDeclType::Let: {
      line << "let ";
      break;
    }

    case VarDeclType::Const: {
      line << "const ";
      break;
    }

    case VarDeclType::Var: {
      line << "var ";
      break;
    }
  }

  if (!n->get_attributes().empty()) {
    line << "[";
    iterate_except_last(
        n->get_attributes().begin(), n->get_attributes().end(),
        [&](auto attr, size_t) { attr.accept(*this); },
        [&](let) { line << ", "; });
    line << "] ";
  }

  line << n->get_name();

  if (n->get_type()) {
    line << ": ";
    n->get_type().value().accept(*this);
  }

  if (n->get_value()) {
    line << " = ";
    n->get_value().value().accept(*this);
  }

  line << ";";
}

void CambrianFormatter::visit(FlowPtr<InlineAsm> n) {
  print_line_comments(n);

  /* Support for inline assembly is not avaliable yet */

  failed = true;

  line << "/* !!! */";
}

void CambrianFormatter::visit(FlowPtr<IfStmt> n) {
  print_line_comments(n);

  line << "if ";
  n->get_cond().accept(*this);
  line << " ";
  n->get_then().accept(*this);

  if (n->get_else()) {
    line << " else ";
    n->get_else().value().accept(*this);
  }

  line << ";";
}

void CambrianFormatter::visit(FlowPtr<WhileStmt> n) {
  print_line_comments(n);

  line << "while ";
  n->get_cond().accept(*this);
  line << " ";
  n->get_body().accept(*this);

  line << ";";
}

void CambrianFormatter::visit(FlowPtr<ForStmt> n) {
  print_line_comments(n);

  line << "for (";

  if (n->get_init().has_value()) {
    n->get_init().value().accept(*this);
    if (!n->get_init().value()->is_stmt()) {
      line << ";";
    }
  } else {
    line << ";";
  }

  if (n->get_cond().has_value()) {
    line << " ";
    n->get_cond().value().accept(*this);
  }
  line << ";";

  if (n->get_step().has_value()) {
    line << " ";
    n->get_step().value().accept(*this);
  }

  line << ") ";
  n->get_body().accept(*this);

  line << ";";
}

void CambrianFormatter::visit(FlowPtr<ForeachStmt> n) {
  print_line_comments(n);

  line << "foreach (";
  if (n->get_idx_ident()->empty()) {
    line << n->get_val_ident();
  } else {
    line << n->get_idx_ident() << ", " << n->get_val_ident();
  }

  line << " in ";
  n->get_expr().accept(*this);
  line << ") ";

  n->get_body().accept(*this);

  line << ";";
}

void CambrianFormatter::visit(FlowPtr<BreakStmt> n) {
  print_line_comments(n);

  line << "break;";
}

void CambrianFormatter::visit(FlowPtr<ContinueStmt> n) {
  print_line_comments(n);

  line << "continue;";
}

void CambrianFormatter::visit(FlowPtr<ReturnStmt> n) {
  print_line_comments(n);

  if (n->get_value().has_value()) {
    line << "ret ";
    n->get_value().value().accept(*this);
    line << ";";
  } else {
    line << "ret;";
  }
}

void CambrianFormatter::visit(FlowPtr<ReturnIfStmt> n) {
  print_line_comments(n);

  line << "retif ";
  n->get_cond().accept(*this);
  line << ", ";
  n->get_value().accept(*this);
  line << ";";
}

void CambrianFormatter::visit(FlowPtr<CaseStmt> n) {
  print_line_comments(n);

  n->get_cond().accept(*this);
  line << " => ";
  wrap_stmt_body(n->get_body(), 10, false);
}

void CambrianFormatter::visit(FlowPtr<SwitchStmt> n) {
  print_line_comments(n);

  line << "switch ";
  n->get_cond().accept(*this);
  line << " {" << std::endl;
  indent += tabSize;

  for (auto c : n->get_cases()) {
    line << get_indent();
    c.accept(*this);
    line << std::endl;
  }

  if (n->get_default()) {
    line << get_indent();
    line << "_ => ";
    wrap_stmt_body(n->get_default().value(), 10, false);
    line << std::endl;
  }

  indent -= tabSize;
  line << get_indent() << "}";
}

void CambrianFormatter::visit(FlowPtr<TypedefStmt> n) {
  print_line_comments(n);

  line << "type " << n->get_name() << " = ";
  n->get_type().accept(*this);
  line << ";";
}

void CambrianFormatter::visit(FlowPtr<Function> n) {
  print_line_comments(n);

  line << "fn";

  if (!n->get_attributes().empty()) {
    line << " [";
    iterate_except_last(
        n->get_attributes().begin(), n->get_attributes().end(),
        [&](auto attr, size_t) { attr.accept(*this); },
        [&](let) { line << ", "; });
    line << "]";
  }

  switch (n->get_purity()) {
    case Purity::Impure: {
      break;
    }

    case Purity::Impure_TSafe: {
      line << " tsafe";
      break;
    }

    case Purity::Pure: {
      line << " pure";
      break;
    }

    case Purity::Quasi: {
      line << " quasi";
      break;
    }

    case Purity::Retro: {
      line << " retro";
      break;
    }
  }

  if (!n->get_captures().empty()) {
    line << " [";
    iterate_except_last(
        n->get_captures().begin(), n->get_captures().end(),
        [&](auto cap, size_t) {
          if (cap.second) {
            line << "&";
          }
          line << cap.first;
        },
        [&](let) { line << ", "; });
    line << "]";
  }

  line << " " << n->get_name();

  if (n->get_template_params()) {
    line << "<";
    iterate_except_last(
        n->get_template_params().value().begin(),
        n->get_template_params().value().end(),
        [&](auto param, size_t) {
          line << std::get<0>(param);

          if (let type = std::get<1>(param)) {
            if (type->getKind() != QAST_INFER) {
              line << ": ";
              type->accept(*this);
            }
          }

          if (auto val = std::get<2>(param)) {
            line << " = ";
            val.value().accept(*this);
          }
        },
        [&](let) { line << ", "; });
    line << ">";
  }

  line << "(";
  iterate_except_last(
      n->get_params().begin(), n->get_params().end(),
      [&](auto param, size_t) {
        line << std::get<0>(param);

        if (let type = std::get<1>(param)) {
          if (type->getKind() != QAST_INFER) {
            line << ": ";
            type->accept(*this);
          }
        }

        if (auto def = std::get<2>(param)) {
          line << " = ";
          def.value().accept(*this);
        }
      },
      [&](let) { line << ", "; });

  if (n->is_variadic()) {
    if (!n->get_params().empty()) {
      line << ", ";
    }
    line << "...";
  }
  line << ")";

  { /* Return type */
    auto return_type = n->get_return();

    if (!return_type->is(QAST_INFER)) {
      line << ": ";
      return_type.accept(*this);
    }
  }

  if (n->is_declaration()) {
    line << ";";
  } else {
    line << " ";
    wrap_stmt_body(n->get_body().value(), 10, true);
  }
}

void CambrianFormatter::visit(FlowPtr<StructDef> n) {
  print_line_comments(n);

  switch (n->get_composite_type()) {
    case CompositeType::Region: {
      line << "region ";
      break;
    }

    case CompositeType::Struct: {
      line << "struct ";
      break;
    }

    case CompositeType::Group: {
      line << "group ";
      break;
    }

    case CompositeType::Class: {
      line << "class ";
      break;
    }

    case CompositeType::Union: {
      line << "union ";
      break;
    }
  }

  if (!n->get_attributes().empty()) {
    line << "[";
    iterate_except_last(
        n->get_attributes().begin(), n->get_attributes().end(),
        [&](auto attr, size_t) { attr.accept(*this); },
        [&](let) { line << ", "; });
    line << "] ";
  }

  line << n->get_name();
  if (n->get_template_params().has_value()) {
    line << "<";
    iterate_except_last(
        n->get_template_params().value().begin(),
        n->get_template_params().value().end(),
        [&](auto param, size_t) {
          line << std::get<0>(param);
          if (auto type = std::get<1>(param); type->getKind() != QAST_INFER) {
            line << ": ";
            type.accept(*this);
          }
          if (auto val = std::get<2>(param)) {
            line << " = ";
            val.value().accept(*this);
          }
        },
        [&](let) { line << ", "; });
    line << ">";
  }

  if (!n->get_names().empty()) {
    line << ": ";
    iterate_except_last(
        n->get_names().begin(), n->get_names().end(),
        [&](auto name, size_t) { line << name; }, [&](let) { line << ", "; });
  }

  bool is_empty = n->get_fields().empty() && n->get_methods().empty() &&
                  n->get_static_methods().empty();

  if (is_empty) {
    line << " {}";
    return;
  }

  line << " {" << std::endl;
  indent += tabSize;

  auto fields_count = n->get_fields();
  auto methods_count = n->get_methods();
  auto static_methods_count = n->get_static_methods();

  std::for_each(n->get_fields().begin(), n->get_fields().end(),
                [&](auto field) {
                  line << get_indent() << field.get_vis() << " ";

                  line << field.get_name() << ": ";
                  field.get_type().accept(*this);

                  if (field.get_value().has_value()) {
                    line << " = ";
                    field.get_value().value().accept(*this);
                  }

                  line << "," << std::endl;
                });

  if (!fields_count.empty() && !methods_count.empty()) {
    line << std::endl;
  }

  std::for_each(n->get_methods().begin(), n->get_methods().end(),
                [&](auto method) {
                  line << get_indent() << method.vis << " ";
                  method.func.accept(*this);
                  line << std::endl;
                });

  if (!static_methods_count.empty() &&
      (!fields_count.empty() || !methods_count.empty())) {
    line << std::endl;
  }

  std::for_each(n->get_static_methods().begin(), n->get_static_methods().end(),
                [&](auto method) {
                  line << get_indent() << method.vis << " static ";
                  method.func.accept(*this);
                  line << std::endl;
                });

  indent -= tabSize;
  line << "}";
}

void CambrianFormatter::visit(FlowPtr<EnumDef> n) {
  print_line_comments(n);

  line << "enum " << n->get_name();
  if (n->get_type()) {
    line << ": ";
    n->get_type().value().accept(*this);
  }

  if (n->get_items().empty()) {
    line << ";";
    return;
  }

  line << " {" << std::endl;
  indent += tabSize;

  for (auto it = n->get_items().begin(); it != n->get_items().end(); ++it) {
    line << get_indent();
    line << it->first;
    if (it->second) {
      line << " = ";
      it->second.value().accept(*this);
    }
    line << "," << std::endl;
  }

  indent -= tabSize;
  line << get_indent() << "}";
}

void CambrianFormatter::visit(FlowPtr<ScopeStmt> n) {
  print_line_comments(n);

  line << "scope ";

  if (!n->get_name()->empty()) {
    line << n->get_name();
  }

  if (!n->get_deps().empty()) {
    line << ": [";
    iterate_except_last(
        n->get_deps().begin(), n->get_deps().end(),
        [&](auto dep, size_t) { line << dep; }, [&](let) { line << ", "; });
    line << "]";
  }

  line << " ";
  wrap_stmt_body(n->get_body(), 50, true);
}

void CambrianFormatter::visit(FlowPtr<ExportStmt> n) {
  print_line_comments(n);

  line << n->get_vis();

  if (!n->get_abi_name()->empty()) {
    line << " ";
    escape_string_literal(n->get_abi_name());
  }

  if (!n->get_attrs().empty()) {
    line << " [";
    iterate_except_last(
        n->get_attrs().begin(), n->get_attrs().end(),
        [&](auto attr, size_t) { attr.accept(*this); },
        [&](let) { line << ", "; });
    line << "]";
  }

  line << " ";

  wrap_stmt_body(n->get_body(), -1, false);
}
