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

#include <parse/nodes/AllNodes.h>
#include <core/Logger.h>

bool libquixcc::ConstExprNode::is_negative() const
{
    return reduce<LiteralNode>()->is_negative();
}

template <typename T>
static T getval_and_cast(const std::shared_ptr<libquixcc::ConstExprNode> node)
{
    using namespace libquixcc;

    switch (node->ntype)
    {
    case NodeType::IntegerNode:
    {
        auto n = static_cast<const IntegerNode *>(node.get());
        switch (n->m_val_type->ntype)
        {
        case NodeType::I64TypeNode:
            return static_cast<T>(std::get<int64_t>(n->m_value));
        case NodeType::U64TypeNode:
            return static_cast<T>(std::get<uint64_t>(n->m_value));
        default:
            throw std::runtime_error("Invalid integer type");
        }
    }
    case NodeType::FloatLiteralNode:
        return static_cast<T>(static_cast<const FloatLiteralNode *>(node.get())->m_value);
    case NodeType::BoolLiteralNode:
        return static_cast<T>(static_cast<const BoolLiteralNode *>(node.get())->m_val);
    case NodeType::CharNode:
        return static_cast<T>(std::stoi(static_cast<const CharNode *>(node.get())->m_val));
    case NodeType::StringNode:
        throw std::runtime_error("string literals cannot be used in binary expressions");
    case NodeType::NullLiteralNode:
        throw std::runtime_error("null literals cannot be used in binary expressions");
    default:
        throw std::runtime_error("Invalid type");
    }
}

template <typename T>
static const std::shared_ptr<libquixcc::LiteralNode> reduce_unary_expr(T val, libquixcc::Operator op)
{
    using namespace libquixcc;

    switch (op)
    {
    case Operator::Minus:
        return IntegerNode::create(std::to_string(-val));
    case Operator::Plus:
        return IntegerNode::create(std::to_string(+val));
    case Operator::LogicalNot:
        return IntegerNode::create(std::to_string(!val));
    case Operator::BitwiseNot:
    {
        if (std::is_same_v<T, bool>)
            return IntegerNode::create(std::to_string(!val));
        return IntegerNode::create(std::to_string(~val));
    }
    case Operator::Increment:
        return IntegerNode::create(std::to_string(val + 1));
    case Operator::Decrement:
        return IntegerNode::create(std::to_string(val - 1));
    default:
        LOG(FATAL) << "const-expr reduction error: invalid operator in unary expression" << std::endl;
        return nullptr;
    }
}

static const std::shared_ptr<libquixcc::LiteralNode> reduce_unary_expr_float_point(double val, libquixcc::Operator op)
{
    using namespace libquixcc;

    switch (op)
    {
    case Operator::Minus:
        return FloatLiteralNode::create(std::to_string(-val));
    case Operator::Plus:
        return FloatLiteralNode::create(std::to_string(+val));
    case Operator::LogicalNot:
        return FloatLiteralNode::create(std::to_string(!val));
    case Operator::BitwiseNot:
        LOG(ERROR) << "const-expr reduction error: bitwise not operation is not supported for floating point numbers" << std::endl;
        return nullptr;
    case Operator::Increment:
        return FloatLiteralNode::create(std::to_string(val + 1));
    case Operator::Decrement:
        return FloatLiteralNode::create(std::to_string(val - 1));
    default:
        LOG(FATAL) << "const-expr reduction error: invalid operator in unary expression" << std::endl;
        return nullptr;
    }
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::ConstUnaryExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    auto x = m_expr->reduce<ConstExprNode>(state);

    if (x->is<FloatLiteralNode>())
        return reduce_unary_expr_float_point(getval_and_cast<double>(x), m_op);

    if (x->is_negative())
        return reduce_unary_expr(getval_and_cast<int64_t>(x), m_op);

    return reduce_unary_expr(getval_and_cast<uint64_t>(x), m_op);
}

