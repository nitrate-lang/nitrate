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

#include <google/protobuf/message.h>

#include <algorithm>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>

#include "core/SyntaxTree.pb.h"
#include "nitrate-core/OldLogger.hh"

using namespace ncc;
using namespace google::protobuf;
using namespace nitrate::parser;
using namespace nitrate::parser::SyntaxTree;

static void WriteKindNode(const FlowPtr<parse::Base> &in, Base &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindBinexpr(const FlowPtr<parse::BinaryExpression> &in,
                             BinaryExpression &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindUnexpr(const FlowPtr<parse::UnaryExpression> &in,
                            UnaryExpression &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindTerexpr(const FlowPtr<parse::TernaryExpression> &in,
                             TernaryExpression &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindInt(const FlowPtr<parse::Integer> &in, Integer &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindFloat(const FlowPtr<parse::Float> &in, Float &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindString(const FlowPtr<parse::String> &in,
                            SyntaxTree::String &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindChar(const FlowPtr<parse::Character> &in, Character &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindBool(const FlowPtr<parse::Boolean> &in, Boolean &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindNull(const FlowPtr<parse::Null> &in, Null &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindUndef(const FlowPtr<parse::Undefined> &in,
                           Undefined &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindCall(const FlowPtr<parse::Call> &in, Call &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindList(const FlowPtr<parse::List> &in, List &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindAssoc(const FlowPtr<parse::Assoc> &in, Assoc &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindIndex(const FlowPtr<parse::Index> &in, Index &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindSlice(const FlowPtr<parse::Slice> &in, Slice &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindFstring(const FlowPtr<parse::FString> &in, FString &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindIdentifier(const FlowPtr<parse::Identifier> &in,
                                Identifier &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindSequence(const FlowPtr<parse::Sequence> &in,
                              Sequence &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindPostUnexpr(const FlowPtr<parse::PostUnaryExpression> &in,
                                PostUnaryExpression &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindStmtExpr(const FlowPtr<parse::StmtExpr> &in,
                              StmtExpr &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindTypeExpr(const FlowPtr<parse::TypeExpr> &in,
                              TypeExpr &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindTemplateCall(const FlowPtr<parse::TemplateCall> &in,
                                  TemplateCall &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindRef(const FlowPtr<parse::RefTy> &in, RefTy &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindU1(const FlowPtr<parse::U1> &in, U1 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindU8(const FlowPtr<parse::U8> &in, U8 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindU16(const FlowPtr<parse::U16> &in, U16 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindU32(const FlowPtr<parse::U32> &in, U32 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindU64(const FlowPtr<parse::U64> &in, U64 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindU128(const FlowPtr<parse::U128> &in, U128 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindI8(const FlowPtr<parse::I8> &in, I8 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindI16(const FlowPtr<parse::I16> &in, I16 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindI32(const FlowPtr<parse::I32> &in, I32 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindI64(const FlowPtr<parse::I64> &in, I64 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindI128(const FlowPtr<parse::I128> &in, I128 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindF16(const FlowPtr<parse::F16> &in, F16 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindF32(const FlowPtr<parse::F32> &in, F32 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindF64(const FlowPtr<parse::F64> &in, F64 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindF128(const FlowPtr<parse::F128> &in, F128 &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindVoid(const FlowPtr<parse::VoidTy> &in, VoidTy &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindPtr(const FlowPtr<parse::PtrTy> &in, PtrTy &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindOpaque(const FlowPtr<parse::OpaqueTy> &in, OpaqueTy &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindArray(const FlowPtr<parse::ArrayTy> &in, ArrayTy &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindTuple(const FlowPtr<parse::TupleTy> &in, TupleTy &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindFuncTy(const FlowPtr<parse::FuncTy> &in, FuncTy &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindUnres(const FlowPtr<parse::NamedTy> &in, NamedTy &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindInfer(const FlowPtr<parse::InferTy> &in, InferTy &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindTempl(const FlowPtr<parse::TemplateType> &in,
                           TemplateType &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindTypedef(const FlowPtr<parse::Typedef> &in, Typedef &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindStruct(const FlowPtr<parse::Struct> &in, Struct &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindEnum(const FlowPtr<parse::Enum> &in, Enum &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindFunction(const FlowPtr<parse::Function> &in,
                              Function &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindScope(const FlowPtr<parse::Scope> &in, Scope &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindExport(const FlowPtr<parse::Export> &in, Export &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindBlock(const FlowPtr<parse::Block> &in, Block &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindLet(const FlowPtr<parse::Variable> &in, Variable &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindAssembly(const FlowPtr<parse::Assembly> &in,
                              Assembly &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindReturn(const FlowPtr<parse::Return> &in, Return &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindRetif(const FlowPtr<parse::ReturnIf> &in, ReturnIf &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindBreak(const FlowPtr<parse::Break> &in, Break &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindContinue(const FlowPtr<parse::Continue> &in,
                              Continue &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindIf(const FlowPtr<parse::If> &in, If &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindWhile(const FlowPtr<parse::While> &in, While &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindFor(const FlowPtr<parse::For> &in, For &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindForeach(const FlowPtr<parse::Foreach> &in, Foreach &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindCase(const FlowPtr<parse::Case> &in, Case &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindSwitch(const FlowPtr<parse::Switch> &in, Switch &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

static void WriteKindExprStmt(const FlowPtr<parse::ExprStmt> &in,
                              ExprStmt &out) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)in;
  (void)out;
}

using namespace ncc::parse;

AstWriter::~AstWriter() = default;

void AstWriter::Visit(FlowPtr<Base> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<ExprStmt> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;

  SyntaxTree::ExprStmt node;
  WriteKindExprStmt(n, node);
}

void AstWriter::Visit(FlowPtr<StmtExpr> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<TypeExpr> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<NamedTy> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<InferTy> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<TemplateType> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<U1> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<U8> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<U16> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<U32> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<U64> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<U128> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<I8> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<I16> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<I32> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<I64> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<I128> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<F16> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<F32> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<F64> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<F128> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<VoidTy> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<PtrTy> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<OpaqueTy> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<TupleTy> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<ArrayTy> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<RefTy> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<FuncTy> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<UnaryExpression> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<BinaryExpression> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<PostUnaryExpression> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<TernaryExpression> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Integer> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Float> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Boolean> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<String> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Character> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Null> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Undefined> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Call> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<TemplateCall> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<List> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Assoc> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Index> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Slice> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<FString> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Identifier> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Sequence> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Block> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Variable> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Assembly> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<If> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<While> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<For> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Foreach> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Break> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Continue> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Return> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<ReturnIf> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Case> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Switch> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Typedef> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Function> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Struct> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Enum> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Scope> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}

void AstWriter::Visit(FlowPtr<Export> n) {
  /// TODO: Implement protocol buffer serialization
  qcore_implement();
  (void)n;
}
