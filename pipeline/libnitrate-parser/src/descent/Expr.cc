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

#include "nitrate-lexer/Lexer.hh"
#include "nitrate-lexer/Token.hh"

#define MAX_RECURSION_DEPTH 4096
#define MAX_LIST_REPEAT_COUNT 4096

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;
using namespace ncc;

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

FlowPtr<Expr> Parser::recurse_fstring() {
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

  return make<FString>(std::move(items))();
}

static bool IsPostUnaryOp(Operator O) { return O == qOpInc || O == qOpDec; }

enum class FrameType : uint8_t {
  Start,
  PreUnary,
  PostUnary,
  Binary,
};

struct Frame {
  FlowPtr<Expr> base;
  LocationID start_pos;
  short minPrecedence;
  FrameType type;
  Operator op;

  Frame(FlowPtr<Expr> base, LocationID start_pos, short minPrecedence,
        FrameType type, Operator op)
      : base(base),
        start_pos(start_pos),
        minPrecedence(minPrecedence),
        type(type),
        op(op) {}
};

static FORCE_INLINE FlowPtr<Expr> UnwindStack(std::stack<Frame> &stack,
                                              FlowPtr<Expr> base,
                                              short minPrecedence) {
  while (!stack.empty()) {
    auto frame = stack.top();

    if (frame.minPrecedence < minPrecedence) {
      break;
    }

    switch (frame.type) {
      case FrameType::Start: {
        return base;
      }

      case FrameType::PreUnary: {
        base = make<UnaryExpr>(frame.op, base)();
        break;
      }

      case FrameType::PostUnary: {
        base = make<PostUnaryExpr>(base, frame.op)();
        break;
      }

      case FrameType::Binary: {
        base = make<BinExpr>(frame.base, frame.op, base)();
        break;
      }
    }

    base->set_offset(frame.start_pos);
    stack.pop();
  }

  return base;
}

