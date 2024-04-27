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

#include <core/Logger.h>
#include <parse/nodes/ExprNode.h>
#include <parse/nodes/FunctionNode.h>
#include <parse/nodes/StructNode.h>

libquixcc::TypeNode *libquixcc::UnaryExprNode::infer(libquixcc::TIState &state) const
{
    switch (m_op)
    {
    case Operator::Minus:
    case Operator::Plus:
    case Operator::BitwiseNot:
    case Operator::Increment:
    case Operator::Decrement:
        return m_expr->infer(state);
    case Operator::LogicalNot:
        return BoolTypeNode::create();
    case Operator::BitwiseAnd:
        return PointerTypeNode::create(m_expr->infer(state));
    case Operator::Multiply:
    {
        auto type = m_expr->infer(state);
        if (!type->is<PointerTypeNode>())
        {
            LOG(ERROR) << "Can not dereference a non-pointer type" << std::endl;
            return nullptr;
        }

        return static_cast<PointerTypeNode *>(type)->m_type;
    }
    default:
        LOG(ERROR) << "Invalid unary operator" << std::endl;
        return nullptr;
    }

    return nullptr;
}

/*

I'm just going to copy C++20 for now.

Here's a comprehensive table of C++20 type promotions (modified for Quix):

| Type A      | Type B      | Result Type |
|-------------|-------------|-------------|
| bool        | any         | i32         |
| i8          | any integer | i32         |
| u8          | any integer | i32         |
| i16         | any integer | i32         |
| u16         | any integer | i32         |
| i32         | any integer | i32         |
| u32         | any integer | u32         |
| i64         | any integer | i64         |
| u64         | any integer | u64         |
| f32         | any integer | f32         |
| f32         | f64         | f64         |
| f64         | any integer | f64         |
| %ptr to any | %ptr to void| %ptr to void|
| %ptr to any | ptr to void | ptr to void |
| %ptr to any | %ptr to any | %ptr to void|
| ptr to any  | %ptr to any | ptr to void |
| *           | *           | undefined   |

*/

