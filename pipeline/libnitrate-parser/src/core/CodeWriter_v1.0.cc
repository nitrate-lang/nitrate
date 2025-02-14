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

using namespace ncc::parse;
using namespace ncc::parse::detail;

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

void CodeWriter_v1_0::Visit(FlowPtr<Base> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<ExprStmt> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<StmtExpr> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<TypeExpr> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<NamedTy> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<InferTy> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<TemplateType> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<U1> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<U8> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<U16> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<U32> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<U64> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<U128> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<I8> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<I16> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<I32> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<I64> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<I128> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<F16> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<F32> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<F64> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<F128> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<VoidTy> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<PtrTy> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<OpaqueTy> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<TupleTy> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<ArrayTy> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<RefTy> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<FuncTy> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Unary> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Binary> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<PostUnary> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Ternary> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Integer> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Float> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Boolean> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<String> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Character> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Null> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

void CodeWriter_v1_0::Visit(FlowPtr<Undefined> n) {
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
}

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
  /// TODO: Implement code writer
  qcore_implement();
  (void)n;
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

CodeWriter_v1_0::CodeWriter_v1_0(std::ostream &os) : m_os(os) { (void)m_os; }
