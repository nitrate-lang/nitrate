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
#include <descent/Recurse.hh>
#include <stack>

#define MAX_RECURSION_DEPTH 4096
#define MAX_LIST_REPEAT_COUNT 4096

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;
using namespace ncc;

CallArgs Parser::recurse_call_arguments(Token terminator) {
  CallArgs call_args;
  size_t positional_index = 0;
  string argument_name;

  while (true) {
    if (next_if(EofF)) [[unlikely]] {
      diagnostic << current()
                 << "Unexpected end of file while parsing call expression";
      return call_args;
    }

    if (peek() == terminator) {
      break;
    }

    auto some_identifier = next_if(Name);
    auto next_is_colon = some_identifier && next_if(PuncColn).has_value();

    if (some_identifier && next_is_colon) {
      argument_name = some_identifier->as_string();
    } else {
      if (some_identifier && !next_is_colon) {
        rd.Undo();
      }

      argument_name = string(std::to_string(positional_index++));
    }

    auto argument_value = recurse_expr({
        Token(Punc, PuncComa),
        terminator,
    });

    call_args.push_back({argument_name, argument_value});

    next_if(PuncComa);
  }

  return call_args;
}

FlowPtr<Expr> Parser::recurse_fstring() {
  std::string buf;
  FStringItems items;
  size_t state = 0, w_beg = 0, w_end = 0;

  if (auto tok = next_if(Text)) {
    auto fstring_raw = tok->as_string().get();
    buf.reserve(fstring_raw.size());

    for (size_t i = 0; i < fstring_raw.size(); i++) {
      auto ch = fstring_raw[i];

      if (ch == '{' && state == 0) {
        w_beg = i + 1;
        state = 1;
      } else if (ch == '}' && state == 1) {
        w_end = i + 1;
        state = 0;

        if (!buf.empty()) {
          items.push_back(string(std::move(buf)));
          buf.clear();
        }

        auto subnode =
            FromString(fstring_raw.substr(w_beg, w_end - w_beg), m_env)
                ->recurse_expr({
                    Token(Punc, PuncRCur),
                });

        items.push_back(subnode);
      } else if (ch == '{') {
        buf += ch;
        state = 0;
      } else if (ch == '}') {
        buf += ch;
        state = 0;
      } else if (state == 0) {
        buf += ch;
      }
    }

    if (!buf.empty()) {
      items.push_back(string(std::move(buf)));
      buf.clear();
    }

    if (state != 0) {
      diagnostic << current() << "F-string expression has mismatched braces";
    }

    return make<FString>(std::move(items))();
  } else {
    diagnostic << current()
               << "Expected a string literal token for F-string expression";
    return mock_expr(QAST_FSTRING);
  }
}

static bool IsPostUnaryOp(Operator O) { return O == OpInc || O == OpDec; }

enum class FrameType : uint8_t {
  Start,
  PreUnary,
  PostUnary,
  Binary,
};

struct Frame {
  FlowPtr<Expr> base;
  short minPrecedence;
  LocationID start_pos;
  FrameType type;
  Operator op;

  Frame(FlowPtr<Expr> base, LocationID start_pos, short minPrecedence,
        FrameType type, Operator op)
      : base(base),
        minPrecedence(minPrecedence),
        start_pos(start_pos),
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
  auto SourceOffset = peek().get_start();

  std::stack<Frame> Stack;
  Stack.push({nullptr, SourceOffset, 0, FrameType::Start, OpPlus});

  /****************************************
   * Parse pre-unary operators
   ****************************************/
  std::stack<std::pair<Operator, LocationID>> PreUnaryOps;
  while (auto Tok = next_if(Oper)) {
    PreUnaryOps.push({Tok->as_op(), Tok->get_start()});
  }

