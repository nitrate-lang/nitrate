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
    bool single_stmt = block->GetItems().size() == 1;
    bool few_children =
        single_stmt && block->RecursiveChildCount() <= size_threshold;

    if (single_stmt && few_children) {
      if (use_arrow_if_wrapped) {
        line << "=> ";
      }

      block->GetItems().front().Accept(*this);
      return;
    }
  }

  n.Accept(*this);
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
  auto range_start = n->Getrange_begin();
  auto range_end = n->Getrange_end();

  if (range_start || range_end) {
    line << ": [";
    if (range_start) range_start.value().Accept(*this);
    line << ":";
    if (range_end) range_end.value().Accept(*this);
    line << "]";
  }

  if (n->GetWidth()) {
    line << ": ";
    n->GetWidth().value().Accept(*this);
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

  n->Getexpr().Accept(*this);
  line << ";";
}

void CambrianFormatter::visit(FlowPtr<StmtExpr> n) {
  print_multiline_comments(n);

  n->Getstmt().Accept(*this);
}

void CambrianFormatter::visit(FlowPtr<TypeExpr> n) {
  print_multiline_comments(n);

  line << "type ";
  n->Gettype().Accept(*this);
}

void CambrianFormatter::visit(FlowPtr<NamedTy> n) {
  print_multiline_comments(n);

  line << n->Getname();
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
      n->Gettemplate()->GetKind() == QAST_NAMED &&
      n->Gettemplate()->as<NamedTy>()->GetName() == "__builtin_result";

  bool is_vector =
      n->Gettemplate()->GetKind() == QAST_NAMED &&
      n->Gettemplate()->as<NamedTy>()->GetName() == "__builtin_vec";

  bool is_map = n->Gettemplate()->GetKind() == QAST_NAMED &&
                n->Gettemplate()->as<NamedTy>()->GetName() == "__builtin_umap";

  bool is_set = n->Gettemplate()->GetKind() == QAST_NAMED &&
                n->Gettemplate()->as<NamedTy>()->GetName() == "__builtin_uset";

  bool is_comptime =
      n->Gettemplate()->GetKind() == QAST_NAMED &&
      n->Gettemplate()->as<NamedTy>()->GetName() == "__builtin_meta" &&
      n->Getargs().size() == 1 &&
      n->Getargs().front().second->is(QAST_UNEXPR) &&
      n->Getargs().front().second.template as<UnaryExpr>()->get_op() ==
          OpComptime;

  const auto print_without_type_keyword = [&](auto node) {
    if (node->is(QAST_TEXPR)) {
      node->template as<TypeExpr>()->get_type().Accept(*this);
    } else {
      node->Accept(*this);
    }
  };

  size_t argc = n->Getargs().size();
  if (is_optional && argc == 1) {
    print_without_type_keyword(n->Getargs().front().second);
    line << "?";
  } else if (is_vector && argc == 1) {
    line << "[";
    print_without_type_keyword(n->Getargs().front().second);
    line << "]";
  } else if (is_map && argc == 2) {
    line << "[";
    print_without_type_keyword(n->Getargs().front().second);
    line << "->";
    print_without_type_keyword(n->Getargs().back().second);
    line << "]";
  } else if (is_set && argc == 1) {
    line << "{";
    print_without_type_keyword(n->Getargs().front().second);
    line << "}";
  } else if (is_comptime) {
    line << "comptime(";
    n->Getargs().front().second.template as<UnaryExpr>()->get_rhs().Accept(
        *this);
    line << ")";
  } else {
    n->Gettemplate().Accept(*this);

    line << "<";
    iterate_except_last(
        n->Getargs().begin(), n->Getargs().end(),
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
  n->Getitem().Accept(*this);

  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<OpaqueTy> n) {
  print_multiline_comments(n);

  line << "opaque(" << n->Getname() << ")";
  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<TupleTy> n) {
  /* If the number of fields exceeds the threshold, arange fields into a
   * matrix of row size ceil(sqrt(n)). */
  print_multiline_comments(n);

  auto wrap_threshold = 8ULL;

  line << "(";

  auto items = n->Getitems();
  auto line_size = line.length();
  auto break_at = items.size() <= wrap_threshold
                      ? wrap_threshold
                      : static_cast<size_t>(std::ceil(std::sqrt(items.size())));

  for (size_t i = 0; i < items.size(); i++) {
    if (i != 0 && i % break_at == 0) {
      line << std::endl << std::string(line_size, ' ');
    }

    auto item = items[i];
    item.Accept(*this);

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
  n->Getitem().Accept(*this);
  line << "; ";
  n->Getsize().Accept(*this);
  line << "]";

  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<RefTy> n) {
  print_multiline_comments(n);

  line << "&";
  n->Getitem().Accept(*this);

  format_type_metadata(n);
}

void CambrianFormatter::visit(FlowPtr<FuncTy> n) {
  print_multiline_comments(n);

  line << "fn";

  if (!n->Getattributes().empty()) {
    line << "[";
    iterate_except_last(
        n->Getattributes().begin(), n->Getattributes().end(),
        [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { line << ", "; });
    line << "] ";
  }

  switch (n->Getpurity()) {
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
      n->Getparams().begin(), n->Getparams().end(),
      [&](auto param, size_t) {
        line << std::get<0>(param);

        if (auto type = std::get<1>(param); type->GetKind() != QAST_INFER) {
          line << ": ";
          type.Accept(*this);
        }

        if (auto def = std::get<2>(param)) {
          line << " = ";
          def.value().Accept(*this);
        }
      },
      [&](let) { line << ", "; });
  if (n->is_variadic()) {
    if (!n->Getparams().empty()) {
      line << ", ";
    }
    line << "...";
  }
  line << ")";

  line << ": ";
  n->Getreturn().Accept(*this);
}

void CambrianFormatter::visit(FlowPtr<UnaryExpr> n) {
  static const std::unordered_set<Operator> WordOps = {
      OpAs,        OpBitcastAs, OpIn,     OpOut,     OpSizeof,
      OpBitsizeof, OpAlignof,   OpTypeof, OpComptime};

  print_multiline_comments(n);

  line << "(" << n->Getop();
  if (WordOps.contains(n->Getop())) {
    line << " ";
  }
  n->Getrhs().Accept(*this);
  line << ")";
}

void CambrianFormatter::visit(FlowPtr<BinExpr> n) {
  print_multiline_comments(n);

  if (n->Getop() == OpDot) {
    n->Getlhs().Accept(*this);
    line << ".";
    n->Getrhs().Accept(*this);
  } else {
    line << "(";
    n->Getlhs().Accept(*this);
    line << " " << n->Getop() << " ";
    n->Getrhs().Accept(*this);
    line << ")";
  }
}

void CambrianFormatter::visit(FlowPtr<PostUnaryExpr> n) {
  print_multiline_comments(n);

  line << "(";
  n->Getlhs().Accept(*this);
  line << n->Getop() << ")";
}

void CambrianFormatter::visit(FlowPtr<TernaryExpr> n) {
  print_multiline_comments(n);

  line << "(";
  n->Getcond().Accept(*this);
  line << " ? ";
  n->Getlhs().Accept(*this);
  line << " : ";
  n->Getrhs().Accept(*this);
  line << ")";
}

void CambrianFormatter::visit(FlowPtr<ConstInt> n) {
  print_multiline_comments(n);

  write_float_literal(n->Getvalue());
}

void CambrianFormatter::visit(FlowPtr<ConstFloat> n) {
  print_multiline_comments(n);

  write_float_literal(n->Getvalue());
}

void CambrianFormatter::visit(FlowPtr<ConstBool> n) {
  print_multiline_comments(n);

  if (n->Getvalue()) {
    line << "true";
  } else {
    line << "false";
  }
}

void CambrianFormatter::visit(FlowPtr<ConstString> n) {
  print_multiline_comments(n);

  escape_string_literal(n->Getvalue());
}

void CambrianFormatter::visit(FlowPtr<ConstChar> n) {
  print_multiline_comments(n);

  line << escape_char_literal(n->Getvalue());
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

  n->Getfunc().Accept(*this);

  size_t argc = n->Getargs().size();

  bool any_named =
      std::any_of(n->Getargs().begin(), n->Getargs().end(), [](CallArg arg) {
        auto name = arg.first;
        return !std::isdigit(name->at(0));
      });

  bool any_lambdas = std::any_of(
      n->Getargs().begin(), n->Getargs().end(),
      [](auto arg) { return std::get<1>(arg)->IsStmtExpr(QAST_FUNCTION); });

  bool is_wrapping = argc >= wrap_threshold || any_named || any_lambdas;

  if (is_wrapping) {
    line << "(";
    size_t line_size = line.length();
    std::swap(indent, line_size);

    for (auto it = n->Getargs().begin(); it != n->Getargs().end(); ++it) {
      auto arg = *it;
      auto name = std::get<0>(arg);
      auto value = std::get<1>(arg);

      if (!std::isdigit(name->at(0))) {
        line << name << ": ";
      }

      value.Accept(*this);

      if (it != n->Getargs().end() - 1) {
        line << ", ";
      }

      if (it != n->Getargs().end() - 1) {
        line << std::endl << get_indent();
      }
    }

    std::swap(indent, line_size);
    line << ")";
  } else {
    line << "(";
    iterate_except_last(
        n->Getargs().begin(), n->Getargs().end(),
        [&](auto arg, size_t) {
          auto name = std::get<0>(arg);
          auto value = std::get<1>(arg);

          if (!std::isdigit(name->at(0))) {
            line << name << ": ";
          }

          value.Accept(*this);
        },
        [&](let) { line << ", "; });
    line << ")";
  }
}

void CambrianFormatter::visit(FlowPtr<TemplCall> n) {
  print_multiline_comments(n);

  n->Getfunc().Accept(*this);

  line << "{";
  iterate_except_last(
      n->Gettemplate_args().begin(), n->Gettemplate_args().end(),
      [&](auto arg, size_t) {
        auto name = std::get<0>(arg);
        auto value = std::get<1>(arg);
        bool should_print_name = !std::isdigit(name->at(0));

        if (should_print_name) {
          line << name << ": ";
        }

        if (value->is(QAST_TEXPR)) {
          value->template as<TypeExpr>()->get_type().Accept(*this);
        } else {
          value.Accept(*this);
        }
      },
      [&](let) { line << ", "; });
  line << "}";

  line << "(";
  iterate_except_last(
      n->Getargs().begin(), n->Getargs().end(),
      [&](auto arg, size_t) {
        auto name = std::get<0>(arg);
        auto value = std::get<1>(arg);

        if (!std::isdigit(name->at(0))) {
          line << name << ": ";
        }

        value.Accept(*this);
      },
      [&](let) { line << ", "; });
  line << ")";
}

void CambrianFormatter::visit(FlowPtr<List> n) {
  print_multiline_comments(n);

  auto wrap_threshold = 8ULL;

  if (n->Getitems().empty()) {
    line << "[]";
    return;
  }

  auto argc = n->Getitems().size();
  bool is_compressing =
      argc >= wrap_threshold &&
      std::all_of(n->Getitems().begin(), n->Getitems().end(),
                  [&](auto x) { return x->IsEq(n->Getitems().front()); });

  if (is_compressing) {
    line << "[";
    n->Getitems().front().Accept(*this);
    line << "; " << argc << "]";
  } else {
    static const std::unordered_set<npar_ty_t> extra_seperation = {
        QAST_TEREXPR, QAST_CALL, QAST_LIST,
        QAST_ASSOC,   QAST_SEQ,  QAST_TEMPL_CALL,
    };

    bool special_case =
        std::any_of(n->Getitems().begin(), n->Getitems().end(), [&](auto x) {
          return extra_seperation.contains(x->GetKind()) ||
                 x->IsStmtExpr(QAST_FUNCTION);
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

        for (size_t i = 0; i < n->Getitems().size(); i++) {
          line << get_indent();
          auto item = n->Getitems()[i];
          item.Accept(*this);

          bool is_last = i == n->Getitems().size() - 1;
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
          std::all_of(n->Getitems().begin(), n->Getitems().end(),
                      [](auto x) { return x->is(QAST_ASSOC); });

      { /* Write list items */
        size_t the_indent = is_assoc_map ? indent + tabSize : line.length();
        std::swap(indent, the_indent);

        for (size_t i = 0; i < n->Getitems().size(); i++) {
          auto item = n->Getitems()[i];
          item.Accept(*this);

          bool is_last = i == n->Getitems().size() - 1;
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
        if (n->Getvalue()->is(QAST_LIST)) {
          auto list = n->Getvalue()->as<List>();
          is_value_map =
              list->GetItems().empty() ||
              std::all_of(list->GetItems().begin(), list->GetItems().end(),
                          [](auto x) { return x->is(QAST_ASSOC); });
        }

        if (use_braces) {
          line << "{" << std::endl;
          indent += tabSize;
          line << get_indent();
        }

        n->Getkey().Accept(*this);
        line << ": ";

        if (is_value_map) {
          auto list = n->Getvalue()->as<List>();

          if (list->GetItems().empty()) {
            line << "{}";
          } else {
            line << "{" << std::endl;
            indent += tabSize;

            for (auto it = list->GetItems().begin();
                 it != list->GetItems().end(); ++it) {
              line << get_indent();

              format(it->as<Assoc>(), false);

              if (it != list->GetItems().end() - 1) {
                line << ",";
              }

              line << std::endl;
            }

            indent -= tabSize;
            line << get_indent() << "}";
          }
        } else {
          n->Getvalue().Accept(*this);
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

  n->Getbase().Accept(*this);
  line << "[";
  n->Getindex().Accept(*this);
  line << "]";
}

void CambrianFormatter::visit(FlowPtr<Slice> n) {
  print_multiline_comments(n);

  n->Getbase().Accept(*this);
  line << "[";
  if (n->Getstart()) {
    n->Getstart().Accept(*this);
  }
  line << ":";
  if (n->Getend()) {
    n->Getend().Accept(*this);
  }
  line << "]";
}

void CambrianFormatter::visit(FlowPtr<FString> n) {
  print_multiline_comments(n);

  line << "f\"";
  for (auto part : n->Getitems()) {
    if (std::holds_alternative<ncc::string>(part)) {
      escape_string_literal(*std::get<ncc::string>(part), false);
    } else {
      line << "{";
      std::get<FlowPtr<Expr>>(part).Accept(*this);
      line << "}";
    }
  }
  line << "\"";
}

void CambrianFormatter::visit(FlowPtr<Ident> n) {
  print_multiline_comments(n);

  line << n->Getname();
}

void CambrianFormatter::visit(FlowPtr<SeqPoint> n) {
  print_multiline_comments(n);

  line << "(";
  iterate_except_last(
      n->Getitems().begin(), n->Getitems().end(),
      [&](auto item, size_t) { item.Accept(*this); },
      [&](let) { line << ", "; });
  line << ")";
}

void CambrianFormatter::visit(FlowPtr<Block> n) {
  print_line_comments(n);

  bool isRootBlock = !did_root;
  did_root = true;

  switch (n->Getsafety()) {
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

  if (!isRootBlock && n->Getitems().empty()) {
    line << "{}";
    return;
  }

  if (!isRootBlock) {
    line << "{" << std::endl;
    indent += tabSize;
  }

  auto items = n->Getitems();

  for (auto it = items.begin(); it != items.end(); ++it) {
    auto item = *it;

    line << get_indent();
    item.Accept(*this);
    line << std::endl;

    bool is_last_item = it == items.end() - 1;

    bool is_next_item_different =
        (it + 1 != items.end() &&
         (*std::next(it))->GetKind() != item->GetKind());

    bool extra_newline =
        !is_last_item &&
        (is_next_item_different || extra_seperation.contains(item->GetKind()));

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

  switch (n->Getdecl_type()) {
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

  if (!n->Getattributes().empty()) {
    line << "[";
    iterate_except_last(
        n->Getattributes().begin(), n->Getattributes().end(),
        [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { line << ", "; });
    line << "] ";
  }

  line << n->Getname();

  if (n->Gettype()) {
    line << ": ";
    n->Gettype().value().Accept(*this);
  }

  if (n->Getvalue()) {
    line << " = ";
    n->Getvalue().value().Accept(*this);
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
  n->Getcond().Accept(*this);
  line << " ";
  n->Getthen().Accept(*this);

  if (n->Getelse()) {
    line << " else ";
    n->Getelse().value().Accept(*this);
  }

  line << ";";
}

void CambrianFormatter::visit(FlowPtr<WhileStmt> n) {
  print_line_comments(n);

  line << "while ";
  n->Getcond().Accept(*this);
  line << " ";
  n->Getbody().Accept(*this);

  line << ";";
}

void CambrianFormatter::visit(FlowPtr<ForStmt> n) {
  print_line_comments(n);

  line << "for (";

  if (n->Getinit().has_value()) {
    n->Getinit().value().Accept(*this);
    if (!n->Getinit().value()->IsStmt()) {
      line << ";";
    }
  } else {
    line << ";";
  }

  if (n->Getcond().has_value()) {
    line << " ";
    n->Getcond().value().Accept(*this);
  }
  line << ";";

  if (n->Getstep().has_value()) {
    line << " ";
    n->Getstep().value().Accept(*this);
  }

  line << ") ";
  n->Getbody().Accept(*this);

  line << ";";
}

void CambrianFormatter::visit(FlowPtr<ForeachStmt> n) {
  print_line_comments(n);

  line << "foreach (";
  if (n->Getidx_ident()->empty()) {
    line << n->Getval_ident();
  } else {
    line << n->Getidx_ident() << ", " << n->Getval_ident();
  }

  line << " in ";
  n->Getexpr().Accept(*this);
  line << ") ";

  n->Getbody().Accept(*this);

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

  if (n->Getvalue().has_value()) {
    line << "ret ";
    n->Getvalue().value().Accept(*this);
    line << ";";
  } else {
    line << "ret;";
  }
}

void CambrianFormatter::visit(FlowPtr<ReturnIfStmt> n) {
  print_line_comments(n);

  line << "retif ";
  n->Getcond().Accept(*this);
  line << ", ";
  n->Getvalue().Accept(*this);
  line << ";";
}

void CambrianFormatter::visit(FlowPtr<CaseStmt> n) {
  print_line_comments(n);

  n->Getcond().Accept(*this);
  line << " => ";
  wrap_stmt_body(n->Getbody(), 10, false);
}

void CambrianFormatter::visit(FlowPtr<SwitchStmt> n) {
  print_line_comments(n);

  line << "switch ";
  n->Getcond().Accept(*this);
  line << " {" << std::endl;
  indent += tabSize;

  for (auto c : n->Getcases()) {
    line << get_indent();
    c.Accept(*this);
    line << std::endl;
  }

  if (n->Getdefault()) {
    line << get_indent();
    line << "_ => ";
    wrap_stmt_body(n->Getdefault().value(), 10, false);
    line << std::endl;
  }

  indent -= tabSize;
  line << get_indent() << "}";
}

void CambrianFormatter::visit(FlowPtr<TypedefStmt> n) {
  print_line_comments(n);

  line << "type " << n->Getname() << " = ";
  n->Gettype().Accept(*this);
  line << ";";
}

void CambrianFormatter::visit(FlowPtr<Function> n) {
  print_line_comments(n);

  line << "fn";

  if (!n->Getattributes().empty()) {
    line << " [";
    iterate_except_last(
        n->Getattributes().begin(), n->Getattributes().end(),
        [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { line << ", "; });
    line << "]";
  }

  switch (n->Getpurity()) {
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

  if (!n->Getcaptures().empty()) {
    line << " [";
    iterate_except_last(
        n->Getcaptures().begin(), n->Getcaptures().end(),
        [&](auto cap, size_t) {
          if (cap.second) {
            line << "&";
          }
          line << cap.first;
        },
        [&](let) { line << ", "; });
    line << "]";
  }

  line << " " << n->Getname();

  if (n->Gettemplate_params()) {
    line << "<";
    iterate_except_last(
        n->Gettemplate_params().value().begin(),
        n->Gettemplate_params().value().end(),
        [&](auto param, size_t) {
          line << std::get<0>(param);

          if (let type = std::get<1>(param)) {
            if (type->GetKind() != QAST_INFER) {
              line << ": ";
              type->Accept(*this);
            }
          }

          if (auto val = std::get<2>(param)) {
            line << " = ";
            val.value().Accept(*this);
          }
        },
        [&](let) { line << ", "; });
    line << ">";
  }

  line << "(";
  iterate_except_last(
      n->Getparams().begin(), n->Getparams().end(),
      [&](auto param, size_t) {
        line << std::get<0>(param);

        if (let type = std::get<1>(param)) {
          if (type->GetKind() != QAST_INFER) {
            line << ": ";
            type->Accept(*this);
          }
        }

        if (auto def = std::get<2>(param)) {
          line << " = ";
          def.value().Accept(*this);
        }
      },
      [&](let) { line << ", "; });

  if (n->is_variadic()) {
    if (!n->Getparams().empty()) {
      line << ", ";
    }
    line << "...";
  }
  line << ")";

  { /* Return type */
    auto return_type = n->Getreturn();

    if (!return_type->is(QAST_INFER)) {
      line << ": ";
      return_type.Accept(*this);
    }
  }

  if (n->is_declaration()) {
    line << ";";
  } else {
    line << " ";
    wrap_stmt_body(n->Getbody().value(), 10, true);
  }
}

void CambrianFormatter::visit(FlowPtr<StructDef> n) {
  print_line_comments(n);

  switch (n->Getcomposite_type()) {
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

  if (!n->Getattributes().empty()) {
    line << "[";
    iterate_except_last(
        n->Getattributes().begin(), n->Getattributes().end(),
        [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { line << ", "; });
    line << "] ";
  }

  line << n->Getname();
  if (n->Gettemplate_params().has_value()) {
    line << "<";
    iterate_except_last(
        n->Gettemplate_params().value().begin(),
        n->Gettemplate_params().value().end(),
        [&](auto param, size_t) {
          line << std::get<0>(param);
          if (auto type = std::get<1>(param); type->GetKind() != QAST_INFER) {
            line << ": ";
            type.Accept(*this);
          }
          if (auto val = std::get<2>(param)) {
            line << " = ";
            val.value().Accept(*this);
          }
        },
        [&](let) { line << ", "; });
    line << ">";
  }

  if (!n->Getnames().empty()) {
    line << ": ";
    iterate_except_last(
        n->Getnames().begin(), n->Getnames().end(),
        [&](auto name, size_t) { line << name; }, [&](let) { line << ", "; });
  }

  bool is_empty = n->Getfields().empty() && n->Getmethods().empty() &&
                  n->Getstatic_methods().empty();

  if (is_empty) {
    line << " {}";
    return;
  }

  line << " {" << std::endl;
  indent += tabSize;

  auto fields_count = n->Getfields();
  auto methods_count = n->Getmethods();
  auto static_methods_count = n->Getstatic_methods();

  std::for_each(n->Getfields().begin(), n->Getfields().end(), [&](auto field) {
    line << get_indent() << field.get_vis() << " ";

    line << field.GetName() << ": ";
    field.get_type().Accept(*this);

    if (field.get_value().has_value()) {
      line << " = ";
      field.get_value().value().Accept(*this);
    }

    line << "," << std::endl;
  });

  if (!fields_count.empty() && !methods_count.empty()) {
    line << std::endl;
  }

  std::for_each(n->Getmethods().begin(), n->Getmethods().end(),
                [&](auto method) {
                  line << get_indent() << method.vis << " ";
                  method.func.Accept(*this);
                  line << std::endl;
                });

  if (!static_methods_count.empty() &&
      (!fields_count.empty() || !methods_count.empty())) {
    line << std::endl;
  }

  std::for_each(n->Getstatic_methods().begin(), n->Getstatic_methods().end(),
                [&](auto method) {
                  line << get_indent() << method.vis << " static ";
                  method.func.Accept(*this);
                  line << std::endl;
                });

  indent -= tabSize;
  line << "}";
}

void CambrianFormatter::visit(FlowPtr<EnumDef> n) {
  print_line_comments(n);

  line << "enum " << n->Getname();
  if (n->Gettype()) {
    line << ": ";
    n->Gettype().value().Accept(*this);
  }

  if (n->Getitems().empty()) {
    line << ";";
    return;
  }

  line << " {" << std::endl;
  indent += tabSize;

  for (auto it = n->Getitems().begin(); it != n->Getitems().end(); ++it) {
    line << get_indent();
    line << it->first;
    if (it->second) {
      line << " = ";
      it->second.value().Accept(*this);
    }
    line << "," << std::endl;
  }

  indent -= tabSize;
  line << get_indent() << "}";
}

void CambrianFormatter::visit(FlowPtr<ScopeStmt> n) {
  print_line_comments(n);

  line << "scope ";

  if (!n->Getname()->empty()) {
    line << n->Getname();
  }

  if (!n->Getdeps().empty()) {
    line << ": [";
    iterate_except_last(
        n->Getdeps().begin(), n->Getdeps().end(),
        [&](auto dep, size_t) { line << dep; }, [&](let) { line << ", "; });
    line << "]";
  }

  line << " ";
  wrap_stmt_body(n->Getbody(), 50, true);
}

void CambrianFormatter::visit(FlowPtr<ExportStmt> n) {
  print_line_comments(n);

  line << n->Getvis();

  if (!n->Getabi_name()->empty()) {
    line << " ";
    escape_string_literal(n->Getabi_name());
  }

  if (!n->Getattrs().empty()) {
    line << " [";
    iterate_except_last(
        n->Getattrs().begin(), n->Getattrs().end(),
        [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { line << ", "; });
    line << "]";
  }

  line << " ";

  wrap_stmt_body(n->Getbody(), -1, false);
}
