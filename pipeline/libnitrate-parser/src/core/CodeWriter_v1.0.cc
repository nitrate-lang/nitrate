////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <nitrate-core/Logger.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/CodeWriter.hh>

using namespace ncc::lex;
using namespace ncc::parse;
using namespace ncc::parse::detail;

static bool IsNamedParameter(std::string_view name) { return std::isdigit(name.at(0)) == 0; }

static bool IsWordOperator(Operator op) {
  switch (op) {
    case OpAs:
    case OpBitcastAs:
    case OpIn:
    case OpOut:
    case OpSizeof:
    case OpBitsizeof:
    case OpAlignof:
    case OpTypeof:
    case OpComptime:
      return true;

    default:
      return false;
  }
}

static std::string StringEscape(std::string_view str) {
  std::stringstream ss;

  for (char ch : str) {
    switch (ch) {
      case '\n': {
        ss << "\\n";
        break;
      }

      case '\t': {
        ss << "\\t";
        break;
      }

      case '\r': {
        ss << "\\r";
        break;
      }

      case '\v': {
        ss << "\\v";
        break;
      }

      case '\f': {
        ss << "\\f";
        break;
      }

      case '\b': {
        ss << "\\b";
        break;
      }

      case '\a': {
        ss << "\\a";
        break;
      }

      case '\\': {
        ss << "\\\\";
        break;
      }

      case '"': {
        ss << "\\\"";
        break;
      }

      default: {
        ss << ch;
        break;
      }
    }
  }

  return ss.str();
}

void CodeWriter_v1_0::PutKeyword(Keyword kw) {
  switch (m_last) {
    case KeyW:
    case Name:
    case Macr:
    case NumL: {
      m_os << ' ' << kw;
      break;
    }

    case Oper: {
      if (IsWordOperator(m_ldata.m_op)) {
        m_os << ' ' << kw;
      } else {
        m_os << kw;
      }
      break;
    }

    case EofF:
    case Punc:
    case IntL:
    case Text:
    case Char:
    case MacB:
    case Note: {
      m_os << kw;
      break;
    }
  }

  m_last = KeyW;
  m_ldata = kw;
}

void CodeWriter_v1_0::PutOperator(Operator op) {
  switch (m_last) {
    case KeyW:
    case Name:
    case Macr: {
      if (IsWordOperator(op)) {
        m_os << ' ' << op;
      } else {
        m_os << op;
      }
      break;
    }

    case Oper:
    case Punc:
    case IntL:
    case NumL: {
      /// FIXME: Minimize redundant whitespace
      m_os << ' ' << op;
      break;
    }

    case EofF:
    case Text:
    case Char:
    case MacB:
    case Note: {
      m_os << op;
      break;
    }
  }

  m_last = Oper;
  m_ldata = op;
}

void CodeWriter_v1_0::PutPunctor(Punctor punc) {
  switch (m_last) {
    case Oper: {
      /// FIXME: Minimize redundant whitespace
      m_os << ' ' << punc;
      break;
    }

    case Punc: {
      /// FIXME: Minimize redundant whitespace
      m_os << ' ' << punc;
      break;
    }

    case IntL:
    case NumL:
    case Macr:
    case EofF:
    case KeyW:
    case Name:
    case Text:
    case Char:
    case MacB:
    case Note: {
      m_os << punc;
      break;
    }
  }

  m_last = Punc;
  m_ldata = punc;
}

void CodeWriter_v1_0::PutIdentifier(std::string_view name) {
  switch (m_last) {
    case Oper: {
      if (IsWordOperator(m_ldata.m_op)) {
        m_os << ' ' << name;
      } else {
        m_os << name;
      }
      break;
    }

    case KeyW:
    case Name:
    case Macr: {
      m_os << ' ' << name;
      break;
    }

    case NumL: {
      /// FIXME: Minimize redundant whitespace
      m_os << ' ' << name;
      break;
    }

    case IntL: {
      /// FIXME: Minimize redundant whitespace
      m_os << ' ' << name;
      break;
    }

    case EofF:
    case Punc:
    case Text:
    case Char:
    case MacB:
    case Note: {
      m_os << name;
      break;
    }
  }

  m_last = Name;
  m_ldata = string(name);
}

