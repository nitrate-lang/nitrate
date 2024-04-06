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
    return reduce()->type();
}

bool libquixcc::ConstUnaryExprNode::is_negative() const
{
    return reduce()->is_negative();
}

template <typename T>
static T getval_and_cast(const libquixcc::ConstExprNode *node)
{
    using namespace libquixcc;

    switch (node->ntype)
    {
    case NodeType::IntegerLiteralNode:
    {
        auto n = static_cast<const IntegerLiteralNode *>(node);
        switch (n->m_val_type->ntype)
        {
        case NodeType::I32TypeNode:
            return static_cast<T>(std::get<int32_t>(n->m_value));
        case NodeType::I64TypeNode:
            return static_cast<T>(std::get<int64_t>(n->m_value));
        case NodeType::U32TypeNode:
            return static_cast<T>(std::get<uint32_t>(n->m_value));
        case NodeType::U64TypeNode:
            return static_cast<T>(std::get<uint64_t>(n->m_value));
        default:
            throw std::runtime_error("Invalid integer type");
        }
    }
    case NodeType::FloatLiteralNode:
        return static_cast<T>(static_cast<const FloatLiteralNode *>(node)->m_value);
    case NodeType::BoolLiteralNode:
        return static_cast<T>(static_cast<const BoolLiteralNode *>(node)->m_val);
    case NodeType::CharLiteralNode:
        return static_cast<T>(std::stoi(static_cast<const CharLiteralNode *>(node)->m_val));
    case NodeType::StringLiteralNode:
        throw std::runtime_error("string literals cannot be used in binary expressions");
    case NodeType::NullLiteralNode:
        throw std::runtime_error("null literals cannot be used in binary expressions");
    default:
        throw std::runtime_error("Invalid type");
    }
}

template <typename T>
static std::unique_ptr<libquixcc::ConstExprNode> reduce_unary_expr(T val, libquixcc::Operator op)
{
    using namespace libquixcc;

    switch (op)
    {
    case Operator::Minus:
        return std::make_unique<IntegerLiteralNode>(std::to_string(-val));
    case Operator::Plus:
        return std::make_unique<IntegerLiteralNode>(std::to_string(+val));
    case Operator::Not:
        return std::make_unique<IntegerLiteralNode>(std::to_string(!val));
    case Operator::BitwiseNot:
    {
        if (std::is_same_v<T, bool>)
            return std::make_unique<IntegerLiteralNode>(std::to_string(!val));
        return std::make_unique<IntegerLiteralNode>(std::to_string(~val));
    }
    case Operator::Increment:
        return std::make_unique<IntegerLiteralNode>(std::to_string(val + 1));
    case Operator::Decrement:
        return std::make_unique<IntegerLiteralNode>(std::to_string(val - 1));
    default:
        LOG(FATAL) << "const-expr reduction error: invalid operator in unary expression" << std::endl;
        return nullptr;
    }
}

static std::unique_ptr<libquixcc::ConstExprNode> reduce_unary_expr_float_point(double val, libquixcc::Operator op)
{
    using namespace libquixcc;

    switch (op)
    {
    case Operator::Minus:
        return std::make_unique<FloatLiteralNode>(std::to_string(-val));
    case Operator::Plus:
        return std::make_unique<FloatLiteralNode>(std::to_string(+val));
    case Operator::Not:
        return std::make_unique<FloatLiteralNode>(std::to_string(!val));
    case Operator::BitwiseNot:
        LOG(ERROR) << "const-expr reduction error: bitwise not operation is not supported for floating point numbers" << std::endl;
        return nullptr;
    case Operator::Increment:
        return std::make_unique<FloatLiteralNode>(std::to_string(val + 1));
    case Operator::Decrement:
        return std::make_unique<FloatLiteralNode>(std::to_string(val - 1));
    default:
        LOG(FATAL) << "const-expr reduction error: invalid operator in unary expression" << std::endl;
        return nullptr;
    }
}

std::unique_ptr<libquixcc::ConstExprNode> libquixcc::ConstUnaryExprNode::reduce() const
{
    auto x = m_expr->reduce();

    if (x->ntype == NodeType::FloatLiteralNode)
        return reduce_unary_expr_float_point(getval_and_cast<double>(x.get()), m_op);

    if (x->is_negative())
        return reduce_unary_expr(getval_and_cast<int64_t>(x.get()), m_op);

    return reduce_unary_expr(getval_and_cast<uint64_t>(x.get()), m_op);
}

libquixcc::TypeNode *libquixcc::ConstBinaryExprNode::type() const
{
    return reduce()->type();
}

bool libquixcc::ConstBinaryExprNode::is_negative() const
{
    return reduce()->is_negative();
}

