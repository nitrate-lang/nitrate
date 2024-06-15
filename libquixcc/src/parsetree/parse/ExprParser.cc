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

static std::shared_ptr<CallExprNode> parse_function_call(
    quixcc_job_t &job, std::shared_ptr<ExprNode> callee, Scanner *scanner,
    size_t depth) {
  Token tok;

  std::vector<std::shared_ptr<ExprNode>> args;
  while (true) {
    tok = scanner->peek();
    if (tok == Token(TT::Punctor, Punctor::CloseParen)) {
      scanner->next();
      break;
    }

    std::shared_ptr<ExprNode> arg;
    if (!parse_expr(job, scanner,
                    {Token(TT::Punctor, Punctor::Comma),
                     Token(TT::Punctor, Punctor::CloseParen)},
                    arg, depth + 1))
      return nullptr;
    args.push_back(arg);

    tok = scanner->peek();
    if (tok.is<Punctor>(Punctor::Comma)) {
      scanner->next();
    }
  }

  auto expr = std::make_shared<CallExprNode>();
  expr->m_callee = callee;

  // if (callee->is<IdentifierNode>())
  // {
  //     expr->m_name =
  //     std::dynamic_pointer_cast<IdentifierNode>(callee)->m_name;
  // }
  // else if (callee->is<MemberAccessNode>())
  // {
  //     auto member = std::dynamic_pointer_cast<MemberAccessNode>(callee);
  //     if (!member->m_expr->is<IdentifierNode>())
  //     {
  //         LOG(ERROR) << "Expected an identifier in member access expression"
  //         << tok << std::endl; return nullptr;
  //     }

  //     std::string name =
  //     std::dynamic_pointer_cast<IdentifierNode>(member->m_expr)->m_name;
  //     expr->m_name = name + "." + member->m_field;
  // }
  // else
  // {
  //     LOG(ERROR) << "Expected an identifier or member access expression" <<
  //     tok << std::endl; return nullptr;
  // }

  expr->m_positional_args = args;

  return expr;
}

static bool parse_fstring(quixcc_job_t &job, std::shared_ptr<FStringNode> &node,
                          Scanner *scanner, size_t depth) {
  Token tok = scanner->next();
  if (tok.type != TT::String) {
    LOG(ERROR) << "Expected a string literal template in f-string" << tok
               << std::endl;
    return false;
  }

  std::string fstr = tok.as<std::string>();
  std::string rectified_template;
  std::vector<std::shared_ptr<ExprNode>> args;
  size_t state = 0, w_beg = 0, w_end = 0;

  for (size_t i = 0; i < fstr.size(); i++) {
    char c = fstr[i];

    if (c == '{' && state == 0) {
      w_beg = i + 1;
      state = 1;
    } else if (c == '}' && state == 1) {
      w_end = i + 1;
      state = 0;

      auto sub = fstr.substr(w_beg, w_end - w_beg) + "\n";

      StringLexer subscanner(sub);

      std::shared_ptr<ExprNode> expr;
      if (!parse_expr(job, &subscanner,
                      {Token(TT::Punctor, Punctor::CloseBrace)}, expr, 0)) {
        return false;
      }

      args.push_back(expr);
      rectified_template += "{}";
    } else if (c == '{') {
      rectified_template += c;
      state = 0;
    } else if (c == '}') {
      rectified_template += c;
      state = 0;
    } else if (state == 0) {
      rectified_template += c;
    }
  }

  if (state != 0) {
    LOG(ERROR) << "Unbalanced braces in f-string" << tok << std::endl;
    return false;
  }

  node = std::make_shared<FStringNode>(rectified_template, args);

  return true;
}

