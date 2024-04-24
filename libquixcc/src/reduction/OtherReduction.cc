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
#include <parse/nodes/RegionNode.h>
#include <parse/nodes/UnionNode.h>
#include <parse/nodes/TypedefNode.h>
#include <parse/nodes/VariableNode.h>
#include <parse/nodes/InlineAsmNode.h>

std::unique_ptr<libquixcc::StmtNode> libquixcc::EnumDefNode::reduce(libquixcc::ReductionState &state) const
{
    return nullptr;
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::FunctionDeclNode::reduce(libquixcc::ReductionState &state) const
{
    // if not in a function definition or export block don't gen func decl
    // LLVM IR does not require function declarations for internal linkage

    if (!state.m_export && !state.m_fn_def)
        return nullptr;

    std::unique_ptr<FunctionDeclNode> decl = std::make_unique<FunctionDeclNode>();

    decl->m_name = m_name;
    decl->m_type = m_type;

    for (auto &param : m_params)
    {
        std::shared_ptr<StmtNode> shared = param->reduce(state);
        decl->m_params.push_back(std::static_pointer_cast<FunctionParamNode>(shared));
    }

    return decl;
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::FunctionParamNode::reduce(libquixcc::ReductionState &state) const
{
    return std::make_unique<libquixcc::FunctionParamNode>(*this);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::FunctionDefNode::reduce(libquixcc::ReductionState &state) const
{
    bool x = state.m_fn_def;
    state.m_fn_def = true;

    std::shared_ptr<StmtNode> block = m_body->reduce(state);
    std::shared_ptr<ParseNode> decl = m_decl->reduce(state);

    auto ret = std::make_unique<libquixcc::FunctionDefNode>(std::static_pointer_cast<FunctionDeclNode>(decl), std::static_pointer_cast<BlockNode>(block));

    state.m_fn_def = x;

    return ret;
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::GroupDefNode::reduce(libquixcc::ReductionState &state) const
{
    std::vector<std::shared_ptr<libquixcc::StructFieldNode>> fields;

    for (auto &field : get_fields())
    {
        auto copy = std::make_shared<libquixcc::StructFieldNode>(field->m_name, field->m_type, field->m_value->reduce<ConstExprNode>());
        fields.push_back(copy);
    }

    return std::make_unique<libquixcc::StructDefNode>(m_name, fields);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::RegionDefNode::reduce(libquixcc::ReductionState &state) const
{
    std::vector<std::shared_ptr<libquixcc::RegionFieldNode>> fields;

    for (auto &field : m_fields)
    {
        auto copy = std::make_shared<libquixcc::RegionFieldNode>(field->m_name, field->m_type, field->m_value->reduce<ConstExprNode>());
        fields.push_back(copy);
    }

    return std::make_unique<libquixcc::RegionDefNode>(m_name, fields);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::StructDefNode::reduce(libquixcc::ReductionState &state) const
{
    std::unique_ptr<StmtGroupNode> group = std::make_unique<StmtGroupNode>();

    group->m_stmts.push_back(std::make_unique<StructDefNode>(m_name, m_fields));

    for (auto &_method : m_methods)
    {
        std::shared_ptr<StmtNode> method = _method->reduce(state);

        if (method->is<FunctionDeclNode>())
        {
            auto fdecl = std::static_pointer_cast<FunctionDeclNode>(method);
            fdecl->m_name = m_name + "::" + fdecl->m_name;
        }
        else
        {
            auto fdef = std::static_pointer_cast<FunctionDefNode>(method);
            fdef->m_decl->m_name = m_name + "::" + fdef->m_decl->m_name;
        }

        group->m_stmts.push_back(method);
    }

    for (auto _static_method : m_static_methods)
    {
        std::shared_ptr<StmtNode> static_method = _static_method->reduce(state);

        if (static_method->is<FunctionDeclNode>())
        {
            auto fdecl = std::static_pointer_cast<FunctionDeclNode>(static_method);
            fdecl->m_name = m_name + "::" + fdecl->m_name;
        }
        else
        {
            auto fdef = std::static_pointer_cast<FunctionDefNode>(static_method);
            fdef->m_decl->m_name = m_name + "::" + fdef->m_decl->m_name;
        }

        group->m_stmts.push_back(static_method);
    }

    return group;
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::UnionDefNode::reduce(libquixcc::ReductionState &state) const
{
    std::vector<std::shared_ptr<libquixcc::UnionFieldNode>> fields;

    for (auto &field : m_fields)
    {
        auto copy = std::make_shared<libquixcc::UnionFieldNode>(field->m_name, field->m_type, field->m_value->reduce<ConstExprNode>());
        fields.push_back(copy);
    }

    return std::make_unique<libquixcc::UnionDefNode>(m_name, fields);
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::TypedefNode::reduce(libquixcc::ReductionState &state) const
{
    return nullptr;
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::VarDeclNode::reduce(libquixcc::ReductionState &state) const
{
    std::unique_ptr<VarDeclNode> decl = std::make_unique<VarDeclNode>();

    decl->m_name = m_name;
    decl->m_type = m_type;
    decl->m_init = m_init->reduce<ExprNode>();
    decl->m_is_mut = m_is_mut;
    decl->m_is_thread_local = m_is_thread_local;
    decl->m_is_static = m_is_static;
    decl->m_is_deprecated = m_is_deprecated;

    return decl;
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::LetDeclNode::reduce(libquixcc::ReductionState &state) const
{
    std::unique_ptr<LetDeclNode> decl = std::make_unique<LetDeclNode>();

    decl->m_name = m_name;
    decl->m_type = m_type;
    decl->m_init = m_init->reduce<ExprNode>();
    decl->m_is_mut = m_is_mut;
    decl->m_is_thread_local = m_is_thread_local;
    decl->m_is_static = m_is_static;
    decl->m_is_deprecated = m_is_deprecated;

    return decl;
}

std::unique_ptr<libquixcc::StmtNode> libquixcc::InlineAsmNode::reduce(libquixcc::ReductionState &state) const
{
    return std::make_unique<libquixcc::InlineAsmNode>(*this);
}