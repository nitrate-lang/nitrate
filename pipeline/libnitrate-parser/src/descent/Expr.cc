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

std::optional<Expr *> Parser::recurse_expr_impl(
    const std::set<Token> &terminators, int minPrecedence) {
  let start_pos = peek().get_start();

  /// TODO: Handle pre-unary, post-unary, and ternary operators in the future
  let opmode = OpMode::Binary;

  let left_opt = recurse_expr_primary();
  if (!left_opt) {
    diagnostic << current() << "Expected an expression";
    return std::nullopt;
  }

  auto left = left_opt.value();

  struct Frame {
    Expr *left;
    Token oper_tok;
    int nextMinPrecedence;
  };

  std::stack<Frame> stack;

  while (true) {
    if (terminators.contains(peek())) {
      // Process the stack to combine binary expressions
      while (!stack.empty()) {
        auto frame = stack.top();
        stack.pop();
        left = make<BinExpr>(frame.left, frame.oper_tok.as_op(), left);
        left->set_offset(start_pos);
      }
      return left;
    }

    if (let oper_tok = peek(); oper_tok.is(qOper)) {
      let op = oper_tok.as_op();
      let precedence = GetOperatorPrecedence(op, opmode);

      if (precedence < minPrecedence) {
        // Process the stack to combine binary expressions
        while (!stack.empty()) {
          let frame = stack.top();
          stack.pop();
          left = make<BinExpr>(frame.left, frame.oper_tok.as_op(), left);
          left->set_offset(start_pos);
        }

        return left;
      } else {
        next();  // Consume the operator token
      }

      let isLeftAssoc = GetOperatorAssociativity(op, opmode) == OpAssoc::Left;
      let nextMinPrecedence = isLeftAssoc ? precedence + 1 : precedence;

      let right_opt = recurse_expr_primary();  // Parse the right-hand operand
      if (!right_opt) {
        diagnostic << current()
                   << "Failed to parse right-hand side of binary expression";

        return std::nullopt;
      }

      if (stack.size() > MAX_RECURSION_DEPTH) {
        diagnostic
            << current()
            << "Maximum recursion depth reached while parsing expression";

        return std::nullopt;
      }

      // Push the current state to the stack
      stack.push({left, oper_tok, nextMinPrecedence});
      left = right_opt.value();
    } else {
      break;
    }
  }

  // Process remaining stack
  while (!stack.empty()) {
    let frame = stack.top();
    stack.pop();
    left = make<BinExpr>(frame.left, frame.oper_tok.as_op(), left);
    left->set_offset(start_pos);
  }

  return left;
}

std::optional<Expr *> Parser::recurse_expr_primary_keyword(lex::Keyword key) {
  switch (key) {
    case qKScope: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKImport: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKPub: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKSec: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKPro: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKType: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKLet: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKVar: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKConst: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKStatic: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKStruct: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKRegion: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKGroup: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKClass: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKUnion: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKOpaque: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKEnum: {
      /// TODO: Implement this case
      qcore_implement();
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
      /// TODO: Implement this case
      qcore_implement();
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
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKWhile: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKDo: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKSwitch: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKBreak: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKContinue: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKReturn: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKRetif: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKForeach: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKTry: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKCatch: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKThrow: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKAsync: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKAwait: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qK__Asm__: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKUndef: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKNull: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKTrue: {
      /// TODO: Implement this case
      qcore_implement();
    }

    case qKFalse: {
      /// TODO: Implement this case
      qcore_implement();
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
  let tok = next();
  let start_pos = tok.get_start();

  switch (tok.get_type()) {
    case qEofF: {
      diagnostic << tok << "Unexpected end of file while parsing expression";
      return std::nullopt;
    }

    case qKeyW: {
      return recurse_expr_primary_keyword(tok.as_key());
    }

    case qOper: {
      diagnostic << tok << "Unexpected operator in expression";
      return std::nullopt;
    }

    case qPunc: {
      return recurse_expr_primary_punctor(tok.as_punc());
    }

    case qName: {
      let identifier = make<Ident>(intern(tok.as_string()));
      identifier->set_offset(start_pos);

      return identifier;
    }

    case qIntL: {
      let integer = make<ConstInt>(intern(tok.as_string()));
      integer->set_offset(start_pos);

      return integer;
    }

    case qNumL: {
      let decimal = make<ConstFloat>(intern(tok.as_string()));
      decimal->set_offset(start_pos);

      return decimal;
    }

    case qText: {
      let string = make<ConstString>(intern(tok.as_string()));
      string->set_offset(start_pos);

      return string;
    }

    case qChar: {
      let str_data = tok.as_string();
      if (str_data.size() != 1) [[unlikely]] {
        diagnostic << tok << "Expected a single byte in character literal";
        return std::nullopt;
      }

      let character = make<ConstChar>(str_data[0]);
      character->set_offset(start_pos);

      return character;
    }

    case qMacB: {
      diagnostic << tok << "Unexpected macro block in expression";
      return std::nullopt;
    }

    case qMacr: {
      diagnostic << tok << "Unexpected macro call in expression";
      return std::nullopt;
    }

    case qNote: {
      diagnostic << tok << "Unexpected comment in expression";
      return std::nullopt;
    }
  }
}
