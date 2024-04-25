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

#include <core/Logger.h>
#include <parse/nodes/ExprNode.h>
#include <parse/nodes/ConstExprNode.h>
#include <parse/nodes/IdentifierNode.h>
#include <parse/nodes/FunctionNode.h>
#include <parse/nodes/StructNode.h>

std::shared_ptr<libquixcc::ExprNode> libquixcc::IdentifierNode::promote_impl() const
{
    return std::make_shared<IdentifierNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::StaticCastExprNode::promote_impl() const
{
    return std::make_shared<StaticCastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::BitCastExprNode::promote_impl() const
{
    return std::make_shared<BitCastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::SignedUpcastExprNode::promote_impl() const
{
    return std::make_shared<SignedUpcastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::UnsignedUpcastExprNode::promote_impl() const
{
    return std::make_shared<UnsignedUpcastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::DowncastExprNode::promote_impl() const
{
    return std::make_shared<DowncastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::IntToPtrCastExprNode::promote_impl() const
{
    return std::make_shared<IntToPtrCastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::PtrToIntCastExprNode::promote_impl() const
{
    return std::make_shared<PtrToIntCastExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::UnaryExprNode::promote_impl() const
{
    return std::make_shared<UnaryExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::BinaryExprNode::promote_impl() const
{
    return std::make_shared<BinaryExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::CallExprNode::promote_impl() const
{
    return std::make_shared<CallExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::ListExprNode::promote_impl() const
{
    return std::make_shared<ListExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::MemberAccessNode::promote_impl() const
{
    return std::make_shared<MemberAccessNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::ConstUnaryExprNode::promote_impl() const
{
    return std::make_shared<ConstUnaryExprNode>(*this);
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::ConstBinaryExprNode::promote_impl() const
{
    return std::make_shared<ConstBinaryExprNode>(*this);
}