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
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
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
libquixcc::CharTypeNode *libquixcc::CharTypeNode::m_instance = nullptr;
libquixcc::VoidTypeNode *libquixcc::VoidTypeNode::m_instance = nullptr;
libquixcc::StringTypeNode *libquixcc::StringTypeNode::m_instance = nullptr;
std::map<std::vector<libquixcc::TypeNode *>, libquixcc::StructTypeNode *> libquixcc::StructTypeNode::m_instances;
std::map<std::vector<libquixcc::TypeNode *>, libquixcc::UnionTypeNode *> libquixcc::UnionTypeNode::m_instances;
std::map<std::pair<libquixcc::TypeNode *, std::shared_ptr<libquixcc::ConstExprNode>>, libquixcc::ArrayTypeNode *> libquixcc::ArrayTypeNode::m_instances;
std::unordered_map<std::string, libquixcc::UserTypeNode *> libquixcc::UserTypeNode::m_instances;