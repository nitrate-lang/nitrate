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

#include <core/Hash.hh>
#include <cstring>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>
#include <sstream>

using namespace ncc;
using namespace ncc::parse;

NCC_EXPORT thread_local std::unique_ptr<ncc::IMemory> parse::npar_allocator =
    std::make_unique<ncc::DynamicArena>();

NCC_EXPORT ASTExtension parse::ExtensionDataStore;

ASTExtensionKey ASTExtension::Add(lex::LocationID begin, lex::LocationID end) {
  bool sync = EnableSync;

  if (sync) {
    m_mutex.lock();
  }

  m_pairs.push_back({begin, end});

  auto r = ASTExtensionKey(m_pairs.size() - 1);

  if (sync) {
    m_mutex.unlock();
  }

  return r;
}

const ASTExtensionPackage &ASTExtension::Get(ASTExtensionKey loc) {
  bool sync = EnableSync;

  if (sync) {
    m_mutex.lock();
  }

  const auto &r = m_pairs.at(loc.Key());

  if (sync) {
    m_mutex.unlock();
  }

  return r;
}

void ASTExtension::Set(ASTExtensionKey id, ASTExtensionPackage &&data) {
  bool sync = EnableSync;

  if (sync) {
    m_mutex.lock();
  }

  m_pairs.at(id.Key()) = std::move(data);

  if (sync) {
    m_mutex.unlock();
  }
}

NCC_EXPORT std::ostream &parse::operator<<(std::ostream &os,
                                           const ASTExtensionKey &idx) {
  os << "${L:" << idx.Key() << "}";
  return os;
}

///=============================================================================

NCC_EXPORT std::ostream &Base::dump(std::ostream &os,
                                    WriterSourceProvider rd) const {
  AST_JsonWriter writer(os, rd);
  this->Accept(writer);

  return os;
}

NCC_EXPORT std::string Base::to_json(WriterSourceProvider rd) const {
  std::stringstream ss;
  AST_JsonWriter writer(ss, rd);
  this->Accept(writer);

  return ss.str();
}

NCC_EXPORT bool Base::isSame(FlowPtr<Base> o) const {
  if (this == o.get()) {
    return true;
  }

  if (getKind() != o->getKind()) {
    return false;
  }

  std::stringstream ss1, ss2;
  AST_MsgPackWriter writer1(ss1), writer2(ss2);

  this->Accept(writer1);
  o.Accept(writer2);

  return ss1.str() == ss2.str();
}

NCC_EXPORT uint64_t Base::hash64() const {
  AST_Hash64 visitor;

  this->Accept(visitor);

  return visitor.get();
}

NCC_EXPORT size_t Base::count_children() {
  size_t count = 0;

  for_each(this, [&](auto, auto) { count++; });

  return count;
}

NCC_EXPORT void Base::BindCodeCommentData(
    std::span<const lex::Token> comment_tokens) {
  auto old = ExtensionDataStore.Get(m_data);
  old.add_comments(comment_tokens);
  ExtensionDataStore.Set(m_data, std::move(old));
}

///=============================================================================

NCC_EXPORT bool Type::is_ptr_to(const Type *type) const {
  if (!is_pointer()) {
    return false;
  }

  auto item = as<PtrTy>()->get_item();
  while (item->is<RefTy>()) {
    item = item->as<RefTy>()->get_item();
  }

  return item->is(type->getKind());
}

FlowPtr<Stmt> Parser::mock_stmt(std::optional<npar_ty_t>) {
  auto node = make<Stmt>(QAST_BASE)();
  node->SetOffset(rd.Current().get_start());

  return node;
}

FlowPtr<Expr> Parser::mock_expr(std::optional<npar_ty_t>) {
  auto node = make<Expr>(QAST_BASE)();
  node->SetOffset(rd.Current().get_start());

  return node;
}

FlowPtr<Type> Parser::mock_type() {
  auto node = make<Type>(QAST_BASE)();
  node->SetOffset(rd.Current().get_start());

  return node;
}