template <typename T>
static const std::shared_ptr<libquixcc::LiteralNode> reduce_binary_expr(T lhs, T rhs, libquixcc::Operator op)
{
    using namespace libquixcc;

    switch (op)
    {
    case Operator::LessThan:
        return BoolLiteralNode::create(lhs < rhs);
    case Operator::GreaterThan:
        return BoolLiteralNode::create(lhs > rhs);
    case Operator::Minus:
        return IntegerNode::create(std::to_string(lhs - rhs));
    case Operator::Plus:
        return IntegerNode::create(std::to_string(lhs + rhs));
    case Operator::Multiply:
        return IntegerNode::create(std::to_string(lhs * rhs));
    case Operator::Divide:
        if (rhs == 0)
            LOG(ERROR) << "const-expr reduction error: division by zero is mathematically undefined" << std::endl;
        return IntegerNode::create(std::to_string(lhs / rhs));
    case Operator::Modulo:
        if (rhs == 0)
            LOG(ERROR) << "const-expr reduction error: modulo by zero is mathematically undefined" << std::endl;
        return IntegerNode::create(std::to_string(lhs % rhs));
    case Operator::BitwiseAnd:
        return IntegerNode::create(std::to_string(lhs & rhs));
    case Operator::BitwiseOr:
        return IntegerNode::create(std::to_string(lhs | rhs));
    case Operator::BitwiseXor:
        return IntegerNode::create(std::to_string(lhs ^ rhs));
    case Operator::LeftShift:
        return IntegerNode::create(std::to_string(lhs << rhs));
    case Operator::RightShift:
        return IntegerNode::create(std::to_string(lhs >> rhs));
    case Operator::Equal:
        return BoolLiteralNode::create(lhs == rhs);
    case Operator::NotEqual:
        return BoolLiteralNode::create(lhs != rhs);
    case Operator::LogicalAnd:
        return BoolLiteralNode::create(lhs && rhs);
    case Operator::LogicalOr:
        return BoolLiteralNode::create(lhs || rhs);
    case Operator::LogicalXor:
        return BoolLiteralNode::create(lhs ^ rhs);
    case Operator::LessThanEqual:
        return BoolLiteralNode::create(lhs <= rhs);
    case Operator::GreaterThanEqual:
        return BoolLiteralNode::create(lhs >= rhs);
    case Operator::Increment:
        return IntegerNode::create(std::to_string(lhs + 1));
    case Operator::Decrement:
        return IntegerNode::create(std::to_string(lhs - 1));
    default:
        LOG(FATAL) << "const-expr reduction error: invalid operator in binary expression" << std::endl;
        return nullptr;
    }
}

static const std::shared_ptr<libquixcc::LiteralNode> reduce_binary_expr_float_point(double lhs, double rhs, libquixcc::Operator op)
{
    using namespace libquixcc;

    switch (op)
    {
    case Operator::LessThan:
        return BoolLiteralNode::create(lhs < rhs);
    case Operator::GreaterThan:
        return BoolLiteralNode::create(lhs > rhs);
    case Operator::Minus:
        return FloatLiteralNode::create(std::to_string(lhs - rhs));
    case Operator::Plus:
        return FloatLiteralNode::create(std::to_string(lhs + rhs));
    case Operator::Multiply:
        return FloatLiteralNode::create(std::to_string(lhs * rhs));
    case Operator::Divide:
        if (rhs == 0)
            LOG(ERROR) << "const-expr reduction error: division by zero is mathematically undefined" << std::endl;
        return FloatLiteralNode::create(std::to_string(lhs / rhs));
    case Operator::Modulo:
        LOG(ERROR) << "const-expr reduction error: modulo operation is not supported for floating point numbers" << std::endl;
        return nullptr;
    case Operator::BitwiseAnd:
    case Operator::BitwiseOr:
    case Operator::BitwiseXor:
        LOG(ERROR) << "const-expr reduction error: bitwise operations are not supported for floating point numbers" << std::endl;
        return nullptr;
    case Operator::LeftShift:
    case Operator::RightShift:
        LOG(ERROR) << "const-expr reduction error: bitwise shift operations are not supported for floating point numbers" << std::endl;
        return nullptr;
    case Operator::Equal:
        return BoolLiteralNode::create(lhs == rhs);
    case Operator::NotEqual:
        return BoolLiteralNode::create(lhs != rhs);
    case Operator::LogicalAnd:
        return BoolLiteralNode::create(lhs && rhs);
    case Operator::LogicalOr:
        return BoolLiteralNode::create(lhs || rhs);
    case Operator::LogicalXor:
    {
        if ((lhs == 0 && rhs == 0) || (lhs != 0 && rhs != 0))
            return BoolLiteralNode::create(false);
        return BoolLiteralNode::create(true);
    }
    case Operator::LessThanEqual:
        return BoolLiteralNode::create(lhs <= rhs);
    case Operator::GreaterThanEqual:
        return BoolLiteralNode::create(lhs >= rhs);
    case Operator::Increment:
        return FloatLiteralNode::create(std::to_string(lhs + 1));
    case Operator::Decrement:
        return FloatLiteralNode::create(std::to_string(lhs - 1));
    default:
        LOG(FATAL) << "const-expr reduction error: invalid operator in binary expression" << std::endl;
        return nullptr;
    }
}

std::shared_ptr<libquixcc::ExprNode> libquixcc::ConstBinaryExprNode::reduce_impl(libquixcc::ReductionState &state) const
{
    auto l = m_lhs->reduce<ConstExprNode>(state);
    auto r = m_rhs->reduce<ConstExprNode>(state);

    if (l->is<FloatLiteralNode>() || r->is<FloatLiteralNode>())
        return reduce_binary_expr_float_point(getval_and_cast<double>(l), getval_and_cast<double>(r), m_op);

    if (l->is_negative() || r->is_negative())
        return reduce_binary_expr<int64_t>(getval_and_cast<int64_t>(l), getval_and_cast<int64_t>(r), m_op);

    return reduce_binary_expr<uint64_t>(getval_and_cast<uint64_t>(l), getval_and_cast<uint64_t>(r), m_op);
}