FlowPtr<Expr> Parser::recurse_expr(const std::set<Token> &terminators) {
  let SourceOffset = peek().get_start();

  std::stack<Frame> Stack;
  Stack.push({nullptr, SourceOffset, 0, FrameType::Start, qOpPlus});

  /****************************************
   * Parse pre-unary operators
   ****************************************/
  std::stack<std::pair<Operator, LocationID>> PreUnaryOps;
  while (let Tok = next_if(qOper)) {
    PreUnaryOps.push({Tok->as_op(), Tok->get_start()});
  }

  if (let LeftSideOpt = recurse_expr_primary(false)) {
    auto LeftSide = LeftSideOpt.value();
    bool Spinning = true;

    /****************************************
     * Combine pre-unary operators
     ****************************************/
    while (!PreUnaryOps.empty()) {
      let[Op, Offset] = PreUnaryOps.top();
      PreUnaryOps.pop();

      LeftSide = make<UnaryExpr>(Op, LeftSide)();
      LeftSide->set_offset(Offset);
    }

    while (!Stack.empty() && Spinning) {
      let Tok = peek();

      if (terminators.contains(Tok)) {
        break;
      }

      switch (Tok.get_type()) {
        case qOper: {
          let Op = Tok.as_op();
          let OpType = IsPostUnaryOp(Op) ? OpMode::PostUnary : OpMode::Binary;
          let OpPrecedence = GetOperatorPrecedence(Op, OpType);

          if (OpPrecedence >= Stack.top().minPrecedence) {
            next();

            /****************************************
             * Handle post-unary operators
             ****************************************/
            if (OpType == OpMode::PostUnary) {
              LeftSide = make<PostUnaryExpr>(LeftSide, Op)();
              LeftSide->set_offset(SourceOffset);

              continue;
            }

            let IsLeftAssoc =
                GetOperatorAssociativity(Op, OpType) == OpAssoc::Left;
            let NextMinPrecedence =
                IsLeftAssoc ? OpPrecedence + 1 : OpPrecedence;
            bool IsType = Op == qOpAs || Op == qOpBitcastAs;

            if (!IsType) {
              /****************************************
               * Parse pre-unary operators
               ****************************************/
              while (let Tok = next_if(qOper)) {
                PreUnaryOps.push({Tok->as_op(), Tok->get_start()});
              }
            }

            /****************************************
             * Handle binary operators
             ****************************************/
            if (auto RightSide = recurse_expr_primary(IsType)) {
              /****************************************
               * Combine pre-unary operators
               ****************************************/
              while (!PreUnaryOps.empty()) {
                let[Op, Offset] = PreUnaryOps.top();
                PreUnaryOps.pop();

                auto PreUnaryExpr = make<UnaryExpr>(Op, RightSide.value())();
                PreUnaryExpr->set_offset(Offset);

                RightSide = PreUnaryExpr;
              }

              if (Stack.size() + 1 > MAX_RECURSION_DEPTH) {
                diagnostic << current()
                           << "Recursion depth exceeds maximum limit";
                return mock_expr();
              }

              Stack.push(Frame(LeftSide, SourceOffset, NextMinPrecedence,
                               FrameType::Binary, Op));
              LeftSide = RightSide.value();
            } else {
              diagnostic
                  << current()
                  << "Failed to parse right-hand side of binary expression";
            }
          } else {
            LeftSide = UnwindStack(Stack, LeftSide, 0);

            continue;
          }

          break;
        }

        case qPunc: {
          // Based on the assumption that function calls have the same
          // precedence as the dot operator (member access)
          static let SuffixOPPrecedence =
              GetOperatorPrecedence(qOpDot, OpMode::Binary);
          LeftSide = UnwindStack(Stack, LeftSide, SuffixOPPrecedence);

          if (next_if(qPuncLPar)) {
            let Arguments = recurse_call_arguments(Token(qPunc, qPuncRPar));
            if (!next_if(qPuncRPar)) {
              diagnostic << current()
                         << "Expected ')' to close the function call";
            }

            LeftSide = make<Call>(LeftSide, std::move(Arguments))();
            LeftSide->set_offset(SourceOffset);
          } else if (next_if(qPuncLBrk)) {
            let first = recurse_expr({
                Token(qPunc, qPuncRBrk),
                Token(qPunc, qPuncColn),
            });

            if (next_if(qPuncColn)) {
              let second = recurse_expr({Token(qPunc, qPuncRBrk)});
              if (!next_if(qPuncRBrk)) {
                diagnostic << current() << "Expected ']' to close the slice";
              }

              LeftSide = make<Slice>(LeftSide, first, second)();
              LeftSide->set_offset(SourceOffset);
            } else {
              if (!next_if(qPuncRBrk)) {
                diagnostic << current()
                           << "Expected ']' to close the index expression";
              }

              LeftSide = make<Index>(LeftSide, first)();
              LeftSide->set_offset(SourceOffset);
            }
          } else {  // Not part of the expression
            Spinning = false;
          }

          break;
        }

        case qEofF:
        case qKeyW:
        case qName:
        case qIntL:
        case qNumL:
        case qText:
        case qChar:
        case qMacB:
        case qMacr:
        case qNote: {
          Spinning = false;
          break;
        }
      }
    }

    return UnwindStack(Stack, LeftSide, 0);
  } else {
    diagnostic << current() << "Expected an expression";

    return mock_expr();
  }
}

