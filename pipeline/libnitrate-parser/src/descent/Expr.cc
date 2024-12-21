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

#include <charconv>
#include <cstddef>
#include <descent/Recurse.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-parser/AST.hh>
#include <stack>

#include "nitrate-lexer/Token.hh"
#include "nitrate-parser/ASTData.hh"

#define MAX_RECURSION_DEPTH 4096
#define MAX_LIST_REPEAT_COUNT 256

using namespace ncc::lex;
using namespace ncc::parse;
using namespace ncc::core;

CallArgs Parser::recurse_call_arguments(Token terminator) {
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
        let value = recurse_expr({Token(qPunc, qPuncComa), terminator});

        call_args.push_back({name, value});
        break;
      }

      case State::ParsePosArg: {
        let name = SaveString(std::to_string(pos_arg_count++));
        let value = recurse_expr({Token(qPunc, qPuncComa), terminator});

        call_args.push_back({name, value});
        break;
      }
    }

    next_if(qPuncComa);
  }

  return call_args;
}

Expr *Parser::recurse_function_call(Expr *callee) {
  let args = recurse_call_arguments(Token(qPunc, qPuncRPar));
  if (!next_if(qPuncRPar)) {
    diagnostic << current() << "Expected ')' to close the function call";
    return mock_expr(QAST_CALL);
  }

  return make<Call>(callee, args);
}

