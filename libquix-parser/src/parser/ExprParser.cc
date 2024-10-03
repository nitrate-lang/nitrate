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

/// TODO: Source location
#define __QUIX_LEXER_IMPL__
#define __QUIX_IMPL__

#include <parser/Parse.h>
#include <quix-parser/Node.h>

#include <cstddef>
#include <stack>
#include <stdexcept>
#include <string_view>

#define MAX_EXPR_DEPTH (10000)
#define MAX_LIST_DUP (10000)

static inline qparse::Expr *LOC_121(qparse::Expr *p, qlex_tok_t t) {
  p->set_start_pos(t.start);
  p->set_end_pos(t.end);
  return p;
}

using namespace qparse;
using namespace qparse::parser;
using namespace qparse::diag;

static Call *parse_function_call(qparse_t &job, Expr *callee, qlex_t *rd, size_t depth) {
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

    tok = qlex_peek(rd);

    if (tok.is<qPuncRPar>()) {
      /**
       * @brief
       */

      qlex_next(rd);
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
      qlex_next(rd);
      tok = qlex_peek(rd);

      if (!tok.is<qPuncColn>()) {
        /**
         * @brief
         */

        qlex_insert(rd, ident);
        qlex_insert(rd, tok);
        goto parse_pos_arg;
      }

      qlex_next(rd);

      Expr *arg = nullptr;
      if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRPar)}, &arg,
                      depth + 1) ||
          !arg) {
        syntax(tok, "Expected an expression in named function call argument");

        return nullptr;
      }

      call_args.push_back({ident.as_string(rd), arg});
      goto comma;
    }

  parse_pos_arg: {
    Expr *arg = nullptr;
    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRPar)}, &arg,
                    depth + 1) ||
        !arg) {
      syntax(tok, "Expected an expression in positional function call argument");

      return nullptr;
    }

    call_args.push_back({std::to_string(pos_arg_count++), arg});

    goto comma;
  }

  comma: {
    tok = qlex_peek(rd);
    if (tok.is<qPuncComa>()) {
      qlex_next(rd);
    }
    continue;
  }
  }

  return Call::get(callee, call_args);
}

