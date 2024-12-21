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
#include <nitrate-core/Logger.hh>
#include <nitrate-parser/AST.hh>

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

  /// TODO: Handle pre-unary, post-unary, and ternary operators
  let opmode = OpMode::Binary;

  let left_opt = recurse_expr_primary();
  if (!left_opt) {
    diagnostic << current() << "Expected an expression";
    return std::nullopt;
  }

  auto left = left_opt.value();

  while (true) {
    if (terminators.contains(peek())) {
      return left;
    }

    if (let oper_tok = peek(); oper_tok.is(qOper)) {
      let op = oper_tok.as_op();

      let precedence = GetOperatorPrecedence(op, opmode);

      if (precedence < minPrecedence) {
        return left;
      } else {
        next();
      }

      let isLeftAssoc = GetOperatorAssociativity(op, opmode) == OpAssoc::Left;

      let nextMinPrecedence = isLeftAssoc ? precedence + 1 : precedence;
      if (let right = recurse_expr_impl(terminators, nextMinPrecedence)) {
        left = make<BinExpr>(left, op, right.value());
        left->set_offset(start_pos);
      } else {
        diagnostic << current()
                   << "Failed to parse right-hand side of binary expression";
        return std::nullopt;
      }

    } else {
      break;
    }
  }

  return left;
}

std::optional<Expr *> Parser::recurse_expr_primary() {
  if (let tok = next_if(qIntL)) {
    return make<ConstInt>(intern(tok->as_string()));
  } else {
    diagnostic << next() << "Expected an integer literal";
    return std::nullopt;
  }

  /// TODO: Implement this function
}
