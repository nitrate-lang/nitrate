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
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
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