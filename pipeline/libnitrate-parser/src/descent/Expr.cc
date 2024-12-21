////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

/// TODO: Cleanup this code; it's a mess from refactoring.

#include <cstddef>
#include <descent/Recurse.hh>
#include <nitrate-parser/AST.hh>
#include <stack>

#define MAX_RECURSION_DEPTH 4096
#define MAX_LIST_DUP (10000)

using namespace ncc::lex;
using namespace ncc::parse;

CallArgs Parser::recurse_call_arguments(Token terminator, size_t depth) {
  Token ident;
  CallArgs call_args;
  size_t pos_arg_count = 0;

  while (true) {
    if (peek() == terminator) {
      break;
    } else if (next_if(qEofF)) {
      diagnostic << current()
                 << "Unexpected end of file while parsing call expression";
      return call_args;
    }

    enum class State {
      ParseNamedArg,
      ParsePosArg,
    } state = State::ParseNamedArg;

    if (peek().is(qName)) {
      ident = next();

      if (next_if(qPuncColn)) {
        state = State::ParseNamedArg;
      } else {
        rd.Undo();
        state = State::ParsePosArg;
      }
    } else {
      state = State::ParsePosArg;
    }

    switch (state) {
      case State::ParseNamedArg: {
        let name = SaveString(ident.as_string());
        let value =
            recurse_expr({Token(qPunc, qPuncComa), terminator}, depth + 1);

        call_args.push_back({name, value});
        break;
      }

      case State::ParsePosArg: {
        let name = SaveString(std::to_string(pos_arg_count++));
        let value =
            recurse_expr({Token(qPunc, qPuncComa), terminator}, depth + 1);

        call_args.push_back({name, value});
        break;
      }
    }

    next_if(qPuncComa);
  }

  return call_args;
}

Expr *Parser::recurse_function_call(Expr *callee, size_t depth) {
  let args = recurse_call_arguments(Token(qPunc, qPuncRPar), depth);
  if (!next_if(qPuncRPar)) {
    diagnostic << current() << "Expected ')' to close the function call";
    return mock_expr(QAST_CALL);
  }

  return make<Call>(callee, args);
}

Expr *Parser::recurse_fstring(size_t depth) {
  Token tok = next();
  if (!tok.is(qText)) {
    diagnostic << tok
               << "Expected a string literal token for F-string expression";
    return mock_expr(QAST_FSTRING);
  }

  let fstr = tok.as_string();

  std::string buf;
  buf.reserve(fstr.size());

  FStringItems items;
  size_t state = 0, w_beg = 0, w_end = 0;

  for (size_t i = 0; i < fstr.size(); i++) {
    let c = fstr[i];

    if (c == '{' && state == 0) {
      w_beg = i + 1;
      state = 1;
    } else if (c == '}' && state == 1) {
      w_end = i + 1;
      state = 0;

      if (!buf.empty()) {
        items.push_back(SaveString(std::move(buf)));
        buf.clear();
      }

      let subnode = FromString(fstr.substr(w_beg, w_end - w_beg), m_env)
                        ->recurse_expr({Token(qPunc, qPuncRCur)}, depth + 1);

      items.push_back(subnode);
    } else if (c == '{') {
      buf += c;
      state = 0;
    } else if (c == '}') {
      buf += c;
      state = 0;
    } else if (state == 0) {
      buf += c;
    }
  }

  if (!buf.empty()) {
    items.push_back(SaveString(std::move(buf)));
    buf.clear();
  }

  if (state != 0) {
    diagnostic << tok << "F-string expression has mismatched braces";
  }

  return make<FString>(std::move(items));
}

/// TODO: Operator precedence
/// TODO: Operator associativity

