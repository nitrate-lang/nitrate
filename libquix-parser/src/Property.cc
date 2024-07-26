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

#include "LibMacro.h"
#include "Nodes.h"
#include <quixcc/interface/SyntaxTreeNodes.h>

LIB_EXPORT quixcc_ast_ntype_t quixcc_ast_typeof(const quixcc_ast_node_t *node) {
  if (node == nullptr) {
    quixcc_panic("Contract violation: quixcc_ast_typeof() called with nullptr");
  }

  return static_cast<const libquixcc::qast::Node *>(node)->this_typeid();
}

LIB_EXPORT size_t quixcc_ast_sizeof(const quixcc_ast_node_t *node) {
  if (node == nullptr) {
    quixcc_panic("Contract violation: quixcc_ast_sizeof() called with nullptr");
  }

  return static_cast<const libquixcc::qast::Node *>(node)->this_sizeof();
}

LIB_EXPORT const char *quixcc_ast_ntype_name(quixcc_ast_ntype_t type) {
  return libquixcc::qast::Node::type_name(type);
}

LIB_EXPORT uint16_t quixcc_ast_subcount(quixcc_ast_ntype_t type) {
  /// TODO: Implement this function

  quixcc_panic("quixcc_ast_subcount() is not implemented");
}

LIB_EXPORT quixcc_ast_ftype_t quixcc_ast_field_type(quixcc_ast_node_t *node, uint16_t index) {
  /// TODO: Implement this function

  quixcc_panic("quixcc_ast_field_type() is not implemented");
}

LIB_EXPORT const char *quixcc_ast_field_name(quixcc_ast_node_t *node, uint16_t index) {
  /// TODO: Implement this function

  quixcc_panic("quixcc_ast_field_name() is not implemented");
}

LIB_EXPORT uint16_t quixcc_ast_field_index(quixcc_ast_node_t *node, const char *name) {
  /// TODO: Implement this function

  quixcc_panic("quixcc_ast_field_index() is not implemented");
}

LIB_EXPORT quixcc_ast_node_t **quixcc_ast_get_child(quixcc_ast_node_t *node, uint16_t index) {
  /// TODO: Implement this function

  quixcc_panic("quixcc_ast_get_child() is not implemented");
}

LIB_EXPORT void *quixcc_ast_get(const quixcc_ast_node_t *node, uint16_t index,
                                quixcc_ast_ftype_t ftype) {
  /// TODO: Implement this function

  quixcc_panic("quixcc_ast_get() is not implemented");
}