  if (auto LeftSideOpt = recurse_expr_primary(false)) {
    auto LeftSide = LeftSideOpt.value();
    auto Spinning = true;

    /****************************************
     * Combine pre-unary operators
     ****************************************/
    while (!PreUnaryOps.empty()) {
      auto [Op, Offset] = PreUnaryOps.top();
      PreUnaryOps.pop();

      LeftSide = make<UnaryExpr>(Op, LeftSide)();
      LeftSide->set_offset(Offset);
    }

    while (!Stack.empty() && Spinning) {
      auto Tok = peek();

      if (terminators.contains(Tok)) {
        break;
      }

      switch (Tok.get_type()) {
        case Oper: {
          auto Op = Tok.as_op();
          auto OpType = IsPostUnaryOp(Op) ? OpMode::PostUnary : OpMode::Binary;
          auto OpPrecedence = GetOperatorPrecedence(Op, OpType);

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

            auto IsLeftAssoc =
                GetOperatorAssociativity(Op, OpType) == OpAssoc::Left;
            auto NextMinPrecedence =
                IsLeftAssoc ? OpPrecedence + 1 : OpPrecedence;
            auto IsType = Op == OpAs || Op == OpBitcastAs;

            if (!IsType) {
              /****************************************
               * Parse pre-unary operators
               ****************************************/
              while (auto Tok = next_if(Oper)) {
                PreUnaryOps.push({Tok->as_op(), Tok->get_start()});
              }
            } else {
              next_if(lex::Type);
            }

            /****************************************
             * Handle binary operators
             ****************************************/
            if (auto RightSide = recurse_expr_primary(IsType)) {
              /****************************************
               * Combine pre-unary operators
               ****************************************/
              while (!PreUnaryOps.empty()) {
                auto [Op, Offset] = PreUnaryOps.top();
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

        case Punc: {
          // Based on the assumption that function calls have the same
          // precedence as the dot operator (member access)
          static auto SuffixOPPrecedence =
              GetOperatorPrecedence(OpDot, OpMode::Binary);

          LeftSide = UnwindStack(Stack, LeftSide, SuffixOPPrecedence);

          if (next_if(PuncLPar)) {
            auto Arguments = recurse_call_arguments(Token(Punc, PuncRPar));
            if (!next_if(PuncRPar)) {
              diagnostic << current()
                         << "Expected ')' to close the function call";
            }

            LeftSide = make<Call>(LeftSide, Arguments)();
            LeftSide->set_offset(SourceOffset);
          } else if (next_if(PuncLBrk)) {
            auto first = recurse_expr({
                Token(Punc, PuncRBrk),
                Token(Punc, PuncColn),
            });

            if (next_if(PuncColn)) {
              auto second = recurse_expr({Token(Punc, PuncRBrk)});
              if (!next_if(PuncRBrk)) {
                diagnostic << current() << "Expected ']' to close the slice";
              }

              LeftSide = make<Slice>(LeftSide, first, second)();
              LeftSide->set_offset(SourceOffset);
            } else {
              if (!next_if(PuncRBrk)) {
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

        case EofF:
        case KeyW:
        case Name:
        case IntL:
        case NumL:
        case Text:
        case Char:
        case MacB:
        case Macr:
        case Note: {
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
    case Scope: {
      diagnostic << current() << "Namespace declaration is not valid here";
      break;
    }

    case Import: {
      diagnostic << current() << "Import statement is not valid here";
      break;
    }

    case Pub: {
      diagnostic << current() << "Public access modifier is not valid here";
      break;
    }

    case Sec: {
      diagnostic << current() << "Private access modifier is not valid here";
      break;
    }

    case Pro: {
      diagnostic << current() << "Protected access modifier is not valid here";
      break;
    }

    case Keyword::Type: {
      auto start_pos = current().get_start();
      auto type = recurse_type();
      type->set_offset(start_pos);

      E = make<TypeExpr>(type)();
      break;
    }

    case Let: {
      diagnostic << current() << "Let declaration is not valid here";
      break;
    }

    case Var: {
      diagnostic << current() << "Var declaration is not valid here";
      break;
    }

    case Const: {
      diagnostic << current() << "Const declaration is not valid here";
      break;
    }

    case Static: {
      diagnostic << current() << "Static modifier is not valid here";
      break;
    }

    case Struct: {
      diagnostic << current() << "Struct declaration is not valid here";
      break;
    }

    case Region: {
      diagnostic << current() << "Region declaration is not valid here";
      break;
    }

    case Group: {
      diagnostic << current() << "Group declaration is not valid here";
      break;
    }

    case Class: {
      diagnostic << current() << "Class declaration is not valid here";
      break;
    }

    case Union: {
      diagnostic << current() << "Union declaration is not valid here";
      break;
    }

    case Opaque: {
      diagnostic << current() << "Opaque type is not valid here";
      break;
    }

    case Enum: {
      diagnostic << current() << "Enum declaration is not valid here";
      break;
    }

    case __FString: {
      E = recurse_fstring();
      break;
    }

    case Fn: {
      auto start_pos = current().get_start();
      auto function = recurse_function(false);
      function->set_offset(start_pos);

      FlowPtr<Expr> expr = make<StmtExpr>(function)();

      if (next_if(PuncLPar)) {
        auto args = recurse_call_arguments(Token(Punc, PuncRPar));

        if (next_if(PuncRPar)) {
          E = make<Call>(expr, args)();
        } else {
          diagnostic << current() << "Expected ')' to close the function call";
          E = mock_expr(QAST_CALL);
        }
      } else {
        E = expr;
      }

      break;
    }

    case Unsafe: {
      diagnostic << current() << "Unsafe keyword is not valid here";
      break;
    }

    case Safe: {
      diagnostic << current() << "Safe keyword is not valid here";
      break;
    }

    case Promise: {
      diagnostic << current() << "Promise keyword is not valid here";
      break;
    }

    case If: {
      diagnostic << current() << "If statement is not valid here";
      break;
    }

    case Else: {
      diagnostic << current() << "Else statement is not valid here";
      break;
    }

    case For: {
      diagnostic << current() << "For loop is not valid here";
      break;
    }

    case While: {
      diagnostic << current() << "While loop is not valid here";
      break;
    }

    case Do: {
      diagnostic << current() << "Do statement is not valid here";
      break;
    }

    case Switch: {
      diagnostic << current() << "Switch statement is not valid here";
      break;
    }

    case Break: {
      diagnostic << current() << "Break statement is not valid here";
      break;
    }

    case Continue: {
      diagnostic << current() << "Continue statement is not valid here";
      break;
    }

    case Return: {
      diagnostic << current() << "Return statement is not valid here";
      break;
    }

    case Retif: {
      diagnostic << current() << "Retif statement is not valid here";
      break;
    }

    case Foreach: {
      diagnostic << current() << "Foreach loop is not valid here";
      break;
    }

    case Try: {
      diagnostic << current() << "Try statement is not valid here";
      break;
    }

    case Catch: {
      diagnostic << current() << "Catch statement is not valid here";
      break;
    }

    case Throw: {
      diagnostic << current() << "Throw statement is not valid here";
      break;
    }

    case Async: {
      diagnostic << current() << "Async statement is not valid here";
      break;
    }

    case Await: {
      diagnostic << current() << "Await statement is not valid here";
      break;
    }

    case __Asm__: {
      diagnostic << current() << "Inline assembly is not valid here";
      break;
    }

    case Undef: {
      E = make<ConstUndef>()();
      break;
    }

    case Null: {
      E = make<ConstNull>()();
      break;
    }

    case True: {
      E = make<ConstBool>(true)();
      break;
    }

    case False: {
      E = make<ConstBool>(false)();
      break;
    }
  }

  return E;
}

NullableFlowPtr<Expr> Parser::recurse_expr_punctor(lex::Punctor punc) {
  NullableFlowPtr<Expr> E;

  switch (punc) {
    case PuncLPar: {
      E = recurse_expr({
          Token(Punc, PuncRPar),
      });

      if (!next_if(PuncRPar)) {
        diagnostic << current() << "Expected ')' to close the expression";
      }

      break;
    }

    case PuncRPar: {
      diagnostic << current() << "Unexpected right parenthesis in expression";
      break;
    }

    case PuncLBrk: {
      ExpressionList items;

      while (true) {
        if (next_if(EofF)) [[unlikely]] {
          diagnostic << current()
                     << "Unexpected end of file while parsing expression";
          break;
        }

        if (next_if(PuncRBrk)) {
          break;
        }

        auto expr = recurse_expr({
            Token(Punc, PuncComa),
            Token(Punc, PuncRBrk),
            Token(Punc, PuncSemi),
        });

        if (next_if(PuncSemi)) {
          if (auto count_tok = next_if(IntL)) {
            auto item_repeat_str = current().as_string();
            size_t item_repeat_count = 0;

            if (std::from_chars(
                    item_repeat_str->data(),
                    item_repeat_str->data() + item_repeat_str->size(),
                    item_repeat_count)
                    .ec == std::errc()) {
              if (item_repeat_count <= MAX_LIST_REPEAT_COUNT) {
                for (size_t i = 0; i < item_repeat_count; i++) {
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

        next_if(PuncComa);
      }

      E = make<List>(items)();
      break;
    }

    case PuncRBrk: {
      diagnostic << current() << "Unexpected right bracket in expression";
      break;
    }

    case PuncLCur: {
      // ExpressionList items;
      std::vector<FlowPtr<Assoc>> items;

      while (true) {
        if (next_if(EofF)) [[unlikely]] {
          diagnostic << current()
                     << "Unexpected end of file while parsing dictionary";
          break;
        }

        if (next_if(PuncRCur)) {
          break;
        }

        auto start_pos = peek().get_start();
        auto key = recurse_expr({
            Token(Punc, PuncColn),
        });

        if (!next_if(PuncColn)) {
          diagnostic << current() << "Expected colon after key in dictionary";
          break;
        }

        auto value = recurse_expr({
            Token(Punc, PuncRCur),
            Token(Punc, PuncComa),
        });

        next_if(PuncComa);

        auto assoc = make<Assoc>(key, value)();
        assoc->set_offset(start_pos);

        items.push_back(assoc);
      }

      if (items.size() == 1) {
        E = items[0];
      } else {
        ExpressionList items_copy(items.begin(), items.end());
        E = make<List>(items_copy)();
      }

      break;
    }

    case PuncRCur: {
      diagnostic << current() << "Unexpected right curly brace in expression";
      break;
    }

    case PuncComa: {
      diagnostic << current() << "Comma is not valid in this context";
      break;
    }

    case PuncColn: {
      diagnostic << current() << "Colon is not valid in this context";
      break;
    }

    case PuncSemi: {
      diagnostic << current() << "Semicolon is not valid in this context";
      break;
    }
  }

  return E;
}

FlowPtr<Expr> Parser::recurse_expr_type_suffix(FlowPtr<Expr> base) {
  auto tok = current();

  auto suffix = recurse_type();
  suffix->set_offset(tok.get_start());

  auto texpr = make<TypeExpr>(suffix)();
  texpr->set_offset(tok.get_start());

  return make<BinExpr>(base, OpAs, texpr)();
}

NullableFlowPtr<Expr> Parser::recurse_expr_primary(bool isType) {
  auto start_pos = peek().get_start();

  NullableFlowPtr<Expr> E;

  if (isType) {
    auto type = recurse_type();
    type->set_offset(start_pos);

    auto texpr = make<TypeExpr>(type)();
    texpr->set_offset(start_pos);

    E = texpr;
  } else {
    switch (auto tok = next(); tok.get_type()) {
      case EofF: {
        break;
      }

      case KeyW: {
        if ((E = recurse_expr_keyword(tok.as_key())).has_value()) {
          E.value()->set_offset(start_pos);
        }

        break;
      }

      case Oper: {
        diagnostic << tok << "Unexpected operator in expression";
        break;
      }

      case Punc: {
        if ((E = recurse_expr_punctor(tok.as_punc())).has_value()) {
          E.value()->set_offset(start_pos);
        }
        break;
      }

      case Name: {
        auto identifier = make<Ident>(tok.as_string())();
        identifier->set_offset(start_pos);

        E = identifier;
        break;
      }

      case IntL: {
        auto integer = make<ConstInt>(tok.as_string())();
        integer->set_offset(start_pos);

        if (auto tok = peek(); tok.is(Name)) {
          auto casted = recurse_expr_type_suffix(integer);
          casted->set_offset(start_pos);

          E = casted;
        } else {
          E = integer;
        }

        break;
      }

      case NumL: {
        auto decimal = make<ConstFloat>(tok.as_string())();
        decimal->set_offset(start_pos);

        if (auto tok = peek(); tok.is(Name)) {
          auto casted = recurse_expr_type_suffix(decimal);
          casted->set_offset(start_pos);

          E = casted;
        } else {
          E = decimal;
        }

        break;
      }

      case Text: {
        auto string = make<ConstString>(tok.as_string())();
        string->set_offset(start_pos);

        if (auto tok = peek(); tok.is(Name)) {
          auto casted = recurse_expr_type_suffix(string);
          casted->set_offset(start_pos);

          E = casted;
        } else {
          E = string;
        }

        break;
      }

      case Char: {
        auto str_data = tok.as_string();
        if (str_data->size() != 1) [[unlikely]] {
          diagnostic << tok << "Expected a single byte in character literal";
          break;
        }

        auto character = make<ConstChar>(str_data->at(0))();
        character->set_offset(start_pos);

        if (auto tok = peek(); tok.is(Name)) {
          auto casted = recurse_expr_type_suffix(character);
          casted->set_offset(start_pos);

          E = casted;
        } else {
          E = character;
        }

        break;
      }

      case MacB: {
        diagnostic << tok << "Unexpected macro block in expression";
        break;
      }

      case Macr: {
        diagnostic << tok << "Unexpected macro call in expression";
        break;
      }

      case Note: {
        diagnostic << tok << "Unexpected comment in expression";
        break;
      }
    }
  }

  return E;
}
