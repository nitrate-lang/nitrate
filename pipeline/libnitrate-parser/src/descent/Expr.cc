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

/// TODO: Source location

#include <nitrate-parser/Node.h>

#include <cstddef>
#include <descent/Recurse.hh>
#include <sstream>
#include <stack>

#define MAX_EXPR_DEPTH (10000)
#define MAX_LIST_DUP (10000)

static inline npar::Expr *LOC_121(npar::Expr *p, qlex_tok_t t) {
  p->set_start_pos(t.start);
  p->set_end_pos(t.end);
  return p;
}

using namespace npar;

static Call *recurse_function_call(npar_t &S, Expr *callee, qlex_t &rd,
                                   size_t depth) {
  /**
   * @brief
   */

  qlex_tok_t tok, ident;
  CallArgs call_args;
  size_t pos_arg_count = 0;

  while (true) {
    /**
     * @brief
     */

    tok = peek();

    if (tok.is(qEofF)) {
      diagnostic << tok << "Unexpected end of file while parsing function call";
      return nullptr;
    }

    if (tok.is<qPuncRPar>()) {
      /**
       * @brief
       */

      next();
      break;
    }

    if (!tok.is(qName)) {
      /**
       * @brief
       */

      goto parse_pos_arg;
    }

    { /* Parse named argument */
      ident = tok;
      next();
      tok = peek();

      if (!tok.is<qPuncColn>()) {
        /**
         * @brief
         */

        qlex_insert(&rd, tok);
        qlex_insert(&rd, ident);
        goto parse_pos_arg;
      }

      next();

      Expr *arg = recurse_expr(
          S, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRPar)},
          depth + 1);

      call_args.push_back({ident.as_string(&rd), arg});
      goto comma;
    }

  parse_pos_arg: {
    Expr *arg = recurse_expr(
        S, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRPar)},
        depth + 1);

    call_args.push_back({std::to_string(pos_arg_count++), arg});

    goto comma;
  }

  comma: {
    tok = peek();
    if (tok.is<qPuncComa>()) {
      next();
    }
    continue;
  }
  }

  return Call::get(callee, call_args);
}

static bool recurse_fstring(npar_t &S, FString **node, qlex_t &rd,
                            size_t depth) {
  /**
   * @brief Parse an F-string expression
   * @return true if it is okay to proceed, false otherwise
   */

  qlex_tok_t tok = next();
  if (!tok.is(qText)) {
    diagnostic << tok << "Expected a string literal in F-string expression";
  }

  auto fstr = tok.as_string(&rd);

  std::string tmp;
  tmp.reserve(fstr.size());

  FStringItems items;
  size_t state = 0, w_beg = 0, w_end = 0;
  Expr *expr = nullptr;

  for (size_t i = 0; i < fstr.size(); i++) {
    char c = fstr[i];

    if (c == '{' && state == 0) {
      w_beg = i + 1;
      state = 1;
    } else if (c == '}' && state == 1) {
      w_end = i + 1;
      state = 0;

      std::string sub(fstr.substr(w_beg, w_end - w_beg));

      std::istringstream ss(sub);

      qlex_t *subrd = qlex_new(ss, "fstring", S.env);

      expr = recurse_expr(S, *subrd, {qlex_tok_t(qPunc, qPuncRCur)}, depth + 1);

      qlex_free(subrd);

      if (!tmp.empty()) {
        items.push_back(std::move(tmp));
        tmp.clear();
      }

      items.push_back(expr);
    } else if (c == '{') {
      tmp += c;
      state = 0;
    } else if (c == '}') {
      tmp += c;
      state = 0;
    } else if (state == 0) {
      tmp += c;
    }
  }

  if (!tmp.empty()) {
    items.push_back(std::move(tmp));
    tmp.clear();
  }

  if (state != 0) {
    diagnostic << tok << "F-string expression is not properly closed with '}'";
  }

  *node = FString::get(std::move(items));

  return true;
}

/// TODO: qlex_op_t precedence
/// TODO: qlex_op_t associativity