static bool parse_fstring(qparse_t &job, FString **node, qlex_t *rd, size_t depth) {
  /**
   * @brief Parse an F-string expression
   * @return true if it is okay to proceed, false otherwise
   */

  qlex_tok_t tok;
  std::string tmp;
  std::string_view fstr;
  FStringItems items;
  size_t state = 0, w_beg = 0, w_end = 0;
  Expr *expr = nullptr;

  tok = qlex_next(rd);
  if (!tok.is(qText)) {
    syntax(tok, "Expected a string literal in F-string expression");
  }

  {
    size_t len;
    const char *ptr = qlex_str(rd, &tok, &len);
    fstr = std::string_view(ptr, len);
  }

  tmp.reserve(fstr.size());

  for (size_t i = 0; i < fstr.size(); i++) {
    char c = fstr[i];

    if (c == '{' && state == 0) {
      w_beg = i + 1;
      state = 1;
    } else if (c == '}' && state == 1) {
      w_end = i + 1;
      state = 0;

      std::string_view sub = fstr.substr(w_beg, w_end - w_beg);

      qlex_t *subrd = qlex_direct(sub.data(), sub.size(), "fstring", job.env);
      qlex_tok_t subtok = qlex_peek(subrd);

      if (!parse_expr(job, subrd, {qlex_tok_t(qPunc, qPuncRCur)}, &expr, depth + 1) || !expr) {
        syntax(subtok, "Expected an expression in F-string parameter");
        qlex_free(subrd);
        return false;
      }

      qlex_free(subrd);

      if (!tmp.empty()) {
        items.push_back(std::move(tmp));
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
  }

  if (state != 0) {
    syntax(tok, "F-string expression is not properly closed with '}'");
  }

  *node = FString::get(std::move(items));

  return true;
}

/// TODO: qlex_op_t precedence
/// TODO: qlex_op_t associativity

bool qparse::parser::parse_expr(qparse_t &job, qlex_t *rd, std::set<qlex_tok_t> terminators,
                                Expr **node, size_t depth) {
  /**
   * @brief
   */

  if (depth > MAX_EXPR_DEPTH) {
    syntax(qlex_peek(rd),
           "Expression depth exceeded; Expressions can not be nested more than %d times",
           MAX_EXPR_DEPTH);
    return false;
  }

  std::stack<Expr *> stack;

  while (true) {
    qlex_tok_t tok = qlex_peek(rd);

    if (tok.is(qEofF)) {
      syntax(tok, "Unexpected end of file while parsing expression");
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
        syntax(tok, "Expected a single expression on the stack");
        return false;
      }

      /* */
      *node = stack.top();
      return true;
    }

    qlex_next(rd);

    switch (tok.ty) {
      case qIntL: {
        /**
         * @brief
         */

        stack.push(LOC_121(ConstInt::get(tok.as_string(rd)), tok));
        continue;
      }
      case qNumL: {
        /**
         * @brief
         */

        stack.push(LOC_121(ConstFloat::get(tok.as_string(rd)), tok));
        continue;
      }
      case qText: {
        /**
         * @brief
         */

        stack.push(LOC_121(ConstString::get(tok.as_string(rd)), tok));
        continue;
      }
      case qChar: {
        /**
         * @brief
         */
        auto str = tok.as_string(rd);
        if (str.size() > 4) {
          syntax(tok, "Invalid character literal");
          return false;
        }
        str.resize(4, '\0');

        char32_t v = 0;
        for (size_t i = 0; i < 4; i++) {
          v = (v << 8) | str[i];
        }

        stack.push(LOC_121(ConstChar::get(v), tok));
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
            Stmt *f = nullptr;
            if (!parse_function(job, rd, &f)) {
              syntax(tok, "Expected a function definition in expression");
              return false;
            }
            StmtExpr *adapter = StmtExpr::get(f);

            if (qlex_peek(rd).is<qPuncLPar>()) {
              qlex_next(rd);
              Call *fcall = parse_function_call(job, adapter, rd, depth);

              if (fcall == nullptr) {
                syntax(tok, "Expected a function call after function definition expression");
                return false;
              }

              stack.push(fcall);
              continue;
            }
            stack.push(adapter);
            continue;
          }
          case qKFString: {
            FString *f = nullptr;
            if (!parse_fstring(job, &f, rd, depth)) {
              syntax(tok, "Expected an F-string in expression");
              return false;
            }
            stack.push(f);
            continue;
          }

          default: {
            syntax(tok, "Unexpected keyword in expression");
            return false;
          }
        }
        break;
      }
      case qPunc: {
        switch (tok.as<qlex_punc_t>()) {
          case qPuncLPar: {
            if (!stack.empty() && stack.top()->is<Field>()) {
              Call *fcall = parse_function_call(job, stack.top(), rd, depth);

              if (fcall == nullptr) {
                syntax(tok, "Expected a function call in expression");
                return false;
              }

              stack.pop();
              stack.push(fcall);
              continue;
            }

            Expr *expr = nullptr;
            if (!parse_expr(job, rd, terminators, &expr, depth + 1) || !expr) {
              syntax(tok, "Expected an expression in parentheses");
              return false;
            }

            stack.push(expr);
            continue;
          }
          case qPuncRPar: {
            if (stack.size() != 1) {
              syntax(tok, "Expected a single expression on the stack");
              return false;
            }

            *node = stack.top();
            stack.pop();
            return true;
          }
          case qPuncLCur: {
            ListData elements;
            while (true) {
              tok = qlex_peek(rd);
              if (tok.is<qPuncRCur>()) {
                qlex_next(rd);
                break;
              }

              Expr *key = nullptr, *value = nullptr;
              if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncColn)}, &key, depth + 1) || !key) {
                syntax(tok, "Expected a key in list element");
                return false;
              }

              tok = qlex_next(rd);
              if (!tok.is<qPuncColn>()) {
                syntax(tok, "Expected ':' in list element");
                return false;
              }

              if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRCur)},
                              &value, depth + 1) ||
                  !value) {
                syntax(tok, "Expected a value in list element");
                return false;
              }

              elements.push_back(Assoc::get(key, value));

              tok = qlex_peek(rd);
              if (tok.is<qPuncComa>()) {
                qlex_next(rd);
              }
            }

            stack.push(List::get(elements));
            continue;
          }
          case qPuncLBrk: {
            if (stack.empty()) {
              ListData elements;
              while (true) {
                tok = qlex_peek(rd);
                if (tok.is<qPuncRBrk>()) {
                  qlex_next(rd);
                  stack.push(List::get(elements));
                  break;
                }

                Expr *element = nullptr;
                if (!parse_expr(job, rd,
                                {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncSemi),
                                 qlex_tok_t(qPunc, qPuncRBrk)},
                                &element, depth + 1) ||
                    !element) {
                  syntax(tok, "Expected an element in list");
                  return false;
                }

                tok = qlex_peek(rd);
                if (tok.is<qPuncSemi>()) {
                  qlex_next(rd);

                  Expr *count = nullptr;
                  if (!parse_expr(job, rd,
                                  {qlex_tok_t(qPunc, qPuncRBrk), qlex_tok_t(qPunc, qPuncComa)},
                                  &count, depth + 1) ||
                      !count) {
                    syntax(tok, "Expected a count in list element");
                    return false;
                  }

                  if (!count->is<ConstInt>()) {
                    syntax(tok, "Expected a constant integer in list element");
                    return false;
                  }
                  size_t count_val = 0;

                  try {
                    count_val = std::stoi(count->as<ConstInt>()->get_value().c_str());
                  } catch (std::out_of_range &) {
                    syntax(tok, "Expected a constant integer in list element. std::stoi() failed");
                    return false;
                  }

                  if (count_val > MAX_LIST_DUP) {
                    syntax(tok, "List element duplication count exceeds the maximum limit");
                    return false;
                  }

                  for (size_t i = 0; i < count_val; i++) {
                    elements.push_back(element);
                  }

                  tok = qlex_peek(rd);
                } else if (element) {
                  elements.push_back(element);
                }

                if (tok.is<qPuncComa>()) {
                  qlex_next(rd);
                }
              }

              continue;
            }

            if (stack.size() != 1) {
              syntax(tok, "Expected a single expression on the stack");
              return false;
            }

            Expr *index = nullptr, *left = stack.top();
            stack.pop();

            if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncRBrk), qlex_tok_t(qPunc, qPuncColn)},
                            &index, depth + 1) ||
                !index) {
              syntax(tok, "Expected an index in list");
              return false;
            }

            tok = qlex_next(rd);
            if (tok.is<qPuncColn>()) {
              Expr *end = nullptr;
              if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncRBrk)}, &end, depth + 1) || !end) {
                syntax(tok, "Expected an end index in list");
                return false;
              }

              tok = qlex_next(rd);
              if (!tok.is<qPuncRBrk>()) {
                syntax(tok, "Expected ']' to close the list index");
                return false;
              }

              stack.push(Slice::get(left, index, end));
              continue;
            }

            if (!tok.is<qPuncRBrk>()) {
              syntax(tok, "Expected ']' to close the list index");
              return false;
            }

            tok = qlex_peek(rd);
            if (tok.is<qOpInc>()) {
              PostUnaryExpr *p = PostUnaryExpr::get(Index::get(left, index), qOpInc);
              stack.push(p);
              qlex_next(rd);
              continue;
            } else if (tok.is<qOpDec>()) {
              PostUnaryExpr *p = PostUnaryExpr::get(Index::get(left, index), qOpDec);
              stack.push(p);
              qlex_next(rd);
              continue;
            }

            stack.push(Index::get(left, index));
            continue;
          }
          case qPuncComa: {
            if (stack.size() != 1) {
              syntax(tok, "Expected a single expression on the stack");
              return false;
            }

            Expr *right = nullptr, *left = stack.top();
            stack.pop();

            if (!parse_expr(job, rd, terminators, &right, depth + 1) || !right) {
              syntax(tok, "Expected an expression after ','");
              return false;
            }

            stack.push(SeqPoint::get(SeqPoint({left, right})));
            continue;
          }
          default: {
            syntax(tok, "Unexpected punctuation in expression");
            return false;
          } break;
        }
      }
      case qOper: {
        qlex_op_t op = tok.as<qlex_op_t>();
        if (op == qOpDot) {
          if (stack.size() != 1) {
            syntax(tok, "Expected a single expression on the stack");
            return false;
          }

          Expr *left = stack.top();
          stack.pop();

          tok = qlex_next(rd);
          if (!tok.is(qName)) {
            syntax(tok, "Expected an identifier after '.'");
            return false;
          }

          std::string ident = tok.as_string(rd);
          tok = qlex_peek(rd);
          if (tok.is<qOpInc>()) {
            PostUnaryExpr *p = PostUnaryExpr::get(Field::get(left, ident), qOpInc);
            stack.push(p);
            qlex_next(rd);
            continue;
          } else if (tok.is<qOpDec>()) {
            PostUnaryExpr *p = PostUnaryExpr::get(Field::get(left, ident), qOpDec);
            stack.push(p);
            qlex_next(rd);
            continue;
          }

          stack.push(Field::get(left, ident));
          continue;
        }
        Expr *expr = nullptr;

        if (op == qOpAs) {
          if (stack.size() != 1) {
            syntax(tok, "Expected a single expression on the stack");
            return false;
          }

          Type *type = nullptr;
          if (!parse_type(job, rd, &type)) {
            syntax(tok, "Failed to parse type in 'as' expression");
            return false;
          }

          Expr *left = stack.top();
          stack.pop();
          stack.push(BinExpr::get(left, qOpAs, TypeExpr::get(type)));
          continue;
        }

        if (op == qOpBitcastAs) {
          if (stack.size() != 1) {
            syntax(tok, "Expected a single expression on the stack");
            return false;
          }

          Type *type = nullptr;
          if (!parse_type(job, rd, &type)) {
            syntax(tok, "Failed to parse type in 'bitcast as' expression");
            return false;
          }

          Expr *left = stack.top();
          stack.pop();
          stack.push(BinExpr::get(left, qOpBitcastAs, TypeExpr::get(type)));
          continue;
        }

        if (!parse_expr(job, rd, terminators, &expr, depth + 1) || !expr) {
          syntax(tok, "Failed to parse expression in binary operation");
          return false;
        }

        if (stack.empty()) {
          stack.push(UnaryExpr::get((qlex_op_t)op, expr));
          continue;
        } else if (stack.size() == 1) {
          Expr *left = stack.top();
          stack.pop();
          stack.push(BinExpr::get(left, (qlex_op_t)op, expr));
          continue;
        } else {
          syntax(tok, "Unexpected operator in expression");
          return false;
        }
        break;
      }
      case qName: {
        std::string ident = tok.as_string(rd);
        if (qlex_peek(rd).ty == qPunc && (qlex_peek(rd)).as<qlex_punc_t>() == qPuncLPar) {
          qlex_next(rd);

          Call *fcall = parse_function_call(job, Ident::get(ident), rd, depth);
          if (fcall == nullptr) {
            syntax(tok, "Expected a function call in expression");
            return false;
          }

          stack.push(fcall);
          continue;
        } else if (qlex_peek(rd).is<qOpInc>()) {
          PostUnaryExpr *p = PostUnaryExpr::get(Ident::get(ident), qOpInc);
          stack.push(p);
          qlex_next(rd);
          continue;
        } else if (qlex_peek(rd).is<qOpDec>()) {
          PostUnaryExpr *p = PostUnaryExpr::get(Ident::get(ident), qOpDec);
          stack.push(p);
          qlex_next(rd);
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
        syntax(tok, "Unexpected token in expression");
        return false;
      }
    }
  }

  syntax(qlex_peek(rd), "Unexpected end of expression");
  return false;
}
