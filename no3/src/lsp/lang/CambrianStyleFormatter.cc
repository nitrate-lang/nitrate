#include <lsp/lang/CambrianStyleFormatter.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTCommon.hh>
#include <sstream>
#include <unordered_set>

using namespace no3::lsp::fmt;

using namespace ncc;
using namespace ncc::parse;
using namespace ncc::lex;

auto CambrianFormatter::LineStreamWritter::operator<<(std::ostream& (*func)(
    std::ostream&)) -> CambrianFormatter::LineStreamWritter& {
  qcore_assert(func ==
               static_cast<std::ostream& (*)(std::ostream&)>(std::endl));

  m_file << m_line_buffer.str() << "\n";
  Reset();

  return *this;
}

auto CambrianFormatter::LineStreamWritter::operator<<(Operator op)
    -> CambrianFormatter::LineStreamWritter& {
  m_line_buffer << op;
  return *this;
}

auto CambrianFormatter::LineStreamWritter::operator<<(Vis v)
    -> CambrianFormatter::LineStreamWritter& {
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

auto CambrianFormatter::EscapeCharLiteral(char ch) const -> std::string {
  if ((std::isspace(ch) == 0) && (std::isprint(ch) == 0)) {
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

auto CambrianFormatter::EscapeStringLiteralChunk(std::string_view str) const
    -> std::string {
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

void CambrianFormatter::WrapStmtBody(FlowPtr<parse::Stmt> n,
                                     size_t size_threshold,
                                     bool use_arrow_if_wrapped) {
  if (n->Is(QAST_BLOCK)) {
    auto block = n.As<Block>();
    bool single_stmt = block->GetItems().size() == 1;
    bool few_children =
        single_stmt && block->RecursiveChildCount() <= size_threshold;

    if (single_stmt && few_children) {
      if (use_arrow_if_wrapped) {
        m_line << "=> ";
      }

      block->GetItems().front().Accept(*this);
      return;
    }
  }

  n.Accept(*this);
}

void CambrianFormatter::PrintLineComments(FlowPtr<parse::Base> n) {
  auto comments = n->Comments();
  auto m_line_size = m_line.Length();

  if (!comments.empty()) {
    for (auto comment : comments) {
      m_line << "#";
      m_line << comment.GetString() << std::endl;

      if (m_line_size) {
        m_line << std::string(m_line_size, ' ');
      }
    }
  }
}

void CambrianFormatter::PrintMultilineComments(FlowPtr<parse::Base> n) {
  auto comments = n->Comments();
  if (!comments.empty()) {
    for (auto comment : comments) {
      m_line << "/*";
      m_line << comment.GetString();
      m_line << "*/ ";
    }
  }
}

void CambrianFormatter::EscapeStringLiteral(std::string_view str,
                                            bool put_quotes) {
  constexpr size_t kMaxChunkSize = 60;

  if (str.empty()) {
    if (put_quotes) {
      m_line << "\"\"";
    }
    return;
  }

  auto chunks_n = str.size() / kMaxChunkSize;
  auto rem = str.size() % kMaxChunkSize;
  auto m_line_size = m_line.Length();

  if (chunks_n) {
    std::vector<std::string> chunks(chunks_n);

    for (size_t i = 0; i < chunks_n; i++) {
      chunks[i] = "\"" +
                  EscapeStringLiteralChunk(
                      str.substr(i * kMaxChunkSize, kMaxChunkSize)) +
                  "\"";
    }

    auto max_segment_size =
        std::max_element(chunks.begin(), chunks.end(), [](auto a, auto b) {
          return a.size() < b.size();
        })->size();

    for (size_t i = 0; i < chunks.size(); ++i) {
      if (i != 0 && m_line_size) {
        m_line << std::string(m_line_size, ' ');
      }

      m_line << chunks[i];

      auto rpad = (max_segment_size - chunks[i].size());
      if (rpad) {
        m_line << std::string(rpad, ' ');
      }

      if (rem > 0 || i < chunks_n - 1) {
        m_line << " \\" << std::endl;
      }
    }
  }

  if (rem > 0) {
    if (m_line_size && chunks_n > 0) {
      m_line << std::string(m_line_size, ' ');
    }

    if (chunks_n > 0 || put_quotes) {
      m_line << "\"";
    }

    m_line << EscapeStringLiteralChunk(
        str.substr(chunks_n * kMaxChunkSize, rem));

    if (chunks_n > 0 || put_quotes) {
      m_line << "\"";
    }
  }
}

void CambrianFormatter::WriteFloatLiteralChunk(std::string_view float_str) {
  constexpr size_t kInsertSepEvery = 10;

  bool already_write_type_suffix = false;

  for (size_t i = 0; i < float_str.size(); i++) {
    bool underscore = false;

    if (!already_write_type_suffix && i != 0 && (i % (kInsertSepEvery)) == 0) {
      underscore = true;
    } else if (!already_write_type_suffix && !std::isdigit(float_str[i]) &&
               float_str[i] != '.') {
      already_write_type_suffix = true;
      underscore = true;
    }

    if (underscore) {
      m_line << "_";
    }

    m_line << float_str[i];
  }
}

void CambrianFormatter::WriteFloatLiteral(std::string_view float_str) {
  constexpr size_t kMaxChunkSize = 50;

  if (float_str.empty()) {
    m_line << "";
  }

  size_t chunks_n = float_str.size() / kMaxChunkSize;
  size_t rem = float_str.size() % kMaxChunkSize;

  size_t m_line_size = m_line.Length();

  for (size_t i = 0; i < chunks_n; i++) {
    WriteFloatLiteralChunk(float_str.substr(i * kMaxChunkSize, kMaxChunkSize));

    if (rem > 0 || i < chunks_n - 1) {
      m_line << "_ \\" << std::endl;
      if (m_line_size) {
        m_line << std::string(m_line_size, ' ');
      }
    }
  }

  if (rem > 0) {
    WriteFloatLiteralChunk(float_str.substr(chunks_n * kMaxChunkSize, rem));
  }
}

void CambrianFormatter::FormatTypeMetadata(FlowPtr<parse::Type> n) {
  auto range_start = n->GetRangeBegin();
  auto range_end = n->GetRangeEnd();

  if (range_start || range_end) {
    m_line << ": [";
    if (range_start) range_start.value().Accept(*this);
    m_line << ":";
    if (range_end) range_end.value().Accept(*this);
    m_line << "]";
  }

  if (n->GetWidth()) {
    m_line << ": ";
    n->GetWidth().value().Accept(*this);
  }
}

void CambrianFormatter::Visit(FlowPtr<Base> n) {
  PrintMultilineComments(n);

  /** This node symbolizes a placeholder value in the event of an error. */
  m_failed = true;

  m_line << "/* !!! */";
}

void CambrianFormatter::Visit(FlowPtr<ExprStmt> n) {
  PrintLineComments(n);

  n->GetExpr().Accept(*this);
  m_line << ";";
}

void CambrianFormatter::Visit(FlowPtr<StmtExpr> n) {
  PrintMultilineComments(n);

  n->GetStmt().Accept(*this);
}

void CambrianFormatter::Visit(FlowPtr<TypeExpr> n) {
  PrintMultilineComments(n);

  m_line << "type ";
  n->GetType().Accept(*this);
}

void CambrianFormatter::Visit(FlowPtr<NamedTy> n) {
  PrintMultilineComments(n);

  m_line << n->GetName();
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<InferTy> n) {
  PrintMultilineComments(n);

  m_line << "?";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<TemplType> n) {
  PrintMultilineComments(n);

  bool is_optional =
      n->GetTemplate()->GetKind() == QAST_NAMED &&
      n->GetTemplate()->As<NamedTy>()->GetName() == "__builtin_result";

  bool is_vector =
      n->GetTemplate()->GetKind() == QAST_NAMED &&
      n->GetTemplate()->As<NamedTy>()->GetName() == "__builtin_vec";

  bool is_map = n->GetTemplate()->GetKind() == QAST_NAMED &&
                n->GetTemplate()->As<NamedTy>()->GetName() == "__builtin_umap";

  bool is_set = n->GetTemplate()->GetKind() == QAST_NAMED &&
                n->GetTemplate()->As<NamedTy>()->GetName() == "__builtin_uset";

  bool is_comptime =
      n->GetTemplate()->GetKind() == QAST_NAMED &&
      n->GetTemplate()->As<NamedTy>()->GetName() == "__builtin_meta" &&
      n->GetArgs().size() == 1 &&
      n->GetArgs().front().second->Is(QAST_UNEXPR) &&
      n->GetArgs().front().second.template As<UnaryExpr>()->GetOp() ==
          OpComptime;

  const auto print_without_type_keyword = [&](auto node) {
    if (node->Is(QAST_TEXPR)) {
      node->template As<TypeExpr>()->GetType().Accept(*this);
    } else {
      node->Accept(*this);
    }
  };

  size_t argc = n->GetArgs().size();
  if (is_optional && argc == 1) {
    print_without_type_keyword(n->GetArgs().front().second);
    m_line << "?";
  } else if (is_vector && argc == 1) {
    m_line << "[";
    print_without_type_keyword(n->GetArgs().front().second);
    m_line << "]";
  } else if (is_map && argc == 2) {
    m_line << "[";
    print_without_type_keyword(n->GetArgs().front().second);
    m_line << "->";
    print_without_type_keyword(n->GetArgs().back().second);
    m_line << "]";
  } else if (is_set && argc == 1) {
    m_line << "{";
    print_without_type_keyword(n->GetArgs().front().second);
    m_line << "}";
  } else if (is_comptime) {
    m_line << "comptime(";
    n->GetArgs().front().second.template As<UnaryExpr>()->GetRHS().Accept(
        *this);
    m_line << ")";
  } else {
    n->GetTemplate().Accept(*this);

    m_line << "<";
    IterateExceptLast(
        n->GetArgs().begin(), n->GetArgs().end(),
        [&](auto arg, size_t) {
          if (!std::isdigit(arg.first->at(0))) {
            m_line << arg.first << ": ";
          }

          print_without_type_keyword(arg.second);
        },
        [&](let) { m_line << ", "; });
    m_line << ">";
  }

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<U1> n) {
  PrintMultilineComments(n);

  m_line << "u1";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<U8> n) {
  PrintMultilineComments(n);

  m_line << "u8";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<U16> n) {
  PrintMultilineComments(n);

  m_line << "u16";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<U32> n) {
  PrintMultilineComments(n);

  m_line << "u32";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<U64> n) {
  PrintMultilineComments(n);

  m_line << "u64";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<U128> n) {
  PrintMultilineComments(n);

  m_line << "u128";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<I8> n) {
  PrintMultilineComments(n);

  m_line << "i8";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<I16> n) {
  PrintMultilineComments(n);

  m_line << "i16";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<I32> n) {
  PrintMultilineComments(n);

  m_line << "i32";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<I64> n) {
  PrintMultilineComments(n);

  m_line << "i64";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<I128> n) {
  PrintMultilineComments(n);

  m_line << "i128";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<F16> n) {
  PrintMultilineComments(n);

  m_line << "f16";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<F32> n) {
  PrintMultilineComments(n);

  m_line << "f32";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<F64> n) {
  PrintMultilineComments(n);

  m_line << "f64";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<F128> n) {
  PrintMultilineComments(n);

  m_line << "f128";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<VoidTy> n) {
  PrintMultilineComments(n);

  m_line << "void";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<PtrTy> n) {
  PrintMultilineComments(n);

  m_line << "*";
  n->GetItem().Accept(*this);

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<OpaqueTy> n) {
  PrintMultilineComments(n);

  m_line << "opaque(" << n->GetName() << ")";
  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<TupleTy> n) {
  /* If the number of fields exceeds the threshold, arange fields into a
   * matrix of row size ceil(sqrt(n)). */
  PrintMultilineComments(n);

  auto wrap_threshold = 8ULL;

  m_line << "(";

  auto items = n->GetItems();
  auto m_line_size = m_line.Length();
  auto break_at = items.size() <= wrap_threshold
                      ? wrap_threshold
                      : static_cast<size_t>(std::ceil(std::sqrt(items.size())));

  for (size_t i = 0; i < items.size(); i++) {
    if (i != 0 && i % break_at == 0) {
      m_line << std::endl << std::string(m_line_size, ' ');
    }

    auto item = items[i];
    item.Accept(*this);

    if (i != items.size() - 1) {
      m_line << ", ";
    }
  }
  m_line << ")";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<ArrayTy> n) {
  PrintMultilineComments(n);

  m_line << "[";
  n->GetItem().Accept(*this);
  m_line << "; ";
  n->GetSize().Accept(*this);
  m_line << "]";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<RefTy> n) {
  PrintMultilineComments(n);

  m_line << "&";
  n->GetItem().Accept(*this);

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<FuncTy> n) {
  PrintMultilineComments(n);

  m_line << "fn";

  if (!n->GetAttributes().empty()) {
    m_line << "[";
    IterateExceptLast(
        n->GetAttributes().begin(), n->GetAttributes().end(),
        [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { m_line << ", "; });
    m_line << "] ";
  }

  switch (n->GetPurity()) {
    case Purity::Impure: {
      break;
    }

    case Purity::Impure_TSafe: {
      m_line << " tsafe";
      break;
    }

    case Purity::Pure: {
      m_line << " pure";
      break;
    }

    case Purity::Quasi: {
      m_line << " quasi";
      break;
    }

    case Purity::Retro: {
      m_line << " retro";
      break;
    }
  }

  m_line << "(";
  IterateExceptLast(
      n->GetParams().begin(), n->GetParams().end(),
      [&](auto param, size_t) {
        m_line << std::get<0>(param);

        if (auto type = std::get<1>(param); type->GetKind() != QAST_INFER) {
          m_line << ": ";
          type.Accept(*this);
        }

        if (auto def = std::get<2>(param)) {
          m_line << " = ";
          def.value().Accept(*this);
        }
      },
      [&](let) { m_line << ", "; });
  if (n->IsVariadic()) {
    if (!n->GetParams().empty()) {
      m_line << ", ";
    }
    m_line << "...";
  }
  m_line << ")";

  m_line << ": ";
  n->GetReturn().Accept(*this);
}

void CambrianFormatter::Visit(FlowPtr<UnaryExpr> n) {
  static const std::unordered_set<Operator> word_ops = {
      OpAs,        OpBitcastAs, OpIn,     OpOut,     OpSizeof,
      OpBitsizeof, OpAlignof,   OpTypeof, OpComptime};

  PrintMultilineComments(n);

  m_line << "(" << n->GetOp();
  if (word_ops.contains(n->GetOp())) {
    m_line << " ";
  }
  n->GetRHS().Accept(*this);
  m_line << ")";
}

void CambrianFormatter::Visit(FlowPtr<BinExpr> n) {
  PrintMultilineComments(n);

  if (n->GetOp() == OpDot) {
    n->GetLHS().Accept(*this);
    m_line << ".";
    n->GetRHS().Accept(*this);
  } else {
    m_line << "(";
    n->GetLHS().Accept(*this);
    m_line << " " << n->GetOp() << " ";
    n->GetRHS().Accept(*this);
    m_line << ")";
  }
}

void CambrianFormatter::Visit(FlowPtr<PostUnaryExpr> n) {
  PrintMultilineComments(n);

  m_line << "(";
  n->GetLHS().Accept(*this);
  m_line << n->GetOp() << ")";
}

void CambrianFormatter::Visit(FlowPtr<TernaryExpr> n) {
  PrintMultilineComments(n);

  m_line << "(";
  n->GetCond().Accept(*this);
  m_line << " ? ";
  n->GetLHS().Accept(*this);
  m_line << " : ";
  n->GetRHS().Accept(*this);
  m_line << ")";
}

void CambrianFormatter::Visit(FlowPtr<ConstInt> n) {
  PrintMultilineComments(n);

  WriteFloatLiteral(n->GetValue());
}

void CambrianFormatter::Visit(FlowPtr<ConstFloat> n) {
  PrintMultilineComments(n);

  WriteFloatLiteral(n->GetValue());
}

void CambrianFormatter::Visit(FlowPtr<ConstBool> n) {
  PrintMultilineComments(n);

  if (n->GetValue()) {
    m_line << "true";
  } else {
    m_line << "false";
  }
}

void CambrianFormatter::Visit(FlowPtr<ConstString> n) {
  PrintMultilineComments(n);

  EscapeStringLiteral(n->GetValue());
}

void CambrianFormatter::Visit(FlowPtr<ConstChar> n) {
  PrintMultilineComments(n);

  m_line << EscapeCharLiteral(n->GetValue());
}

void CambrianFormatter::Visit(FlowPtr<ConstNull> n) {
  PrintMultilineComments(n);

  m_line << "null";
}

void CambrianFormatter::Visit(FlowPtr<ConstUndef> n) {
  PrintMultilineComments(n);

  m_line << "undef";
}

void CambrianFormatter::Visit(FlowPtr<Call> n) {
  PrintMultilineComments(n);

  auto wrap_threshold = 8ULL;

  n->GetFunc().Accept(*this);

  size_t argc = n->GetArgs().size();

  bool any_named =
      std::any_of(n->GetArgs().begin(), n->GetArgs().end(), [](CallArg arg) {
        auto name = arg.first;
        return !std::isdigit(name->at(0));
      });

  bool any_lambdas = std::any_of(
      n->GetArgs().begin(), n->GetArgs().end(),
      [](auto arg) { return std::get<1>(arg)->IsStmtExpr(QAST_FUNCTION); });

  bool is_wrapping = argc >= wrap_threshold || any_named || any_lambdas;

  if (is_wrapping) {
    m_line << "(";
    size_t m_line_size = m_line.Length();
    std::swap(m_indent, m_line_size);

    for (auto it = n->GetArgs().begin(); it != n->GetArgs().end(); ++it) {
      auto arg = *it;
      auto name = std::get<0>(arg);
      auto value = std::get<1>(arg);

      if (!std::isdigit(name->at(0))) {
        m_line << name << ": ";
      }

      value.Accept(*this);

      if (it != n->GetArgs().end() - 1) {
        m_line << ", ";
      }

      if (it != n->GetArgs().end() - 1) {
        m_line << std::endl << GetIndent();
      }
    }

    std::swap(m_indent, m_line_size);
    m_line << ")";
  } else {
    m_line << "(";
    IterateExceptLast(
        n->GetArgs().begin(), n->GetArgs().end(),
        [&](auto arg, size_t) {
          auto name = std::get<0>(arg);
          auto value = std::get<1>(arg);

          if (!std::isdigit(name->at(0))) {
            m_line << name << ": ";
          }

          value.Accept(*this);
        },
        [&](let) { m_line << ", "; });
    m_line << ")";
  }
}

void CambrianFormatter::Visit(FlowPtr<TemplCall> n) {
  PrintMultilineComments(n);

  n->GetFunc().Accept(*this);

  m_line << "{";
  IterateExceptLast(
      n->GetTemplateArgs().begin(), n->GetTemplateArgs().end(),
      [&](auto arg, size_t) {
        auto name = std::get<0>(arg);
        auto value = std::get<1>(arg);
        bool should_print_name = !std::isdigit(name->at(0));

        if (should_print_name) {
          m_line << name << ": ";
        }

        if (value->Is(QAST_TEXPR)) {
          value->template As<TypeExpr>()->GetType().Accept(*this);
        } else {
          value.Accept(*this);
        }
      },
      [&](let) { m_line << ", "; });
  m_line << "}";

  m_line << "(";
  IterateExceptLast(
      n->GetArgs().begin(), n->GetArgs().end(),
      [&](auto arg, size_t) {
        auto name = std::get<0>(arg);
        auto value = std::get<1>(arg);

        if (!std::isdigit(name->at(0))) {
          m_line << name << ": ";
        }

        value.Accept(*this);
      },
      [&](let) { m_line << ", "; });
  m_line << ")";
}

void CambrianFormatter::Visit(FlowPtr<List> n) {
  PrintMultilineComments(n);

  auto wrap_threshold = 8ULL;

  if (n->GetItems().empty()) {
    m_line << "[]";
    return;
  }

  auto argc = n->GetItems().size();
  bool is_compressing =
      argc >= wrap_threshold &&
      std::all_of(n->GetItems().begin(), n->GetItems().end(),
                  [&](auto x) { return x->IsEq(n->GetItems().front()); });

  if (is_compressing) {
    m_line << "[";
    n->GetItems().front().Accept(*this);
    m_line << "; " << argc << "]";
  } else {
    static const std::unordered_set<npar_ty_t> extra_seperation = {
        QAST_TEREXPR, QAST_CALL, QAST_LIST,
        QAST_ASSOC,   QAST_SEQ,  QAST_TEMPL_CALL,
    };

    bool special_case =
        std::any_of(n->GetItems().begin(), n->GetItems().end(), [&](auto x) {
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
      m_line << "[";

      m_line << std::endl;

      { /* Write list items */
        size_t the_indent = m_indent + m_tabSize;
        std::swap(m_indent, the_indent);

        for (size_t i = 0; i < n->GetItems().size(); i++) {
          m_line << GetIndent();
          auto item = n->GetItems()[i];
          item.Accept(*this);

          bool is_last = i == n->GetItems().size() - 1;
          if (!is_last) {
            m_line << ",";
          }

          m_line << std::endl;
        }

        std::swap(m_indent, the_indent);
      }

      m_line << GetIndent() << "]";
    } else {
      m_line << "[";

      bool is_assoc_map =
          std::all_of(n->GetItems().begin(), n->GetItems().end(),
                      [](auto x) { return x->Is(QAST_ASSOC); });

      { /* Write list items */
        size_t the_indent =
            is_assoc_map ? m_indent + m_tabSize : m_line.Length();
        std::swap(m_indent, the_indent);

        for (size_t i = 0; i < n->GetItems().size(); i++) {
          auto item = n->GetItems()[i];
          item.Accept(*this);

          bool is_last = i == n->GetItems().size() - 1;
          if (!is_last) {
            m_line << ",";
          }

          bool is_break = !is_last && i != 0 && (i + 1) % break_at == 0;

          if (is_break) {
            m_line << std::endl << GetIndent();
          } else if (!is_last) {
            m_line << " ";
          }
        }

        std::swap(m_indent, the_indent);
      }

      m_line << "]";
    }
  }
}

void CambrianFormatter::Visit(FlowPtr<Assoc> node) {
  PrintMultilineComments(node);

  const std::function<void(FlowPtr<Assoc>, bool)> format =
      [&](FlowPtr<Assoc> n, bool use_braces) {
        bool is_value_map = false;
        if (n->GetValue()->Is(QAST_LIST)) {
          auto list = n->GetValue()->As<List>();
          is_value_map =
              list->GetItems().empty() ||
              std::all_of(list->GetItems().begin(), list->GetItems().end(),
                          [](auto x) { return x->Is(QAST_ASSOC); });
        }

        if (use_braces) {
          m_line << "{" << std::endl;
          m_indent += m_tabSize;
          m_line << GetIndent();
        }

        n->GetKey().Accept(*this);
        m_line << ": ";

        if (is_value_map) {
          auto list = n->GetValue()->As<List>();

          if (list->GetItems().empty()) {
            m_line << "{}";
          } else {
            m_line << "{" << std::endl;
            m_indent += m_tabSize;

            for (auto it = list->GetItems().begin();
                 it != list->GetItems().end(); ++it) {
              m_line << GetIndent();

              format(it->As<Assoc>(), false);

              if (it != list->GetItems().end() - 1) {
                m_line << ",";
              }

              m_line << std::endl;
            }

            m_indent -= m_tabSize;
            m_line << GetIndent() << "}";
          }
        } else {
          n->GetValue().Accept(*this);
        }

        if (use_braces) {
          m_indent -= m_tabSize;
          m_line << std::endl << GetIndent() << "}";
        }
      };

  format(node, true);
}

void CambrianFormatter::Visit(FlowPtr<Index> n) {
  PrintMultilineComments(n);

  n->GetBase().Accept(*this);
  m_line << "[";
  n->GetIndex().Accept(*this);
  m_line << "]";
}

void CambrianFormatter::Visit(FlowPtr<Slice> n) {
  PrintMultilineComments(n);

  n->GetBase().Accept(*this);
  m_line << "[";
  if (n->GetStart()) {
    n->GetStart().Accept(*this);
  }
  m_line << ":";
  if (n->GetEnd()) {
    n->GetEnd().Accept(*this);
  }
  m_line << "]";
}

void CambrianFormatter::Visit(FlowPtr<FString> n) {
  PrintMultilineComments(n);

  m_line << "f\"";
  for (auto part : n->GetItems()) {
    if (std::holds_alternative<ncc::string>(part)) {
      EscapeStringLiteral(*std::get<ncc::string>(part), false);
    } else {
      m_line << "{";
      std::get<FlowPtr<Expr>>(part).Accept(*this);
      m_line << "}";
    }
  }
  m_line << "\"";
}

void CambrianFormatter::Visit(FlowPtr<Ident> n) {
  PrintMultilineComments(n);

  m_line << n->GetName();
}

void CambrianFormatter::Visit(FlowPtr<SeqPoint> n) {
  PrintMultilineComments(n);

  m_line << "(";
  IterateExceptLast(
      n->GetItems().begin(), n->GetItems().end(),
      [&](auto item, size_t) { item.Accept(*this); },
      [&](let) { m_line << ", "; });
  m_line << ")";
}

void CambrianFormatter::Visit(FlowPtr<Block> n) {
  PrintLineComments(n);

  bool is_root_block = !m_did_root;
  m_did_root = true;

  switch (n->GetSafety()) {
    case SafetyMode::Safe: {
      m_line << "safe ";
      break;
    }

    case SafetyMode::Unsafe: {
      m_line << "unsafe ";
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

  if (!is_root_block && n->GetItems().empty()) {
    m_line << "{}";
    return;
  }

  if (!is_root_block) {
    m_line << "{" << std::endl;
    m_indent += m_tabSize;
  }

  auto items = n->GetItems();

  for (auto it = items.begin(); it != items.end(); ++it) {
    auto item = *it;

    m_line << GetIndent();
    item.Accept(*this);
    m_line << std::endl;

    bool is_last_item = it == items.end() - 1;

    bool is_next_item_different =
        (it + 1 != items.end() &&
         (*std::next(it))->GetKind() != item->GetKind());

    bool extra_newline =
        !is_last_item &&
        (is_next_item_different || extra_seperation.contains(item->GetKind()));

    if (extra_newline) {
      m_line << std::endl;
    }
  }

  if (!is_root_block) {
    m_indent -= m_tabSize;
    m_line << GetIndent() << "}";
  }
}

void CambrianFormatter::Visit(FlowPtr<VarDecl> n) {
  PrintLineComments(n);

  switch (n->GetDeclType()) {
    case VarDeclType::Let: {
      m_line << "let ";
      break;
    }

    case VarDeclType::Const: {
      m_line << "const ";
      break;
    }

    case VarDeclType::Var: {
      m_line << "var ";
      break;
    }
  }

  if (!n->GetAttributes().empty()) {
    m_line << "[";
    IterateExceptLast(
        n->GetAttributes().begin(), n->GetAttributes().end(),
        [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { m_line << ", "; });
    m_line << "] ";
  }

  m_line << n->GetName();

  if (n->GetType()) {
    m_line << ": ";
    n->GetType().value().Accept(*this);
  }

  if (n->GetValue()) {
    m_line << " = ";
    n->GetValue().value().Accept(*this);
  }

  m_line << ";";
}

void CambrianFormatter::Visit(FlowPtr<InlineAsm> n) {
  PrintLineComments(n);

  /* Support for inline assembly is not avaliable yet */

  m_failed = true;

  m_line << "/* !!! */";
}

void CambrianFormatter::Visit(FlowPtr<IfStmt> n) {
  PrintLineComments(n);

  m_line << "if ";
  n->GetCond().Accept(*this);
  m_line << " ";
  n->GetThen().Accept(*this);

  if (n->GetElse()) {
    m_line << " else ";
    n->GetElse().value().Accept(*this);
  }

  m_line << ";";
}

void CambrianFormatter::Visit(FlowPtr<WhileStmt> n) {
  PrintLineComments(n);

  m_line << "while ";
  n->GetCond().Accept(*this);
  m_line << " ";
  n->GetBody().Accept(*this);

  m_line << ";";
}

void CambrianFormatter::Visit(FlowPtr<ForStmt> n) {
  PrintLineComments(n);

  m_line << "for (";

  if (n->GetInit().has_value()) {
    n->GetInit().value().Accept(*this);
    if (!n->GetInit().value()->IsStmt()) {
      m_line << ";";
    }
  } else {
    m_line << ";";
  }

  if (n->GetCond().has_value()) {
    m_line << " ";
    n->GetCond().value().Accept(*this);
  }
  m_line << ";";

  if (n->GetStep().has_value()) {
    m_line << " ";
    n->GetStep().value().Accept(*this);
  }

  m_line << ") ";
  n->GetBody().Accept(*this);

  m_line << ";";
}

void CambrianFormatter::Visit(FlowPtr<ForeachStmt> n) {
  PrintLineComments(n);

  m_line << "foreach (";
  if (n->GetIdxIdent()->empty()) {
    m_line << n->GetValIdent();
  } else {
    m_line << n->GetIdxIdent() << ", " << n->GetValIdent();
  }

  m_line << " in ";
  n->GetExpr().Accept(*this);
  m_line << ") ";

  n->GetBody().Accept(*this);

  m_line << ";";
}

void CambrianFormatter::Visit(FlowPtr<BreakStmt> n) {
  PrintLineComments(n);

  m_line << "break;";
}

void CambrianFormatter::Visit(FlowPtr<ContinueStmt> n) {
  PrintLineComments(n);

  m_line << "continue;";
}

void CambrianFormatter::Visit(FlowPtr<ReturnStmt> n) {
  PrintLineComments(n);

  if (n->GetValue().has_value()) {
    m_line << "ret ";
    n->GetValue().value().Accept(*this);
    m_line << ";";
  } else {
    m_line << "ret;";
  }
}

void CambrianFormatter::Visit(FlowPtr<ReturnIfStmt> n) {
  PrintLineComments(n);

  m_line << "retif ";
  n->GetCond().Accept(*this);
  m_line << ", ";
  n->GetValue().Accept(*this);
  m_line << ";";
}

void CambrianFormatter::Visit(FlowPtr<CaseStmt> n) {
  PrintLineComments(n);

  n->GetCond().Accept(*this);
  m_line << " => ";
  WrapStmtBody(n->GetBody(), 10, false);
}

void CambrianFormatter::Visit(FlowPtr<SwitchStmt> n) {
  PrintLineComments(n);

  m_line << "switch ";
  n->GetCond().Accept(*this);
  m_line << " {" << std::endl;
  m_indent += m_tabSize;

  for (auto c : n->GetCases()) {
    m_line << GetIndent();
    c.Accept(*this);
    m_line << std::endl;
  }

  if (n->GetDefault()) {
    m_line << GetIndent();
    m_line << "_ => ";
    WrapStmtBody(n->GetDefault().value(), 10, false);
    m_line << std::endl;
  }

  m_indent -= m_tabSize;
  m_line << GetIndent() << "}";
}

void CambrianFormatter::Visit(FlowPtr<TypedefStmt> n) {
  PrintLineComments(n);

  m_line << "type " << n->GetName() << " = ";
  n->GetType().Accept(*this);
  m_line << ";";
}

void CambrianFormatter::Visit(FlowPtr<Function> n) {
  PrintLineComments(n);

  m_line << "fn";

  if (!n->GetAttributes().empty()) {
    m_line << " [";
    IterateExceptLast(
        n->GetAttributes().begin(), n->GetAttributes().end(),
        [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { m_line << ", "; });
    m_line << "]";
  }

  switch (n->GetPurity()) {
    case Purity::Impure: {
      break;
    }

    case Purity::Impure_TSafe: {
      m_line << " tsafe";
      break;
    }

    case Purity::Pure: {
      m_line << " pure";
      break;
    }

    case Purity::Quasi: {
      m_line << " quasi";
      break;
    }

    case Purity::Retro: {
      m_line << " retro";
      break;
    }
  }

  if (!n->GetCaptures().empty()) {
    m_line << " [";
    IterateExceptLast(
        n->GetCaptures().begin(), n->GetCaptures().end(),
        [&](auto cap, size_t) {
          if (cap.second) {
            m_line << "&";
          }
          m_line << cap.first;
        },
        [&](let) { m_line << ", "; });
    m_line << "]";
  }

  m_line << " " << n->GetName();

  if (n->GetTemplateParams()) {
    m_line << "<";
    IterateExceptLast(
        n->GetTemplateParams().value().begin(),
        n->GetTemplateParams().value().end(),
        [&](auto param, size_t) {
          m_line << std::get<0>(param);

          if (let type = std::get<1>(param)) {
            if (type->GetKind() != QAST_INFER) {
              m_line << ": ";
              type->Accept(*this);
            }
          }

          if (auto val = std::get<2>(param)) {
            m_line << " = ";
            val.value().Accept(*this);
          }
        },
        [&](let) { m_line << ", "; });
    m_line << ">";
  }

  m_line << "(";
  IterateExceptLast(
      n->GetParams().begin(), n->GetParams().end(),
      [&](auto param, size_t) {
        m_line << std::get<0>(param);

        if (let type = std::get<1>(param)) {
          if (type->GetKind() != QAST_INFER) {
            m_line << ": ";
            type->Accept(*this);
          }
        }

        if (auto def = std::get<2>(param)) {
          m_line << " = ";
          def.value().Accept(*this);
        }
      },
      [&](let) { m_line << ", "; });

  if (n->IsVariadic()) {
    if (!n->GetParams().empty()) {
      m_line << ", ";
    }
    m_line << "...";
  }
  m_line << ")";

  { /* Return type */
    auto return_type = n->GetReturn();

    if (!return_type->Is(QAST_INFER)) {
      m_line << ": ";
      return_type.Accept(*this);
    }
  }

  if (n->IsDeclaration()) {
    m_line << ";";
  } else {
    m_line << " ";
    WrapStmtBody(n->GetBody().value(), 10, true);
  }
}

void CambrianFormatter::Visit(FlowPtr<StructDef> n) {
  PrintLineComments(n);

  switch (n->GetCompositeType()) {
    case CompositeType::Region: {
      m_line << "region ";
      break;
    }

    case CompositeType::Struct: {
      m_line << "struct ";
      break;
    }

    case CompositeType::Group: {
      m_line << "group ";
      break;
    }

    case CompositeType::Class: {
      m_line << "class ";
      break;
    }

    case CompositeType::Union: {
      m_line << "union ";
      break;
    }
  }

  if (!n->GetAttributes().empty()) {
    m_line << "[";
    IterateExceptLast(
        n->GetAttributes().begin(), n->GetAttributes().end(),
        [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { m_line << ", "; });
    m_line << "] ";
  }

  m_line << n->GetName();
  if (n->GetTemplateParams().has_value()) {
    m_line << "<";
    IterateExceptLast(
        n->GetTemplateParams().value().begin(),
        n->GetTemplateParams().value().end(),
        [&](auto param, size_t) {
          m_line << std::get<0>(param);
          if (auto type = std::get<1>(param); type->GetKind() != QAST_INFER) {
            m_line << ": ";
            type.Accept(*this);
          }
          if (auto val = std::get<2>(param)) {
            m_line << " = ";
            val.value().Accept(*this);
          }
        },
        [&](let) { m_line << ", "; });
    m_line << ">";
  }

  if (!n->GetNames().empty()) {
    m_line << ": ";
    IterateExceptLast(
        n->GetNames().begin(), n->GetNames().end(),
        [&](auto name, size_t) { m_line << name; },
        [&](let) { m_line << ", "; });
  }

  bool is_empty = n->GetFields().empty() && n->GetMethods().empty() &&
                  n->GetStaticMethods().empty();

  if (is_empty) {
    m_line << " {}";
    return;
  }

  m_line << " {" << std::endl;
  m_indent += m_tabSize;

  auto fields_count = n->GetFields();
  auto methods_count = n->GetMethods();
  auto static_methods_count = n->GetStaticMethods();

  std::for_each(n->GetFields().begin(), n->GetFields().end(), [&](auto field) {
    m_line << GetIndent() << field.GetVis() << " ";

    m_line << field.GetName() << ": ";
    field.GetType().Accept(*this);

    if (field.GetValue().has_value()) {
      m_line << " = ";
      field.GetValue().value().Accept(*this);
    }

    m_line << "," << std::endl;
  });

  if (!fields_count.empty() && !methods_count.empty()) {
    m_line << std::endl;
  }

  std::for_each(n->GetMethods().begin(), n->GetMethods().end(),
                [&](auto method) {
                  m_line << GetIndent() << method.m_vis << " ";
                  method.m_func.Accept(*this);
                  m_line << std::endl;
                });

  if (!static_methods_count.empty() &&
      (!fields_count.empty() || !methods_count.empty())) {
    m_line << std::endl;
  }

  std::for_each(n->GetStaticMethods().begin(), n->GetStaticMethods().end(),
                [&](auto method) {
                  m_line << GetIndent() << method.m_vis << " static ";
                  method.m_func.Accept(*this);
                  m_line << std::endl;
                });

  m_indent -= m_tabSize;
  m_line << "}";
}

void CambrianFormatter::Visit(FlowPtr<EnumDef> n) {
  PrintLineComments(n);

  m_line << "enum " << n->GetName();
  if (n->GetType()) {
    m_line << ": ";
    n->GetType().value().Accept(*this);
  }

  if (n->GetItems().empty()) {
    m_line << ";";
    return;
  }

  m_line << " {" << std::endl;
  m_indent += m_tabSize;

  for (auto it = n->GetItems().begin(); it != n->GetItems().end(); ++it) {
    m_line << GetIndent();
    m_line << it->first;
    if (it->second) {
      m_line << " = ";
      it->second.value().Accept(*this);
    }
    m_line << "," << std::endl;
  }

  m_indent -= m_tabSize;
  m_line << GetIndent() << "}";
}

void CambrianFormatter::Visit(FlowPtr<ScopeStmt> n) {
  PrintLineComments(n);

  m_line << "scope ";

  if (!n->GetName()->empty()) {
    m_line << n->GetName();
  }

  if (!n->GetDeps().empty()) {
    m_line << ": [";
    IterateExceptLast(
        n->GetDeps().begin(), n->GetDeps().end(),
        [&](auto dep, size_t) { m_line << dep; }, [&](let) { m_line << ", "; });
    m_line << "]";
  }

  m_line << " ";
  WrapStmtBody(n->GetBody(), 50, true);
}

void CambrianFormatter::Visit(FlowPtr<ExportStmt> n) {
  PrintLineComments(n);

  m_line << n->GetVis();

  if (!n->GetAbiName()->empty()) {
    m_line << " ";
    EscapeStringLiteral(n->GetAbiName());
  }

  if (!n->GetAttrs().empty()) {
    m_line << " [";
    IterateExceptLast(
        n->GetAttrs().begin(), n->GetAttrs().end(),
        [&](auto attr, size_t) { attr.Accept(*this); },
        [&](let) { m_line << ", "; });
    m_line << "]";
  }

  m_line << " ";

  WrapStmtBody(n->GetBody(), -1, false);
}