Expr *npar::recurse_expr(npar_t &S, qlex_t &rd,
                         std::unordered_set<qlex_tok_t, tok_hash> terminators,
                         size_t depth) {
  if (depth > MAX_EXPR_DEPTH) {
    diagnostic
        << peek()
        << "Expression depth exceeded; Expressions can not be nested more than "
        << MAX_EXPR_DEPTH << " times";

    return mock_expr(QAST_NODE_VOID_TY);
  }

  std::stack<Expr *> stack;

  while (true) {
    qlex_tok_t tok = peek();

    if (tok.is(qEofF)) {
      // diagnostic << tok << "Unexpected end of file while parsing expression";
      return mock_expr(QAST_NODE_VOID_TY);
    }

    if (terminators.contains(tok)) {
      if (stack.empty()) {
        return mock_expr(QAST_NODE_VOID_TY);
      }

      if (stack.size() != 1) {
        diagnostic << tok << "Expected a single expression on the stack";
        return mock_expr(QAST_NODE_VOID_TY);
      }

      return stack.top();
    }

    next();

    switch (tok.ty) {
      case qIntL: {
        /**
         * @brief Parse integer literal with type suffix
         */

        stack.push(LOC_121(ConstInt::get(tok.as_string(&rd)), tok));

        tok = peek();
        if (tok.is(qName)) {
          Type *suffix = recurse_type(S, rd);

          Expr *integer = stack.top();
          stack.pop();
          stack.push(LOC_121(
              BinExpr::get(integer, qOpAs, TypeExpr::get(suffix)), tok));
        }
        continue;
      }
      case qNumL: {
        /**
         * @brief Parse floating-point literal with type suffix
         */

        stack.push(LOC_121(ConstFloat::get(tok.as_string(&rd)), tok));

        tok = peek();
        if (tok.is(qName)) {
          Type *suffix = recurse_type(S, rd);

          Expr *num = stack.top();
          stack.pop();
          stack.push(
              LOC_121(BinExpr::get(num, qOpAs, TypeExpr::get(suffix)), tok));
        }
        continue;
      }
      case qText: {
        /**
         * @brief Parse string literal with type suffix
         */

        stack.push(LOC_121(ConstString::get(tok.as_string(&rd)), tok));

        tok = peek();
        if (tok.is(qName)) {
          Type *suffix = recurse_type(S, rd);

          Expr *num = stack.top();
          stack.pop();
          stack.push(
              LOC_121(BinExpr::get(num, qOpAs, TypeExpr::get(suffix)), tok));
        }
        continue;
      }
      case qChar: {
        /**
         * @brief
         */
        auto str = tok.as_string(&rd);
        qcore_assert(str.size() == 1);

        stack.push(LOC_121(ConstChar::get(str.at(0)), tok));

        tok = peek();
        if (tok.is(qName)) {
          Type *suffix = recurse_type(S, rd);

          Expr *num = stack.top();
          stack.pop();
          stack.push(
              LOC_121(BinExpr::get(num, qOpAs, TypeExpr::get(suffix)), tok));
        }
        continue;
      }
      case qKeyW: {
        switch (tok.as<qlex_key_t>()) {
          case qKTrue: {
            stack.push(LOC_121(ConstBool::get(true), tok));
            continue;
          }
          case qKFalse: {
            stack.push(LOC_121(ConstBool::get(false), tok));
            continue;
          }
          case qKNull: {
            stack.push(LOC_121(ConstNull::get(), tok));
            continue;
          }
          case qKUndef: {
            stack.push(LOC_121(ConstUndef::get(), tok));
            continue;
          }
          case qKFn: {
            Stmt *f = recurse_function(S, rd);
            StmtExpr *adapter = StmtExpr::get(f);

            if (peek().is<qPuncLPar>()) {
              next();
              Call *fcall = recurse_function_call(S, adapter, rd, depth);

              if (fcall == nullptr) {
                diagnostic
                    << tok
                    << "Expected a function call after function definition "
                       "expression";
                return mock_expr(QAST_NODE_VOID_TY);
              }

              stack.push(fcall);
              continue;
            }
            stack.push(adapter);
            continue;
          }
          case qK__FString: {
            FString *f = nullptr;
            if (!recurse_fstring(S, &f, rd, depth)) {
              diagnostic << tok << "Expected an F-string in expression";
              return mock_expr(QAST_NODE_VOID_TY);
            }
            stack.push(f);
            continue;
          }

          default: {
            diagnostic << tok << "Unexpected keyword in expression";
            return mock_expr(QAST_NODE_VOID_TY);
          }
        }
        break;
      }
      case qPunc: {
        switch (tok.as<qlex_punc_t>()) {
          case qPuncLPar: {
            if (!stack.empty() && stack.top()->is<Field>()) {
              Call *fcall = recurse_function_call(S, stack.top(), rd, depth);

              if (fcall == nullptr) {
                diagnostic << tok << "Expected a function call in expression";
                return mock_expr(QAST_NODE_VOID_TY);
              }

              stack.pop();
              stack.push(fcall);
              continue;
            }

            Expr *expr = nullptr;
            auto terminators_copy = terminators;
            terminators_copy.insert(qlex_tok_t(qPunc, qPuncRPar));
            expr = recurse_expr(S, rd, terminators_copy, depth + 1);

            if (!next().is<qPuncRPar>()) {
              diagnostic << tok << "Expected ')' to close the parentheses";
              return mock_expr(QAST_NODE_VOID_TY);
            }

            stack.push(expr);
            continue;
          }
          case qPuncRPar: {
            if (stack.size() != 1) {
              diagnostic << tok << "Expected a single expression on the stack";
              return mock_expr(QAST_NODE_VOID_TY);
            }

            return stack.top();
          }
          case qPuncLCur: {
            ListData elements;
            while (true) {
              tok = peek();
              if (tok.is<qPuncRCur>()) {
                next();
                break;
              }

              Expr *key = nullptr, *value = nullptr;
              key = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncColn)},
                                 depth + 1);

              tok = next();
              if (!tok.is<qPuncColn>()) {
                diagnostic << tok << "Expected ':' in list element";
                return mock_expr(QAST_NODE_VOID_TY);
              }

              value = recurse_expr(
                  S, rd,
                  {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRCur)},
                  depth + 1);

              elements.push_back(Assoc::get(key, value));

              tok = peek();
              if (tok.is<qPuncComa>()) {
                next();
              }
            }

            if (elements.size() == 1) {
              stack.push(elements[0]);
            } else {
              stack.push(List::get(elements));
            }

            continue;
          }
          case qPuncLBrk: {
            if (stack.empty()) {
              ListData elements;
              while (true) {
                tok = peek();
                if (tok.is<qPuncRBrk>()) {
                  next();
                  stack.push(List::get(elements));
                  break;
                }

                Expr *element = recurse_expr(
                    S, rd,
                    {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncSemi),
                     qlex_tok_t(qPunc, qPuncRBrk)},
                    depth + 1);

                tok = peek();
                if (tok.is<qPuncSemi>()) {
                  next();

                  Expr *count = recurse_expr(S, rd,
                                             {qlex_tok_t(qPunc, qPuncRBrk),
                                              qlex_tok_t(qPunc, qPuncComa)},
                                             depth + 1);

                  if (!count->is<ConstInt>()) {
                    diagnostic << tok
                               << "Expected a constant integer in list element";
                    return mock_expr(QAST_NODE_VOID_TY);
                  }

                  size_t count_val =
                      std::stoi(count->as<ConstInt>()->get_value().c_str());

                  if (count_val > MAX_LIST_DUP) {
                    diagnostic << tok
                               << "List element duplication count exceeds the "
                                  "maximum limit";
                    return mock_expr(QAST_NODE_VOID_TY);
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
              return mock_expr(QAST_NODE_VOID_TY);
            }

            Expr *index = nullptr, *left = stack.top();
            stack.pop();

            index = recurse_expr(
                S, rd,
                {qlex_tok_t(qPunc, qPuncRBrk), qlex_tok_t(qPunc, qPuncColn)},
                depth + 1);

            tok = next();
            if (tok.is<qPuncColn>()) {
              Expr *end = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncRBrk)},
                                       depth + 1);

              tok = next();
              if (!tok.is<qPuncRBrk>()) {
                diagnostic << tok << "Expected ']' to close the list index";
                return mock_expr(QAST_NODE_VOID_TY);
              }

              stack.push(Slice::get(left, index, end));
              continue;
            }

            if (!tok.is<qPuncRBrk>()) {
              diagnostic << tok << "Expected ']' to close the list index";
              return mock_expr(QAST_NODE_VOID_TY);
            }

            tok = peek();
            if (tok.is<qOpInc>()) {
              PostUnaryExpr *p =
                  PostUnaryExpr::get(Index::get(left, index), qOpInc);
              stack.push(p);
              next();
              continue;
            } else if (tok.is<qOpDec>()) {
              PostUnaryExpr *p =
                  PostUnaryExpr::get(Index::get(left, index), qOpDec);
              stack.push(p);
              next();
              continue;
            }

            stack.push(Index::get(left, index));
            continue;
          }
          case qPuncComa: {
            if (stack.size() != 1) {
              diagnostic << tok << "Expected a single expression on the stack";
              return mock_expr(QAST_NODE_VOID_TY);
            }

            Expr *right = nullptr, *left = stack.top();
            stack.pop();

            right = recurse_expr(S, rd, terminators, depth + 1);

            stack.push(SeqPoint::get(SeqPoint({left, right})));
            continue;
          }
          default: {
            diagnostic << tok << "Unexpected punctuation in expression";
            return mock_expr(QAST_NODE_VOID_TY);
          } break;
        }
      }
      case qOper: {
        qlex_op_t op = tok.as<qlex_op_t>();
        if (op == qOpDot) {
          if (stack.size() != 1) {
            diagnostic << tok << "Expected a single expression on the stack";
            return mock_expr(QAST_NODE_VOID_TY);
          }

          Expr *left = stack.top();
          stack.pop();

          tok = next();
          if (!tok.is(qName)) {
            diagnostic << tok << "Expected an identifier after '.'";
            return mock_expr(QAST_NODE_VOID_TY);
          }

          let ident = tok.as_string(&rd);
          tok = peek();
          if (tok.is<qOpInc>()) {
            PostUnaryExpr *p =
                PostUnaryExpr::get(Field::get(left, ident), qOpInc);
            stack.push(p);
            next();
            continue;
          } else if (tok.is<qOpDec>()) {
            PostUnaryExpr *p =
                PostUnaryExpr::get(Field::get(left, ident), qOpDec);
            stack.push(p);
            next();
            continue;
          }

          stack.push(Field::get(left, ident));
          continue;
        }
        Expr *expr = nullptr;

        if (op == qOpAs) {
          if (stack.size() != 1) {
            diagnostic << tok << "Expected a single expression on the stack";
            return mock_expr(QAST_NODE_VOID_TY);
          }

          Type *type = recurse_type(S, rd);

          Expr *left = stack.top();
          stack.pop();
          stack.push(BinExpr::get(left, qOpAs, TypeExpr::get(type)));
          continue;
        }

        if (op == qOpBitcastAs) {
          if (stack.size() != 1) {
            diagnostic << tok << "Expected a single expression on the stack";
            return mock_expr(QAST_NODE_VOID_TY);
          }

          Type *type = recurse_type(S, rd);

          Expr *left = stack.top();
          stack.pop();
          stack.push(BinExpr::get(left, qOpBitcastAs, TypeExpr::get(type)));
          continue;
        }

        expr = recurse_expr(S, rd, terminators, depth + 1);

        if (stack.empty()) {
          stack.push(UnaryExpr::get((qlex_op_t)op, expr));
          continue;
        } else if (stack.size() == 1) {
          Expr *left = stack.top();
          stack.pop();
          stack.push(BinExpr::get(left, (qlex_op_t)op, expr));
          continue;
        } else {
          diagnostic << tok << "Unexpected operator in expression";
          return mock_expr(QAST_NODE_VOID_TY);
        }
        break;
      }
      case qName: {
        let ident = tok.as_string(&rd);
        if (peek().ty == qPunc && (peek()).as<qlex_punc_t>() == qPuncLPar) {
          next();

          Call *fcall = recurse_function_call(S, Ident::get(ident), rd, depth);
          if (fcall == nullptr) {
            diagnostic << tok << "Expected a function call in expression";
            return mock_expr(QAST_NODE_VOID_TY);
          }

          stack.push(fcall);
          continue;
        } else if (peek().is<qOpInc>()) {
          PostUnaryExpr *p = PostUnaryExpr::get(Ident::get(ident), qOpInc);
          stack.push(p);
          next();
          continue;
        } else if (peek().is<qOpDec>()) {
          PostUnaryExpr *p = PostUnaryExpr::get(Ident::get(ident), qOpDec);
          stack.push(p);
          next();
          continue;
        } else {
          Ident *id = Ident::get(ident);
          id->set_start_pos(tok.start);
          id->set_end_pos(tok.end);
          stack.push(id);
          continue;
        }
      }
      default: {
        diagnostic << tok << "Unexpected token in expression";
        return mock_expr(QAST_NODE_VOID_TY);
      }
    }
  }

  diagnostic << peek() << "Unexpected end of expression";
  return mock_expr(QAST_NODE_VOID_TY);
}
