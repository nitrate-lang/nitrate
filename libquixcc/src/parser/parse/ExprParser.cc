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

#include <parse/Parser.h>
#include <LibMacro.h>
#include <error/Logger.h>
#include <stack>

using namespace libquixcc;

bool libquixcc::parse_expr(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, std::set<Token> terminators, std::shared_ptr<libquixcc::ExprNode> &node, size_t depth)
{
    std::stack<std::shared_ptr<libquixcc::ExprNode>> stack;

    // Operator precedence
    static std::map<Operator, int> operator_precedence = {
        {Operator::LessThan, 10},
        {Operator::GreaterThan, 10},
        {Operator::Assign, 10},
        {Operator::At, 10},
        {Operator::Minus, 20},
        {Operator::Plus, 20},
        {Operator::Multiply, 30},
        {Operator::Divide, 30},
        {Operator::Modulo, 30},
        {Operator::BitAnd, 5},
        {Operator::BitOr, 3},
        {Operator::BitXor, 4},
        {Operator::BitNot, 35},
        {Operator::Not, 35},
        {Operator::Question, 1},
        {Operator::PlusAssign, 10},
        {Operator::MinusAssign, 10},
        {Operator::MultiplyAssign, 10},
        {Operator::DivideAssign, 10},
        {Operator::ModuloAssign, 10},
        {Operator::BitOrAssign, 10},
        {Operator::BitAndAssign, 10},
        {Operator::BitXorAssign, 10},
        {Operator::LeftShift, 25},
        {Operator::RightShift, 25},
        {Operator::Equal, 9},
        {Operator::NotEqual, 9},
        {Operator::And, 8},
        {Operator::Or, 7},
        {Operator::Xor, 6},
        {Operator::LessThanEqual, 9},
        {Operator::GreaterThanEqual, 9},
        {Operator::Increment, 40},
        {Operator::Decrement, 40},
        {Operator::XorAssign, 10},
        {Operator::OrAssign, 10},
        {Operator::AndAssign, 10},
        {Operator::LeftShiftAssign, 10},
        {Operator::RightShiftAssign, 10}};

    while (true)
    {
        auto tok = scanner->peek();
        if (tok.type() == TokenType::Eof)
            return false;

        if (terminators.contains(tok))
        {
            if (stack.empty())
            {
                node = nullptr;
                return true;
            }

            if (stack.size() != 1)
            {
                LOG(ERROR) << "Expected a single expression" << tok << std::endl;
                return false;
            }

            node = stack.top();
            return true;
        }

        scanner->next();

        switch (tok.type())
        {
        case TokenType::IntegerLiteral:
            stack.push(libquixcc::IntegerLiteralNode::create(std::get<std::string>(tok.val())));
            continue;
        case TokenType::FloatingLiteral:
            stack.push(libquixcc::FloatLiteralNode::create(std::get<std::string>(tok.val())));
            continue;
        case TokenType::StringLiteral:
            stack.push(libquixcc::StringLiteralNode::create(std::get<std::string>(tok.val())));
            continue;
        case TokenType::CharLiteral:
            stack.push(libquixcc::CharLiteralNode::create(std::get<std::string>(tok.val())));
            continue;
        case TokenType::Keyword:
            switch (std::get<Keyword>(tok.val()))
            {
            case Keyword::True:
                stack.push(libquixcc::BoolLiteralNode::create(true));
                continue;
            case Keyword::False:
                stack.push(libquixcc::BoolLiteralNode::create(false));
                continue;
            case Keyword::Null:
                stack.push(libquixcc::NullLiteralNode::create());
                continue;
            default:
                LOG(ERROR) << "Unexpected token in non-constant expression '{}'" << tok.serialize() << tok << std::endl;
                return false;
            }
            break;
        case TokenType::Punctor:
            switch (std::get<Punctor>(tok.val()))
            {
            case Punctor::OpenParen:
            {
                std::shared_ptr<libquixcc::ExprNode> expr;
                if (!parse_expr(job, scanner, terminators, expr, depth + 1))
                    return false;
                stack.push(expr);
                continue;
            }
            case Punctor::CloseParen:
            {
                if (stack.size() != 1)
                {
                    LOG(ERROR) << "Expected a single expression" << tok << std::endl;
                    return false;
                }

                node = stack.top();
                stack.pop();
                return true;
            }
            default:
                LOG(ERROR) << "Unexpected token in non-constant expression '{}'" << tok.serialize() << tok << std::endl;
                return false;
            }
            break;
        case TokenType::Operator:
        {
            auto op = std::get<Operator>(tok.val());
            std::shared_ptr<libquixcc::ExprNode> expr;
            if (!parse_expr(job, scanner, terminators, expr, depth + 1))
                return false;

            if (stack.empty())
            {
                // Unary operator
                stack.push(std::make_shared<libquixcc::UnaryExprNode>(op, expr));
                continue;
            }
            else if (stack.size() == 1)
            {
                // Binary operator
                auto left = stack.top();
                stack.pop();
                stack.push(std::make_shared<libquixcc::BinaryExprNode>(op, left, expr));
                continue;
            }
            else
            {
                LOG(ERROR) << "Unexpected token {}" << tok.serialize() << tok << std::endl;
                return false;
            }
            break;
        }
        case TokenType::Identifier:
        {
            auto ident = std::get<std::string>(tok.val());
            if (scanner->peek().type() == TokenType::Punctor && std::get<Punctor>(scanner->peek().val()) == Punctor::OpenParen)
            {
                // Function call
                scanner->next();
                std::vector<std::shared_ptr<libquixcc::ExprNode>> args;
                while (true)
                {
                    auto tok = scanner->peek();
                    if (tok == Token(TokenType::Punctor, Punctor::CloseParen))
                    {
                        scanner->next();
                        break;
                    }

                    std::shared_ptr<libquixcc::ExprNode> arg;
                    if (!parse_expr(job, scanner, {Token(TokenType::Punctor, Punctor::Comma), Token(TokenType::Punctor, Punctor::CloseParen)}, arg, depth + 1))
                        return false;
                    args.push_back(arg);

                    tok = scanner->peek();
                    if (tok.is<Punctor>(Punctor::Comma))
                    {
                        scanner->next();
                    }
                }

                auto expr = std::make_shared<libquixcc::CallExprNode>();
                expr->m_name = ident;
                expr->m_positional_args = args;
                stack.push(expr);

                continue;
            }
            else
            {
                stack.push(std::make_shared<libquixcc::IdentifierNode>(ident));
                continue;
            }
        }
        default:
            LOG(ERROR) << "Unexpected token {}" << tok.serialize() << tok << std::endl;
            return false;
        }
    }

    return false;
}
