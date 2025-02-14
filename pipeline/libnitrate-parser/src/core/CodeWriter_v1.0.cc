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

void CodeWriter_v1_0::PutKeyword(lex::Keyword kw) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)kw;
}

void CodeWriter_v1_0::PutOperator(lex::Operator op) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)op;
}

void CodeWriter_v1_0::PutPunctor(lex::Punctor punc) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)punc;
}

void CodeWriter_v1_0::PutIdentifier(std::string_view name) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)name;
}

void CodeWriter_v1_0::PutInteger(std::string_view num) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)num;
}

void CodeWriter_v1_0::PutFloat(std::string_view num) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)num;
}

void CodeWriter_v1_0::PutString(std::string_view str) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)str;
}

void CodeWriter_v1_0::PutCharacter(std::string_view ch) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)ch;
}

void CodeWriter_v1_0::PutMacroBlock(std::string_view macro) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)macro;
}

void CodeWriter_v1_0::PutMacroCall(std::string_view macro) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)macro;
}

void CodeWriter_v1_0::PutComment(std::string_view note) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)note;
}

///=============================================================================

void CodeWriter_v1_0::PutTypeStuff(const FlowPtr<Type>& n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Base>) {}

void CodeWriter_v1_0::Visit(FlowPtr<ExprStmt> n) {
  n->GetExpr()->Accept(*this);
  PutPunctor(PuncSemi);
}

void CodeWriter_v1_0::Visit(FlowPtr<StmtExpr> n) {
  PutPunctor(PuncLPar);
  n->GetStmt()->Accept(*this);
  PutPunctor(PuncRPar);
}

void CodeWriter_v1_0::Visit(FlowPtr<TypeExpr> n) {
  PutPunctor(PuncLPar);
  n->GetType()->Accept(*this);
  PutPunctor(PuncRPar);
}

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
  PutPunctor(lex::PuncColn);
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

void CodeWriter_v1_0::Visit(FlowPtr<Identifier> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Sequence> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Block> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
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
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<While> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<For> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Foreach> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Break> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Continue> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Return> n) {
  PutKeyword(lex::Return);
  if (n->GetValue()) {
    n->GetValue().value()->Accept(*this);
  }
  PutPunctor(PuncSemi);
}

void CodeWriter_v1_0::Visit(FlowPtr<ReturnIf> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
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
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
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

CodeWriter_v1_0::CodeWriter_v1_0(std::ostream& os) : m_os(os) { (void)m_os; }
