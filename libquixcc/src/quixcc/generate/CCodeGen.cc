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

#define QUIXCC_INTERNAL

#include <openssl/evp.h>
#include <quixcc/core/Logger.h>
#include <quixcc/generate/CodeGen.h>
#include <quixcc/mangle/Symbol.h>

#include <iomanip>

using namespace libquixcc;
using namespace libquixcc::codegen;
using namespace libquixcc::ir;
using namespace libquixcc::ir::delta;

static std::string escape_cstr(const std::string &str) {
  std::string escaped;
  for (char c : str) {
    switch (c) {
      case '\n':
        escaped += "\\n";
        break;
      case '\r':
        escaped += "\\r";
        break;
      case '\t':
        escaped += "\\t";
        break;
      case '\v':
        escaped += "\\v";
        break;
      case '\f':
        escaped += "\\f";
        break;
      case '\a':
        escaped += "\\a";
        break;
      case '\b':
        escaped += "\\b";
        break;
      case '\\':
        escaped += "\\\\";
        break;
      case '\"':
        escaped += "\\\"";
        break;
      default:
        escaped += c;
        break;
    }
  }
  return escaped;
}

void C11Codegen::ind(std::ostream &code) {
  code << std::string(m_state.indent * INDENT_SIZE, ' ');
}

void C11Codegen::gen(const ir::delta::I1 *n, std::ostream &code) {
  code << "qbool";
}

void C11Codegen::gen(const ir::delta::I8 *n, std::ostream &code) {
  code << "qint8";
}

void C11Codegen::gen(const ir::delta::I16 *n, std::ostream &code) {
  code << "qint16";
}

void C11Codegen::gen(const ir::delta::I32 *n, std::ostream &code) {
  code << "qint32";
}

void C11Codegen::gen(const ir::delta::I64 *n, std::ostream &code) {
  code << "qint64";
}

void C11Codegen::gen(const ir::delta::I128 *n, std::ostream &code) {
  code << "qint128";
}

void C11Codegen::gen(const ir::delta::U8 *n, std::ostream &code) {
  code << "quint8";
}

void C11Codegen::gen(const ir::delta::U16 *n, std::ostream &code) {
  code << "quint16";
}

void C11Codegen::gen(const ir::delta::U32 *n, std::ostream &code) {
  code << "quint32";
}

void C11Codegen::gen(const ir::delta::U64 *n, std::ostream &code) {
  code << "quint64";
}

void C11Codegen::gen(const ir::delta::U128 *n, std::ostream &code) {
  code << "quint128";
}

void C11Codegen::gen(const ir::delta::F32 *n, std::ostream &code) {
  code << "qfloat32";
}

void C11Codegen::gen(const ir::delta::F64 *n, std::ostream &code) {
  code << "qfloat64";
}

void C11Codegen::gen(const ir::delta::Void *n, std::ostream &code) {
  code << "qvoid";
}

void C11Codegen::gen(const ir::delta::Ptr *n, std::ostream &code) {
  gen(n->type, code);
  code << " *";
}

void C11Codegen::gen(const ir::delta::PacketDef *n, std::ostream &code) {
  std::stringstream ss;
  ss << "struct quix_" << n->name << " {\n";

  m_state.indent++;

  for (size_t i = 0; i < n->fields.size(); i++) {
    ind(ss);
    gen(n->fields[i].second, ss);
    ss << " _" << std::to_string(i) << ";\n";
  }

  m_state.indent--;

  ss << "} __attribute__((packed));\n";

  m_state.structs.push_back(ss.str());
}

void C11Codegen::gen(const ir::delta::Packet *n, std::ostream &code) {
  code << "struct quix_" << n->def->name;
}

void C11Codegen::gen(const ir::delta::Array *n, std::ostream &code) {
  gen(n->type, code);
  code << "[" << n->size << "]";
}