bool libquixcc::parse_expr(quixcc_job_t &job, Scanner *scanner,
                           std::set<Token> terminators,
                           std::shared_ptr<ExprNode> &node, size_t depth) {
  std::stack<std::shared_ptr<ExprNode>> stack;

  /// TODO: Operator precedence
  /// TODO: Operator associativity
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
      {Operator::RotateRight, 25},
      {Operator::RotateLeft, 25},
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

  while (true) {
    auto tok = scanner->peek();
    if (tok.type == TT::Eof) return false;

    if (terminators.contains(tok)) {
      if (stack.empty()) {
        node = nullptr;
        return true;
      }

      if (stack.size() != 1) {
        LOG(ERROR) << "Expected a single expression" << tok << std::endl;
        return false;
      }

      node = stack.top();
      return true;
    }

    scanner->next();

    switch (tok.type) {
      case TT::Integer:
        stack.push(IntegerNode::create(tok.as<std::string>()));
        continue;
      case TT::Float:
        stack.push(FloatLiteralNode::create(tok.as<std::string>()));
        continue;
      case TT::String:
        stack.push(StringNode::create(tok.as<std::string>()));
        continue;
      case TT::Char:
        stack.push(CharNode::create(tok.as<std::string>()));
        continue;
      case TT::Keyword:
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
          case Keyword::Fn: {
            std::shared_ptr<StmtNode> f;
            if (!parse_function(job, scanner, f)) return false;
            auto adapter = std::make_shared<StmtExprNode>(f);
            stack.push(adapter);
            continue;
          }
          case Keyword::FString: {
            std::shared_ptr<FStringNode> f;
            if (!parse_fstring(job, f, scanner, depth)) return false;
            stack.push(f);
            continue;
          }

          default:
            LOG(ERROR) << "Unexpected keyword in non-constant expression '{}'"
                       << tok.serialize() << tok << std::endl;
            return false;
        }
        break;
      case TT::Punctor:
        switch (tok.as<Punctor>()) {
          case Punctor::OpenParen: {
            if (!stack.empty() && stack.top()->is<MemberAccessNode>()) {
              // Method call
              auto fcall =
                  parse_function_call(job, stack.top(), scanner, depth);
              if (fcall == nullptr) return false;

              stack.pop();
              stack.push(fcall);
              continue;
            }

            std::shared_ptr<ExprNode> expr;
            if (!parse_expr(job, scanner, terminators, expr, depth + 1))
              return false;
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
          case Punctor::OpenBrace: {
            std::vector<std::shared_ptr<ExprNode>> elements;
            while (true) {
              auto tok = scanner->peek();
              if (tok == Token(TT::Punctor, Punctor::CloseBrace)) {
                scanner->next();
                break;
              }

              std::shared_ptr<ExprNode> element;
              if (!parse_expr(job, scanner,
                              {Token(TT::Punctor, Punctor::Comma),
                               Token(TT::Punctor, Punctor::CloseBrace)},
                              element, depth + 1))
                return false;
              elements.push_back(element);

              tok = scanner->peek();
              if (tok.is<Punctor>(Punctor::Comma)) {
                scanner->next();
              }
            }

            stack.push(std::make_shared<ListExprNode>(elements));
            continue;
          }
          case Punctor::OpenBracket: {
            if (stack.empty()) {
              // List literal
              std::vector<std::shared_ptr<ExprNode>> elements;
              while (true) {
                auto tok = scanner->peek();
                if (tok == Token(TT::Punctor, Punctor::CloseBracket)) {
                  scanner->next();
                  break;
                }

                std::shared_ptr<ExprNode> element;
                if (!parse_expr(job, scanner,
                                {Token(TT::Punctor, Punctor::Comma),
                                 Token(TT::Punctor, Punctor::CloseBracket)},
                                element, depth + 1))
                  return false;
                elements.push_back(element);

                tok = scanner->peek();
                if (tok.is<Punctor>(Punctor::Comma)) {
                  scanner->next();
                }
              }

              stack.push(std::make_shared<ListExprNode>(elements));
              continue;
            }

            if (stack.size() != 1) {
              LOG(ERROR) << "Expected a single expression" << tok << std::endl;
              return false;
            }

            auto left = stack.top();
            stack.pop();

            std::shared_ptr<ExprNode> index;
            if (!parse_expr(job, scanner,
                            {Token(TT::Punctor, Punctor::CloseBracket),
                             Token(TT::Punctor, Punctor::Colon)},
                            index, depth + 1)) {
              return false;
            }

            auto tok = scanner->next();
            if (tok.is<Punctor>(Punctor::Colon)) {
              std::shared_ptr<ExprNode> end;
              if (!parse_expr(job, scanner,
                              {Token(TT::Punctor, Punctor::CloseBracket)}, end,
                              depth + 1)) {
                return false;
              }

              tok = scanner->next();
              if (!tok.is<Punctor>(Punctor::CloseBracket)) {
                LOG(ERROR) << "Expected a closing bracket" << tok << std::endl;
                return false;
              }

              stack.push(std::make_shared<SliceNode>(left, index, end));
              continue;
            }

            if (!tok.is<Punctor>(Punctor::CloseBracket)) {
              LOG(ERROR) << "Expected a closing bracket" << tok << std::endl;
              return false;
            }

            stack.push(std::make_shared<IndexNode>(left, index));
            continue;
          }
          default:
            LOG(ERROR) << "Unexpected token in non-constant expression '{}'"
                       << tok.serialize() << tok << std::endl;
            return false;
        }
        break;
      case TT::Operator: {
        auto op = tok.as<Operator>();
        if (op == Operator::Dot) {
          if (stack.size() != 1) {
            LOG(ERROR) << "Expected a single expression" << tok << std::endl;
            return false;
          }

          auto left = stack.top();
          stack.pop();

          tok = scanner->peek();
          if (tok.type != TT::Identifier) {
            LOG(ERROR) << "Expected an identifier in member access" << tok
                       << std::endl;
            return false;
          }

          auto ident = tok.as<std::string>();
          stack.push(std::make_shared<MemberAccessNode>(left, ident));
          scanner->next();
          continue;
        }
        std::shared_ptr<ExprNode> expr;

        if (op != Operator::As) {
          if (!parse_expr(job, scanner, terminators, expr, depth + 1) || !expr)
            return false;
        } else {
          TypeNode *type;
          if (!parse_type(job, scanner, &type)) return false;

          /// TODO: Type -> Expression Adapter

          auto left = stack.top();
          stack.pop();
          stack.push(std::make_shared<StaticCastExprNode>(left, type));
          continue;
        }

        if (stack.empty()) {
          stack.push(std::make_shared<UnaryExprNode>(op, expr));
          continue;
        } else if (stack.size() == 1) {
          auto left = stack.top();
          stack.pop();
          stack.push(std::make_shared<BinaryExprNode>(op, left, expr));
          continue;
        } else {
          LOG(ERROR) << "Unexpected token {}" << tok.serialize() << tok
                     << std::endl;
          return false;
        }
        break;
      }
      case TT::Identifier: {
        auto ident = tok.as<std::string>();
        if (scanner->peek().type == TT::Punctor &&
            (scanner->peek()).as<Punctor>() == Punctor::OpenParen) {
          scanner->next();
          auto fcall = parse_function_call(
              job, std::make_shared<IdentifierNode>(ident), scanner, depth);
          if (fcall == nullptr) return false;

          stack.push(fcall);
          continue;
        } else if (scanner->peek().is<Operator>(Operator::Increment)) {
          auto p = std::make_shared<PostUnaryExprNode>(
              Operator::Increment, std::make_shared<IdentifierNode>(ident));
          stack.push(p);
          scanner->next();
          continue;
        } else if (scanner->peek().is<Operator>(Operator::Decrement)) {
          auto p = std::make_shared<PostUnaryExprNode>(
              Operator::Decrement, std::make_shared<IdentifierNode>(ident));
          stack.push(p);
          scanner->next();
          continue;
        } else {
          stack.push(std::make_shared<IdentifierNode>(ident));
          continue;
        }
      }
      default:
        LOG(ERROR) << "Unexpected token {}" << tok.serialize() << tok
                   << std::endl;
        return false;
    }
  }

  return false;
}