template <typename T>
static std::unique_ptr<libquixcc::ConstExprNode> reduce_binary_expr(T lhs, T rhs, libquixcc::Operator op)
{
    using namespace libquixcc;

    switch (op)
    {
    case Operator::LessThan:
        return std::make_unique<BoolLiteralNode>(lhs < rhs);
    case Operator::GreaterThan:
        return std::make_unique<BoolLiteralNode>(lhs > rhs);
    case Operator::Minus:
        return std::make_unique<IntegerLiteralNode>(std::to_string(lhs - rhs));
    case Operator::Plus:
        return std::make_unique<IntegerLiteralNode>(std::to_string(lhs + rhs));
    case Operator::Multiply:
        return std::make_unique<IntegerLiteralNode>(std::to_string(lhs * rhs));
    case Operator::Divide:
        if (rhs == 0)
            LOG(ERROR) << "const-expr reduction error: division by zero is mathematically undefined" << std::endl;
        return std::make_unique<IntegerLiteralNode>(std::to_string(lhs / rhs));
    case Operator::Modulo:
        if (rhs == 0)
            LOG(ERROR) << "const-expr reduction error: modulo by zero is mathematically undefined" << std::endl;
        return std::make_unique<IntegerLiteralNode>(std::to_string(lhs % rhs));
    case Operator::BitwiseAnd:
        return std::make_unique<IntegerLiteralNode>(std::to_string(lhs & rhs));
    case Operator::BitwiseOr:
        return std::make_unique<IntegerLiteralNode>(std::to_string(lhs | rhs));
    case Operator::BitwiseXor:
        return std::make_unique<IntegerLiteralNode>(std::to_string(lhs ^ rhs));
    case Operator::LeftShift:
        return std::make_unique<IntegerLiteralNode>(std::to_string(lhs << rhs));
    case Operator::RightShift:
        return std::make_unique<IntegerLiteralNode>(std::to_string(lhs >> rhs));
    case Operator::Equal:
        return std::make_unique<BoolLiteralNode>(lhs == rhs);
    case Operator::NotEqual:
        return std::make_unique<BoolLiteralNode>(lhs != rhs);
    case Operator::And:
        return std::make_unique<BoolLiteralNode>(lhs && rhs);
    case Operator::Or:
        return std::make_unique<BoolLiteralNode>(lhs || rhs);
    case Operator::Xor:
        return std::make_unique<BoolLiteralNode>(lhs ^ rhs);
    case Operator::LessThanEqual:
        return std::make_unique<BoolLiteralNode>(lhs <= rhs);
    case Operator::GreaterThanEqual:
        return std::make_unique<BoolLiteralNode>(lhs >= rhs);
    case Operator::Increment:
        return std::make_unique<IntegerLiteralNode>(std::to_string(lhs + 1));
    case Operator::Decrement:
        return std::make_unique<IntegerLiteralNode>(std::to_string(lhs - 1));
    default:
        LOG(FATAL) << "const-expr reduction error: invalid operator in binary expression" << std::endl;
        return nullptr;
    }
}

static std::unique_ptr<libquixcc::ConstExprNode> reduce_binary_expr_float_point(double lhs, double rhs, libquixcc::Operator op)
{
    using namespace libquixcc;

    switch (op)
    {
    case Operator::LessThan:
        return std::make_unique<BoolLiteralNode>(lhs < rhs);
    case Operator::GreaterThan:
        return std::make_unique<BoolLiteralNode>(lhs > rhs);
    case Operator::Minus:
        return std::make_unique<FloatLiteralNode>(std::to_string(lhs - rhs));
    case Operator::Plus:
        return std::make_unique<FloatLiteralNode>(std::to_string(lhs + rhs));
    case Operator::Multiply:
        return std::make_unique<FloatLiteralNode>(std::to_string(lhs * rhs));
    case Operator::Divide:
        if (rhs == 0)
            LOG(ERROR) << "const-expr reduction error: division by zero is mathematically undefined" << std::endl;
        return std::make_unique<FloatLiteralNode>(std::to_string(lhs / rhs));
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
        return std::make_unique<BoolLiteralNode>(lhs == rhs);
    case Operator::NotEqual:
        return std::make_unique<BoolLiteralNode>(lhs != rhs);
    case Operator::And:
        return std::make_unique<BoolLiteralNode>(lhs && rhs);
    case Operator::Or:
        return std::make_unique<BoolLiteralNode>(lhs || rhs);
    case Operator::Xor:
    {
        if ((lhs == 0 && rhs == 0) || (lhs != 0 && rhs != 0))
            return std::make_unique<BoolLiteralNode>(false);
        return std::make_unique<BoolLiteralNode>(true);
    }
    case Operator::LessThanEqual:
        return std::make_unique<BoolLiteralNode>(lhs <= rhs);
    case Operator::GreaterThanEqual:
        return std::make_unique<BoolLiteralNode>(lhs >= rhs);
    case Operator::Increment:
        return std::make_unique<FloatLiteralNode>(std::to_string(lhs + 1));
    case Operator::Decrement:
        return std::make_unique<FloatLiteralNode>(std::to_string(lhs - 1));
    default:
        LOG(FATAL) << "const-expr reduction error: invalid operator in binary expression" << std::endl;
        return nullptr;
    }
}

std::unique_ptr<libquixcc::ConstExprNode> libquixcc::ConstBinaryExprNode::reduce() const
{
    auto l = m_lhs->reduce();
    auto r = m_rhs->reduce();

    if (l->ntype == NodeType::FloatLiteralNode || r->ntype == NodeType::FloatLiteralNode)
        return reduce_binary_expr_float_point(getval_and_cast<double>(l.get()), getval_and_cast<double>(r.get()), m_op);

    if (l->is_negative() || r->is_negative())
        return reduce_binary_expr<int64_t>(getval_and_cast<int64_t>(l.get()), getval_and_cast<int64_t>(r.get()), m_op);

    return reduce_binary_expr<uint64_t>(getval_and_cast<uint64_t>(l.get()), getval_and_cast<uint64_t>(r.get()), m_op);
}