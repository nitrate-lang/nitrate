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

#include <parse/Parser.h>
#include <LibMacro.h>
#include <error/Logger.h>
#include <stack>

using namespace libquixcc;

static std::shared_ptr<libquixcc::CallExprNode> parse_function_call(quixcc_job_t &job, std::shared_ptr<ExprNode> callee, std::shared_ptr<libquixcc::Scanner> scanner, size_t depth)
{
    Token tok;

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
            return nullptr;
        args.push_back(arg);

        tok = scanner->peek();
        if (tok.is<Punctor>(Punctor::Comma))
        {
            scanner->next();
        }
    }

    auto expr = std::make_shared<libquixcc::CallExprNode>();

    if (callee->ntype == NodeType::IdentifierNode)
    {
        expr->m_name = std::dynamic_pointer_cast<libquixcc::IdentifierNode>(callee)->m_name;
    }
    else if (callee->ntype == NodeType::MemberAccessNode)
    {
        auto member = std::dynamic_pointer_cast<libquixcc::MemberAccessNode>(callee);
        if (member->m_expr->ntype != NodeType::IdentifierNode)
        {
            LOG(ERROR) << "Expected an identifier" << tok << std::endl;
            return nullptr;
        }

        std::string name = std::dynamic_pointer_cast<libquixcc::IdentifierNode>(member->m_expr)->m_name;
        expr->m_name = name + "." + member->m_field;
    }
    else
    {
        LOG(ERROR) << "Expected an identifier or member access expression" << tok << std::endl;
        return nullptr;
    }

    expr->m_positional_args = args;

    return expr;
}

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
        {Operator::BitwiseAnd, 5},
        {Operator::BitwiseOr, 3},
        {Operator::BitwiseXor, 4},
        {Operator::BitwiseNot, 35},
        {Operator::LogicalNot, 35},
        {Operator::Question, 1},
        {Operator::PlusAssign, 10},
        {Operator::MinusAssign, 10},
        {Operator::MultiplyAssign, 10},
        {Operator::DivideAssign, 10},
        {Operator::ModuloAssign, 10},
        {Operator::BitwiseOrAssign, 10},
        {Operator::BitwiseAndAssign, 10},
        {Operator::BitwiseXorAssign, 10},
        {Operator::LeftShift, 25},
        {Operator::RightShift, 25},
        {Operator::Equal, 9},
        {Operator::NotEqual, 9},
        {Operator::LogicalAnd, 8},
        {Operator::LogicalOr, 7},
        {Operator::LogicalXor, 6},
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
                if (!stack.empty() && stack.top()->ntype == NodeType::MemberAccessNode)
                {
                    // Method call
                    auto fcall = parse_function_call(job, stack.top(), scanner, depth);
                    if (fcall == nullptr)
                        return false;

                    stack.pop();
                    stack.push(fcall);
                    continue;
                }

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
            case Punctor::OpenBrace:
            {
                std::vector<std::shared_ptr<libquixcc::ExprNode>> elements;
                while (true)
                {
                    auto tok = scanner->peek();
                    if (tok == Token(TokenType::Punctor, Punctor::CloseBrace))
                    {
                        scanner->next();
                        break;
                    }

                    std::shared_ptr<libquixcc::ExprNode> element;
                    if (!parse_expr(job, scanner, {Token(TokenType::Punctor, Punctor::Comma), Token(TokenType::Punctor, Punctor::CloseBrace)}, element, depth + 1))
                        return false;
                    elements.push_back(element);

                    tok = scanner->peek();
                    if (tok.is<Punctor>(Punctor::Comma))
                    {
                        scanner->next();
                    }
                }

                stack.push(std::make_shared<libquixcc::ListExprNode>(elements));
                continue;
            }
            case Punctor::Dot:
            {
                if (stack.size() != 1)
                {
                    LOG(ERROR) << "Expected a single expression" << tok << std::endl;
                    return false;
                }

                auto left = stack.top();
                stack.pop();

                auto tok = scanner->peek();
                if (tok.type() != TokenType::Identifier)
                {
                    LOG(ERROR) << "Expected an identifier in member access" << tok << std::endl;
                    return false;
                }

                auto ident = std::get<std::string>(tok.val());
                stack.push(std::make_shared<libquixcc::MemberAccessNode>(left, ident));
                scanner->next();
                continue;
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
                scanner->next();
                auto fcall = parse_function_call(job, std::make_shared<IdentifierNode>(ident), scanner, depth);
                if (fcall == nullptr)
                    return false;

                stack.push(fcall);
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
