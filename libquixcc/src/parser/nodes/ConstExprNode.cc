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
#include <error/Logger.h>

libquixcc::TypeNode *libquixcc::ConstUnaryExprNode::type() const
{
    /// TODO: correct this code

    if (!m_expr->is_primitive())
        LOG(ERROR) << "Cannot apply unary operator to non-primitive type within a constant expression" << std::endl;

    switch (m_op)
    {
    case Operator::Multiply: // dereference
        LOG(ERROR) << "Dereference operator is illegal in a constant expression" << std::endl;
        return nullptr;
    case Operator::BitAnd: // address-of
        LOG(ERROR) << "Address-of operator is illegal in a constant expression" << std::endl;
        return nullptr;
    case Operator::Plus:
    case Operator::Minus:
    case Operator::BitNot:
    case Operator::Not:
    case Operator::Increment:
    case Operator::Decrement:
        return m_expr->type();

    default:
        LOG(FATAL) << "Unhandeled unary operator in constant expression type deduction" << std::endl;
        return nullptr;
    }
}

bool libquixcc::ConstUnaryExprNode::is_signed() const
{
    /// TODO: correct this code
    return m_expr->is_signed();
}

libquixcc::TypeNode *libquixcc::ConstBinaryExprNode::type() const
{
    /// TODO: correct this code

    if (!m_lhs->is_primitive() || !m_rhs->is_primitive())
        LOG(ERROR) << "Cannot apply binary operator to non-primitive types within a constant expression" << std::endl;

    if (m_lhs->type() != m_rhs->type())
        LOG(ERROR) << "Cannot apply binary operator to two different types within a constant expression" << std::endl;

    return m_lhs->type();
}

bool libquixcc::ConstBinaryExprNode::is_signed() const
{
    if (!m_lhs->is_signed() && !m_rhs->is_signed())
        return false;

    return true;
}