NullableFlowPtr<Expr> Parser::recurse_expr_keyword(lex::Keyword key) {
  NullableFlowPtr<Expr> E;

  switch (key) {
    case qKScope: {
      diagnostic << current() << "Namespace declaration is not valid here";
      break;
    }

    case qKImport: {
      diagnostic << current() << "Import statement is not valid here";
      break;
    }

    case qKPub: {
      diagnostic << current() << "Public access modifier is not valid here";
      break;
    }

    case qKSec: {
      diagnostic << current() << "Private access modifier is not valid here";
      break;
    }

    case qKPro: {
      diagnostic << current() << "Protected access modifier is not valid here";
      break;
    }

    case qKType: {
      let start_pos = current().get_start();
      let type = recurse_type();
      type->set_offset(start_pos);

      E = make<TypeExpr>(type)();
      break;
    }

    case qKLet: {
      diagnostic << current() << "Let declaration is not valid here";
      break;
    }

    case qKVar: {
      diagnostic << current() << "Var declaration is not valid here";
      break;
    }

    case qKConst: {
      diagnostic << current() << "Const declaration is not valid here";
      break;
    }

    case qKStatic: {
      diagnostic << current() << "Static modifier is not valid here";
      break;
    }

    case qKStruct: {
      diagnostic << current() << "Struct declaration is not valid here";
      break;
    }

    case qKRegion: {
      diagnostic << current() << "Region declaration is not valid here";
      break;
    }

    case qKGroup: {
      diagnostic << current() << "Group declaration is not valid here";
      break;
    }

    case qKClass: {
      diagnostic << current() << "Class declaration is not valid here";
      break;
    }

    case qKUnion: {
      diagnostic << current() << "Union declaration is not valid here";
      break;
    }

    case qKOpaque: {
      diagnostic << current() << "Opaque type is not valid here";
      break;
    }

    case qKEnum: {
      diagnostic << current() << "Enum declaration is not valid here";
      break;
    }

    case qK__FString: {
      E = recurse_fstring();
      break;
    }

    case qKFn: {
      let start_pos = current().get_start();

      let function = recurse_function(false);
      function->set_offset(start_pos);

      FlowPtr<Expr> expr = make<StmtExpr>(function)();

      if (next_if(qPuncLPar)) {
        let args = recurse_call_arguments(Token(qPunc, qPuncRPar));
        if (next_if(qPuncRPar)) {
          E = make<Call>(expr, args)();
        } else {
          diagnostic << current() << "Expected ')' to close the function call";
          E = mock_expr(QAST_CALL);
        }
      }

      break;
    }

    case qKUnsafe: {
      diagnostic << current() << "Unsafe keyword is not valid here";
      break;
    }

    case qKSafe: {
      diagnostic << current() << "Safe keyword is not valid here";
      break;
    }

    case qKPromise: {
      diagnostic << current() << "Promise keyword is not valid here";
      break;
    }

    case qKIf: {
      diagnostic << current() << "If statement is not valid here";
      break;
    }

    case qKElse: {
      diagnostic << current() << "Else statement is not valid here";
      break;
    }

    case qKFor: {
      diagnostic << current() << "For loop is not valid here";
      break;
    }

    case qKWhile: {
      diagnostic << current() << "While loop is not valid here";
      break;
    }

    case qKDo: {
      diagnostic << current() << "Do statement is not valid here";
      break;
    }

    case qKSwitch: {
      diagnostic << current() << "Switch statement is not valid here";
      break;
    }

    case qKBreak: {
      diagnostic << current() << "Break statement is not valid here";
      break;
    }

    case qKContinue: {
      diagnostic << current() << "Continue statement is not valid here";
      break;
    }

    case qKReturn: {
      diagnostic << current() << "Return statement is not valid here";
      break;
    }

    case qKRetif: {
      diagnostic << current() << "Retif statement is not valid here";
      break;
    }

    case qKForeach: {
      diagnostic << current() << "Foreach loop is not valid here";
      break;
    }

    case qKTry: {
      diagnostic << current() << "Try statement is not valid here";
      break;
    }

    case qKCatch: {
      diagnostic << current() << "Catch statement is not valid here";
      break;
    }

    case qKThrow: {
      diagnostic << current() << "Throw statement is not valid here";
      break;
    }

    case qKAsync: {
      diagnostic << current() << "Async statement is not valid here";
      break;
    }

    case qKAwait: {
      diagnostic << current() << "Await statement is not valid here";
      break;
    }

    case qK__Asm__: {
      diagnostic << current() << "Inline assembly is not valid here";
      break;
    }

    case qKUndef: {
      E = make<ConstUndef>()();
      break;
    }

    case qKNull: {
      E = make<ConstNull>()();
      break;
    }

    case qKTrue: {
      E = make<ConstBool>(true)();
      break;
    }

    case qKFalse: {
      E = make<ConstBool>(false)();
      break;
    }
  }

  return E;
}

