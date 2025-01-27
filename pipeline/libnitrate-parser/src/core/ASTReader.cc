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

#include <core/SyntaxTree.pb.h>

#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTReader.hh>

using namespace ncc;
using namespace ncc::parse;
using namespace nitrate::parser;

auto AstReader::Unmarshal(const SyntaxTree::Expr &in) -> Result<Expr> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Root &in) -> Result<Base> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Stmt &in) -> Result<Stmt> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Type &in) -> Result<Type> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Base &in) -> Result<Base> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::ExprStmt &in) -> Result<ExprStmt> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::StmtExpr &in) -> Result<StmtExpr> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::TypeExpr &in) -> Result<TypeExpr> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::NamedTy &in) -> Result<NamedTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::InferTy &in) -> Result<InferTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::TemplateType &in)
    -> Result<TemplateType> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::U1 &in) -> Result<U1> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::U8 &in) -> Result<U8> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::U16 &in) -> Result<U16> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::U32 &in) -> Result<U32> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::U64 &in) -> Result<U64> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::U128 &in) -> Result<U128> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::I8 &in) -> Result<I8> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::I16 &in) -> Result<I16> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::I32 &in) -> Result<I32> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::I64 &in) -> Result<I64> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::I128 &in) -> Result<I128> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::F16 &in) -> Result<F16> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::F32 &in) -> Result<F32> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::F64 &in) -> Result<F64> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::F128 &in) -> Result<F128> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::VoidTy &in) -> Result<VoidTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::PtrTy &in) -> Result<PtrTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::OpaqueTy &in) -> Result<OpaqueTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::TupleTy &in) -> Result<TupleTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::ArrayTy &in) -> Result<ArrayTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::RefTy &in) -> Result<RefTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::FuncTy &in) -> Result<FuncTy> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Unary &in) -> Result<Unary> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Binary &in) -> Result<Binary> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::PostUnary &in)
    -> Result<PostUnary> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Ternary &in) -> Result<Ternary> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Integer &in) -> Result<Integer> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Float &in) -> Result<Float> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Boolean &in) -> Result<Boolean> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::String &in) -> Result<String> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Character &in)
    -> Result<Character> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Null &in) -> Result<Null> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Undefined &in)
    -> Result<Undefined> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Call &in) -> Result<Call> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::TemplateCall &in)
    -> Result<TemplateCall> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::List &in) -> Result<List> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Assoc &in) -> Result<Assoc> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Index &in) -> Result<Index> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Slice &in) -> Result<Slice> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::FString &in) -> Result<FString> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Identifier &in)
    -> Result<Identifier> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Sequence &in) -> Result<Sequence> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Block &in) -> Result<Block> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Variable &in) -> Result<Variable> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Assembly &in) -> Result<Assembly> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::If &in) -> Result<If> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::While &in) -> Result<While> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::For &in) -> Result<For> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Foreach &in) -> Result<Foreach> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Break &in) -> Result<Break> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Continue &in) -> Result<Continue> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Return &in) -> Result<Return> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::ReturnIf &in) -> Result<ReturnIf> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Case &in) -> Result<Case> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Switch &in) -> Result<Switch> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Typedef &in) -> Result<Typedef> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Function &in) -> Result<Function> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Struct &in) -> Result<Struct> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Enum &in) -> Result<Enum> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Scope &in) -> Result<Scope> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

auto AstReader::Unmarshal(const SyntaxTree::Export &in) -> Result<Export> {
  /// TODO: Unmarshal the protobuf object
  qcore_implement();
  (void)in;
}

AstReader::AstReader(std::istream &protobuf_data,
                     ReaderSourceManager source_manager)
    : m_rd(source_manager), m_mm(std::make_unique<DynamicArena>()) {
  SyntaxTree::Root root;
  if (!root.ParseFromIstream(&protobuf_data)) [[unlikely]] {
    return;
  }

  std::swap(NparAllocator, m_mm);
  m_root = Unmarshal(root);
  std::swap(NparAllocator, m_mm);
}

AstReader::AstReader(std::string_view protobuf_data,
                     ReaderSourceManager source_manager)
    : m_rd(source_manager) {
  SyntaxTree::Root root;
  if (!root.ParseFromArray(protobuf_data.data(), protobuf_data.size()))
      [[unlikely]] {
    return;
  }

  std::swap(NparAllocator, m_mm);
  m_root = Unmarshal(root);
  std::swap(NparAllocator, m_mm);
}

auto AstReader::Get() -> std::optional<ASTRoot> {
  if (!m_root.has_value()) [[unlikely]] {
    return std::nullopt;
  }

  return ASTRoot(m_root.value(), std::move(m_mm), false);
}