libquixcc::TypeNode *libquixcc::BinaryExprNode::infer(libquixcc::TIState &state) const
{
    auto lhs = m_lhs->infer(state);
    auto rhs = m_rhs->infer(state);

    if (!lhs || !rhs)
        return nullptr;

    /* Logical operations return bool */
    switch (m_op)
    {
    case Operator::LogicalNot:
    case Operator::LogicalAnd:
    case Operator::LogicalOr:
    case Operator::LogicalXor:
    case Operator::LessThan:
    case Operator::GreaterThan:
    case Operator::LessThanEqual:
    case Operator::GreaterThanEqual:
    case Operator::Equal:
    case Operator::NotEqual:
        return BoolTypeNode::create();
    default:
        break;
    }

    if (lhs->is_bool() || rhs->is_bool())
        return I32TypeNode::create();

    if ((lhs->is<I8TypeNode>() && rhs->is_integer()) || (rhs->is<I8TypeNode>() && lhs->is_integer()))
        return I32TypeNode::create();

    if ((lhs->is<U8TypeNode>() && rhs->is_integer()) || (rhs->is<U8TypeNode>() && lhs->is_integer()))
        return I32TypeNode::create();

    if ((lhs->is<I16TypeNode>() && rhs->is_integer()) || (rhs->is<I16TypeNode>() && lhs->is_integer()))
        return I32TypeNode::create();

    if ((lhs->is<U16TypeNode>() && rhs->is_integer()) || (rhs->is<U16TypeNode>() && lhs->is_integer()))
        return I32TypeNode::create();

    if ((lhs->is<I32TypeNode>() && rhs->is_integer()) || (rhs->is<I32TypeNode>() && lhs->is_integer()))
        return I32TypeNode::create();

    if ((lhs->is<U32TypeNode>() && rhs->is_integer()) || (rhs->is<U32TypeNode>() && lhs->is_integer()))
        return U32TypeNode::create();

    if ((lhs->is<I64TypeNode>() && rhs->is_integer()) || (rhs->is<I64TypeNode>() && lhs->is_integer()))
        return I64TypeNode::create();

    if ((lhs->is<U64TypeNode>() && rhs->is_integer()) || (rhs->is<U64TypeNode>() && lhs->is_integer()))
        return U64TypeNode::create();

    if ((lhs->is<F32TypeNode>() && rhs->is_integer()) || (rhs->is<F32TypeNode>() && lhs->is_integer()))
        return F32TypeNode::create();

    if ((lhs->is<F32TypeNode>() && rhs->is<F64TypeNode>()) || (rhs->is<F32TypeNode>() && lhs->is<F64TypeNode>()))
        return F64TypeNode::create();

    if ((lhs->is<F64TypeNode>() && rhs->is_integer()) || (rhs->is<F64TypeNode>() && lhs->is_integer()))
        return F64TypeNode::create();

    if ((lhs->is<MutTypeNode>() && lhs->as<MutTypeNode *>()->m_type->is_ptr() &&
         rhs->is<MutTypeNode>() && rhs->as<MutTypeNode *>()->m_type->is_ptr() &&
         lhs->as<MutTypeNode *>()->m_type->as<PointerTypeNode *>()->m_type->is_void()) ||
        (lhs->is<MutTypeNode>() && lhs->as<MutTypeNode *>()->m_type->is_ptr() &&
         lhs->as<MutTypeNode *>()->m_type->as<PointerTypeNode *>()->m_type->is_void() &&
         rhs->is<MutTypeNode>() && rhs->as<MutTypeNode *>()->m_type->is_ptr()))
        return MutTypeNode::create(PointerTypeNode::create(VoidTypeNode::create()));

    if ((lhs->is<MutTypeNode>() && lhs->as<MutTypeNode *>()->m_type->is_ptr() &&
         rhs->is<PointerTypeNode>() && rhs->as<PointerTypeNode *>()->m_type->is_void()) ||
        (lhs->is<PointerTypeNode>() && lhs->as<PointerTypeNode *>()->m_type->is_void() &&
         rhs->is<MutTypeNode>() && rhs->as<MutTypeNode *>()->m_type->is_ptr()))
        return PointerTypeNode::create(VoidTypeNode::create());

    if ((lhs->is<MutTypeNode>() && lhs->as<MutTypeNode *>()->m_type->is_ptr() &&
         rhs->is<MutTypeNode>() && rhs->as<MutTypeNode *>()->m_type->is_ptr()))
        return MutTypeNode::create(PointerTypeNode::create(VoidTypeNode::create()));

    if ((lhs->is<PointerTypeNode>() && lhs->as<PointerTypeNode *>()->m_type->is_void() &&
         rhs->is<MutTypeNode>() && rhs->as<MutTypeNode *>()->m_type->is_ptr()) ||
        (lhs->is<MutTypeNode>() && lhs->as<MutTypeNode *>()->m_type->is_ptr() &&
         rhs->is<PointerTypeNode>() && rhs->as<PointerTypeNode *>()->m_type->is_void()))
        return PointerTypeNode::create(VoidTypeNode::create());

    LOG(ERROR) << "No implicit conversion from " << lhs->to_source() << " to " << rhs->to_source() << std::endl;

    return nullptr;
}

libquixcc::TypeNode *libquixcc::CallExprNode::infer(libquixcc::TIState &state) const
{
    return m_decl->m_type->m_return_type;
}

libquixcc::TypeNode *libquixcc::ListExprNode::infer(libquixcc::TIState &state) const
{
    std::vector<TypeNode *> types;
    for (auto &expr : m_elements)
    {
        types.push_back(expr->infer(state));
    }

    return StructTypeNode::create(types);
}

libquixcc::TypeNode *libquixcc::MemberAccessNode::infer(libquixcc::TIState &state) const
{
    /// TODO: Implement this function
    return nullptr;
}

libquixcc::TypeNode *libquixcc::CastExprNode::infer(libquixcc::TIState &state) const
{
    return m_type;
}
