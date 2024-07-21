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

#include <core/Macro.h>
#include <qast/Parser.h>
#include <quixcc/core/Logger.h>

#include <stack>

using namespace libquixcc;
using namespace libquixcc::qast;
using namespace libquixcc::qast::parser;

static Call *parse_function_call(quixcc_cc_job_t &job, Expr *callee, Scanner *scanner,
                                 size_t depth) {
  /**
   * @brief
   */

  Token tok, ident;
  CallArgs call_args;
  size_t pos_arg_count = 0;

  while (true) {
    /**
     * @brief
     */

    tok = scanner->peek();

    if (tok == Token(tPunc, CloseParen)) {
      /**
       * @brief
       */

      scanner->next();
      break;
    }

    if (!tok.is(tName)) {
      /**
       * @brief
       */

      goto parse_pos_arg;
    }

    { /* Parse named argument */
      ident = tok;
      scanner->next();
      tok = scanner->peek();

      if (!tok.is<Punctor>(Colon)) {
        /**
         * @brief
         */

        scanner->push(ident);
        goto parse_pos_arg;
      }

      scanner->next();

      Expr *arg = nullptr;
      if (!parse_expr(job, scanner, {Token(tPunc, Comma), Token(tPunc, CloseParen)}, &arg,
                      depth + 1)) {
        /**
         * @brief
         */

        return nullptr;
      }

      call_args.insert({ident.as<std::string>(), arg});
      goto comma;
    }

  parse_pos_arg: {
    /**
     * @brief
     */

    Expr *arg = nullptr;
    if (!parse_expr(job, scanner, {Token(tPunc, Comma), Token(tPunc, CloseParen)}, &arg,
                    depth + 1)) {
      /**
       * @brief
       */

      return nullptr;
    }

    /**
     * @brief
     */
    call_args.insert({"__" + std::to_string(pos_arg_count++), arg});

    goto comma;
  }

  comma: {
    tok = scanner->peek();
    if (tok.is<Punctor>(Comma)) {
      scanner->next();
    }
    continue;
  }
  }

  return Call::get(callee, call_args);
}

