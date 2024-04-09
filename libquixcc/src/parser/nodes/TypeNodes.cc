////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (C) 2020-2024 Wesley C. Jones                                ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the GNU C Library; if not, see                        ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <parse/nodes/AllNodes.h>

libquixcc::U8TypeNode *libquixcc::U8TypeNode::m_instance = nullptr;
libquixcc::U16TypeNode *libquixcc::U16TypeNode::m_instance = nullptr;
libquixcc::U32TypeNode *libquixcc::U32TypeNode::m_instance = nullptr;
libquixcc::U64TypeNode *libquixcc::U64TypeNode::m_instance = nullptr;
libquixcc::I8TypeNode *libquixcc::I8TypeNode::m_instance = nullptr;
libquixcc::I16TypeNode *libquixcc::I16TypeNode::m_instance = nullptr;
libquixcc::I32TypeNode *libquixcc::I32TypeNode::m_instance = nullptr;
libquixcc::I64TypeNode *libquixcc::I64TypeNode::m_instance = nullptr;
libquixcc::F32TypeNode *libquixcc::F32TypeNode::m_instance = nullptr;
libquixcc::F64TypeNode *libquixcc::F64TypeNode::m_instance = nullptr;
libquixcc::BoolTypeNode *libquixcc::BoolTypeNode::m_instance = nullptr;
libquixcc::VoidTypeNode *libquixcc::VoidTypeNode::m_instance = nullptr;
std::map<libquixcc::TypeNode *, libquixcc::PointerTypeNode *> libquixcc::PointerTypeNode::m_instances;
libquixcc::StringTypeNode *libquixcc::StringTypeNode::m_instance = nullptr;
std::map<std::vector<libquixcc::TypeNode *>, libquixcc::GroupTypeNode *> libquixcc::GroupTypeNode::m_instances;
std::map<std::vector<libquixcc::TypeNode *>, libquixcc::StructTypeNode *> libquixcc::StructTypeNode::m_instances;
std::map<std::vector<libquixcc::TypeNode *>, libquixcc::UnionTypeNode *> libquixcc::UnionTypeNode::m_instances;
std::map<std::pair<libquixcc::TypeNode *, std::shared_ptr<libquixcc::ConstExprNode>>, libquixcc::ArrayTypeNode *> libquixcc::ArrayTypeNode::m_instances;
std::unordered_map<std::string, libquixcc::UserTypeNode *> libquixcc::UserTypeNode::m_instances;
std::map<libquixcc::FunctionTypeNode::Inner, libquixcc::FunctionTypeNode *> libquixcc::FunctionTypeNode::s_instances;
std::map<std::pair<std::string, libquixcc::TypeNode *>, libquixcc::EnumTypeNode *> libquixcc::EnumTypeNode::m_instances;