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

#include <parse/nodes/EnumNode.h>
#include <parse/nodes/FunctionNode.h>
#include <parse/nodes/GroupNode.h>
#include <parse/nodes/StructNode.h>
#include <parse/nodes/UnionNode.h>
#include <parse/nodes/TypedefNode.h>
#include <parse/nodes/VariableNode.h>

std::unique_ptr<libquixcc::StmtNode> libquixcc::EnumDefNode::reduce() const
{
    return nullptr;
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::FunctionDeclNode::reduce() const
{
    return std::make_unique<libquixcc::FunctionDeclNode>(*this);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::FunctionDefNode::reduce() const
{
    std::shared_ptr<StmtNode> block = m_body->reduce();

    return std::make_unique<libquixcc::FunctionDefNode>(m_decl, std::static_pointer_cast<BlockNode>(block));
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::GroupDefNode::reduce() const
{
    return std::make_unique<libquixcc::GroupDefNode>(*this);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::StructDefNode::reduce() const
{
    return std::make_unique<libquixcc::StructDefNode>(*this);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::UnionDefNode::reduce() const
{
    return std::make_unique<libquixcc::UnionDefNode>(*this);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::TypedefNode::reduce() const
{
    return nullptr;
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::VarDeclNode::reduce() const
{
    return std::make_unique<libquixcc::VarDeclNode>(*this);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::LetDeclNode::reduce() const
{
    return std::make_unique<libquixcc::LetDeclNode>(*this);
}