void CodeWriter_v1_0::PutInteger(std::string_view num) {
  switch (m_last) {
    case Oper: {
      if (IsWordOperator(m_ldata.m_op)) {
        m_os << ' ' << num;
      } else {
        m_os << num;
      }
      break;
    }

    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr: {
      m_os << ' ' << num;
      break;
    }

    case EofF:
    case Punc:
    case Text:
    case Char:
    case MacB:
    case Note: {
      m_os << num;
      break;
    }
  }

  m_last = IntL;
  m_ldata = string(num);
}

void CodeWriter_v1_0::PutFloat(std::string_view num) {
  switch (m_last) {
    case Oper: {
      if (IsWordOperator(m_ldata.m_op)) {
        m_os << ' ' << num;
      } else {
        m_os << num;
      }
      break;
    }

    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr: {
      m_os << ' ' << num;
      break;
    }

    case EofF:
    case Punc:
    case Text:
    case Char:
    case MacB:
    case Note: {
      m_os << num;
      break;
    }
  }

  m_last = NumL;
  m_ldata = string(num);
}

void CodeWriter_v1_0::PutString(std::string_view str) {
  switch (m_last) {
    case Text: {
      /// FIXME: Minimize redundant whitespace
      /// FIXME: Ensure adjacent strings are not concatenated?
      m_os << ' ' << '"' << StringEscape(str) << '"';
      break;
    }

    case Char:
    case Oper:
    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr:
    case EofF:
    case Punc:
    case MacB:
    case Note: {
      m_os << '"' << StringEscape(str) << '"';
      break;
    }
  }

  m_last = Text;
  m_ldata = string(str);
}

void CodeWriter_v1_0::PutCharacter(std::string_view ch) {
  switch (m_last) {
    case Char: {
      /// FIXME: Minimize redundant whitespace
      /// FIXME: Ensure adjacent characters are not concatenated?
      m_os << ' ' << '\'' << StringEscape(ch) << '\'';
      break;
    }

    case Text:
    case Oper:
    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr:
    case EofF:
    case Punc:
    case MacB:
    case Note: {
      m_os << '\'' << StringEscape(ch) << '\'';
      break;
    }
  }

  m_last = Char;
  m_ldata = string(ch);
}

void CodeWriter_v1_0::PutMacroBlock(std::string_view macro) {
  switch (m_last) {
    case Text:
    case Char:
    case Oper:
    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr:
    case EofF:
    case Punc:
    case MacB:
    case Note: {
      m_os << "@(" << macro << ")";
      break;
    }
  }

  m_last = MacB;
  m_ldata = string(macro);
}

void CodeWriter_v1_0::PutMacroCall(std::string_view macro) {
  switch (m_last) {
    case Text:
    case Char:
    case Oper:
    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr:
    case EofF:
    case Punc:
    case MacB:
    case Note: {
      m_os << "@" << macro;
      break;
    }
  }

  m_last = Macr;
  m_ldata = string(macro);
}

void CodeWriter_v1_0::PutComment(std::string_view note) {
  switch (m_last) {
    case Text:
    case Char:
    case Oper:
    case KeyW:
    case Name:
    case IntL:
    case NumL:
    case Macr:
    case EofF:
    case Punc:
    case MacB:
    case Note: {
      m_os << "/*" << note << "*/";
      break;
    }
  }

  m_last = Note;
  m_ldata = string(note);
}

///=============================================================================

void CodeWriter_v1_0::PutTypeStuff(const FlowPtr<Type>& n) {
  if (n->GetRangeBegin() || n->GetRangeEnd()) {
    PutPunctor(PuncColn);
    PutPunctor(PuncLBrk);
    if (n->GetRangeBegin()) {
      n->GetRangeBegin().value()->Accept(*this);
    }
    PutPunctor(PuncColn);
    if (n->GetRangeEnd()) {
      n->GetRangeEnd().value()->Accept(*this);
    }
    PutPunctor(PuncRBrk);
  }

  if (n->GetWidth()) {
    PutPunctor(PuncColn);
    n->GetWidth().value()->Accept(*this);
  }
}

