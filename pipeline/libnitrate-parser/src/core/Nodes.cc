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
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTData.hh>
#include <nitrate-parser/ASTFactory.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Algorithm.hh>
#include <sstream>

using namespace ncc;
using namespace ncc::parse;

struct ASTExtensionPackage {
  std::vector<string> m_comments;
  lex::LocationID m_source_begin;
  lex::LocationID m_source_end;
  size_t m_parenthesis_depth = 0;
};

static std::vector<ASTExtensionPackage> GExtensionPackages;
static std::mutex GExtensionPackagesLock;

void ASTExtension::ResetStorage() {
  SmartLock lock(GExtensionPackagesLock);
  GExtensionPackages.clear();
}

void ASTExtension::LazyInitialize() {
  if (IsNull()) {
    m_key = GExtensionPackages.size();
    GExtensionPackages.emplace_back();
  }
}

void ASTExtension::SetSourceLocationBound(lex::LocationID begin, lex::LocationID end) {
  if (!begin.HasValue() && !end.HasValue()) {
    return;
  }

  LazyInitialize();

  SmartLock lock(GExtensionPackagesLock);

  auto &pkg = GExtensionPackages.at(m_key);
  pkg.m_source_begin = begin;
  pkg.m_source_end = end;
}

void ASTExtension::SetComments(std::span<const string> comments) {
  if (comments.empty()) {
    return;
  }

  LazyInitialize();

  SmartLock lock(GExtensionPackagesLock);

  auto &pkg = GExtensionPackages.at(m_key);
  pkg.m_comments.clear();
  pkg.m_comments.insert(pkg.m_comments.end(), comments.begin(), comments.end());
}

auto ASTExtension::SetParenthesisDepth(size_t depth) -> void {
  if (depth == 0) {
    return;
  }

  LazyInitialize();

  SmartLock lock(GExtensionPackagesLock);

  auto &pkg = GExtensionPackages.at(m_key);
  pkg.m_parenthesis_depth = depth;
}

auto ASTExtension::GetSourceLocationBound() const -> std::pair<lex::LocationID, lex::LocationID> {
  if (IsNull()) {
    return {lex::LocationID(), lex::LocationID()};
  }

  SmartLock lock(GExtensionPackagesLock);
  return {GExtensionPackages.at(m_key).m_source_begin, GExtensionPackages.at(m_key).m_source_end};
}

auto ASTExtension::GetComments() const -> std::span<const string> {
  if (IsNull()) {
    return {};
  }

  SmartLock lock(GExtensionPackagesLock);
  return GExtensionPackages.at(m_key).m_comments;
}

auto ASTExtension::GetParenthesisDepth() const -> size_t {
  if (IsNull()) {
    return 0;
  }

  SmartLock lock(GExtensionPackagesLock);
  return GExtensionPackages.at(m_key).m_parenthesis_depth;
}

NCC_EXPORT auto parse::operator<<(std::ostream &os, const ASTExtension &idx) -> std::ostream & {
  os << "${L:" << idx.Key() << "}";
  return os;
}

NCC_EXPORT std::ostream &parse::operator<<(std::ostream &os, ASTNodeKind kind) {
  os << Expr::GetKindName(kind);
  return os;
}

///=============================================================================

std::string Expr::DebugString(WriterSourceProvider rd) const {
  std::stringstream ss;
  AstWriter writer(ss, true, rd);

  const_cast<Expr *>(this)->Accept(writer);

  return ss.str();
}

void Expr::DebugString(std::ostream &os, WriterSourceProvider rd) const {
  AstWriter writer(os, true, rd);
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

  for_each(this, [&](auto) { count++; });

  return count;
}

void Expr::SetComments(std::span<const string> comments) { m_data.SetComments(comments); }
