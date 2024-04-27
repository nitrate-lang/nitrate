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
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
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

#include <parse/nodes/IdentifierNode.h>
#include <parse/nodes/BasicNodes.h>
#include <parse/nodes/ExprNode.h>

std::shared_ptr<libquixcc::ExprNode> libquixcc::IdentifierNode::reduce_impl(libquixcc::ReductionState &state) const
{
    return std::make_shared<IdentifierNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::StaticCastExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    /*
    | Type A    | Type B    | Cast Type     |
    |-----------|-----------|---------------|
    | signed    | unsigned  | signed        |
    | signed    | signed    | signed        |
    | unsigned  | signed    | signed        |
    | unsigned  | unsigned  | unsigned      |
    | pointer   | integer   | ptr-to-int    |
    | integer   | pointer   | int-to-ptr    |
    */
   
    TIState ti_state(state.m_root);
    auto red = m_expr->reduce<ExprNode>(state);
    if (!red)
        LOG(FATAL) << "Failed to reduce static_cast expression" << std::endl;

    auto from = red->infer(ti_state);
    if (!from)
        LOG(FATAL) << "Failed to infer type of static_cast expression" << std::endl;
    
    auto to = m_type;

    size_t from_size = from->size(state.m_ptr_size);
    size_t to_size = to->size(state.m_ptr_size);
    bool upcasting = from_size < to_size;

    std::shared_ptr<ExprNode> res;

    if (from->is_signed() && !to->is_signed())
        return upcasting ? res = std::make_shared<SignedUpcastExprNode>(m_expr, to) : res = std::make_shared<DowncastExprNode>(m_expr, to);

    if (from->is_signed() && to->is_signed())
        return upcasting ? res = std::make_shared<SignedUpcastExprNode>(m_expr, to) : res = std::make_shared<DowncastExprNode>(m_expr, to);

    if (!from->is_signed() && to->is_signed())
        return upcasting ? res = std::make_shared<SignedUpcastExprNode>(m_expr, to) : res = std::make_shared<DowncastExprNode>(m_expr, to);

    if (!from->is_signed() && !to->is_signed())
        return upcasting ? res = std::make_shared<UnsignedUpcastExprNode>(m_expr, to) : res = std::make_shared<DowncastExprNode>(m_expr, to);

    if (from->is_ptr() && to->is_integer())
        return res = std::make_shared<PtrToIntCastExprNode>(m_expr, to);

    if (from->is_integer() && to->is_ptr())
        return res = std::make_shared<IntToPtrCastExprNode>(m_expr, to);

    LOG(FATAL) << "error converting from static_cast to primitive casts" << std::endl;
    return nullptr;
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::BitCastExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    return std::make_shared<BitCastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::SignedUpcastExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    return std::make_shared<SignedUpcastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::UnsignedUpcastExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    return std::make_shared<UnsignedUpcastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::DowncastExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    return std::make_shared<DowncastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::PtrToIntCastExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    return std::make_shared<PtrToIntCastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::IntToPtrCastExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    return std::make_shared<IntToPtrCastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::UnaryExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    return std::make_shared<UnaryExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::BinaryExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    return std::make_shared<BinaryExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::CallExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    return std::make_shared<CallExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::ListExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    return std::make_shared<ListExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::MemberAccessNode::reduce_impl(libquixcc::ReductionState &state) const
{
    return std::make_shared<MemberAccessNode>(*this);
}