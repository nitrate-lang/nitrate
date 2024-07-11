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

#include <core/Macro.h>
#include <qast/Nodes.h>
#include <quixcc/Library.h>
#include <quixcc/interface/SyntaxTreeNodes.h>

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

using namespace libquixcc::qast;

size_t Node::children_count(bool recursive) const {
  /// TODO: Implement this function
  quixcc_panic("Node::children_count() is not implemented");
}

bool Node::inherits_from(quixcc_ast_ntype_t type) const {
  /// TODO: Implement this function
  quixcc_panic("Node::inherits_from() is not implemented");
}

bool Node::is_type() const { return inherits_from(QUIXCC_AST_NODE_TYPE); }

bool Node::is_stmt() const { return inherits_from(QUIXCC_AST_NODE_STMT); }

bool Node::is_decl() const { return inherits_from(QUIXCC_AST_NODE_DECL); }

bool Node::is_expr() const { return inherits_from(QUIXCC_AST_NODE_EXPR); }

bool Node::is_const_expr() const { return is_expr() && as<Expr>()->is_const(); }

bool Node::is(quixcc_ast_ntype_t type) const { return type == type_of(); }

bool Node::verify(std::ostream &os) const { return verify_impl(os); }

void Node::canonicalize() { canonicalize_impl(); }

void Node::dump(std::ostream &os, bool isForDebug) const {
  dump_impl(os, isForDebug);
}

///=============================================================================

size_t Type::infer_size_bits() const { return infer_size_bits_impl(); }

size_t Type::infer_size() const { return std::ceil(infer_size_bits() / 8.0); }

bool Type::is_primitive() const { quixcc_panic(""); }
bool Type::is_array() const { quixcc_panic(""); }
bool Type::is_vector() const { quixcc_panic(""); }
bool Type::is_tuple() const { quixcc_panic(""); }
bool Type::is_set() const { quixcc_panic(""); }
bool Type::is_map() const { quixcc_panic(""); }
bool Type::is_pointer() const { quixcc_panic(""); }
bool Type::is_function() const { quixcc_panic(""); }
bool Type::is_composite() const { quixcc_panic(""); }
bool Type::is_union() const { quixcc_panic(""); }
bool Type::is_numeric() const { quixcc_panic(""); }
bool Type::is_integral() const { quixcc_panic(""); }
bool Type::is_floating_point() const { quixcc_panic(""); }
bool Type::is_signed() const { quixcc_panic(""); }
bool Type::is_unsigned() const { quixcc_panic(""); }
bool Type::is_void() const { quixcc_panic(""); }
bool Type::is_bool() const { quixcc_panic(""); }
bool Type::is_mutable() const { quixcc_panic(""); }
bool Type::is_const() const { quixcc_panic(""); }
bool Type::is_volatile() const { quixcc_panic(""); }
bool Type::is_ptr_to(const Type *type) const { quixcc_panic(""); }
bool Type::is_mut_ptr_to(const Type *type) const { quixcc_panic(""); }
bool Type::is_const_ptr_to(const Type *type) const { quixcc_panic(""); }
bool Type::is_string() const { quixcc_panic(""); }

///=============================================================================

bool Decl::decl_has_name() const { return get_name_impl().has_value(); }
std::string Decl::get_decl_name() const { return get_name_impl().value(); }
const std::set<std::string> &Decl::get_tags() const { return m_tags; }
void Decl::add_tag(std::string_view tag) { m_tags.insert(tag.data()); }

void Decl::add_tags(std::initializer_list<std::string_view> tags) {
  for (auto tag : tags) {
    add_tag(tag);
  }
}

void Decl::clear_tags() { m_tags.clear(); }

void Decl::remove_tag(std::string_view tag) {
  std::erase_if(m_tags, [tag](const std::string &t) { return t == tag; });
}

bool Decl::has_type() const { return infer_type() != nullptr; }

///=============================================================================

bool ConstExpr::is_const() const { return true; }
bool ConstExpr::is_stochastic() const { return false; }

///=============================================================================

LIB_EXPORT quixcc_ast_node_t *quixcc_ast_alloc(quixcc_ast_ntype_t type,
                                               quixcc_arena_t *arena) {
  quixcc_panic("quixcc_ast_alloc() is not implemented");
}

LIB_EXPORT void quixcc_ast_done(quixcc_ast_node_t *node) {
  quixcc_panic("quixcc_ast_done() is not implemented");
}

///=============================================================================
///=============================================================================

void test() {
  using namespace libquixcc::qast;

  Expr *lit = new IntLitExpr(0xffffffffffffffff);

  if (lit->is_const()) {
    static_cast<ConstExpr *>(lit)->eval_as<std::tuple<bool, int, unsigned>>();
  }
}