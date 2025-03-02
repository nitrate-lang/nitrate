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

#include <cstring>
#include <descent/Recurse.hh>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/SmartLock.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTData.hh>
#include <nitrate-parser/ASTFactory.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Algorithm.hh>
#include <optional>
#include <sstream>

using namespace ncc;
using namespace ncc::parse;

NCC_EXPORT ASTExtension parse::ExtensionDataStore;

auto ASTExtension::Add(lex::LocationID begin, lex::LocationID end) -> ASTExtensionKey {
  SmartLock lock(m_mutex);

  m_pairs.push_back({begin, end});

  return {m_pairs.size() - 1};
}

auto ASTExtension::Get(ASTExtensionKey loc) -> const ASTExtensionPackage & {
  SmartLock lock(m_mutex);

  return m_pairs.at(loc.Key());
}

void ASTExtension::Set(ASTExtensionKey id, ASTExtensionPackage &&data) {
  SmartLock lock(m_mutex);

  m_pairs.at(id.Key()) = std::move(data);
}

NCC_EXPORT auto parse::operator<<(std::ostream &os, const ASTExtensionKey &idx) -> std::ostream & {
  os << "${L:" << idx.Key() << "}";
  return os;
}

///=============================================================================

std::string Expr::DebugString(WriterSourceProvider rd) const {
  std::stringstream ss;
  AstWriter writer(ss, rd, true);

  const_cast<Expr *>(this)->Accept(writer);

  return ss.str();
}

void Expr::DebugString(std::ostream &os, WriterSourceProvider rd) const {
  AstWriter writer(os, rd, true);
  const_cast<Expr *>(this)->Accept(writer);
}

void Expr::Serialize(std::ostream &os) const {
  AstWriter writer(os);
  const_cast<Expr *>(this)->Accept(writer);
}

std::string Expr::Serialize() const {
  std::stringstream ss;
  AstWriter writer(ss);
  const_cast<Expr *>(this)->Accept(writer);

  return ss.str();
}

auto Expr::IsEq(FlowPtr<Expr> o) const -> bool {
  if (this == o.get()) {
    return true;
  }

  if (GetKind() != o->GetKind()) {
    return false;
  }

  std::stringstream ss1;
  std::stringstream ss2;
  AstWriter writer1(ss1);
  AstWriter writer2(ss2);

  const_cast<Expr *>(this)->Accept(writer1);
  o.Accept(writer2);

  return ss1.str() == ss2.str();
}

auto Expr::Hash64() const -> uint64_t {
  std::stringstream ss;
  AstWriter writer(ss);
  const_cast<Expr *>(this)->Accept(writer);

  return std::hash<std::string>{}(ss.str());
}

auto Expr::RecursiveChildCount() -> size_t {
  size_t count = 0;

  for_each(this, [&](auto, auto) { count++; });

  return count;
}

void Expr::SetComments(std::span<const string> comments) {
  auto old = ExtensionDataStore.Get(m_data);
  old.AddComments(comments);
  ExtensionDataStore.Set(m_data, std::move(old));
}