Expr *Parser::recurse_fstring() {
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
                        ->recurse_expr({Token(qPunc, qPuncRCur)});

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

Expr *Parser::recurse_expr(const std::set<Token> &terminators,
                           int minPrecedence) {
  return recurse_expr_impl(terminators, minPrecedence, 0).value_or(mock_expr());
}

static bool IsPreUnaryOperator(Operator) {
  // Parse everything and analyze the semantics later

  return true;
}

static bool IsPostUnaryOperator(Operator O) {
  return O == qOpInc || O == qOpDec;
}

std::optional<Expr *> Parser::recurse_expr_impl(
    const std::set<Token> &terminators, int minPrecedence, size_t depth) {
  /* Make the fuzzer happy by not crashing */
  if (depth > MAX_RECURSION_DEPTH) {
    diagnostic << current() << "Recursion depth limit exceeded";
    return std::nullopt;
  }

  let start_pos = peek().get_start();

  /****************************************
   * Parse the primary expression
   ****************************************/
  let left_opt = recurse_expr_primary();
  if (!left_opt) {
    diagnostic << current() << "Expected an expression";
    return std::nullopt;
  }
  auto left = left_opt.value();

  /****************************************
   * Recursive processing for operators
   ****************************************/
  while (true) {
    if (terminators.contains(peek())) {
      return left;
    }

    if (let oper_tok = peek(); oper_tok.is(qOper)) {
      let op = oper_tok.as_op();

      // Assumes no overlap between post-unary and binary operators
      // Ternary operators are handled separately
      let op_type =
          IsPostUnaryOperator(op) ? OpMode::PostUnary : OpMode::Binary;

      /****************************************
       * Handle binary operators
       ****************************************/
      let precedence = GetOperatorPrecedence(op, op_type);
      if (precedence < minPrecedence) {
        return left;  // Precedence too low, return current left expression
      } else {
        /* Consume the operator token */
        next();

        /****************************************
         * Handle post-unary operators
         ****************************************/
        if (op_type == OpMode::PostUnary) {
          left = make<PostUnaryExpr>(left, op);
          left->set_offset(start_pos);

          continue;
        }

        let isLeftAssoc =
            GetOperatorAssociativity(op, OpMode::Binary) == OpAssoc::Left;
        let nextMinPrecedence = isLeftAssoc ? precedence + 1 : precedence;

        // Parse the right-hand side with increased precedence
        let right_opt =
            recurse_expr_impl(terminators, nextMinPrecedence, depth + 1);
        if (!right_opt) {
          diagnostic << current()
                     << "RHS expression expected for binary operator";
          return std::nullopt;
        }

        left = make<BinExpr>(left, op, right_opt.value());
        left->set_offset(start_pos);
      }
    } else {
      break;
    }
  }

  return left;
}

std::optional<Expr *> Parser::recurse_expr_primary_keyword(lex::Keyword key) {
  switch (key) {
    case qKScope: {
      diagnostic << current() << "Namespace declaration is not valid here";
      return std::nullopt;
    }

    case qKImport: {
      diagnostic << current() << "Import statement is not valid here";
      return std::nullopt;
    }

    case qKPub: {
      diagnostic << current() << "Public access modifier is not valid here";
      return std::nullopt;
    }

    case qKSec: {
      diagnostic << current() << "Private access modifier is not valid here";
      return std::nullopt;
    }

    case qKPro: {
      diagnostic << current() << "Protected access modifier is not valid here";
      return std::nullopt;
    }

    case qKType: {
      let start_pos = current().get_start();
      let type = recurse_type();
      type->set_offset(start_pos);

      return make<TypeExpr>(type);
    }

    case qKLet: {
      diagnostic << current() << "Let declaration is not valid here";
      return std::nullopt;
    }

    case qKVar: {
      diagnostic << current() << "Var declaration is not valid here";
      return std::nullopt;
    }

    case qKConst: {
      diagnostic << current() << "Const declaration is not valid here";
      return std::nullopt;
    }

    case qKStatic: {
      diagnostic << current() << "Static modifier is not valid here";
      return std::nullopt;
    }

    case qKStruct: {
      diagnostic << current() << "Struct declaration is not valid here";
      return std::nullopt;
    }

    case qKRegion: {
      diagnostic << current() << "Region declaration is not valid here";
      return std::nullopt;
    }

    case qKGroup: {
      diagnostic << current() << "Group declaration is not valid here";
      return std::nullopt;
    }

    case qKClass: {
      diagnostic << current() << "Class declaration is not valid here";
      return std::nullopt;
    }

    case qKUnion: {
      diagnostic << current() << "Union declaration is not valid here";
      return std::nullopt;
    }

    case qKOpaque: {
      diagnostic << current() << "Opaque type is not valid here";
      return std::nullopt;
    }

    case qKEnum: {
      diagnostic << current() << "Enum declaration is not valid here";
      return std::nullopt;
    }

    case qK__FString: {
      return recurse_fstring();
    }

    case qKFn: {
      let start_pos = current().get_start();

      let function = recurse_function(false);
      function->set_offset(start_pos);

      Expr *expr = make<StmtExpr>(function);

      if (next_if(qPuncLPar)) {
        let args = recurse_call_arguments(Token(qPunc, qPuncRPar));
        if (!next_if(qPuncRPar)) {
          diagnostic << current() << "Expected ')' to close the function call";
          return mock_expr(QAST_CALL);
        }

        expr = make<Call>(expr, args);
      }

      return expr;
    }

    case qKUnsafe: {
      diagnostic << current() << "Unsafe keyword is not valid here";
      return std::nullopt;
    }

    case qKSafe: {
      diagnostic << current() << "Safe keyword is not valid here";
      return std::nullopt;
    }

    case qKPromise: {
      diagnostic << current() << "Promise keyword is not valid here";
      return std::nullopt;
    }

    case qKIf: {
      diagnostic << current() << "If statement is not valid here";
      return std::nullopt;
    }

    case qKElse: {
      diagnostic << current() << "Else statement is not valid here";
      return std::nullopt;
    }

    case qKFor: {
      diagnostic << current() << "For loop is not valid here";
      return std::nullopt;
    }

    case qKWhile: {
      diagnostic << current() << "While loop is not valid here";
      return std::nullopt;
    }

    case qKDo: {
      diagnostic << current() << "Do statement is not valid here";
      return std::nullopt;
    }

    case qKSwitch: {
      diagnostic << current() << "Switch statement is not valid here";
      return std::nullopt;
    }

    case qKBreak: {
      diagnostic << current() << "Break statement is not valid here";
      return std::nullopt;
    }

    case qKContinue: {
      diagnostic << current() << "Continue statement is not valid here";
      return std::nullopt;
    }

    case qKReturn: {
      diagnostic << current() << "Return statement is not valid here";
      return std::nullopt;
    }

    case qKRetif: {
      diagnostic << current() << "Retif statement is not valid here";
      return std::nullopt;
    }

    case qKForeach: {
      diagnostic << current() << "Foreach loop is not valid here";
      return std::nullopt;
    }

    case qKTry: {
      diagnostic << current() << "Try statement is not valid here";
      return std::nullopt;
    }

    case qKCatch: {
      diagnostic << current() << "Catch statement is not valid here";
      return std::nullopt;
    }

    case qKThrow: {
      diagnostic << current() << "Throw statement is not valid here";
      return std::nullopt;
    }

    case qKAsync: {
      diagnostic << current() << "Async statement is not valid here";
      return std::nullopt;
    }

    case qKAwait: {
      diagnostic << current() << "Await statement is not valid here";
      return std::nullopt;
    }

    case qK__Asm__: {
      diagnostic << current() << "Inline assembly is not valid here";
      return std::nullopt;
    }

    case qKUndef: {
      return make<ConstUndef>();
    }

    case qKNull: {
      return make<ConstNull>();
    }

    case qKTrue: {
      return make<ConstBool>(true);
    }

    case qKFalse: {
      return make<ConstBool>(false);
    }
  }
}

std::optional<Expr *> Parser::recurse_expr_primary_punctor(lex::Punctor punc) {
  switch (punc) {
    case qPuncLPar: {
      let expr = recurse_expr({Token(qPunc, qPuncRPar)});
      if (!next_if(qPuncRPar)) {
        diagnostic << current() << "Expected ')' to close the expression";
        return std::nullopt;
      }

      return expr;
    }

    case qPuncRPar: {
      diagnostic << current() << "Unexpected right parenthesis in expression";
      return std::nullopt;
    }

    case qPuncLBrk: {
      ExpressionList items;

      while (true) {
        if (next_if(qEofF)) {
          diagnostic << current()
                     << "Unexpected end of file while parsing expression";
          return std::nullopt;
        }

        if (next_if(qPuncRBrk)) {
          break;
        }

        let expr = recurse_expr({
            Token(qPunc, qPuncComa),
            Token(qPunc, qPuncRBrk),
            Token(qPunc, qPuncSemi),
        });

        if (next_if(qPuncSemi)) {
          if (let count_tok = next_if(qIntL)) {
            let count_str = current().as_string();
            size_t count;
            if (std::from_chars(count_str.data(),
                                count_str.data() + count_str.size(), count)
                    .ec == std::errc()) {
              if (count > MAX_LIST_REPEAT_COUNT) {
                diagnostic << current()
                           << "List element repeat count exceeds maximum limit";
                return std::nullopt;
              } else {
                for (size_t i = 0; i < count; i++) {
                  items.push_back(expr);
                }
              }

            } else {
              diagnostic
                  << current()
                  << "Expected a valid integer literal for the list element "
                     "repeat count";
              return std::nullopt;
            }
          } else {
            diagnostic << current()
                       << "Expected an integer literal for the list element "
                          "repeat count";
            return std::nullopt;
          }
        } else {
          items.push_back(expr);
        }

        next_if(qPuncComa);
      }

      return make<List>(std::move(items));
    }

    case qPuncRBrk: {
      diagnostic << current() << "Unexpected right bracket in expression";
      return std::nullopt;
    }

    case qPuncLCur: {
      ExpressionList items;

      while (true) {
        if (next_if(qEofF)) {
          diagnostic << current()
                     << "Unexpected end of file while parsing dictionary";
          return std::nullopt;
        }

        if (next_if(qPuncRCur)) {
          break;
        }

        let start_pos = peek().get_start();

        let key = recurse_expr({Token(qPunc, qPuncColn)});
        if (!next_if(qPuncColn)) {
          diagnostic << current() << "Expected colon after key in dictionary";
          return std::nullopt;
        }

        let value = recurse_expr({
            Token(qPunc, qPuncRCur),
            Token(qPunc, qPuncComa),
        });

        next_if(qPuncComa);

        let assoc = make<Assoc>(key, value);
        assoc->set_offset(start_pos);

        items.push_back(assoc);
      }

      return make<List>(std::move(items));
    }

    case qPuncRCur: {
      diagnostic << current() << "Unexpected right curly brace in expression";
      return std::nullopt;
    }

    case qPuncComa: {
      diagnostic << current() << "Comma is not valid in this context";
      return std::nullopt;
    }

    case qPuncColn: {
      diagnostic << current() << "Colon is not valid in this context";
      return std::nullopt;
    }

    case qPuncSemi: {
      diagnostic << current() << "Semicolon is not valid in this context";
      return std::nullopt;
    }
  }
}

std::optional<Expr *> Parser::recurse_expr_primary() {
  let start_pos = peek().get_start();

  /****************************************
   * Parse pre-unary operators
   ****************************************/
  std::vector<Token> preUnaryOps;
  while (true) {
    let oper_tok = peek();

    if (!oper_tok.is(qOper) || !IsPreUnaryOperator(oper_tok.as_op())) {
      break;
    }

    /* Consume the operator token */
    next();

    preUnaryOps.push_back(oper_tok);
  }

  std::optional<Expr *> E;

  switch (let tok = next(); tok.get_type()) {
    case qEofF: {
      diagnostic << tok << "Unexpected end of file while parsing expression";
      break;
    }

    case qKeyW: {
      if ((E = recurse_expr_primary_keyword(tok.as_key())).has_value()) {
        E.value()->set_offset(start_pos);
      }

      break;
    }

    case qOper: {
      diagnostic << tok << "Unexpected operator in expression";
      break;
    }

    case qPunc: {
      if ((E = recurse_expr_primary_punctor(tok.as_punc())).has_value()) {
        E.value()->set_offset(start_pos);
      }
      break;
    }

    case qName: {
      let identifier = make<Ident>(intern(tok.as_string()));
      identifier->set_offset(start_pos);

      E = identifier;

      break;
    }

    case qIntL: {
      let integer = make<ConstInt>(intern(tok.as_string()));
      integer->set_offset(start_pos);

      E = integer;
      break;
    }

    case qNumL: {
      let decimal = make<ConstFloat>(intern(tok.as_string()));
      decimal->set_offset(start_pos);

      E = decimal;
      break;
    }

    case qText: {
      let string = make<ConstString>(intern(tok.as_string()));
      string->set_offset(start_pos);

      E = string;
      break;
    }

    case qChar: {
      let str_data = tok.as_string();
      if (str_data.size() != 1) [[unlikely]] {
        diagnostic << tok << "Expected a single byte in character literal";
        break;
      }

      let character = make<ConstChar>(str_data[0]);
      character->set_offset(start_pos);

      E = character;
      break;
    }

    case qMacB: {
      diagnostic << tok << "Unexpected macro block in expression";
      break;
    }

    case qMacr: {
      diagnostic << tok << "Unexpected macro call in expression";
      break;
    }

    case qNote: {
      diagnostic << tok << "Unexpected comment in expression";
      break;
    }
  }

  if (E.has_value()) {
    /****************************************
     * Process syntax sugar
     ****************************************/

    while (true) {
      if (next_if(qPuncLPar)) {
        E = recurse_function_call(E.value());

        /// TODO: Add support template'd function calls
      } else if (next_if(qPuncLBrk)) {
        let first = recurse_expr({
            Token(qPunc, qPuncRBrk),
            Token(qPunc, qPuncColn),
        });

        if (next_if(qPuncColn)) {
          let second = recurse_expr({Token(qPunc, qPuncRBrk)});
          if (!next_if(qPuncRBrk)) {
            diagnostic << current() << "Expected ']' to close the slice";
            return std::nullopt;
          }

          let slice = make<Slice>(E.value(), first, second);
          slice->set_offset(start_pos);

          E = slice;
        } else {
          if (!next_if(qPuncRBrk)) {
            diagnostic << current() << "Expected ']' to close the index access";
            return std::nullopt;
          }

          let index = make<Index>(E.value(), first);
          index->set_offset(start_pos);

          E = index;
        }
      } else {
        break;
      }
    }

    /****************************************
     * Process the pre-unary operators
     ****************************************/
    for (auto it = preUnaryOps.rbegin(); it != preUnaryOps.rend(); ++it) {
      E = make<UnaryExpr>(it->as_op(), E.value());
      E.value()->set_offset(start_pos);
    }
  }

  return E;
}
