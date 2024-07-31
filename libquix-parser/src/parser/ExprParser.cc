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

#define __QUIX_IMPL__

#include <stack>

#include "LibMacro.h"
#include "parser/Parse.h"

#define MAX_EXPR_DEPTH (10000)

using namespace qparse;
using namespace qparse::parser;

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
        
        qlex_push(rd, ident);
        qlex_push(rd, tok);
        goto parse_pos_arg;
      }

      qlex_next(rd);

      Expr *arg = nullptr;
      if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRPar)}, &arg,
                      depth + 1)) {
        /**
         * @brief
         */

        return nullptr;
      }

      call_args.push_back({ident.as_string(rd), arg});
      goto comma;
    }

  parse_pos_arg: {
    /**
     * @brief
     */

    Expr *arg = nullptr;
    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRPar)}, &arg,
                    depth + 1)) {
      /**
       * @brief
       */

      return nullptr;
    }

    /**
     * @brief
     */
    call_args.push_back({"__" + std::to_string(pos_arg_count++), arg});

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
  /// TODO: Implement FStrings again
  // throw std::runtime_error("FStrings are not implemented yet");
  return false;

  /**
   * @brief
   */

  qlex_tok_t tok = qlex_next(rd);
  std::string fstr, rectified_template;
  FStringArgs args;
  size_t state = 0, w_beg = 0, w_end = 0;
  Expr *expr = nullptr;

  if (!tok.is(qText)) {
    /// TODO: Write the ERROR message
    return false;
  }

  fstr = tok.as_string(rd);

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

      // NOT POSS: StringLexer subrd(sub);

      // if (!parse_expr(job, &subrd, {qlex_tok_t(qPunc, qPuncRCur)}, &expr, 0)) {
      //   return false;
      // }

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
    /// TODO: Write the ERROR message
    return false;
  }

  *node = FString::get(rectified_template, args);

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
    return false;
  }

  std::stack<Expr *> stack;

  while (true) {
    qlex_tok_t tok = qlex_peek(rd);

    if (tok.is(qEofF)) {
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
        /// TODO: Write the ERROR message
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

        stack.push(ConstInt::get(tok.as_string(rd)));
        continue;
      }
      case qNumL: {
        /**
         * @brief
         */

        stack.push(ConstFloat::get(tok.as_string(rd)));
        continue;
      }
      case qText: {
        /**
         * @brief
         */

        stack.push(ConstString::get(tok.as_string(rd)));
        continue;
      }
      case qChar: {
        /**
         * @brief
         */

        stack.push(ConstChar::get(tok.as_string(rd)));
        continue;
      }
      case qKeyW: {
        switch (tok.as<qlex_key_t>()) {
          case qKTrue: {
            stack.push(ConstBool::get(true));
            continue;
          }
          case qKFalse: {
            stack.push(ConstBool::get(false));
            continue;
          }
          case qKNull: {
            stack.push(ConstNull::get());
            continue;
          }
          case qKUndef: {
            stack.push(ConstUndef::get());
            continue;
          }
          case qKFn: {
            Stmt *f = nullptr;
            if (!parse_function(job, rd, &f)) {
              return false;
            }
            StmtExpr *adapter = StmtExpr::get(f);

            if (qlex_peek(rd).is<qPuncLPar>()) {
              qlex_next(rd);
              Call *fcall = parse_function_call(job, adapter, rd, depth);

              if (fcall == nullptr) {
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
              return false;
            }
            stack.push(f);
            continue;
          }

          default: {
            /// TODO: Write the ERROR message
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
                return false;
              }

              stack.pop();
              stack.push(fcall);
              continue;
            }

            Expr *expr = nullptr;
            if (!parse_expr(job, rd, terminators, &expr, depth + 1)) {
              return false;
            }

            stack.push(expr);
            continue;
          }
          case qPuncRPar: {
            if (stack.size() != 1) {
              /// TODO: Write the ERROR message
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
              if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncColn)}, &key, depth + 1)) {
                return false;
              }

              tok = qlex_next(rd);
              if (!tok.is<qPuncColn>()) {
                /// TODO: Write the ERROR message
                return false;
              }

              if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRCur)},
                              &value, depth + 1)) {
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
                                &element, depth + 1)) {
                  return false;
                }

                tok = qlex_peek(rd);
                if (tok.is<qPuncSemi>()) {
                  qlex_next(rd);

                  Expr *count = nullptr;
                  if (!parse_expr(job, rd,
                                  {qlex_tok_t(qPunc, qPuncRBrk), qlex_tok_t(qPunc, qPuncComa)},
                                  &count, depth + 1) || !count) {
                    return false;
                  }

                  if (!count->is<ConstInt>()) {
                    /// TODO: Write the ERROR message
                    return false;
                  }

                  size_t count_val = std::atoi(count->as<ConstInt>()->get_value().c_str());
                  for (size_t i = 0; i < count_val; i++) {
                    elements.push_back(element);
                  }
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
              /// TODO: Write the ERROR message
              return false;
            }

            Expr *index = nullptr, *left = stack.top();
            stack.pop();

            if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncRBrk), qlex_tok_t(qPunc, qPuncColn)},
                            &index, depth + 1)) {
              return false;
            }

            tok = qlex_next(rd);
            if (tok.is<qPuncColn>()) {
              Expr *end = nullptr;
              if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncRBrk)}, &end, depth + 1)) {
                return false;
              }

              tok = qlex_next(rd);
              if (!tok.is<qPuncRBrk>()) {
                /// TODO: Write the ERROR message
                return false;
              }

              stack.push(Slice::get(left, index, end));
              continue;
            }

            if (!tok.is<qPuncRBrk>()) {
              /// TODO: Write the ERROR message
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
              /// TODO: Write the ERROR message
              return false;
            }

            Expr *right = nullptr, *left = stack.top();
            stack.pop();

            if (!parse_expr(job, rd, terminators, &right, depth + 1)) {
              return false;
            }

            stack.push(SeqPoint::get(SeqPoint({left, right})));
            continue;
          }
          default: {
            /// TODO: Write the ERROR message
            return false;
          } break;
        }
      }
      case qOper: {
        qlex_op_t op = tok.as<qlex_op_t>();
        if (op == qOpDot) {
          if (stack.size() != 1) {
            /// TODO: Write the ERROR message
            return false;
          }

          Expr *left = stack.top();
          stack.pop();

          tok = qlex_next(rd);
          if (!tok.is(qName)) {
            /// TODO: Write the ERROR message
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
            /// TODO: Write the ERROR message
            return false;
          }

          Type *type = nullptr;
          if (!parse_type(job, rd, &type)) {
            return false;
          }

          Expr *left = stack.top();
          stack.pop();
          stack.push(BinExpr::get(left, qOpAs, TypeExpr::get(type)));
          continue;
        }

        if (op == qOpBitcastAs) {
          if (stack.size() != 1) {
            /// TODO: Write the ERROR message
            return false;
          }

          Type *type = nullptr;
          if (!parse_type(job, rd, &type)) {
            return false;
          }

          Expr *left = stack.top();
          stack.pop();
          stack.push(BinExpr::get(left, qOpBitcastAs, TypeExpr::get(type)));
          continue;
        }

        if (op == qOpReinterpretAs) {
          if (stack.size() != 1) {
            /// TODO: Write the ERROR message
            return false;
          }

          Type *type = nullptr;
          if (!parse_type(job, rd, &type)) {
            return false;
          }

          Expr *left = stack.top();
          stack.pop();

          stack.push(BinExpr::get(left, qOpReinterpretAs, TypeExpr::get(type)));
          continue;
        }

        if (!parse_expr(job, rd, terminators, &expr, depth + 1) || !expr) {
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
          /// TODO: Write the ERROR message
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
          stack.push(Ident::get(ident));
          continue;
        }
      }
      default: {
        /// TODO: Write the ERROR message
        return false;
      }
    }
  }

  return false;
}
