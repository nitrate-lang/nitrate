#include <nitrate-core/Macro.h>

#include <lsp/lang/CambrianStyleFormatter.hh>

#include "nitrate-parser/Node.h"

using namespace lsp::fmt;

void CambrianFormatter::flush_line() {
  file << line.str();
  line.str("");
  (void)tabSize;
}

std::string CambrianFormatter::escape_char_literal(char ch) {
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

void CambrianFormatter::escape_string_literal_chunk(std::string_view str) {
  for (char ch : str) {
    switch (ch) {
      case '\n':
        line << "\\n";
        break;
      case '\t':
        line << "\\t";
        break;
      case '\r':
        line << "\\r";
        break;
      case '\v':
        line << "\\v";
        break;
      case '\f':
        line << "\\f";
        break;
      case '\b':
        line << "\\b";
        break;
      case '\a':
        line << "\\a";
        break;
      case '\\':
        line << "\\\\";
        break;
      case '"':
        line << "\\\"";
        break;
      default:
        line << ch;
        break;
    }
  }
}

void CambrianFormatter::escape_string_literal(std::string_view str) {
  constexpr size_t max_chunk_size = 60;

  if (str.empty()) {
    line << "\"\"";
  }

  size_t num_chunks = str.size() / max_chunk_size;
  size_t rem = str.size() % max_chunk_size;

  line.seekg(0, std::ios::end);
  size_t line_size = line.tellg();

  for (size_t i = 0; i < num_chunks; i++) {
    line << "\"";
    escape_string_literal_chunk(str.substr(i * max_chunk_size, max_chunk_size));
    line << "\"";

    if (rem > 0 || i < num_chunks - 1) {
      line << " \\\n";
      flush_line();
      if (line_size) {
        line << std::string(line_size, ' ');
      }
    }
  }

  if (rem > 0) {
    line << "\"";
    escape_string_literal_chunk(str.substr(num_chunks * max_chunk_size, rem));
    line << "\"";
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

  size_t num_chunks = float_str.size() / max_chunk_size;
  size_t rem = float_str.size() % max_chunk_size;

  line.seekg(0, std::ios::end);
  size_t line_size = line.tellg();

  for (size_t i = 0; i < num_chunks; i++) {
    write_float_literal_chunk(
        float_str.substr(i * max_chunk_size, max_chunk_size));

    if (rem > 0 || i < num_chunks - 1) {
      line << "_ \\\n";
      flush_line();
      if (line_size) {
        line << std::string(line_size, ' ');
      }
    }
  }

  if (rem > 0) {
    write_float_literal_chunk(
        float_str.substr(num_chunks * max_chunk_size, rem));
  }
}

void CambrianFormatter::format_type_metadata(npar::Type& n) {
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

void CambrianFormatter::visit(npar_node_t&) {
  /** This node symbolizes a placeholder value in the event of an error. */
  failed = true;

  line << "/* !!! */";
}

void CambrianFormatter::visit(npar::ExprStmt& n) {
  n.get_expr()->accept(*this);
  line << ";";
}

void CambrianFormatter::visit(npar::StmtExpr& n) {
  n.get_stmt()->accept(*this);
}

void CambrianFormatter::visit(npar::TypeExpr& n) {
  n.get_type()->accept(*this);
}

void CambrianFormatter::visit(npar::NamedTy& n) {
  line << n.get_name();
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::InferTy& n) {
  line << "?";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::TemplType& n) {
  n.get_template()->accept(*this);

  line << "<";
  iterate_except_last(
      n.get_args().begin(), n.get_args().end(),
      [&](let arg, size_t) { arg->accept(*this); }, [&](let) { line << ", "; });
  line << ">";

  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::U1& n) {
  line << "u1";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::U8& n) {
  line << "u8";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::U16& n) {
  line << "u16";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::U32& n) {
  line << "u32";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::U64& n) {
  line << "u64";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::U128& n) {
  line << "u128";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::I8& n) {
  line << "i8";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::I16& n) {
  line << "i16";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::I32& n) {
  line << "i32";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::I64& n) {
  line << "i64";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::I128& n) {
  line << "i128";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::F16& n) {
  line << "f16";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::F32& n) {
  line << "f32";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::F64& n) {
  line << "f64";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::F128& n) {
  line << "f128";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::VoidTy& n) {
  line << "void";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::PtrTy& n) {
  n.get_item()->accept(*this);
  line << "*";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::OpaqueTy& n) {
  line << "opaque(" << n.get_name() << ")";
  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::TupleTy& n) {
  line << "(";
  iterate_except_last(
      n.get_items().begin(), n.get_items().end(),
      [&](let item, size_t) { item->accept(*this); },
      [&](let) { line << ", "; });
  line << ")";

  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::ArrayTy& n) {
  line << "[";
  n.get_item()->accept(*this);
  line << ";";
  n.get_size()->accept(*this);
  line << "]";

  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::RefTy& n) {
  line << "&";
  n.get_item()->accept(*this);

  format_type_metadata(n);
}

void CambrianFormatter::visit(npar::FuncTy& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::UnaryExpr& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::BinExpr& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::PostUnaryExpr& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::TernaryExpr& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ConstInt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ConstFloat& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ConstBool& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ConstString& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ConstChar& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ConstNull&) { line << "null"; }

void CambrianFormatter::visit(npar::ConstUndef&) { line << "undef"; }

void CambrianFormatter::visit(npar::Call& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::TemplCall& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::List& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::Assoc& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::Field& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::Index& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::Slice& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::FString& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::Ident& n) { line << n.get_name(); }

void CambrianFormatter::visit(npar::SeqPoint& n) {
  line << "(";
  iterate_except_last(
      n.get_items().begin(), n.get_items().end(),
      [&](let item, size_t) { item->accept(*this); },
      [&](let) { line << ", "; });
  line << ")";
}

void CambrianFormatter::visit(npar::Block& n) {
  switch (n.get_safety()) {
    case npar::SafetyMode::Safe: {
      line << "safe ";
      break;
    }

    case npar::SafetyMode::Unsafe: {
      line << "unsafe ";
      break;
    }

    case npar::SafetyMode::Unknown: {
      break;
    }
  }

  std::for_each(n.get_items().begin(), n.get_items().end(), [&](let item) {
    item->accept(*this);
    line << "\n";
  });
}

void CambrianFormatter::visit(npar::VarDecl& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::InlineAsm&) {
  /* Support for inline assembly is not avaliable yet */

  failed = true;

  line << "/* !!! */";
}

void CambrianFormatter::visit(npar::IfStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::WhileStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ForStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ForeachStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::BreakStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ContinueStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ReturnStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ReturnIfStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::CaseStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::SwitchStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::TypedefStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::FnDecl& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::FnDef& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::StructField& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::StructDef& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::EnumDef& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ScopeStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}

void CambrianFormatter::visit(npar::ExportStmt& n) {
  /// TODO: Implement format for node
  qcore_implement();
}