void CodeWriter_v1_0::Visit(FlowPtr<Base>) {}

void CodeWriter_v1_0::Visit(FlowPtr<ExprStmt> n) {
  n->GetExpr()->Accept(*this);
  PutPunctor(PuncSemi);
}

void CodeWriter_v1_0::Visit(FlowPtr<LambdaExpr> n) { n->GetFunc()->Accept(*this); }

void CodeWriter_v1_0::Visit(FlowPtr<NamedTy> n) {
  PutIdentifier(n->GetName());
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<InferTy> n) {
  PutOperator(OpTernary);
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<TemplateType> n) {
  n->GetTemplate()->Accept(*this);
  PutOperator(OpLT);
  for (auto it = n->GetArgs().begin(); it != n->GetArgs().end(); ++it) {
    if (it != n->GetArgs().begin()) {
      PutPunctor(PuncComa);
    }

    const auto [pname, pval] = *it;
    if (IsNamedParameter(pname)) {
      PutIdentifier(pname);
      PutPunctor(PuncColn);
    }

    pval->Accept(*this);
  }
  PutOperator(OpGT);

  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<U1> n) {
  PutIdentifier("u1");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<U8> n) {
  PutIdentifier("u8");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<U16> n) {
  PutIdentifier("u16");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<U32> n) {
  PutIdentifier("u32");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<U64> n) {
  PutIdentifier("u64");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<U128> n) {
  PutIdentifier("u128");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<I8> n) {
  PutIdentifier("i8");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<I16> n) {
  PutIdentifier("i16");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<I32> n) {
  PutIdentifier("i32");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<I64> n) {
  PutIdentifier("i64");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<I128> n) {
  PutIdentifier("i128");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<F16> n) {
  PutIdentifier("f16");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<F32> n) {
  PutIdentifier("f32");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<F64> n) {
  PutIdentifier("f64");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<F128> n) {
  PutIdentifier("f128");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<VoidTy> n) {
  PutIdentifier("void");
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<PtrTy> n) {
  PutOperator(OpTimes);
  n->GetItem()->Accept(*this);
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<OpaqueTy> n) {
  PutKeyword(Opaque);
  PutPunctor(PuncLPar);
  PutIdentifier(n->GetName());
  PutPunctor(PuncRPar);
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<TupleTy> n) {
  PutPunctor(PuncLPar);
  for (auto it = n->GetItems().begin(); it != n->GetItems().end(); ++it) {
    if (it != n->GetItems().begin()) {
      PutPunctor(PuncComa);
    }

    (*it)->Accept(*this);
  }
  PutPunctor(PuncRPar);
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<ArrayTy> n) {
  PutPunctor(PuncLBrk);
  n->GetItem()->Accept(*this);
  PutPunctor(PuncColn);
  n->GetSize()->Accept(*this);
  PutPunctor(PuncRBrk);
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<RefTy> n) {
  PutOperator(OpBitAnd);
  n->GetItem()->Accept(*this);
  PutTypeStuff(n);
}

void CodeWriter_v1_0::Visit(FlowPtr<FuncTy> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Unary> n) {
  PutPunctor(PuncLPar);
  PutOperator(n->GetOp());
  n->GetRHS()->Accept(*this);
  PutPunctor(PuncRPar);
}

void CodeWriter_v1_0::Visit(FlowPtr<Binary> n) {
  PutPunctor(PuncLPar);
  n->GetLHS()->Accept(*this);
  PutOperator(n->GetOp());
  n->GetRHS()->Accept(*this);
  PutPunctor(PuncRPar);
}

void CodeWriter_v1_0::Visit(FlowPtr<PostUnary> n) {
  PutPunctor(PuncLPar);
  n->GetLHS()->Accept(*this);
  PutOperator(n->GetOp());
  PutPunctor(PuncRPar);
}

