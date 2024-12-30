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
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>
#include <sstream>

#include "nitrate-parser/ASTBase.hh"

using namespace ncc;
using namespace ncc::parse;

CPP_EXPORT thread_local std::unique_ptr<ncc::IMemory> parse::npar_allocator =
    std::make_unique<ncc::dyn_arena>();

CPP_EXPORT LocationPairAlias parse::g_location_pairs;

LocationPairAlias::Index LocationPairAlias::Add(lex::LocationID begin,
                                                lex::LocationID end) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_pairs.emplace_back(begin, end);

  return {.v = m_pairs.size() - 1};
}

std::pair<lex::LocationID, lex::LocationID> LocationPairAlias::Get(
    LocationPairAlias::Index loc) {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_pairs.at(loc.v);
}

///=============================================================================

CPP_EXPORT bool Base::isSame(FlowPtr<Base> o) const {
  if (this == o.get()) {
    return true;
  }

  if (getKind() != o->getKind()) {
    return false;
  }

  std::stringstream ss1, ss2;
  AST_MsgPackWriter writer1(ss1), writer2(ss2);

  this->accept(writer1);
  o.accept(writer2);

  return ss1.str() == ss2.str();
}

CPP_EXPORT uint64_t Base::hash64() const {
  AST_Hash64 visitor;

  this->accept(visitor);

  return visitor.get();
}

CPP_EXPORT size_t Base::count_children() {
  size_t count = 0;

  for_each(this, [&](auto, auto) { count++; });

  return count;
}

///=============================================================================

CPP_EXPORT bool Type::is_ptr_to(Type *type) const {
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
  node->set_offset(rd.Current()->get_start());

  return node;
}

FlowPtr<Expr> Parser::mock_expr(std::optional<npar_ty_t>) {
  auto node = make<Expr>(QAST_BASE)();
  node->set_offset(rd.Current()->get_start());

  return node;
}

FlowPtr<Type> Parser::mock_type() {
  auto node = make<Type>(QAST_BASE)();
  node->set_offset(rd.Current()->get_start());

  return node;
}
