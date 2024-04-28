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

#include <parse/Parser.h>
#include <LibMacro.h>
#include <core/Logger.h>
#include <stack>

using namespace libquixcc;

bool libquixcc::parse_const_expr(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, Token terminator, std::shared_ptr<libquixcc::ConstExprNode> &node)
{
    std::stack<std::shared_ptr<ConstExprNode>> stack;

    while (true)
    {
        auto tok = scanner->peek();
        if (tok.type() == TT::Eof)
            return false;

        if (tok == terminator)
        {
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
        case TT::Integer:
            stack.push(IntegerNode::create(std::get<std::string>(tok.val())));
            continue;
        case TT::Float:
            stack.push(FloatLiteralNode::create(std::get<std::string>(tok.val())));
            continue;
        case TT::String:
            stack.push(StringNode::create(std::get<std::string>(tok.val())));
            continue;
        case TT::Char:
            stack.push(CharNode::create(std::get<std::string>(tok.val())));
            continue;
        case TT::Keyword:
            switch (std::get<Keyword>(tok.val()))
            {
            case Keyword::True:
                stack.push(BoolLiteralNode::create(true));
                continue;
            case Keyword::False:
                stack.push(BoolLiteralNode::create(false));
                continue;
            case Keyword::Null:
                stack.push(NullLiteralNode::create());
                continue;
            default:
                LOG(ERROR) << "Unexpected token {} 1" << tok.serialize() << tok << std::endl;
                return false;
            }
            break;
        case TT::Punctor:
            switch (std::get<Punctor>(tok.val()))
            {
            case Punctor::OpenParen:
            {
                std::shared_ptr<ConstExprNode> expr;
                if (!parse_const_expr(job, scanner, terminator, expr))
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
                LOG(ERROR) << "Unexpected token {} 2" << tok.serialize() << tok << std::endl;
                return false;
            }
            break;
        case TT::Operator:
        {
            auto op = std::get<Operator>(tok.val());
            std::shared_ptr<ConstExprNode> expr;
            if (!parse_const_expr(job, scanner, terminator, expr))
                return false;

            if (stack.empty())
            {
                // Unary operator
                stack.push(std::make_shared<ConstUnaryExprNode>(op, expr));
                continue;
            }
            else if (stack.size() == 1)
            {
                // Binary operator
                auto left = stack.top();
                stack.pop();
                stack.push(std::make_shared<ConstBinaryExprNode>(op, left, expr));
                continue;
            }
            else
            {
                LOG(ERROR) << "Unexpected token {}" << tok.serialize() << tok << std::endl;
                return false;
            }
            break;
        }
        default:
            LOG(ERROR) << "Unexpected token {}" << tok.serialize() << tok << std::endl;
            return false;
        }
    }

    return false;
}