void C11Codegen::gen(const ir::delta::FType *n, std::ostream &code) {
  gen(n->ret, code);
  code << " (*)(";
  for (size_t i = 0; i < n->params.size(); i++) {
    gen(n->params[i], code);
    if (i < n->params.size() - 1)
      code << ", ";
    else if (n->variadic)
      code << ", ...";
  }

  code << ")";
}

void C11Codegen::gen(const ir::delta::Local *n, std::ostream &code) {
  if (n->type->is<Array>()) {
    gen(n->type->as<Array>()->type, code);
    code << " " << n->name << "[" << n->type->as<Array>()->size << "]";
  } else {
    gen(n->type, code);
    if (!n->type->is<Ptr>()) code << " ";
    code << n->name;
  }

  if (n->value) {
    code << " = ";
    gen(n->value, code);
  }
}

void C11Codegen::gen(const ir::delta::Global *n, std::ostream &code) {
  enum class Linkage {
    Private,
    Public,
    External,
  };

  Linkage linkage = Linkage::Private;

  if (n->value && n->value->is<Segment>()) {
    auto seg = n->value->as<Segment>();

    if (n->_extern && (!seg->block))
      linkage = Linkage::External;
    else if (n->_extern && seg->block)
      linkage = Linkage::Public;
  } else {
    if (n->_extern && (!n->value))
      linkage = Linkage::External;
    else if (n->_extern && n->value)
      linkage = Linkage::Public;
  }

  switch (linkage) {
    case Linkage::Private:
      code << "static ";
      break;
    case Linkage::Public:
      code << "qexport ";
      break;
    case Linkage::External:
      code << "extern ";
      break;
  }

  // DeltaIR Stored functions as values called Segments
  if (n->value && n->value->is<Segment>()) {
    auto old = m_state.segmentvar;
    m_state.segmentvar = n;
    gen(n->value, code);
    m_state.segmentvar = old;
  } else {
    gen(n->type, code);

    if (!n->type->is<Ptr>()) code << " ";

    code << n->name;

    if (n->value) {
      code << " = ";
      gen(n->value, code);
    }
  }
}

void C11Codegen::gen(const ir::delta::Number *n, std::ostream &code) {
  /// TODO: Implement Number 128 bit stuff

  code << n->value;
}

void C11Codegen::gen(const ir::delta::String *n, std::ostream &code) {
  code << "\"" << escape_cstr(n->value) << "\"";
}

void C11Codegen::gen(const ir::delta::List *n, std::ostream &code) {
  code << "{";

  for (size_t i = 0; i < n->values.size(); i++) {
    gen(n->values[i], code);
    if (i < n->values.size() - 1) code << ", ";
  }

  code << "}";
}

void C11Codegen::gen(const ir::delta::Ident *n, std::ostream &code) {
  code << n->name;
}

void C11Codegen::gen(const ir::delta::Assign *n, std::ostream &code) {
  gen(n->var, code);
  code << " = ";
  gen(n->value, code);
}

void C11Codegen::gen(const libquixcc::ir::delta::PostInc *node,
                     std::ostream &code) {
  gen(node->var, code);
  code << "++";
}

void C11Codegen::gen(const libquixcc::ir::delta::PostDec *node,
                     std::ostream &code) {
  gen(node->var, code);
  code << "--";
}

void C11Codegen::gen(const ir::delta::AddressOf *n, std::ostream &code) {
  code << "&";
  gen(n->lhs, code);
}

void C11Codegen::gen(const ir::delta::Deref *n, std::ostream &code) {
  code << "*";
  gen(n->lhs, code);
}

void C11Codegen::gen(const ir::delta::Member *n, std::ostream &code) {
  gen(n->lhs, code);

  if (n->lhs->infer()->is_ptr())
    code << "->";
  else
    code << ".";

  code << "_" << n->field;
}

