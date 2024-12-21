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

#include <cstddef>
#include <descent/Recurse.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-parser/AST.hh>
#include <stack>

#define MAX_RECURSION_DEPTH 4096

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
  return recurse_expr_impl(terminators, minPrecedence).value_or(mock_expr());
}

static bool IsPreUnaryOperator(Operator) {
  // Parse everything and analyze the semantics later

  return true;
}

static bool IsPostUnaryOperator(Operator) {
  // Parse everything and analyze the semantics later

  return true;
}

std::optional<Expr *> Parser::recurse_expr_impl(
    const std::set<Token> &terminators, int minPrecedence) {
  let start_pos = peek().get_start();

  struct Frame {
    Expr *left;
    Token oper_tok;
    int nextMinPrecedence;
  };

  /****************************************
   * Parse the primary expression
   ****************************************/
  let left_opt = recurse_expr_primary();
  if (!left_opt) {
    diagnostic << current() << "Expected an expression";

    return std::nullopt;
  }
  auto left = left_opt.value();

  /* Parse expressions non-recursively */
  std::stack<Frame> stack;

  while (true) {
    if (!terminators.contains(peek())) {
      if (let oper_tok = peek(); oper_tok.is(qOper)) {
        let op = oper_tok.as_op();

        /****************************************
         * Handle post-unary operators
         ****************************************/
        if (IsPostUnaryOperator(op)) {
          let precedence = GetOperatorPrecedence(op, OpMode::PostUnary);
          if (precedence >= minPrecedence) {
            left = make<PostUnaryExpr>(left, op);
            left->set_offset(start_pos);

            /* Consume the operator token */
            next();

            continue;
          }
        }

        /****************************************************************
         * Handle binary operators
         ****************************************************************/
        let precedence = GetOperatorPrecedence(op, OpMode::Binary);
        if (precedence < minPrecedence) {
          /* Process the stack to combine binary expressions */
          while (!stack.empty()) {
            auto frame = stack.top();
            stack.pop();
            left = make<BinExpr>(frame.left, frame.oper_tok.as_op(), left);
            left->set_offset(start_pos);
          }

          return left;
        } else {
          /* Consume the operator token */
          next();
        }

        let isLeftAssoc =
            GetOperatorAssociativity(op, OpMode::Binary) == OpAssoc::Left;
        let nextMinPrecedence = isLeftAssoc ? precedence + 1 : precedence;

        /****************************************************************
         * Parse the right-hand operand
         ****************************************************************/
        let right_opt = recurse_expr_primary();
        if (!right_opt) {
          diagnostic << current()
                     << "Failed to parse right-hand side of binary expression";

          return std::nullopt;
        }

        stack.push({left, oper_tok, nextMinPrecedence});
        left = right_opt.value();
      } else {
        break;
      }
    } else { /* Terminator break */
      break;
    }
  }

  /* Process the stack to combine binary expressions */
  while (!stack.empty()) {
    auto frame = stack.top();
    stack.pop();
    left = make<BinExpr>(frame.left, frame.oper_tok.as_op(), left);
    left->set_offset(start_pos);
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
      diagnostic << current() << "Type declaration is not valid here";
      return std::nullopt;
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
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKFn: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKUnsafe: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKSafe: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKPromise: {
      diagnostic << current() << "Promise keyword is not valid here";
      return std::nullopt;
    }

    case qKIf: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKElse: {
      /// TODO: Implement this case
      qcore_implement();
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
      let undef = make<ConstUndef>();
      undef->set_offset(current().get_start());

      return undef;
    }

    case qKNull: {
      let null = make<ConstNull>();
      null->set_offset(current().get_start());

      return null;
    }

    case qKTrue: {
      let boolean = make<ConstBool>(true);
      boolean->set_offset(current().get_start());

      return boolean;
    }

    case qKFalse: {
      let boolean = make<ConstBool>(false);
      boolean->set_offset(current().get_start());

      return boolean;
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
      /// TODO: Implement this case
      qcore_implement();
    }

    case qPuncRBrk: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qPuncLCur: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qPuncRCur: {
      /// TODO: Implement this case
      qcore_implement();
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

  let tok = next();
  let start_pos = tok.get_start();
  std::optional<Expr *> E;

  switch (tok.get_type()) {
    case qEofF: {
      diagnostic << tok << "Unexpected end of file while parsing expression";
      break;
    }

    case qKeyW: {
      E = recurse_expr_primary_keyword(tok.as_key());
      break;
    }

    case qOper: {
      diagnostic << tok << "Unexpected operator in expression";
      break;
    }

    case qPunc: {
      E = recurse_expr_primary_punctor(tok.as_punc());
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
     * Process the pre-unary operators
     ****************************************/
    for (auto it = preUnaryOps.rbegin(); it != preUnaryOps.rend(); ++it) {
      E = make<UnaryExpr>(it->as_op(), E.value());
      E.value()->set_offset(start_pos);
    }
  }

  return E;
}
