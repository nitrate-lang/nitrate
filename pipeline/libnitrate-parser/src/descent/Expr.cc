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

#include <cstddef>
#include <descent/Recurse.hh>
#include <nitrate-parser/AST.hh>
#include <sstream>
#include <stack>

#define MAX_EXPR_DEPTH (10000)
#define MAX_LIST_DUP (10000)

using namespace ncc::parse;

CallArgs Parser::recurse_caller_arguments(NCCToken terminator, size_t depth) {
  NCCToken tok, ident;
  CallArgs call_args;
  size_t pos_arg_count = 0;

  while (true) {
    tok = peek();

    if (tok.is(qEofF)) {
      diagnostic << tok << "Unexpected end of file while parsing function call";
      return call_args;
    }

    if (tok == terminator) {
      break;
    }

    if (!tok.is(qName)) {
      goto parse_pos_arg;
    }

    { /* Parse named argument */
      ident = tok;
      next();
      tok = peek();

      if (!tok.is<qPuncColn>()) {
        qlex_insert(&rd, tok);
        qlex_insert(&rd, ident);
        goto parse_pos_arg;
      }

      next();

      Expr *arg =
          recurse_expr({NCCToken(qPunc, qPuncComa), terminator}, depth + 1);

      call_args.push_back({SaveString(ident.as_string(&rd)), arg});
      goto comma;
    }

  parse_pos_arg: {
    Expr *arg =
        recurse_expr({NCCToken(qPunc, qPuncComa), terminator}, depth + 1);

    call_args.push_back({SaveString(std::to_string(pos_arg_count++)), arg});

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

  return call_args;
}

Call *Parser::recurse_function_call(Expr *callee, size_t depth) {
  auto args = recurse_caller_arguments(NCCToken(qPunc, qPuncRPar), depth);
  if (!next_if(qPuncRPar)) {
    diagnostic << current() << "Expected ')' to close the function call";
    return nullptr;
  }
  return make<Call>(callee, args);
}

bool Parser::recurse_fstring(FString **node, size_t depth) {
  /**
   * @brief Parse an F-string expression
   * @return true if it is okay to proceed, false otherwise
   */

  NCCToken tok = next();
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

      NCCLexer subrd = NCCLexer(ss, "fstring", m_env);

      /// TODO: Fstring is broken

      expr = recurse_expr({NCCToken(qPunc, qPuncRCur)}, depth + 1);

      if (!tmp.empty()) {
        items.push_back(SaveString(std::move(tmp)));
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
    items.push_back(SaveString(std::move(tmp)));
    tmp.clear();
  }

  if (state != 0) {
    diagnostic << tok << "F-string expression is not properly closed with '}'";
  }

  *node = make<FString>(std::move(items));

  return true;
}

/// TODO: qlex_op_t precedence
/// TODO: qlex_op_t associativity

Expr *Parser::recurse_expr(std::set<NCCToken> terminators, size_t depth) {
  if (depth > MAX_EXPR_DEPTH) {
    diagnostic
        << peek()
        << "Expression depth exceeded; Expressions can not be nested more than "
        << MAX_EXPR_DEPTH << " times";

    return mock_expr(QAST_VOID);
  }

  std::stack<Expr *> stack;

  while (true) {
    NCCToken tok = peek();

    if (tok.is(qEofF)) {
      // diagnostic << tok << "Unexpected end of file while parsing expression";
      return mock_expr(QAST_VOID);
    }

    if (terminators.contains(tok)) {
      if (stack.empty()) {
        return mock_expr(QAST_VOID);
      }

      if (stack.size() != 1) {
        diagnostic << tok << "Expected a single expression on the stack";
        return mock_expr(QAST_VOID);
      }

      return stack.top();
    }

    next();

    switch (tok.ty) {
      case qIntL: {
        /**
         * @brief Parse integer literal with type suffix
         */

        stack.push(make<ConstInt>(SaveString(tok.as_string(&rd))));

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

        stack.push(make<ConstFloat>(SaveString(tok.as_string(&rd))));

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

        stack.push(make<ConstString>(SaveString(tok.as_string(&rd))));

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
        auto str = tok.as_string(&rd);
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
        switch (tok.as<qlex_key_t>()) {
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
              Call *fcall = recurse_function_call(adapter, depth);

              if (fcall == nullptr) {
                diagnostic
                    << tok
                    << "Expected a function call after function definition "
                       "expression";
                return mock_expr(QAST_VOID);
              }

              stack.push(fcall);
              continue;
            }
            stack.push(adapter);
            continue;
          }
          case qK__FString: {
            FString *f = nullptr;
            if (!recurse_fstring(&f, depth)) {
              diagnostic << tok << "Expected an F-string in expression";
              return mock_expr(QAST_VOID);
            }
            stack.push(f);
            continue;
          }

          default: {
            diagnostic << tok << "Unexpected keyword in expression";
            return mock_expr(QAST_VOID);
          }
        }
        break;
      }
      case qPunc: {
        switch (tok.as<qlex_punc_t>()) {
          case qPuncLPar: {
            if (!stack.empty() && stack.top()->is<Field>()) {
              Call *fcall = recurse_function_call(stack.top(), depth);

              if (fcall == nullptr) {
                diagnostic << tok << "Expected a function call in expression";
                return mock_expr(QAST_VOID);
              }

              stack.pop();
              stack.push(fcall);
              continue;
            }

            Expr *expr = nullptr;
            auto terminators_copy = terminators;
            terminators_copy.insert(NCCToken(qPunc, qPuncRPar));
            expr = recurse_expr(terminators_copy, depth + 1);

            if (!next().is<qPuncRPar>()) {
              diagnostic << tok << "Expected ')' to close the parentheses";
              return mock_expr(QAST_VOID);
            }

            stack.push(expr);
            continue;
          }
          case qPuncRPar: {
            if (stack.size() != 1) {
              diagnostic << tok << "Expected a single expression on the stack";
              return mock_expr(QAST_VOID);
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
              key = recurse_expr({NCCToken(qPunc, qPuncColn)}, depth + 1);

              tok = next();
              if (!tok.is<qPuncColn>()) {
                diagnostic << tok << "Expected ':' in list element";
                return mock_expr(QAST_VOID);
              }

              value = recurse_expr(

                  {NCCToken(qPunc, qPuncComa), NCCToken(qPunc, qPuncRCur)},
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

                    {NCCToken(qPunc, qPuncComa), NCCToken(qPunc, qPuncSemi),
                     NCCToken(qPunc, qPuncRBrk)},
                    depth + 1);

                tok = peek();
                if (tok.is<qPuncSemi>()) {
                  next();

                  Expr *count = recurse_expr(
                      {NCCToken(qPunc, qPuncRBrk), NCCToken(qPunc, qPuncComa)},
                      depth + 1);

                  if (!count->is<ConstInt>()) {
                    diagnostic << tok
                               << "Expected a constant integer in list element";
                    return mock_expr(QAST_VOID);
                  }

                  size_t count_val = std::stoi(
                      std::string(count->as<ConstInt>()->get_value()));

                  if (count_val > MAX_LIST_DUP) {
                    diagnostic << tok
                               << "List element duplication count exceeds the "
                                  "maximum limit";
                    return mock_expr(QAST_VOID);
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
              return mock_expr(QAST_VOID);
            }

            Expr *index = nullptr, *left = stack.top();
            stack.pop();

            index = recurse_expr(

                {NCCToken(qPunc, qPuncRBrk), NCCToken(qPunc, qPuncColn)},
                depth + 1);

            tok = next();
            if (tok.is<qPuncColn>()) {
              Expr *end = recurse_expr({NCCToken(qPunc, qPuncRBrk)}, depth + 1);

              tok = next();
              if (!tok.is<qPuncRBrk>()) {
                diagnostic << tok << "Expected ']' to close the list index";
                return mock_expr(QAST_VOID);
              }

              stack.push(make<Slice>(left, index, end));
              continue;
            }

            if (!tok.is<qPuncRBrk>()) {
              diagnostic << tok << "Expected ']' to close the list index";
              return mock_expr(QAST_VOID);
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
              return mock_expr(QAST_VOID);
            }

            Expr *right = nullptr, *left = stack.top();
            stack.pop();

            right = recurse_expr(terminators, depth + 1);

            stack.push(make<SeqPoint>(SeqPoint({left, right})));
            continue;
          }
          default: {
            diagnostic << tok << "Unexpected punctuation in expression";
            return mock_expr(QAST_VOID);
          } break;
        }
      }
      case qOper: {
        qlex_op_t op = tok.as<qlex_op_t>();
        if (op == qOpDot) {
          if (stack.size() != 1) {
            diagnostic << tok << "Expected a single expression on the stack";
            return mock_expr(QAST_VOID);
          }

          Expr *left = stack.top();
          stack.pop();

          tok = next();
          if (!tok.is(qName)) {
            diagnostic << tok << "Expected an identifier after '.'";
            return mock_expr(QAST_VOID);
          }

          let ident = tok.as_string(&rd);
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
            return mock_expr(QAST_VOID);
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
            return mock_expr(QAST_VOID);
          }

          Type *type = recurse_type();

          Expr *left = stack.top();
          stack.pop();
          stack.push(make<BinExpr>(left, qOpBitcastAs, make<TypeExpr>(type)));
          continue;
        }

        expr = recurse_expr(terminators, depth + 1);

        if (stack.empty()) {
          stack.push(make<UnaryExpr>((qlex_op_t)op, expr));
          continue;
        } else if (stack.size() == 1) {
          Expr *left = stack.top();
          stack.pop();
          stack.push(make<BinExpr>(left, (qlex_op_t)op, expr));
          continue;
        } else {
          diagnostic << tok << "Unexpected operator in expression";
          return mock_expr(QAST_VOID);
        }
        break;
      }
      case qName: {
        let ident = tok.as_string(&rd);
        if (peek().ty == qPunc && (peek()).as<qlex_punc_t>() == qPuncLPar) {
          next();

          Call *fcall =
              recurse_function_call(make<Ident>(SaveString(ident)), depth);
          if (fcall == nullptr) {
            diagnostic << tok << "Expected a function call in expression";
            return mock_expr(QAST_VOID);
          }

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
          id->set_offset(tok.start);

          stack.push(id);
          continue;
        }
      }
      default: {
        diagnostic << tok << "Unexpected token in expression";
        return mock_expr(QAST_VOID);
      }
    }
  }

  diagnostic << peek() << "Unexpected end of expression";
  return mock_expr(QAST_VOID);
}