void CodeWriter_v1_0::Visit(FlowPtr<Ternary> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Integer> n) { PutInteger(n->GetValue()); }
void CodeWriter_v1_0::Visit(FlowPtr<Float> n) { PutFloat(n->GetValue()); }
void CodeWriter_v1_0::Visit(FlowPtr<Boolean> n) { PutKeyword(n->GetValue() ? True : False); }
void CodeWriter_v1_0::Visit(FlowPtr<String> n) { PutString(n->GetValue()); }

void CodeWriter_v1_0::Visit(FlowPtr<Character> n) {
  std::array buf = {static_cast<char>(n->GetValue()), '\0'};
  PutCharacter(std::string_view(buf.data(), buf.size()));
}

void CodeWriter_v1_0::Visit(FlowPtr<Null>) { PutKeyword(lex::Null); }
void CodeWriter_v1_0::Visit(FlowPtr<Undefined>) { PutKeyword(Undef); }

void CodeWriter_v1_0::Visit(FlowPtr<Call> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<TemplateCall> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<List> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Assoc> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Index> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Slice> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<FString> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Identifier> n) { PutIdentifier(n->GetName()); }

void CodeWriter_v1_0::Visit(FlowPtr<Sequence> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Block> n) {
  if (!m_did_root) {
    m_did_root = true;

    for (const auto& stmt : n->GetStatements()) {
      stmt->Accept(*this);
    }
  } else {
    PutPunctor(PuncLCur);
    for (const auto& stmt : n->GetStatements()) {
      stmt->Accept(*this);
    }
    PutPunctor(PuncRCur);
  }
}

void CodeWriter_v1_0::Visit(FlowPtr<Variable> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Assembly> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<If> n) {
  PutKeyword(lex::If);
  n->GetCond()->Accept(*this);
  n->GetThen()->Accept(*this);
  if (n->GetElse()) {
    PutKeyword(lex::Else);
    n->GetElse().value()->Accept(*this);
  }
}

void CodeWriter_v1_0::Visit(FlowPtr<While> n) {
  PutKeyword(lex::While);
  n->GetCond()->Accept(*this);
  n->GetBody()->Accept(*this);
}

void CodeWriter_v1_0::Visit(FlowPtr<For> n) {
  PutKeyword(lex::For);

  if (n->GetInit()) {
    n->GetInit().value()->Accept(*this);
  } else {
    PutPunctor(PuncSemi);
  }

  if (n->GetCond()) {
    n->GetCond().value()->Accept(*this);
  } else {
    PutPunctor(PuncSemi);
  }

  if (n->GetStep()) {
    n->GetStep().value()->Accept(*this);
  }

  n->GetBody()->Accept(*this);
}

void CodeWriter_v1_0::Visit(FlowPtr<Foreach> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Break>) {
  PutKeyword(lex::Break);
  PutPunctor(PuncSemi);
}

void CodeWriter_v1_0::Visit(FlowPtr<Continue>) {
  PutKeyword(lex::Continue);
  PutPunctor(PuncSemi);
}

void CodeWriter_v1_0::Visit(FlowPtr<Return> n) {
  PutKeyword(lex::Return);
  if (n->GetValue()) {
    n->GetValue().value()->Accept(*this);
  }
  PutPunctor(PuncSemi);
}

void CodeWriter_v1_0::Visit(FlowPtr<ReturnIf> n) {
  PutKeyword(lex::Retif);
  n->GetCond()->Accept(*this);
  PutPunctor(PuncComa);
  n->GetValue()->Accept(*this);
  PutPunctor(PuncSemi);
}

void CodeWriter_v1_0::Visit(FlowPtr<Case> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Switch> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Typedef> n) {
  PutKeyword(lex::Type);
  PutIdentifier(n->GetName());
  PutOperator(OpSet);
  n->GetType()->Accept(*this);
  PutPunctor(PuncSemi);
}

void CodeWriter_v1_0::Visit(FlowPtr<Function> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Struct> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Enum> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Scope> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Export> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

CodeWriter_v1_0::CodeWriter_v1_0(std::ostream& os) : m_os(os), m_last(EofF), m_ldata(OpPlus), m_did_root(false) {
  (void)m_os;
}
