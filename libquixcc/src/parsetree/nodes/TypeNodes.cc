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

#define QUIXCC_INTERNAL

#include <parsetree/nodes/AllNodes.h>

using namespace libquixcc;

std::map<TypeNode *, MutTypeNode *> MutTypeNode::m_instances;
U8TypeNode *U8TypeNode::m_instance = nullptr;
U16TypeNode *U16TypeNode::m_instance = nullptr;
U32TypeNode *U32TypeNode::m_instance = nullptr;
U64TypeNode *U64TypeNode::m_instance = nullptr;
U128TypeNode *U128TypeNode::m_instance = nullptr;
I8TypeNode *I8TypeNode::m_instance = nullptr;
I16TypeNode *I16TypeNode::m_instance = nullptr;
I32TypeNode *I32TypeNode::m_instance = nullptr;
I64TypeNode *I64TypeNode::m_instance = nullptr;
I128TypeNode *I128TypeNode::m_instance = nullptr;
F32TypeNode *F32TypeNode::m_instance = nullptr;
F64TypeNode *F64TypeNode::m_instance = nullptr;
BoolTypeNode *BoolTypeNode::m_instance = nullptr;
VoidTypeNode *VoidTypeNode::m_instance = nullptr;
NullTypeNode *NullTypeNode::m_instance = nullptr;
std::map<TypeNode *, PointerTypeNode *> PointerTypeNode::m_instances;
std::map<std::string, OpaqueTypeNode *> OpaqueTypeNode::m_instances;
StringTypeNode *StringTypeNode::m_instance = nullptr;
std::map<std::pair<std::vector<TypeNode *>, std::string>, StructTypeNode *>
    StructTypeNode::m_instances;
std::map<std::pair<std::vector<TypeNode *>, std::string>, GroupTypeNode *>
    GroupTypeNode::m_instances;
std::map<std::pair<std::vector<TypeNode *>, std::string>, RegionTypeNode *>
    RegionTypeNode::m_instances;
std::map<std::pair<std::vector<TypeNode *>, std::string>, UnionTypeNode *>
    UnionTypeNode::m_instances;
thread_local std::map<std::pair<TypeNode *, std::shared_ptr<ConstExprNode>>,
                      ArrayTypeNode *>
    ArrayTypeNode::m_instances;
thread_local std::map<TypeNode *, VectorTypeNode *> VectorTypeNode::m_instances;
thread_local std::map<TypeNode *, ResultTypeNode *> ResultTypeNode::m_instances;
thread_local std::map<TypeNode *, GeneratorTypeNode *>
    GeneratorTypeNode::m_instances;
thread_local std::unordered_map<std::string, std::shared_ptr<UserTypeNode>>
    UserTypeNode::m_instances;
std::map<FunctionTypeNode::Inner, FunctionTypeNode *>
    FunctionTypeNode::s_instances;
std::map<std::pair<std::string, TypeNode *>, EnumTypeNode *>
    EnumTypeNode::m_instances;