void C11Codegen::gen(const ir::delta::Index *n, std::ostream &code) {
  gen(n->expr, code);
  code << "[";
  gen(n->index, code);
  code << "]";
}

void C11Codegen::gen(const ir::delta::SCast *n, std::ostream &code) {
  code << "(";
  gen(n->type, code);
  code << ")";
  gen(n->value, code);
}

void C11Codegen::gen(const ir::delta::UCast *n, std::ostream &code) {
  code << "(";
  gen(n->type, code);
  code << ")";
  gen(n->value, code);
}

void C11Codegen::gen(const ir::delta::PtrICast *n, std::ostream &code) {
  code << "(quintptr)";
  gen(n->value, code);
}

void C11Codegen::gen(const ir::delta::IPtrCast *n, std::ostream &code) {
  code << "(";
  gen(n->type, code);
  code << ")" << "(quintptr)";
  gen(n->value, code);
}

static std::string escape_into_unqiue_ident(const std::string &str) {
  const char VALID_CHARS[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
  std::array<uint8_t, 32> hash;
  unsigned int len = 0;

  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  EVP_DigestInit(ctx, EVP_sha256());
  EVP_DigestUpdate(ctx, str.c_str(), str.size());
  EVP_DigestFinal(ctx, (uint8_t *)hash.data(), &len);
  EVP_MD_CTX_free(ctx);

  std::stringstream ss;
  for (size_t i = 0; i < 12; i++)
    ss << VALID_CHARS[hash[i] % (sizeof(VALID_CHARS) - 1)];

  return "_" + ss.str();
}

void C11Codegen::gen(const ir::delta::Bitcast *n, std::ostream &code) {
  std::stringstream fn, tmp;

  std::string vali, tp;

  fn << "static inline ";
  gen(n->type, fn);
  fn << " ";

  gen(n->value->infer(), tmp);
  vali = tmp.str();

  tmp.str("");
  gen(n->type, tmp);
  tp = tmp.str();

  std::string name = "qbc" + escape_into_unqiue_ident(vali + tp);

  fn << name << "(" << vali << " x) {\n";
  ind(fn);
  fn << "union { " << vali << " x; " << tp << " y; } u;\n";
  ind(fn);
  fn << "u.x = x;\n";
  ind(fn);
  fn << "return u.y;\n";
  fn << "}\n";

  m_state.autogen.insert(fn.str());

  code << name << "(";
  gen(n->value, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::IfElse *n, std::ostream &code) {
  code << "if (";
  gen(n->cond, code);
  code << ") ";
  gen(n->then, code);

  if (n->els) {
    code << "else ";
    gen(n->els, code);
  }
}

void C11Codegen::gen(const ir::delta::While *n, std::ostream &code) {
  code << "while (";
  gen(n->cond, code);
  code << ") ";
  gen(n->body, code);
}

void C11Codegen::gen(const ir::delta::Jmp *n, std::ostream &code) {
  code << "goto " << n->target;
}

void C11Codegen::gen(const ir::delta::Label *n, std::ostream &code) {
  code << n->name << ":";

  ind(code);
  gen(n->code, code);
}

void C11Codegen::gen(const ir::delta::Ret *n, std::ostream &code) {
  if (n->value) {
    code << "return ";
    gen(n->value, code);
  } else {
    code << "return";
  }
}

void C11Codegen::gen(const ir::delta::Call *n, std::ostream &code) {
  code << n->callee << "(";
  for (size_t i = 0; i < n->args.size(); i++) {
    gen(n->args[i], code);
    if (i < n->args.size() - 1) code << ", ";
  }

  code << ")";
}

void C11Codegen::gen(const ir::delta::PtrCall *n, std::ostream &code) {
  code << "(";
  gen(n->callee, code);
  code << ")(";
  for (size_t i = 0; i < n->args.size(); i++) {
    gen(n->args[i], code);
    if (i < n->args.size() - 1) code << ", ";
  }

  code << ")";
}

void C11Codegen::gen(const ir::delta::Halt *n, std::ostream &code) {
  code << "quix__halt()";
}

void C11Codegen::gen(const ir::delta::Break *n, std::ostream &code) {
  code << "break";
}

void C11Codegen::gen(const ir::delta::Continue *n, std::ostream &code) {
  code << "continue";
}

void C11Codegen::gen(const ir::delta::Switch *n, std::ostream &code) {
  code << "switch (";
  gen(n->cond, code);
  code << ") {\n";

  m_state.indent++;

  for (auto &node : n->cases) {
    gen(node, code);
  }

  if (n->def) {
    ind(code);
    code << "default: ";
    gen(n->def, code);

    code << "\n";
  }

  m_state.indent--;

  ind(code);
  code << "}";
}

void C11Codegen::gen(const libquixcc::ir::delta::Case *node,
                     std::ostream &code) {
  ind(code);
  code << "case ";
  gen(node->value, code);
  code << ": ";

  gen(node->code, code);

  code << "\n";
}

void C11Codegen::gen(const ir::delta::Block *n, std::ostream &code) {
  code << "{\n";

  m_state.indent++;

  for (auto &node : n->stmts) {
    ind(code);
    gen(node, code);
    code << ";\n";
  }

  m_state.indent--;

  ind(code);
  code << "}";
}

void C11Codegen::gen(const ir::delta::Segment *n, std::ostream &code) {
  if (!m_state.segmentvar)
    throw std::runtime_error("C11Codegen: Segment outside of global scope");

  gen(m_state.segmentvar->type->as<FType>()->ret, code);
  code << " ";
  code << m_state.segmentvar->name << "(";

  for (size_t i = 0; i < n->params.size(); i++) {
    gen(n->params[i].second, code);

    if (n->params[i].second->is<Ptr>())
      code << n->params[i].first;
    else
      code << " " << n->params[i].first;

    if (i < n->params.size() - 1)
      code << ", ";
    else if (n->variadic)
      code << ", ...";
  }

  code << ")";

  if (n->block) {
    code << " ";
    gen(n->block, code);
  }
}

void C11Codegen::gen(const libquixcc::ir::delta::Asm *node,
                     std::ostream &code) {
  /// TODO: Implement Asm
  throw std::runtime_error("C11Codegen: Asm not implemented");
}

void C11Codegen::gen(const ir::delta::Add *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " + ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Sub *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " - ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Mul *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " * ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Div *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " / ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Mod *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " % ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::BitAnd *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " & ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::BitOr *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " | ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::BitXor *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " ^ ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::BitNot *n, std::ostream &code) {
  code << "(~";
  gen(n->operand, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Shl *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " << ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Shr *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " >> ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Rotl *n, std::ostream &code) {
  code << "QROTL_EXPR(";
  gen(n->lhs, code);
  code << ", ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Rotr *n, std::ostream &code) {
  code << "QROTR_EXPR(";
  gen(n->lhs, code);
  code << ", ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Eq *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " == ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Ne *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " != ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Lt *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " < ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Gt *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " > ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Le *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " <= ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Ge *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " >= ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::And *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " && ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Or *n, std::ostream &code) {
  code << "(";
  gen(n->lhs, code);
  code << " || ";
  gen(n->rhs, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Not *n, std::ostream &code) {
  code << "(!";
  gen(n->operand, code);
  code << ")";
}

void C11Codegen::gen(const ir::delta::Xor *n, std::ostream &code) {
  code << "((!";
  gen(n->lhs, code);
  code << " && ";
  gen(n->rhs, code);
  code << ") || (";
  gen(n->lhs, code);
  code << " && !";
  gen(n->rhs, code);
  code << "))";
}

void C11Codegen::gen(const ir::delta::RootNode *n, std::ostream &code) {
  /* Comment headers */
  code << "/////////////////////////////////////////////////////////\n";
  code << "///                                                   ///\n";
  code << "///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒   ///\n";
  code << "///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒   ///\n";
  code << "///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒   ///\n";
  code << "///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░   ///\n";
  code << "///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒   ///\n";
  code << "///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒   ///\n";
  code << "///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒   ///\n";
  code << "///      ░▒▓█▓▒░                                      ///\n";
  code << "///       ░▒▓██▓▒░                                    ///\n";
  code << "///                                                   ///\n";
  code << "///   * This C (11) code was generated by the QUIXCC  ///\n";
  code << "///     Language Compiler Suite.                      ///\n";
  code << "///                                                   ///\n";
  code << "///   Transcompiled source code may be encumbered by  ///\n";
  code << "///   original source code licensing.                 ///\n";
  code << "///                                                   ///\n";
  code << "/////////////////////////////////////////////////////////\n\n";

  /* Include standard integer header */
  code << "#include <stdint.h>\n";
  code << "#include <stdbool.h>\n\n";

  code << "#ifndef __QUIX_CODEGEN__\n";
  code << "#define __QUIX_CODEGEN__\n\n";
  code << "typedef uint8_t quint8;\n";
  code << "typedef uint16_t quint16;\n";
  code << "typedef uint32_t quint32;\n";
  code << "typedef uint64_t quint64;\n";
  code << "typedef int8_t qint8;\n";
  code << "typedef int16_t qint16;\n";
  code << "typedef int32_t qint32;\n";
  code << "typedef int64_t qint64;\n";
  code << "typedef bool qbool;\n";
  code << "typedef float qfloat32;\n";
  code << "typedef double qfloat64;\n";
  code << "typedef void qvoid;\n\n";
  code << "#ifndef __SIZEOF_INT128__\n";
  code << "#error \"QUIXCC requires support for __int128 to support qint128 "
          "and quint128 types.\"\n";
  code << "#endif\n\n";
  code << "typedef __int128 qint128;\n";
  code << "typedef unsigned __int128 quint128;\n\n";

  /* Architecture-specific definitions */
  code << "/* Architecture-specific definitions */\n";
  code << "#if UINTPTR_MAX == 0xFF\n";
  code << "#define QPTRSIZE 8\n";
  code << "#elif UINTPTR_MAX == 0xFFFF\n";
  code << "#define QPTRSIZE 16\n";
  code << "#elif UINTPTR_MAX == 0xFFFFFFFF\n";
  code << "#define QPTRSIZE 32\n";
  code << "#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu\n";
  code << "#define QPTRSIZE 64\n";
  code << "#else\n";
  code << "#error \"Target architecture not supported by QUIXCC \" \\\n";
  code << "       \"because pointer size is unknown.\"\n";
  code << "#endif\n\n";

  code << "#if QPTRSIZE == 8\n";
  code << "typedef quint8 quintptr;\n";
  code << "typedef qint8 qintptr;\n";
  code << "#elif QPTRSIZE == 16\n";
  code << "typedef quint16 quintptr;\n";
  code << "typedef qint16 qintptr;\n";
  code << "#elif QPTRSIZE == 32\n";
  code << "typedef quint32 quintptr;\n";
  code << "typedef qint32 qintptr;\n";
  code << "#elif QPTRSIZE == 64\n";
  code << "typedef quint64 quintptr;\n";
  code << "typedef qint64 qintptr;\n";
  code << "#endif\n\n";

  /* Macros */
  code << "/* Macro definitions */\n";
  code << "#ifndef qexport\n";
  code << "#if defined(__clang__)\n";
  code << "#define qexport __attribute__((visibility(\"default\")))\n";
  code << "#elif defined(__GNUC__)\n";
  code << "#define qexport __attribute__((visibility(\"default\")))\n";
  code << "#else\n";
  code << "#error \"Unsupported compiler; cannot define qexport.\"\n";
  code << "#endif\n";
  code << "#endif\n\n";
  code << "#ifndef QROTL_EXPR\n";
  code << "#define QROTL_EXPR(x, n) ((x << n) | (x >> (8 * sizeof(x) - n)))\n";
  code << "#endif\n";
  code << "#ifndef QROTR_EXPR\n";
  code << "#define QROTR_EXPR(x, n) ((x >> n) | (x << (8 * sizeof(x) - n)))\n";
  code << "#endif\n\n";

  code << "#endif /* __QUIX_CODEGEN__ */\n\n";

  std::stringstream ss, tmp;

  for (size_t i = 0; i < n->children.size(); i++) {
    gen(n->children[i], tmp);
    if (tmp.tellp() == 0) continue;

    ss << tmp.str();
    tmp.str("");

    if (i < n->children.size() - 1)
      ss << ";\n\n";
    else
      ss << ";\n";
  }

  for (auto &node : m_state.structs) code << node << "\n";
  for (auto &node : m_state.autogen) code << node << "\n";

  code << ss.str();
}

void C11Codegen::gen(const libquixcc::ir::delta::Value *n, std::ostream &code) {
  switch ((delta::NodeType)n->ntype) {
    case delta::NodeType::I1:
      return gen(n->as<I1>(), code);
    case delta::NodeType::I8:
      return gen(n->as<I8>(), code);
    case delta::NodeType::I16:
      return gen(n->as<I16>(), code);
    case delta::NodeType::I32:
      return gen(n->as<I32>(), code);
    case delta::NodeType::I64:
      return gen(n->as<I64>(), code);
    case delta::NodeType::I128:
      return gen(n->as<I128>(), code);
    case delta::NodeType::U8:
      return gen(n->as<U8>(), code);
    case delta::NodeType::U16:
      return gen(n->as<U16>(), code);
    case delta::NodeType::U32:
      return gen(n->as<U32>(), code);
    case delta::NodeType::U64:
      return gen(n->as<U64>(), code);
    case delta::NodeType::U128:
      return gen(n->as<U128>(), code);
    case delta::NodeType::F32:
      return gen(n->as<F32>(), code);
    case delta::NodeType::F64:
      return gen(n->as<F64>(), code);
    case delta::NodeType::Void:
      return gen(n->as<Void>(), code);
    case delta::NodeType::Ptr:
      return gen(n->as<Ptr>(), code);
    case delta::NodeType::PacketDef:
      return gen(n->as<PacketDef>(), code);
    case delta::NodeType::Packet:
      return gen(n->as<Packet>(), code);
    case delta::NodeType::Array:
      return gen(n->as<Array>(), code);
    case delta::NodeType::FType:
      return gen(n->as<FType>(), code);
    case delta::NodeType::Local:
      return gen(n->as<Local>(), code);
    case delta::NodeType::Global:
      return gen(n->as<Global>(), code);
    case delta::NodeType::Number:
      return gen(n->as<Number>(), code);
    case delta::NodeType::String:
      return gen(n->as<String>(), code);
    case delta::NodeType::List:
      return gen(n->as<List>(), code);
    case delta::NodeType::Ident:
      return gen(n->as<Ident>(), code);
    case delta::NodeType::Assign:
      return gen(n->as<Assign>(), code);
    case delta::NodeType::PostInc:
      return gen(n->as<PostInc>(), code);
    case delta::NodeType::PostDec:
      return gen(n->as<PostDec>(), code);
    case delta::NodeType::AddressOf:
      return gen(n->as<AddressOf>(), code);
    case delta::NodeType::Deref:
      return gen(n->as<Deref>(), code);
    case delta::NodeType::Member:
      return gen(n->as<Member>(), code);
    case delta::NodeType::Index:
      return gen(n->as<Index>(), code);
    case delta::NodeType::SCast:
      return gen(n->as<SCast>(), code);
    case delta::NodeType::UCast:
      return gen(n->as<UCast>(), code);
    case delta::NodeType::PtrICast:
      return gen(n->as<PtrICast>(), code);
    case delta::NodeType::IPtrCast:
      return gen(n->as<IPtrCast>(), code);
    case delta::NodeType::Bitcast:
      return gen(n->as<Bitcast>(), code);
    case delta::NodeType::IfElse:
      return gen(n->as<IfElse>(), code);
    case delta::NodeType::While:
      return gen(n->as<While>(), code);
    case delta::NodeType::Jmp:
      return gen(n->as<Jmp>(), code);
    case delta::NodeType::Label:
      return gen(n->as<Label>(), code);
    case delta::NodeType::Ret:
      return gen(n->as<Ret>(), code);
    case delta::NodeType::Call:
      return gen(n->as<Call>(), code);
    case delta::NodeType::PtrCall:
      return gen(n->as<PtrCall>(), code);
    case delta::NodeType::Halt:
      return gen(n->as<Halt>(), code);
    case delta::NodeType::Break:
      return gen(n->as<Break>(), code);
    case delta::NodeType::Continue:
      return gen(n->as<Continue>(), code);
    case delta::NodeType::Switch:
      return gen(n->as<Switch>(), code);
    case delta::NodeType::Block:
      return gen(n->as<Block>(), code);
    case delta::NodeType::Segment:
      return gen(n->as<Segment>(), code);
    case delta::NodeType::Asm:
      return gen(n->as<Asm>(), code);
    case delta::NodeType::Add:
      return gen(n->as<Add>(), code);
    case delta::NodeType::Sub:
      return gen(n->as<Sub>(), code);
    case delta::NodeType::Mul:
      return gen(n->as<Mul>(), code);
    case delta::NodeType::Div:
      return gen(n->as<Div>(), code);
    case delta::NodeType::Mod:
      return gen(n->as<Mod>(), code);
    case delta::NodeType::BitAnd:
      return gen(n->as<BitAnd>(), code);
    case delta::NodeType::BitOr:
      return gen(n->as<BitOr>(), code);
    case delta::NodeType::BitXor:
      return gen(n->as<BitXor>(), code);
    case delta::NodeType::BitNot:
      return gen(n->as<BitNot>(), code);
    case delta::NodeType::Shl:
      return gen(n->as<Shl>(), code);
    case delta::NodeType::Shr:
      return gen(n->as<Shr>(), code);
    case delta::NodeType::Rotl:
      return gen(n->as<Rotl>(), code);
    case delta::NodeType::Rotr:
      return gen(n->as<Rotr>(), code);
    case delta::NodeType::Eq:
      return gen(n->as<Eq>(), code);
    case delta::NodeType::Ne:
      return gen(n->as<Ne>(), code);
    case delta::NodeType::Lt:
      return gen(n->as<Lt>(), code);
    case delta::NodeType::Gt:
      return gen(n->as<Gt>(), code);
    case delta::NodeType::Le:
      return gen(n->as<Le>(), code);
    case delta::NodeType::Ge:
      return gen(n->as<Ge>(), code);
    case delta::NodeType::And:
      return gen(n->as<And>(), code);
    case delta::NodeType::Or:
      return gen(n->as<Or>(), code);
    case delta::NodeType::Not:
      return gen(n->as<Not>(), code);
    case delta::NodeType::Xor:
      return gen(n->as<Xor>(), code);
    case delta::NodeType::Root:
      return gen(n->as<RootNode>(), code);
    default:
      throw std::runtime_error(
          "Codegen failed: codegen not implemented for value: " +
          std::to_string(n->ntype));
  }
}

bool C11Codegen::codegen(
    const std::unique_ptr<libquixcc::ir::delta::IRDelta> &ir,
    std::ostream &os) {
  C11Codegen codegen;

  codegen.gen(ir->root(), os);
  return true; /* Errors -> exceptions */
}