static bool parse_fstring(quixcc_cc_job_t &job, FString **node, Scanner *scanner, size_t depth) {
  /**
   * @brief
   */

  Token tok = scanner->next();
  std::string fstr, rectified_template;
  FStringArgs args;
  size_t state = 0, w_beg = 0, w_end = 0;
  Expr *expr = nullptr;

  if (!tok.is(tText)) {
    LOG(ERROR) << "Expected a string literal template in f-string" << tok << std::endl;
    return false;
  }

  fstr = tok.as_string();

  for (size_t i = 0; i < fstr.size(); i++) {
    /**
     * @brief
     */

    char c = fstr[i];

    if (c == '{' && state == 0) {
      w_beg = i + 1;
      state = 1;
    } else if (c == '}' && state == 1) {
      w_end = i + 1;
      state = 0;

      std::string sub = fstr.substr(w_beg, w_end - w_beg) + "\n";

      StringLexer subscanner(sub);

      if (!parse_expr(job, &subscanner, {Token(tPunc, CloseBrace)}, &expr, 0)) {
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

  *node = FString::get(rectified_template, args);

  return true;
}

/// TODO: Operator precedence
/// TODO: Operator associativity

bool libquixcc::qast::parser::parse_expr(quixcc_cc_job_t &job, Scanner *scanner,
                                         std::set<Token> terminators, Expr **node, size_t depth) {
  /**
   * @brief
   */

  std::stack<Expr *> stack;

  while (true) {
    Token tok = scanner->peek();

    if (tok.is(tEofF)) {
      /*  */
      return false;
    }

    if (terminators.contains(tok)) {
      /**
       * @brief
       */

      if (stack.empty()) {
        node = nullptr;
        return true;
      }

      if (stack.size() != 1) {
        LOG(ERROR) << "Expected a single expression" << tok << std::endl;
        return false;
      }

      /* */
      *node = stack.top();
      return true;
    }

    scanner->next();

    switch (tok.type()) {
    case tIntL: {
      /**
       * @brief
       */

      stack.push(ConstInt::get(tok.as_string()));
      continue;
    }
    case tNumL: {
      /**
       * @brief
       */

      stack.push(ConstFloat::get(tok.as_string()));
      continue;
    }
    case tText: {
      /**
       * @brief
       */

      stack.push(ConstString::get(tok.as_string()));
      continue;
    }
    case tChar: {
      /**
       * @brief
       */

      stack.push(ConstChar::get(tok.as_string()));
      continue;
    }
    case tKeyW: {
      switch (tok.as<Keyword>()) {
      case Keyword::True: {
        stack.push(ConstBool::get(true));
        continue;
      }
      case Keyword::False: {
        stack.push(ConstBool::get(false));
        continue;
      }
      case Keyword::Null: {
        stack.push(ConstNull::get());
        continue;
      }
      case Keyword::Undef: {
        stack.push(ConstUndef::get());
        continue;
      }
      case Keyword::Fn: {
        Stmt *f = nullptr;
        if (!parse_function(job, scanner, &f)) {
          return false;
        }
        StmtExpr *adapter = StmtExpr::get(f);

        if (scanner->peek().is<Punctor>(OpenParen)) {
          scanner->next();
          Call *fcall = parse_function_call(job, adapter, scanner, depth);

          if (fcall == nullptr) {
            return false;
          }

          stack.push(fcall);
          continue;
        }
        stack.push(adapter);
        continue;
      }
      case Keyword::FString: {
        FString *f = nullptr;
        if (!parse_fstring(job, &f, scanner, depth)) {
          return false;
        }
        stack.push(f);
        continue;
      }

      default: {
        LOG(ERROR) << "Unexpected keyword in non-constant expression '{}'" << tok.serialize() << tok
                   << std::endl;
        return false;
      }
      }
      break;
    }
    case tPunc: {
      switch (tok.as<Punctor>()) {
      case OpenParen: {
        if (!stack.empty() && stack.top()->is<Field>()) {
          Call *fcall = parse_function_call(job, stack.top(), scanner, depth);

          if (fcall == nullptr) {
            return false;
          }

          stack.pop();
          stack.push(fcall);
          continue;
        }

        Expr *expr = nullptr;
        if (!parse_expr(job, scanner, terminators, &expr, depth + 1)) {
          return false;
        }

        stack.push(expr);
        continue;
      }
      case CloseParen: {
        if (stack.size() != 1) {
          LOG(ERROR) << "Expected a single expression" << tok << std::endl;
          return false;
        }

        *node = stack.top();
        stack.pop();
        return true;
      }
      case OpenBrace: {
        ListData elements;
        while (true) {
          tok = scanner->peek();
          if (tok == Token(tPunc, CloseBrace)) {
            scanner->next();
            break;
          }

          Expr *key = nullptr, *value = nullptr;
          if (!parse_expr(job, scanner, {Token(tPunc, Colon)}, &key, depth + 1)) {
            return false;
          }

          tok = scanner->next();
          if (!tok.is<Punctor>(Colon)) {
            LOG(ERROR) << "Expected a colon in associative array" << tok << std::endl;
            return false;
          }

          if (!parse_expr(job, scanner, {Token(tPunc, Comma), Token(tPunc, CloseBrace)}, &value,
                          depth + 1)) {
            return false;
          }

          elements.push_back(Assoc::get(key, value));

          tok = scanner->peek();
          if (tok.is<Punctor>(Comma)) {
            scanner->next();
          }
        }

        stack.push(List::get(elements));
        continue;
      }
      case OpenBracket: {
        if (stack.empty()) {
          ListData elements;
          while (true) {
            tok = scanner->peek();
            if (tok == Token(tPunc, CloseBracket)) {
              scanner->next();
              stack.push(List::get(elements));
              break;
            }

            Expr *element = nullptr;
            if (!parse_expr(
                    job, scanner,
                    {Token(tPunc, Comma), Token(tPunc, Semicolon), Token(tPunc, CloseBracket)},
                    &element, depth + 1)) {
              return false;
            }

            tok = scanner->peek();
            if (tok.is<Punctor>(Semicolon)) {
              scanner->next();

              Expr *count = nullptr;
              if (!parse_expr(job, scanner, {Token(tPunc, CloseBracket), Token(tPunc, Comma)},
                              &count, depth + 1)) {
                return false;
              }

              if (!count->is<ConstInt>()) {
                LOG(ERROR) << "Expected an integer literal as array count" << tok << std::endl;
                return false;
              }

              size_t count_val = std::atoi(count->as<ConstInt>()->get_value().c_str());
              for (size_t i = 0; i < count_val; i++) {
                elements.push_back(element);
              }
            } else if (element) {
              elements.push_back(element);
            }

            if (tok.is<Punctor>(Comma)) {
              scanner->next();
            }
          }

          continue;
        }

        if (stack.size() != 1) {
          LOG(ERROR) << "Expected a single expression" << tok << std::endl;
          return false;
        }

        Expr *index = nullptr, *left = stack.top();
        stack.pop();

        if (!parse_expr(job, scanner, {Token(tPunc, CloseBracket), Token(tPunc, Colon)}, &index,
                        depth + 1)) {
          return false;
        }

        tok = scanner->next();
        if (tok.is<Punctor>(Colon)) {
          Expr *end = nullptr;
          if (!parse_expr(job, scanner, {Token(tPunc, CloseBracket)}, &end, depth + 1)) {
            return false;
          }

          tok = scanner->next();
          if (!tok.is<Punctor>(CloseBracket)) {
            LOG(ERROR) << "Expected a closing bracket" << tok << std::endl;
            return false;
          }

          stack.push(Slice::get(left, index, end));
          continue;
        }

        if (!tok.is<Punctor>(CloseBracket)) {
          LOG(ERROR) << "Expected a closing bracket" << tok << std::endl;
          return false;
        }

        tok = scanner->peek();
        if (tok.is<Operator>(Increment)) {
          PostUnaryExpr *p = PostUnaryExpr::get(Index::get(left, index), PostUnaryOp::Increment);
          stack.push(p);
          scanner->next();
          continue;
        } else if (tok.is<Operator>(Decrement)) {
          PostUnaryExpr *p = PostUnaryExpr::get(Index::get(left, index), PostUnaryOp::Decrement);
          stack.push(p);
          scanner->next();
          continue;
        }

        stack.push(Index::get(left, index));
        continue;
      }
      case Comma: {
        if (stack.size() != 1) {
          LOG(ERROR) << "Expected a single expression before sequence point" << tok << std::endl;
          return false;
        }

        Expr *right = nullptr, *left = stack.top();
        stack.pop();

        if (!parse_expr(job, scanner, terminators, &right, depth + 1)) {
          return false;
        }

        stack.push(SeqPoint::get(SeqPoint({left, right})));
        continue;
      }
      default: {
        LOG(ERROR) << "Unexpected token in non-constant expression '{}'" << tok.serialize() << tok
                   << std::endl;
        return false;
      } break;
      }
    }
    case tOper: {
      Operator op = tok.as<Operator>();
      if (op == Dot) {
        if (stack.size() != 1) {
          LOG(ERROR) << "Expected a single expression" << tok << std::endl;
          return false;
        }

        Expr *left = stack.top();
        stack.pop();

        tok = scanner->next();
        if (!tok.is(tName)) {
          LOG(ERROR) << "Expected an identifier in member access" << tok << std::endl;
          return false;
        }

        std::string ident = tok.as_string();
        tok = scanner->peek();
        if (tok.is<Operator>(Increment)) {
          PostUnaryExpr *p = PostUnaryExpr::get(Field::get(left, ident), PostUnaryOp::Increment);
          stack.push(p);
          scanner->next();
          continue;
        } else if (tok.is<Operator>(Decrement)) {
          PostUnaryExpr *p = PostUnaryExpr::get(Field::get(left, ident), PostUnaryOp::Decrement);
          stack.push(p);
          scanner->next();
          continue;
        }

        stack.push(Field::get(left, ident));
        continue;
      }
      Expr *expr = nullptr;

      if (op == As) {
        if (stack.size() != 1) {
          LOG(ERROR) << "Expected a single expression" << tok << std::endl;
          return false;
        }

        Type *type = nullptr;
        if (!parse_type(job, scanner, &type)) {
          return false;
        }

        Expr *left = stack.top();
        stack.pop();
        stack.push(BinExpr::get(left, BinOp::As, TypeExpr::get(type)));
        continue;
      }

      if (op == BitcastAs) {
        if (stack.size() != 1) {
          LOG(ERROR) << "Expected a single expression" << tok << std::endl;
          return false;
        }

        Type *type = nullptr;
        if (!parse_type(job, scanner, &type)) {
          return false;
        }

        Expr *left = stack.top();
        stack.pop();
        stack.push(BinExpr::get(left, BinOp::BitcastAs, TypeExpr::get(type)));
        continue;
      }

      if (op == ReinterpretAs) {
        if (stack.size() != 1) {
          LOG(ERROR) << "Expected a single expression" << tok << std::endl;
          return false;
        }

        Type *type = nullptr;
        if (!parse_type(job, scanner, &type)) {
          return false;
        }

        Expr *left = stack.top();
        stack.pop();

        stack.push(BinExpr::get(left, BinOp::ReinterpretAs, TypeExpr::get(type)));
        continue;
      }

      if (!parse_expr(job, scanner, terminators, &expr, depth + 1) || !expr) {
        return false;
      }

      if (stack.empty()) {
        stack.push(UnaryExpr::get((UnaryOp)op, expr));
        continue;
      } else if (stack.size() == 1) {
        Expr *left = stack.top();
        stack.pop();
        stack.push(BinExpr::get(left, (BinOp)op, expr));
        continue;
      } else {
        LOG(ERROR) << "Unexpected token {}" << tok.serialize() << tok << std::endl;
        return false;
      }
      break;
    }
    case tName: {
      std::string ident = tok.as_string();
      if (scanner->peek().type() == tPunc && (scanner->peek()).as<Punctor>() == OpenParen) {
        scanner->next();

        Call *fcall = parse_function_call(job, Ident::get(ident), scanner, depth);
        if (fcall == nullptr) {
          return false;
        }

        stack.push(fcall);
        continue;
      } else if (scanner->peek().is<Operator>(Increment)) {
        PostUnaryExpr *p = PostUnaryExpr::get(Ident::get(ident), PostUnaryOp::Increment);
        stack.push(p);
        scanner->next();
        continue;
      } else if (scanner->peek().is<Operator>(Decrement)) {
        PostUnaryExpr *p = PostUnaryExpr::get(Ident::get(ident), PostUnaryOp::Decrement);
        stack.push(p);
        scanner->next();
        continue;
      } else {
        stack.push(Ident::get(ident));
        continue;
      }
    }
    default: {
      LOG(ERROR) << "Unexpected token {}" << tok.serialize() << tok << std::endl;
      return false;
    }
    }
  }

  return false;
}