Expr *Parser::recurse_expr(const std::set<Token> &terminators, size_t depth) {
  if (depth > MAX_RECURSION_DEPTH) {
    diagnostic
        << current()
        << "The expression parser has reached the maximum recursion depth of: "
        << MAX_RECURSION_DEPTH;

    return mock_expr();
  }

  std::stack<Expr *> stack;

  while (true) {
    Token tok = peek();

    if (tok.is(qEofF)) {
      // diagnostic << tok << "Unexpected end of file while parsing expression";
      return mock_expr();
    }

    if (terminators.contains(tok)) {
      if (stack.empty()) {
        return mock_expr();
      }

      if (stack.size() != 1) {
        diagnostic << tok << "Expected a single expression on the stack";
        return mock_expr();
      }

      return stack.top();
    }

    next();

    switch (tok.get_type()) {
      case qIntL: {
        /**
         * @brief Parse integer literal with type suffix
         */

        stack.push(make<ConstInt>(SaveString(tok.as_string())));

        tok = peek();
        if (tok.is(qName)) {
          Type *suffix = recurse_type();

          Expr *integer = stack.top();
          stack.pop();
          stack.push(make<BinExpr>(integer, qOpAs, make<TypeExpr>(suffix)));
        }
        continue;
      }
      case qNumL: {
        /**
         * @brief Parse floating-point literal with type suffix
         */

        stack.push(make<ConstFloat>(SaveString(tok.as_string())));

        tok = peek();
        if (tok.is(qName)) {
          Type *suffix = recurse_type();

          Expr *num = stack.top();
          stack.pop();
          stack.push(make<BinExpr>(num, qOpAs, make<TypeExpr>(suffix)));
        }
        continue;
      }
      case qText: {
        /**
         * @brief Parse string literal with type suffix
         */

        stack.push(make<ConstString>(SaveString(tok.as_string())));

        tok = peek();
        if (tok.is(qName)) {
          Type *suffix = recurse_type();

          Expr *num = stack.top();
          stack.pop();
          stack.push(make<BinExpr>(num, qOpAs, make<TypeExpr>(suffix)));
        }
        continue;
      }
      case qChar: {
        /**
         * @brief
         */
        auto str = tok.as_string();
        qcore_assert(str.size() == 1);

        stack.push(make<ConstChar>(str.at(0)));

        tok = peek();
        if (tok.is(qName)) {
          Type *suffix = recurse_type();

          Expr *num = stack.top();
          stack.pop();
          stack.push(make<BinExpr>(num, qOpAs, make<TypeExpr>(suffix)));
        }
        continue;
      }
      case qKeyW: {
        switch (tok.as_key()) {
          case qKTrue: {
            stack.push(make<ConstBool>(true));
            continue;
          }
          case qKFalse: {
            stack.push(make<ConstBool>(false));
            continue;
          }
          case qKNull: {
            stack.push(make<ConstNull>());
            continue;
          }
          case qKUndef: {
            stack.push(make<ConstUndef>());
            continue;
          }
          case qKFn: {
            Stmt *f = recurse_function(false);
            StmtExpr *adapter = make<StmtExpr>(f);

            if (peek().is<qPuncLPar>()) {
              next();
              let fcall = recurse_function_call(adapter, depth);

              stack.push(fcall);
              continue;
            }
            stack.push(adapter);
            continue;
          }
          case qK__FString: {
            Expr *f = recurse_fstring(depth);
            stack.push(f);
            continue;
          }

          default: {
            diagnostic << tok << "Unexpected keyword in expression";
            return mock_expr();
          }
        }
        break;
      }
      case qPunc: {
        switch (tok.as_punc()) {
          case qPuncLPar: {
            if (!stack.empty() && stack.top()->is<Field>()) {
              let fcall = recurse_function_call(stack.top(), depth);

              stack.pop();
              stack.push(fcall);
              continue;
            }

            Expr *expr = nullptr;
            auto terminators_copy = terminators;
            terminators_copy.insert(Token(qPunc, qPuncRPar));
            expr = recurse_expr(terminators_copy, depth + 1);

            if (!next().is<qPuncRPar>()) {
              diagnostic << tok << "Expected ')' to close the parentheses";
              return mock_expr();
            }

            stack.push(expr);
            continue;
          }
          case qPuncRPar: {
            if (stack.size() != 1) {
              diagnostic << tok << "Expected a single expression on the stack";
              return mock_expr();
            }

            return stack.top();
          }
          case qPuncLCur: {
            ExpressionList elements;
            while (true) {
              tok = peek();
              if (tok.is<qPuncRCur>()) {
                next();
                break;
              }

              Expr *key = nullptr, *value = nullptr;
              key = recurse_expr({Token(qPunc, qPuncColn)}, depth + 1);

              tok = next();
              if (!tok.is<qPuncColn>()) {
                diagnostic << tok << "Expected ':' in list element";
                return mock_expr();
              }

              value = recurse_expr(

                  {Token(qPunc, qPuncComa), Token(qPunc, qPuncRCur)},
                  depth + 1);

              elements.push_back(make<Assoc>(key, value));

              tok = peek();
              if (tok.is<qPuncComa>()) {
                next();
              }
            }

            if (elements.size() == 1) {
              stack.push(elements[0]);
            } else {
              stack.push(make<List>(elements));
            }

            continue;
          }
          case qPuncLBrk: {
            if (stack.empty()) {
              ExpressionList elements;
              while (true) {
                tok = peek();
                if (tok.is<qPuncRBrk>()) {
                  next();
                  stack.push(make<List>(elements));
                  break;
                }

                Expr *element = recurse_expr(

                    {Token(qPunc, qPuncComa), Token(qPunc, qPuncSemi),
                     Token(qPunc, qPuncRBrk)},
                    depth + 1);

                tok = peek();
                if (tok.is<qPuncSemi>()) {
                  next();

                  Expr *count = recurse_expr(
                      {Token(qPunc, qPuncRBrk), Token(qPunc, qPuncComa)},
                      depth + 1);

                  if (!count->is<ConstInt>()) {
                    diagnostic << tok
                               << "Expected a constant integer in list element";
                    return mock_expr();
                  }

                  size_t count_val = std::stoi(
                      std::string(count->as<ConstInt>()->get_value()));

                  if (count_val > MAX_LIST_DUP) {
                    diagnostic << tok
                               << "List element duplication count exceeds the "
                                  "maximum limit";
                    return mock_expr();
                  }

                  for (size_t i = 0; i < count_val; i++) {
                    elements.push_back(element);
                  }

                  tok = peek();
                } else if (element) {
                  elements.push_back(element);
                }

                if (tok.is<qPuncComa>()) {
                  next();
                }
              }

              continue;
            }

            if (stack.size() != 1) {
              diagnostic << tok << "Expected a single expression on the stack";
              return mock_expr();
            }

            Expr *index = nullptr, *left = stack.top();
            stack.pop();

            index = recurse_expr(

                {Token(qPunc, qPuncRBrk), Token(qPunc, qPuncColn)}, depth + 1);

            tok = next();
            if (tok.is<qPuncColn>()) {
              Expr *end = recurse_expr({Token(qPunc, qPuncRBrk)}, depth + 1);

              tok = next();
              if (!tok.is<qPuncRBrk>()) {
                diagnostic << tok << "Expected ']' to close the list index";
                return mock_expr();
              }

              stack.push(make<Slice>(left, index, end));
              continue;
            }

            if (!tok.is<qPuncRBrk>()) {
              diagnostic << tok << "Expected ']' to close the list index";
              return mock_expr();
            }

            tok = peek();
            if (tok.is<qOpInc>()) {
              PostUnaryExpr *p =
                  make<PostUnaryExpr>(make<Index>(left, index), qOpInc);
              stack.push(p);
              next();
              continue;
            } else if (tok.is<qOpDec>()) {
              PostUnaryExpr *p =
                  make<PostUnaryExpr>(make<Index>(left, index), qOpDec);
              stack.push(p);
              next();
              continue;
            }

            stack.push(make<Index>(left, index));
            continue;
          }
          case qPuncComa: {
            if (stack.size() != 1) {
              diagnostic << tok << "Expected a single expression on the stack";
              return mock_expr();
            }

            Expr *right = nullptr, *left = stack.top();
            stack.pop();

            right = recurse_expr(terminators, depth + 1);

            stack.push(make<SeqPoint>(SeqPoint({left, right})));
            continue;
          }
          default: {
            diagnostic << tok << "Unexpected punctuation in expression";
            return mock_expr();
          } break;
        }
      }
      case qOper: {
        Operator op = tok.as_op();
        if (op == qOpDot) {
          if (stack.size() != 1) {
            diagnostic << tok << "Expected a single expression on the stack";
            return mock_expr();
          }

          Expr *left = stack.top();
          stack.pop();

          tok = next();
          if (!tok.is(qName)) {
            diagnostic << tok << "Expected an identifier after '.'";
            return mock_expr();
          }

          let ident = tok.as_string();
          tok = peek();
          if (tok.is<qOpInc>()) {
            PostUnaryExpr *p = make<PostUnaryExpr>(
                make<Field>(left, SaveString(ident)), qOpInc);
            stack.push(p);
            next();
            continue;
          } else if (tok.is<qOpDec>()) {
            PostUnaryExpr *p = make<PostUnaryExpr>(
                make<Field>(left, SaveString(ident)), qOpDec);
            stack.push(p);
            next();
            continue;
          }

          stack.push(make<Field>(left, SaveString(ident)));
          continue;
        }
        Expr *expr = nullptr;

        if (op == qOpAs) {
          if (stack.size() != 1) {
            diagnostic << tok << "Expected a single expression on the stack";
            return mock_expr();
          }

          Type *type = recurse_type();

          Expr *left = stack.top();
          stack.pop();
          stack.push(make<BinExpr>(left, qOpAs, make<TypeExpr>(type)));
          continue;
        }

        if (op == qOpBitcastAs) {
          if (stack.size() != 1) {
            diagnostic << tok << "Expected a single expression on the stack";
            return mock_expr();
          }

          Type *type = recurse_type();

          Expr *left = stack.top();
          stack.pop();
          stack.push(make<BinExpr>(left, qOpBitcastAs, make<TypeExpr>(type)));
          continue;
        }

        expr = recurse_expr(terminators, depth + 1);

        if (stack.empty()) {
          stack.push(make<UnaryExpr>((Operator)op, expr));
          continue;
        } else if (stack.size() == 1) {
          Expr *left = stack.top();
          stack.pop();
          stack.push(make<BinExpr>(left, (Operator)op, expr));
          continue;
        } else {
          diagnostic << tok << "Unexpected operator in expression";
          return mock_expr();
        }
        break;
      }
      case qName: {
        let ident = tok.as_string();
        if (peek().is<qPuncLPar>()) {
          next();

          let fcall =
              recurse_function_call(make<Ident>(SaveString(ident)), depth);

          stack.push(fcall);
          continue;
        } else if (peek().is<qOpInc>()) {
          PostUnaryExpr *p =
              make<PostUnaryExpr>(make<Ident>(SaveString(ident)), qOpInc);
          stack.push(p);
          next();
          continue;
        } else if (peek().is<qOpDec>()) {
          PostUnaryExpr *p =
              make<PostUnaryExpr>(make<Ident>(SaveString(ident)), qOpDec);
          stack.push(p);
          next();
          continue;
        } else {
          Ident *id = make<Ident>(SaveString(ident));
          id->set_offset(tok.get_start());

          stack.push(id);
          continue;
        }
      }
      default: {
        diagnostic << tok << "Unexpected token in expression";
        return mock_expr();
      }
    }
  }

  diagnostic << peek() << "Unexpected end of expression";
  return mock_expr();
}
