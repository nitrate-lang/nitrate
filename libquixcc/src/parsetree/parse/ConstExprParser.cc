////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <LibMacro.h>
#include <core/Logger.h>
#include <parsetree/Parser.h>

#include <stack>

using namespace libquixcc;

bool libquixcc::parse_const_expr(
    quixcc_job_t &job, libquixcc::Scanner *scanner, Token terminator,
    std::shared_ptr<libquixcc::ConstExprNode> &node) {
  std::stack<std::shared_ptr<ConstExprNode>> stack;

  while (true) {
    auto tok = scanner->peek();
    if (tok.type() == tEofF) return false;

    if (tok == terminator) {
      if (stack.size() != 1) {
        LOG(ERROR) << "Expected a single expression" << tok << std::endl;
        return false;
      }

      node = stack.top();
      return true;
    }

    scanner->next();

    switch (tok.type()) {
      case tIntL:
        stack.push(IntegerNode::create(tok.as<std::string>()));
        continue;
      case tNumL:
        stack.push(FloatLiteralNode::create(tok.as<std::string>()));
        continue;
      case tText:
        stack.push(StringNode::create(tok.as<std::string>()));
        continue;
      case tChar:
        stack.push(CharNode::create(tok.as<std::string>()));
        continue;
      case tKeyW:
        switch (tok.as<Keyword>()) {
          case Keyword::True:
            stack.push(BoolLiteralNode::create(true));
            continue;
          case Keyword::False:
            stack.push(BoolLiteralNode::create(false));
            continue;
          case Keyword::Null:
            stack.push(NullLiteralNode::create());
            continue;
          case Keyword::Undef:
            stack.push(UndefLiteralNode::create());
            continue;
          default:
            LOG(ERROR) << "Unexpected token {} 1" << tok.serialize() << tok
                       << std::endl;
            return false;
        }
        break;
      case tPunc:
        switch (tok.as<Punctor>()) {
          case Punctor::OpenParen: {
            std::shared_ptr<ConstExprNode> expr;
            if (!parse_const_expr(job, scanner, terminator, expr)) return false;
            stack.push(expr);
            continue;
          }
          case Punctor::CloseParen: {
            if (stack.size() != 1) {
              LOG(ERROR) << "Expected a single expression" << tok << std::endl;
              return false;
            }

            node = stack.top();
            stack.pop();
            return true;
          }
          default:
            LOG(ERROR) << "Unexpected token {} 2" << tok.serialize() << tok
                       << std::endl;
            return false;
        }
        break;
      case tOper: {
        auto op = tok.as<Operator>();
        std::shared_ptr<ConstExprNode> expr;
        if (!parse_const_expr(job, scanner, terminator, expr)) return false;

        if (stack.empty()) {
          // Unary operator
          stack.push(std::make_shared<ConstUnaryExprNode>(op, expr));
          continue;
        } else if (stack.size() == 1) {
          // Binary operator
          auto left = stack.top();
          stack.pop();
          stack.push(std::make_shared<ConstBinaryExprNode>(op, left, expr));
          continue;
        } else {
          LOG(ERROR) << "Unexpected token {}" << tok.serialize() << tok
                     << std::endl;
          return false;
        }
        break;
      }
      default:
        LOG(ERROR) << "Unexpected token {}" << tok.serialize() << tok
                   << std::endl;
        return false;
    }
  }

  return false;
}