NullableFlowPtr<Expr> Parser::recurse_expr_punctor(lex::Punctor punc) {
  NullableFlowPtr<Expr> E;

  switch (punc) {
    case qPuncLPar: {
      E = recurse_expr({Token(qPunc, qPuncRPar)});
      if (!next_if(qPuncRPar)) {
        diagnostic << current() << "Expected ')' to close the expression";
      }

      break;
    }

    case qPuncRPar: {
      diagnostic << current() << "Unexpected right parenthesis in expression";
      break;
    }

    case qPuncLBrk: {
      ExpressionList items;

      while (true) {
        if (next_if(qEofF)) {
          diagnostic << current()
                     << "Unexpected end of file while parsing expression";
          break;
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
            size_t count{};
            if (std::from_chars(count_str.data(),
                                count_str.data() + count_str.size(), count)
                    .ec == std::errc()) {
              if (count <= MAX_LIST_REPEAT_COUNT) {
                for (size_t i = 0; i < count; i++) {
                  items.push_back(expr);
                }
              } else {
                diagnostic << current()
                           << "Compressed list size exceeds maximum limit";
              }

            } else {
              diagnostic << current()
                         << "Expected an integer literal for the compressed "
                            "list size";
            }
          } else {
            diagnostic << current()
                       << "Expected an integer literal for the compressed list "
                          "size";
          }
        } else {
          items.push_back(expr);
        }

        next_if(qPuncComa);
      }

      E = make<List>(std::move(items))();
      break;
    }

    case qPuncRBrk: {
      diagnostic << current() << "Unexpected right bracket in expression";
      break;
    }

    case qPuncLCur: {
      ExpressionList items;

      while (true) {
        if (next_if(qEofF)) {
          diagnostic << current()
                     << "Unexpected end of file while parsing dictionary";
          break;
        }

        if (next_if(qPuncRCur)) {
          break;
        }

        let start_pos = peek().get_start();

        let key = recurse_expr({Token(qPunc, qPuncColn)});
        if (!next_if(qPuncColn)) {
          diagnostic << current() << "Expected colon after key in dictionary";
          break;
        }

        let value = recurse_expr({
            Token(qPunc, qPuncRCur),
            Token(qPunc, qPuncComa),
        });

        next_if(qPuncComa);

        auto assoc = make<Assoc>(key, value)();
        assoc->set_offset(start_pos);

        items.push_back(assoc);
      }

      E = make<List>(std::move(items))();
      break;
    }

    case qPuncRCur: {
      diagnostic << current() << "Unexpected right curly brace in expression";
      break;
    }

    case qPuncComa: {
      diagnostic << current() << "Comma is not valid in this context";
      break;
    }

    case qPuncColn: {
      diagnostic << current() << "Colon is not valid in this context";
      break;
    }

    case qPuncSemi: {
      diagnostic << current() << "Semicolon is not valid in this context";
      break;
    }
  }

  return E;
}

FlowPtr<Expr> Parser::recurse_expr_type_suffix(FlowPtr<Expr> base) {
  let tok = current();

  let suffix = recurse_type();
  suffix->set_offset(tok.get_start());

  auto texpr = make<TypeExpr>(suffix)();
  texpr->set_offset(tok.get_start());

  return make<BinExpr>(base, qOpAs, texpr)();
}

NullableFlowPtr<Expr> Parser::recurse_expr_primary(bool isType) {
  let start_pos = peek().get_start();
  NullableFlowPtr<Expr> E;

  if (isType) {
    let type = recurse_type();
    type->set_offset(start_pos);

    auto texpr = make<TypeExpr>(type)();
    texpr->set_offset(start_pos);

    E = texpr;
  } else {
    switch (let tok = next(); tok.get_type()) {
      case qEofF: {
        break;
      }

      case qKeyW: {
        if ((E = recurse_expr_keyword(tok.as_key())).has_value()) {
          E.value()->set_offset(start_pos);
        }

        break;
      }

      case qOper: {
        diagnostic << tok << "Unexpected operator in expression";
        break;
      }

      case qPunc: {
        if ((E = recurse_expr_punctor(tok.as_punc())).has_value()) {
          E.value()->set_offset(start_pos);
        }
        break;
      }

      case qName: {
        auto identifier = make<Ident>(intern(tok.as_string()))();
        identifier->set_offset(start_pos);

        E = identifier;
        break;
      }

      case qIntL: {
        auto integer = make<ConstInt>(intern(tok.as_string()))();
        integer->set_offset(start_pos);

        if (let tok = peek(); tok.is(qName)) {
          let casted = recurse_expr_type_suffix(integer);
          casted->set_offset(start_pos);

          E = casted;
        } else {
          E = integer;
        }

        break;
      }

      case qNumL: {
        auto decimal = make<ConstFloat>(intern(tok.as_string()))();
        decimal->set_offset(start_pos);

        if (let tok = peek(); tok.is(qName)) {
          let casted = recurse_expr_type_suffix(decimal);
          casted->set_offset(start_pos);

          E = casted;
        } else {
          E = decimal;
        }

        break;
      }

      case qText: {
        auto string = make<ConstString>(intern(tok.as_string()))();
        string->set_offset(start_pos);

        if (let tok = peek(); tok.is(qName)) {
          let casted = recurse_expr_type_suffix(string);
          casted->set_offset(start_pos);

          E = casted;
        } else {
          E = string;
        }

        break;
      }

      case qChar: {
        let str_data = tok.as_string();
        if (str_data.size() != 1) [[unlikely]] {
          diagnostic << tok << "Expected a single byte in character literal";
          break;
        }

        auto character = make<ConstChar>(str_data[0])();
        character->set_offset(start_pos);

        if (let tok = peek(); tok.is(qName)) {
          let casted = recurse_expr_type_suffix(character);
          casted->set_offset(start_pos);

          E = casted;
        } else {
          E = character;
        }

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
  }

  return E;
}
