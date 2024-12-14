#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <nitrate-lexer/Lexer.h>

#include <lsp/lang/CambrianStyleFormatter.hh>
#include <nitrate-parser/AST.hh>
#include <sstream>
#include <unordered_set>

using namespace lsp::fmt;
using namespace npar;

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
CambrianFormatter::LineStreamWritter::operator<<(qlex_op_t op) {
  m_line_buffer << qlex_opstr(op);
  return *this;
}

CambrianFormatter::LineStreamWritter&
CambrianFormatter::LineStreamWritter::operator<<(npar::Vis v) {
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

void CambrianFormatter::escape_string_literal(std::string_view str,
                                              bool put_quotes) {
  constexpr size_t max_chunk_size = 60;

  if (str.empty()) {
    if (put_quotes) {
      line << "\"\"";
    }
    return;
  }

  let chunks_n = str.size() / max_chunk_size;
  let rem = str.size() % max_chunk_size;
  let line_size = line.length();

  if (chunks_n) {
    std::vector<std::string> chunks(chunks_n);

    for (size_t i = 0; i < chunks_n; i++) {
      chunks[i] = "\"" +
                  escape_string_literal_chunk(
                      str.substr(i * max_chunk_size, max_chunk_size)) +
                  "\"";
    }

    let max_segment_size =
        std::max_element(chunks.begin(), chunks.end(), [](let a, let b) {
          return a.size() < b.size();
        })->size();

    for (size_t i = 0; i < chunks.size(); ++i) {
      if (i != 0 && line_size) {
        line << std::string(line_size, ' ');
      }

      line << chunks[i];

      let rpad = (max_segment_size - chunks[i].size());
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

void CambrianFormatter::format_type_metadata(Type const& n) {
  let range = n.get_range();

  if (range.first || range.second) {
    line << ": [";
    if (range.first) range.first->accept(*this);
    line << ":";
    if (range.second) range.second->accept(*this);
    line << "]";
  }

  if (n.get_width()) {
    line << ": ";
    n.get_width()->accept(*this);
  }
}

void CambrianFormatter::visit(npar_node_t const&) {
  /** This node symbolizes a placeholder value in the event of an error. */
  failed = true;

  line << "/* !!! */";
}

void CambrianFormatter::visit(ExprStmt const& n) {
  n.get_expr()->accept(*this);
  line << ";";
}

void CambrianFormatter::visit(StmtExpr const& n) {
  n.get_stmt()->accept(*this);
}

void CambrianFormatter::visit(TypeExpr const& n) {
  n.get_type()->accept(*this);
}

void CambrianFormatter::visit(NamedTy const& n) {
  line << n.get_name();
  format_type_metadata(n);
}

void CambrianFormatter::visit(InferTy const& n) {
  line << "?";
  format_type_metadata(n);
}

void CambrianFormatter::visit(TemplType const& n) {
  bool is_optional =
      n.get_template()->getKind() == QAST_NAMED &&
      n.get_template()->as<NamedTy>()->get_name() == "__builtin_result";

  bool is_vector =
      n.get_template()->getKind() == QAST_NAMED &&
      n.get_template()->as<NamedTy>()->get_name() == "__builtin_vec";

  bool is_map = n.get_template()->getKind() == QAST_NAMED &&
                n.get_template()->as<NamedTy>()->get_name() == "__builtin_umap";

  bool is_set = n.get_template()->getKind() == QAST_NAMED &&
                n.get_template()->as<NamedTy>()->get_name() == "__builtin_uset";

  size_t argc = n.get_args().size();
  if (is_optional && argc == 1) {
    n.get_args().front().second->accept(*this);
    line << "?";
  } else if (is_vector && argc == 1) {
    line << "[";
    n.get_args().front().second->accept(*this);
    line << "]";
  } else if (is_map && argc == 2) {
    line << "[";
    n.get_args().front().second->accept(*this);
    line << "->";
    n.get_args().back().second->accept(*this);
    line << "]";
  } else if (is_set && argc == 1) {
    line << "{";
    n.get_args().front().second->accept(*this);
    line << "}";
  } else {
    n.get_template()->accept(*this);

    line << "<";
    iterate_except_last(
        n.get_args().begin(), n.get_args().end(),
        [&](let arg, size_t) {
          if (!std::isdigit(arg.first->at(0))) {
            line << arg.first << ": ";
          }
          arg.second->accept(*this);
        },
        [&](let) { line << ", "; });
    line << ">";
  }

  format_type_metadata(n);
}

void CambrianFormatter::visit(U1 const& n) {
  line << "u1";
  format_type_metadata(n);
}

void CambrianFormatter::visit(U8 const& n) {
  line << "u8";
  format_type_metadata(n);
}

void CambrianFormatter::visit(U16 const& n) {
  line << "u16";
  format_type_metadata(n);
}

void CambrianFormatter::visit(U32 const& n) {
  line << "u32";
  format_type_metadata(n);
}

void CambrianFormatter::visit(U64 const& n) {
  line << "u64";
  format_type_metadata(n);
}

void CambrianFormatter::visit(U128 const& n) {
  line << "u128";
  format_type_metadata(n);
}

void CambrianFormatter::visit(I8 const& n) {
  line << "i8";
  format_type_metadata(n);
}

void CambrianFormatter::visit(I16 const& n) {
  line << "i16";
  format_type_metadata(n);
}

void CambrianFormatter::visit(I32 const& n) {
  line << "i32";
  format_type_metadata(n);
}

void CambrianFormatter::visit(I64 const& n) {
  line << "i64";
  format_type_metadata(n);
}

void CambrianFormatter::visit(I128 const& n) {
  line << "i128";
  format_type_metadata(n);
}

void CambrianFormatter::visit(F16 const& n) {
  line << "f16";
  format_type_metadata(n);
}

void CambrianFormatter::visit(F32 const& n) {
  line << "f32";
  format_type_metadata(n);
}

void CambrianFormatter::visit(F64 const& n) {
  line << "f64";
  format_type_metadata(n);
}

void CambrianFormatter::visit(F128 const& n) {
  line << "f128";
  format_type_metadata(n);
}

void CambrianFormatter::visit(VoidTy const& n) {
  line << "void";
  format_type_metadata(n);
}

void CambrianFormatter::visit(PtrTy const& n) {
  line << "*";
  n.get_item()->accept(*this);

  format_type_metadata(n);
}

void CambrianFormatter::visit(OpaqueTy const& n) {
  line << "opaque(" << n.get_name() << ")";
  format_type_metadata(n);
}

void CambrianFormatter::visit(TupleTy const& n) {
  /* If the number of fields exceeds the threshold, arange fields into a
   * matrix of row size ceil(sqrt(n)). */

  let wrap_threshold = 8ULL;

  line << "(";

  let items = n.get_items();
  let line_size = line.length();
  let break_at = items.size() <= wrap_threshold
                     ? wrap_threshold
                     : static_cast<size_t>(std::ceil(std::sqrt(items.size())));

  for (size_t i = 0; i < items.size(); i++) {
    if (i != 0 && i % break_at == 0) {
      line << std::endl << std::string(line_size, ' ');
    }

    let item = items[i];
    item->accept(*this);

    if (i != items.size() - 1) {
      line << ", ";
    }
  }
  line << ")";

  format_type_metadata(n);
}

void CambrianFormatter::visit(ArrayTy const& n) {
  line << "[";
  n.get_item()->accept(*this);
  line << "; ";
  n.get_size()->accept(*this);
  line << "]";

  format_type_metadata(n);
}

void CambrianFormatter::visit(RefTy const& n) {
  line << "&";
  n.get_item()->accept(*this);

  format_type_metadata(n);
}

void CambrianFormatter::visit(FuncTy const& n) {
  line << "fn";

  switch (n.get_purity()) {
    case FuncPurity::IMPURE_THREAD_UNSAFE: {
      line << " impure";
      break;
    }

    case FuncPurity::IMPURE_THREAD_SAFE: {
      line << " impure tsafe";
      break;
    }

    case FuncPurity::PURE: {
      line << " pure";
      break;
    }

    case FuncPurity::QUASI: {
      line << " quasi";
      break;
    }

    case FuncPurity::RETRO: {
      line << " retro";
      break;
    }
  }

  if (n.is_foreign()) {
    line << " foreign";
  }

  line << "(";
  iterate_except_last(
      n.get_params().begin(), n.get_params().end(),
      [&](let param, size_t) {
        let name = std::get<0>(param);
        let type = std::get<1>(param);
        let def = std::get<2>(param);

        line << name;

        if (type->getKind() != QAST_INFER) {
          line << ": ";
          type->accept(*this);
        }

        if (def) {
          line << " = ";
          def->accept(*this);
        }
      },
      [&](let) { line << ", "; });
  if (n.is_variadic()) {
    if (!n.get_params().empty()) {
      line << ", ";
    }
    line << "...";
  }
  line << ")";

  if (n.is_noreturn()) {
    line << ": null";
  } else {
    line << ": ";
    n.get_return_ty()->accept(*this);
  }
}

void CambrianFormatter::visit(UnaryExpr const& n) {
  line << "(" << n.get_op();
  n.get_rhs()->accept(*this);
  line << ")";
}

void CambrianFormatter::visit(BinExpr const& n) {
  line << "(";
  n.get_lhs()->accept(*this);
  line << " " << n.get_op() << " ";
  n.get_rhs()->accept(*this);
  line << ")";
}

void CambrianFormatter::visit(PostUnaryExpr const& n) {
  line << "(";
  n.get_lhs()->accept(*this);
  line << n.get_op() << ")";
}

void CambrianFormatter::visit(TernaryExpr const& n) {
  line << "(";
  n.get_cond()->accept(*this);
  line << " ? ";
  n.get_lhs()->accept(*this);
  line << " : ";
  n.get_rhs()->accept(*this);
  line << ")";
}

void CambrianFormatter::visit(ConstInt const& n) { line << n.get_value(); }

void CambrianFormatter::visit(ConstFloat const& n) {
  write_float_literal(*n.get_value());
}

void CambrianFormatter::visit(ConstBool const& n) {
  if (n.get_value()) {
    line << "true";
  } else {
    line << "false";
  }
}

void CambrianFormatter::visit(ConstString const& n) {
  escape_string_literal(*n.get_value());
}

void CambrianFormatter::visit(ConstChar const& n) {
  line << escape_char_literal(n.get_value());
}

void CambrianFormatter::visit(ConstNull const&) { line << "null"; }

void CambrianFormatter::visit(ConstUndef const&) { line << "undef"; }

void CambrianFormatter::visit(Call const& n) {
  let wrap_threshold = 8ULL;

  n.get_func()->accept(*this);

  size_t argc = n.get_args().size();

  bool any_named =
      std::any_of(n.get_args().begin(), n.get_args().end(), [](let arg) {
        let name = std::get<0>(arg);
        return !std::isdigit(name->at(0));
      });

  bool any_lambdas = std::any_of(
      n.get_args().begin(), n.get_args().end(),
      [](let arg) { return std::get<1>(arg)->is_stmt_expr(QAST_FUNCTION); });

  bool is_wrapping = argc >= wrap_threshold || any_named || any_lambdas;

  if (is_wrapping) {
    line << "(";
    size_t line_size = line.length();
    std::swap(indent, line_size);

    for (auto it = n.get_args().begin(); it != n.get_args().end(); ++it) {
      let arg = *it;
      let name = std::get<0>(arg);
      let value = std::get<1>(arg);

      if (!std::isdigit(name->at(0))) {
        line << name << ": ";
      }

      value->accept(*this);

      if (it != n.get_args().end() - 1) {
        line << ", ";
      }

      if (it != n.get_args().end() - 1) {
        line << std::endl << get_indent();
      }
    }

    std::swap(indent, line_size);
    line << ")";
  } else {
    line << "(";
    iterate_except_last(
        n.get_args().begin(), n.get_args().end(),
        [&](let arg, size_t) {
          let name = std::get<0>(arg);
          let value = std::get<1>(arg);

          if (!std::isdigit(name->at(0))) {
            line << name << ": ";
          }

          value->accept(*this);
        },
        [&](let) { line << ", "; });
    line << ")";
  }
}

void CambrianFormatter::visit(TemplCall const& n) {
  n.get_func()->accept(*this);

  line << "<";
  iterate_except_last(
      n.get_template_args().begin(), n.get_template_args().end(),
      [&](let arg, size_t) {
        let name = std::get<0>(arg);
        let value = std::get<1>(arg);

        if (!std::isdigit(name->at(0))) {
          line << name << ": ";
        }

        value->accept(*this);
      },
      [&](let) { line << ", "; });
  line << ">";

  line << "(";
  iterate_except_last(
      n.get_args().begin(), n.get_args().end(),
      [&](let arg, size_t) {
        let name = std::get<0>(arg);
        let value = std::get<1>(arg);

        if (!std::isdigit(name->at(0))) {
          line << name << ": ";
        }

        value->accept(*this);
      },
      [&](let) { line << ", "; });
  line << ")";
}

void CambrianFormatter::visit(List const& n) {
  let wrap_threshold = 8ULL;

  if (n.get_items().empty()) {
    line << "[]";
    return;
  }

  let argc = n.get_items().size();
  bool is_compressing =
      argc >= wrap_threshold &&
      std::all_of(n.get_items().begin(), n.get_items().end(),
                  [&](let x) { return x->isSame(n.get_items().front()); });

  if (is_compressing) {
    line << "[";
    n.get_items().front()->accept(*this);
    line << "; " << argc << "]";
  } else {
    let break_at = argc <= wrap_threshold
                       ? wrap_threshold
                       : static_cast<size_t>(std::ceil(std::sqrt(argc)));

    line << "[";

    { /* Write list items */
      size_t line_size = line.length();
      std::swap(indent, line_size);

      for (size_t i = 0; i < n.get_items().size(); i++) {
        let item = n.get_items()[i];
        item->accept(*this);

        bool is_last = i == n.get_items().size() - 1;
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

      std::swap(indent, line_size);
    }

    line << "]";
  }
}

void CambrianFormatter::visit(Assoc const& n) {
  line << "{";
  n.get_key()->accept(*this);
  line << ": ";
  n.get_value()->accept(*this);
  line << "}";
}

void CambrianFormatter::visit(Field const& n) {
  n.get_base()->accept(*this);
  line << "." << n.get_field();
}

void CambrianFormatter::visit(Index const& n) {
  n.get_base()->accept(*this);
  line << "[";
  n.get_index()->accept(*this);
  line << "]";
}

void CambrianFormatter::visit(Slice const& n) {
  n.get_base()->accept(*this);
  line << "[";
  if (n.get_start()) {
    n.get_start()->accept(*this);
  }
  line << ":";
  if (n.get_end()) {
    n.get_end()->accept(*this);
  }
  line << "]";
}

void CambrianFormatter::visit(FString const& n) {
  line << "f\"";
  for (let part : n.get_items()) {
    if (std::holds_alternative<SmallString>(part)) {
      escape_string_literal(*std::get<SmallString>(part), false);
    } else {
      line << "{";
      std::get<Expr*>(part)->accept(*this);
      line << "}";
    }
  }
  line << "\"";
}

void CambrianFormatter::visit(Ident const& n) { line << n.get_name(); }

void CambrianFormatter::visit(SeqPoint const& n) {
  line << "(";
  iterate_except_last(
      n.get_items().begin(), n.get_items().end(),
      [&](let item, size_t) { item->accept(*this); },
      [&](let) { line << ", "; });
  line << ")";
}

void CambrianFormatter::visit(Block const& n) {
  bool isRootBlock = !did_root;
  did_root = true;

  switch (n.get_safety()) {
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

  if (!isRootBlock && n.get_items().empty()) {
    line << "{}";
    return;
  }

  if (!isRootBlock) {
    line << "{" << std::endl;
    indent += tabSize;
  }

  let items = n.get_items();

  for (auto it = items.begin(); it != items.end(); ++it) {
    let item = *it;

    line << get_indent();
    item->accept(*this);
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

void CambrianFormatter::visit(VarDecl const& n) {
  switch (n.get_decl_type()) {
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

  if (!n.get_attributes().empty()) {
    line << "[";
    iterate_except_last(
        n.get_attributes().begin(), n.get_attributes().end(),
        [&](let attr, size_t) { attr->accept(*this); },
        [&](let) { line << ", "; });
    line << "] ";
  }

  line << n.get_name();

  if (n.get_type()) {
    line << ": ";
    n.get_type()->accept(*this);
  }

  if (n.get_value()) {
    line << " = ";
    n.get_value()->accept(*this);
  }

  line << ";";
}

void CambrianFormatter::visit(InlineAsm const&) {
  /* Support for inline assembly is not avaliable yet */

  failed = true;

  line << "/* !!! */";
}

void CambrianFormatter::visit(IfStmt const& n) {
  line << "if ";
  n.get_cond()->accept(*this);
  line << " ";
  n.get_then()->accept(*this);

  if (n.get_else()) {
    line << " else ";
    n.get_else()->accept(*this);
  }

  line << ";";
}

void CambrianFormatter::visit(WhileStmt const& n) {
  line << "while ";
  n.get_cond()->accept(*this);
  line << " ";
  n.get_body()->accept(*this);

  line << ";";
}

void CambrianFormatter::visit(ForStmt const& n) {
  line << "for (";

  if (n.get_init().has_value()) {
    n.get_init().value()->accept(*this);
    if (!n.get_init().value()->is_stmt()) {
      line << ";";
    }
  } else {
    line << ";";
  }

  if (n.get_cond().has_value()) {
    line << " ";
    n.get_cond().value()->accept(*this);
  }
  line << ";";

  if (n.get_step().has_value()) {
    line << " ";
    n.get_step().value()->accept(*this);
  }

  line << ") ";
  n.get_body()->accept(*this);

  line << ";";
}

void CambrianFormatter::visit(ForeachStmt const& n) {
  line << "foreach (";
  if (n.get_idx_ident()->empty()) {
    line << n.get_val_ident();
  } else {
    line << n.get_idx_ident() << ", " << n.get_val_ident();
  }

  line << " in ";
  n.get_expr()->accept(*this);
  line << ") ";

  n.get_body()->accept(*this);

  line << ";";
}

void CambrianFormatter::visit(BreakStmt const&) { line << "break;"; }

void CambrianFormatter::visit(ContinueStmt const&) { line << "continue;"; }

void CambrianFormatter::visit(ReturnStmt const& n) {
  if (n.get_value().has_value()) {
    line << "ret ";
    n.get_value().value()->accept(*this);
    line << ";";
  } else {
    line << "ret;";
  }
}

void CambrianFormatter::visit(ReturnIfStmt const& n) {
  line << "retif ";
  n.get_cond()->accept(*this);
  line << ", ";
  n.get_value()->accept(*this);
  line << ";";
}

void CambrianFormatter::visit(CaseStmt const& n) {
  n.get_cond()->accept(*this);
  line << " {";
  n.get_body()->accept(*this);
  line << "};";
}

void CambrianFormatter::visit(SwitchStmt const& n) {
  line << "switch ";
  n.get_cond()->accept(*this);
  line << " {";
  for (let c : n.get_cases()) {
    c->accept(*this);
  }
  if (n.get_default()) {
    n.get_default()->accept(*this);
  }

  line << "};";
}

void CambrianFormatter::visit(TypedefStmt const& n) {
  line << "type " << n.get_name() << " = ";
  n.get_type()->accept(*this);
  line << ";";
}

void CambrianFormatter::visit(FnDef const& n) {
  line << "fn";

  switch (n.get_type()->get_purity()) {
    case FuncPurity::IMPURE_THREAD_UNSAFE: {
      line << " impure";
      break;
    }

    case FuncPurity::IMPURE_THREAD_SAFE: {
      line << " impure tsafe";
      break;
    }

    case FuncPurity::PURE: {
      line << " pure";
      break;
    }

    case FuncPurity::QUASI: {
      line << " quasi";
      break;
    }

    case FuncPurity::RETRO: {
      line << " retro";
      break;
    }
  }

  if (n.get_type()->is_foreign()) {
    line << " foreign";
  }

  line << " " << n.get_name();

  if (n.get_template_params().has_value()) {
    line << "<";
    iterate_except_last(
        n.get_template_params().value().begin(),
        n.get_template_params().value().end(),
        [&](let param, size_t) {
          line << std::get<0>(param) << ": ";
          std::get<1>(param)->accept(*this);
          let val = std::get<2>(param);
          if (val) {
            line << " = ";
            val->accept(*this);
          }
        },
        [&](let) { line << ", "; });
    line << ">";
  }

  line << "(";
  iterate_except_last(
      n.get_type()->get_params().begin(), n.get_type()->get_params().end(),
      [&](let param, size_t) {
        let name = std::get<0>(param);
        let type = std::get<1>(param);
        let def = std::get<2>(param);

        line << name << ": ";
        type->accept(*this);
        if (def) {
          line << " = ";
          def->accept(*this);
        }
      },
      [&](let) { line << ", "; });
  line << ")";

  if (n.get_type()->is_noreturn()) {
    line << ": null";
  } else {
    line << ": ";
    n.get_type()->get_return_ty()->accept(*this);
  }

  if (n.is_decl()) {
    line << ";";
  } else {
    line << " ";
    n.get_body().value()->accept(*this);
  }
}

void CambrianFormatter::visit(StructDef const& n) {
  switch (n.get_composite_type()) {
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

  if (!n.get_attributes().empty()) {
    line << "[";
    iterate_except_last(
        n.get_attributes().begin(), n.get_attributes().end(),
        [&](let attr, size_t) { attr->accept(*this); },
        [&](let) { line << ", "; });
    line << "] ";
  }

  line << n.get_name();
  if (n.get_template_params().has_value()) {
    line << "<";
    iterate_except_last(
        n.get_template_params().value().begin(),
        n.get_template_params().value().end(),
        [&](let param, size_t) {
          line << std::get<0>(param) << ": ";
          std::get<1>(param)->accept(*this);
          let val = std::get<2>(param);
          if (val) {
            line << " = ";
            val->accept(*this);
          }
        },
        [&](let) { line << ", "; });
    line << ">";
  }

  if (!n.get_names().empty()) {
    line << ": ";
    iterate_except_last(
        n.get_names().begin(), n.get_names().end(),
        [&](let name, size_t) { line << name; }, [&](let) { line << ", "; });
  }

  bool is_empty = n.get_fields().empty() && n.get_methods().empty() &&
                  n.get_static_methods().empty();

  if (is_empty) {
    line << " {}";
    return;
  }

  line << " {" << std::endl;
  indent += tabSize;

  std::for_each(n.get_fields().begin(), n.get_fields().end(), [&](let field) {
    line << get_indent() << field.get_vis() << " ";

    line << field.get_name() << ": ";
    field.get_type()->accept(*this);

    if (field.get_value().has_value()) {
      line << " = ";
      field.get_value().value()->accept(*this);
    }

    line << "," << std::endl;
  });

  std::for_each(n.get_methods().begin(), n.get_methods().end(),
                [&](let method) {
                  line << get_indent() << method.vis << " ";
                  method.func->accept(*this);
                  line << std::endl;
                });

  std::for_each(n.get_static_methods().begin(), n.get_static_methods().end(),
                [&](let method) {
                  line << get_indent() << method.vis << " ";
                  method.func->accept(*this);
                  line << std::endl;
                });

  indent -= tabSize;
  line << "}";
}

void CambrianFormatter::visit(EnumDef const& n) {
  line << "enum " << n.get_name();
  if (n.get_type()) {
    line << ": ";
    n.get_type()->accept(*this);
  }

  line << " {";
  std::for_each(n.get_items().begin(), n.get_items().end(), [&](let item) {
    line << item.first;
    if (item.second) {
      line << " = ";
      item.second->accept(*this);
    }
    line << "," << std::endl;
  });
  line << "};";
}

void CambrianFormatter::visit(ScopeStmt const& n) {
  line << "scope";

  if (!n.get_name()->empty()) {
    line << " " << n.get_name();
  }

  if (!n.get_deps().empty()) {
    line << " [";
    iterate_except_last(
        n.get_deps().begin(), n.get_deps().end(),
        [&](let dep, size_t) { line << dep; }, [&](let) { line << ", "; });
    line << "]";
  }

  n.get_body()->accept(*this);
}

void CambrianFormatter::visit(ExportStmt const& n) {
  line << n.get_vis() << " ";

  escape_string_literal(*n.get_abi_name());

  if (!n.get_attrs().empty()) {
    line << "[";
    iterate_except_last(
        n.get_attrs().begin(), n.get_attrs().end(),
        [&](let attr, size_t) { attr->accept(*this); },
        [&](let) { line << ", "; });
    line << "]";
  }

  n.get_body()->accept(*this);
}
