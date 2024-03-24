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
#include <error/Message.h>
#include <stack>

using namespace libquixcc;

bool libquixcc::parse_expr(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, Token terminator, std::shared_ptr<libquixcc::ExprNode> &node)
{
    std::stack<std::shared_ptr<libquixcc::ExprNode>> stack;

    while (true)
    {
        auto tok = scanner->peek();
        if (tok.type() == TokenType::Eof)
            return false;

        if (tok == terminator)
        {
            if (stack.size() != 1)
            {
                PARMSG(tok, libquixcc::Err::ERROR, "Expected a single expression");
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
                PARMSG(tok, libquixcc::Err::ERROR, "Unexpected token %s 1", tok.serialize().c_str());
                return false;
            }
            break;
        case TokenType::Punctor:
            switch (std::get<Punctor>(tok.val()))
            {
            case Punctor::OpenParen:
            {
                std::shared_ptr<libquixcc::ExprNode> expr;
                if (!parse_expr(job, scanner, terminator, expr))
                    return false;
                stack.push(expr);
                continue;
            }
            case Punctor::CloseParen:
            {
                if (stack.size() != 1)
                {
                    PARMSG(tok, libquixcc::Err::ERROR, "Expected a single expression");
                    return false;
                }

                node = stack.top();
                stack.pop();
                return true;
            }
            default:
                PARMSG(tok, libquixcc::Err::ERROR, "Unexpected token %s 2", tok.serialize().c_str());
                return false;
            }
            break;
        case TokenType::Operator:
        {
            auto op = std::get<Operator>(tok.val());
            std::shared_ptr<libquixcc::ExprNode> expr;
            if (!parse_expr(job, scanner, terminator, expr))
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
                PARMSG(tok, libquixcc::Err::ERROR, "Unexpected token %s", tok.serialize().c_str());
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
                    if (tok.type() == TokenType::Eof)
                    {
                        PARMSG(tok, libquixcc::Err::ERROR, "Unexpected EOF");
                        return false;
                    }

                    if (tok == Token(TokenType::Punctor, Punctor::CloseParen))
                    {
                        scanner->next();
                        break;
                    }

                    std::shared_ptr<libquixcc::ExprNode> arg;
                    if (!parse_expr(job, scanner, Token(TokenType::Punctor, Punctor::Comma), arg))
                        return false;

                    args.push_back(arg);
                }

                // stack.push(std::make_shared<libquixcc::CallExprNode>(ident, args));
                // continue;

                throw std::runtime_error("Function calls are not yet implemented");
            }
            else
            {
                stack.push(std::make_shared<libquixcc::IdentifierNode>(ident));
                continue;
            }
            break;
        
        }
        default:
            PARMSG(tok, libquixcc::Err::ERROR, "Unexpected token %s", tok.serialize().c_str());
            return false;
        }
    }

    return false;
}