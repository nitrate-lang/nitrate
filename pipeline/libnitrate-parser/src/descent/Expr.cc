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

CallArgs Parser::RecurseCallArguments(const std::set<lex::Token> &terminators,
                                      bool type_by_default) {
  CallArgs call_args;
  size_t positional_index = 0;
  string argument_name;

  while (true) {
    if (next_if(EofF)) [[unlikely]] {
      Log << SyntaxError << current()
          << "Unexpected end of file while parsing call expression";
      return call_args;
    }

    if (terminators.contains(peek())) {
      break;
    }

    auto some_identifier = next_if(Name);
    auto next_is_colon = some_identifier && next_if(PuncColn).has_value();

    if (some_identifier && next_is_colon) {
      argument_name = some_identifier->as_string();
    } else {
      if (some_identifier && !next_is_colon) {
        m_rd.Insert(some_identifier.value());
      }

      argument_name = string(std::to_string(positional_index++));
    }

    if (type_by_default) {
      auto argument_value = RecurseType();
      auto type_expr = make<TypeExpr>(argument_value)();

      call_args.emplace_back(argument_name, type_expr);
    } else {
      std::set<Token> terminators_copy(terminators);
      terminators_copy.insert(Token(Punc, PuncComa));
      auto argument_value = RecurseExpr(terminators_copy);
      call_args.push_back({argument_name, argument_value});
    }

    next_if(PuncComa);
  }

  return call_args;
}

FlowPtr<Expr> Parser::RecurseFstring() {
  FStringItems items;

  if (auto tok = next_if(Text)) {
    size_t state = 0, w_beg = 0, w_end{};
    auto fstring_raw = tok->as_string().Get();

    std::string buf;
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
                ->RecurseExpr({
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
      Log << SyntaxError << current()
          << "F-string expression has mismatched braces";
    }

    return make<FString>(std::move(items))();
  } else {
    Log << SyntaxError << current()
        << "Expected a string literal token for F-string expression";
    return MockExpr(QAST_FSTRING);
  }
}

static bool IsPostUnaryOp(Operator o) { return o == OpInc || o == OpDec; }

enum class FrameType : uint8_t {
  Start,
  PreUnary,
  PostUnary,
  Binary,
};

struct Frame {
  FlowPtr<Expr> m_base;
  short m_minPrecedence;
  LocationID m_start_pos;
  FrameType m_type;
  Operator m_op;

  Frame(FlowPtr<Expr> base, LocationID start_pos, short min_precedence,
        FrameType type, Operator op)
      : m_base(base),
        m_minPrecedence(min_precedence),
        m_start_pos(start_pos),
        m_type(type),
        m_op(op) {}
};

static NCC_FORCE_INLINE FlowPtr<Expr> UnwindStack(std::stack<Frame> &stack,
                                                  FlowPtr<Expr> base,
                                                  short min_precedence) {
  while (!stack.empty()) {
    auto frame = stack.top();

    if (frame.m_minPrecedence < min_precedence) {
      break;
    }

    switch (frame.m_type) {
      case FrameType::Start: {
        return base;
      }

      case FrameType::PreUnary: {
        base = make<UnaryExpr>(frame.m_op, base)();
        break;
      }

      case FrameType::PostUnary: {
        base = make<PostUnaryExpr>(base, frame.m_op)();
        break;
      }

      case FrameType::Binary: {
        base = make<BinExpr>(frame.m_base, frame.m_op, base)();
        break;
      }
    }

    base->SetOffset(frame.m_start_pos);
    stack.pop();
  }

  return base;
}

FlowPtr<Expr> Parser::RecurseExpr(const std::set<Token> &terminators) {
  auto source_offset = peek().get_start();

  std::stack<Frame> stack;
  ConstBool start_node(false);
  stack.push({&start_node, source_offset, 0, FrameType::Start, OpPlus});

  /****************************************
   * Parse pre-unary operators
   ****************************************/
  std::stack<std::pair<Operator, LocationID>> pre_unary_ops;
  while (auto tok = next_if(Oper)) {
    pre_unary_ops.push({tok->as_op(), tok->get_start()});
  }

  if (auto left_side_opt = RecurseExprPrimary(false)) {
    auto left_side = left_side_opt.value();
    auto spinning = true;

    /****************************************
     * Combine pre-unary operators
     ****************************************/
    while (!pre_unary_ops.empty()) {
      auto [Op, Offset] = pre_unary_ops.top();
      pre_unary_ops.pop();

      left_side = make<UnaryExpr>(Op, left_side)();
      left_side->SetOffset(Offset);
    }

    while (!stack.empty() && spinning) {
      auto tok = peek();

      if (terminators.contains(tok)) {
        break;
      }

      switch (tok.get_type()) {
        case Oper: {
          auto op = tok.as_op();
          auto op_type = IsPostUnaryOp(op) ? OpMode::PostUnary : OpMode::Binary;
          auto op_precedence = GetOperatorPrecedence(op, op_type);

          if (op_precedence >= stack.top().m_minPrecedence) {
            next();

            /****************************************
             * Handle post-unary operators
             ****************************************/
            if (op_type == OpMode::PostUnary) {
              left_side = make<PostUnaryExpr>(left_side, op)();
              left_side->SetOffset(source_offset);

              continue;
            }

            auto is_left_assoc =
                GetOperatorAssociativity(op, op_type) == Associativity::Left;
            auto next_min_precedence =
                is_left_assoc ? op_precedence + 1 : op_precedence;
            auto is_type = op == OpAs || op == OpBitcastAs;

            if (!is_type) {
              /****************************************
               * Parse pre-unary operators
               ****************************************/
              while (auto tok_op = next_if(Oper)) {
                pre_unary_ops.push({tok_op->as_op(), tok_op->get_start()});
              }
            } else {
              next_if(lex::Type);
            }

            /****************************************
             * Handle binary operators
             ****************************************/
            if (auto right_side = RecurseExprPrimary(is_type)) {
              /****************************************
               * Combine pre-unary operators
               ****************************************/
              while (!pre_unary_ops.empty()) {
                auto [op, Offset] = pre_unary_ops.top();
                pre_unary_ops.pop();

                auto pre_unary_expr = make<UnaryExpr>(op, right_side.value())();
                pre_unary_expr->SetOffset(Offset);

                right_side = pre_unary_expr;
              }

              if (stack.size() + 1 > MAX_RECURSION_DEPTH) {
                Log << SyntaxError << current()
                    << "Recursion depth exceeds maximum limit";
                return MockExpr();
              }

              stack.push(Frame(left_side, source_offset, next_min_precedence,
                               FrameType::Binary, op));
              left_side = right_side.value();
            } else {
              Log << SyntaxError << current()
                  << "Failed to parse right-hand side of binary expression";
            }
          } else {
            left_side = UnwindStack(stack, left_side, 0);

            continue;
          }

          break;
        }

        case Punc: {
          // Based on the assumption that function calls have the same
          // precedence as the dot operator (member access)
          static auto suffix_op_precedence =
              GetOperatorPrecedence(OpDot, OpMode::Binary);

          left_side = UnwindStack(stack, left_side, suffix_op_precedence);

          if (next_if(PuncLPar)) {
            auto arguments =
                RecurseCallArguments({Token(Punc, PuncRPar)}, false);
            if (!next_if(PuncRPar)) {
              Log << SyntaxError << current()
                  << "Expected ')' to close the function call";
            }

            left_side = make<Call>(left_side, arguments)();
            left_side->SetOffset(source_offset);
          } else if (next_if(PuncLBrk)) {
            auto first = RecurseExpr({
                Token(Punc, PuncRBrk),
                Token(Punc, PuncColn),
            });

            if (next_if(PuncColn)) {
              auto second = RecurseExpr({Token(Punc, PuncRBrk)});
              if (!next_if(PuncRBrk)) {
                Log << SyntaxError << current()
                    << "Expected ']' to close the slice";
              }

              left_side = make<Slice>(left_side, first, second)();
              left_side->SetOffset(source_offset);
            } else {
              if (!next_if(PuncRBrk)) {
                Log << SyntaxError << current()
                    << "Expected ']' to close the index expression";
              }

              left_side = make<Index>(left_side, first)();
              left_side->SetOffset(source_offset);
            }
          } else if (next_if(PuncLCur)) {
            auto template_arguments =
                RecurseCallArguments({Token(Punc, PuncRCur)}, true);
            if (!next_if(PuncRCur)) {
              Log << SyntaxError << current()
                  << "Expected '}' to close the template arguments";
            }

            if (!next_if(PuncLPar)) {
              Log << SyntaxError << current()
                  << "Expected '(' to open the call arguments";
            }

            auto call_arguments =
                RecurseCallArguments({Token(Punc, PuncRPar)}, false);
            if (!next_if(PuncRPar)) {
              Log << SyntaxError << current()
                  << "Expected ')' to close the call arguments";
            }

            left_side = make<TemplCall>(left_side, std::move(call_arguments),
                                        std::move(template_arguments))();
            left_side->SetOffset(source_offset);
          } else {  // Not part of the expression
            spinning = false;
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
          spinning = false;
          break;
        }
      }
    }

    return UnwindStack(stack, left_side, 0);
  } else {
    Log << SyntaxError << current() << "Expected an expression";

    return MockExpr();
  }
}

NullableFlowPtr<Expr> Parser::RecurseExprKeyword(lex::Keyword key) {
  NullableFlowPtr<Expr> e;

  switch (key) {
    case Keyword::Type: {
      auto start_pos = current().get_start();
      auto type = RecurseType();
      type->SetOffset(start_pos);

      e = make<TypeExpr>(type)();
      break;
    }

    case __FString: {
      e = RecurseFstring();
      break;
    }

    case Fn: {
      auto start_pos = current().get_start();
      auto function = RecurseFunction(false);
      function->SetOffset(start_pos);

      FlowPtr<Expr> expr = make<StmtExpr>(function)();

      if (next_if(PuncLPar)) {
        auto args = RecurseCallArguments({Token(Punc, PuncRPar)}, false);

        if (next_if(PuncRPar)) {
          e = make<Call>(expr, args)();
        } else {
          Log << SyntaxError << current()
              << "Expected ')' to close the function call";
          e = MockExpr(QAST_CALL);
        }
      } else {
        e = expr;
      }

      break;
    }

    case Undef: {
      e = make<ConstUndef>()();
      break;
    }

    case Null: {
      e = make<ConstNull>()();
      break;
    }

    case True: {
      e = make<ConstBool>(true)();
      break;
    }

    case False: {
      e = make<ConstBool>(false)();
      break;
    }

    default: {
      Log << SyntaxError << current() << "Unexpected '" << key
          << "' in expression context";
      break;
    }
  }

  return e;
}

NullableFlowPtr<Expr> Parser::RecurseExprPunctor(lex::Punctor punc) {
  NullableFlowPtr<Expr> e;

  switch (punc) {
    case PuncLPar: {
      e = RecurseExpr({
          Token(Punc, PuncRPar),
      });

      if (!next_if(PuncRPar)) {
        Log << SyntaxError << current()
            << "Expected ')' to close the expression";
      }

      break;
    }

    case PuncRPar: {
      Log << SyntaxError << current()
          << "Unexpected right parenthesis in expression";
      break;
    }

    case PuncLBrk: {
      ExpressionList items;

      while (true) {
        if (next_if(EofF)) [[unlikely]] {
          Log << SyntaxError << current()
              << "Unexpected end of file while parsing expression";
          break;
        }

        if (next_if(PuncRBrk)) {
          break;
        }

        auto expr = RecurseExpr({
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
                Log << SyntaxError << current()
                    << "Compressed list size exceeds maximum limit";
              }

            } else {
              Log << SyntaxError << current()
                  << "Expected an integer literal for the compressed "
                     "list size";
            }
          } else {
            Log << SyntaxError << current()
                << "Expected an integer literal for the compressed list "
                   "size";
          }
        } else {
          items.push_back(expr);
        }

        next_if(PuncComa);
      }

      e = make<List>(items)();
      break;
    }

    case PuncRBrk: {
      Log << SyntaxError << current()
          << "Unexpected right bracket in expression";
      break;
    }

    case PuncLCur: {
      // ExpressionList items;
      std::vector<FlowPtr<Assoc>> items;

      while (true) {
        if (next_if(EofF)) [[unlikely]] {
          Log << SyntaxError << current()
              << "Unexpected end of file while parsing dictionary";
          break;
        }

        if (next_if(PuncRCur)) {
          break;
        }

        auto start_pos = peek().get_start();
        auto key = RecurseExpr({
            Token(Punc, PuncColn),
        });

        if (!next_if(PuncColn)) {
          Log << SyntaxError << current()
              << "Expected colon after key in dictionary";
          break;
        }

        auto value = RecurseExpr({
            Token(Punc, PuncRCur),
            Token(Punc, PuncComa),
        });

        next_if(PuncComa);

        auto assoc = make<Assoc>(key, value)();
        assoc->SetOffset(start_pos);

        items.push_back(assoc);
      }

      if (items.size() == 1) {
        e = items[0];
      } else {
        ExpressionList items_copy(items.begin(), items.end());
        e = make<List>(items_copy)();
      }

      break;
    }

    case PuncRCur: {
      Log << SyntaxError << current()
          << "Unexpected right curly brace in expression";
      break;
    }

    case PuncComa: {
      Log << SyntaxError << current()
          << "Unexpected comma in expression context";
      break;
    }

    case PuncColn: {
      Log << SyntaxError << current()
          << "Unexpected colon in expression context";
      break;
    }

    case PuncSemi: {
      Log << SyntaxError << current()
          << "Unexpected semicolon in expression context";
      break;
    }
  }

  return e;
}

FlowPtr<Expr> Parser::RecurseExprTypeSuffix(FlowPtr<Expr> base) {
  auto tok = current();

  auto suffix = RecurseType();
  suffix->SetOffset(tok.get_start());

  auto texpr = make<TypeExpr>(suffix)();
  texpr->SetOffset(tok.get_start());

  return make<BinExpr>(base, OpAs, texpr)();
}

NullableFlowPtr<Expr> Parser::RecurseExprPrimary(bool is_type) {
  auto start_pos = peek().get_start();

  NullableFlowPtr<Expr> e;

  if (is_type) {
    auto comments = m_rd.CommentBuffer();
    m_rd.ClearCommentBuffer();

    auto type = RecurseType();
    type->SetOffset(start_pos);

    auto texpr = make<TypeExpr>(type)();
    texpr->SetOffset(start_pos);

    e = BindComments(texpr, comments);
  } else {
    auto tok = next();

    auto comments = m_rd.CommentBuffer();
    m_rd.ClearCommentBuffer();

    switch (tok.get_type()) {
      case EofF: {
        break;
      }

      case KeyW: {
        if ((e = RecurseExprKeyword(tok.as_key())).has_value()) {
          e.value()->SetOffset(start_pos);
        }

        break;
      }

      case Oper: {
        Log << SyntaxError << tok << "Unexpected operator in expression";
        break;
      }

      case Punc: {
        if ((e = RecurseExprPunctor(tok.as_punc())).has_value()) {
          e.value()->SetOffset(start_pos);
        }
        break;
      }

      case Name: {
        auto identifier = make<Ident>(tok.as_string())();
        identifier->SetOffset(start_pos);

        e = identifier;
        break;
      }

      case IntL: {
        auto integer = make<ConstInt>(tok.as_string())();
        integer->SetOffset(start_pos);

        if (tok = peek(); tok.is(Name)) {
          auto casted = RecurseExprTypeSuffix(integer);
          casted->SetOffset(start_pos);

          e = casted;
        } else {
          e = integer;
        }

        break;
      }

      case NumL: {
        auto decimal = make<ConstFloat>(tok.as_string())();
        decimal->SetOffset(start_pos);

        if (tok = peek(); tok.is(Name)) {
          auto casted = RecurseExprTypeSuffix(decimal);
          casted->SetOffset(start_pos);

          e = casted;
        } else {
          e = decimal;
        }

        break;
      }

      case Text: {
        auto string = make<ConstString>(tok.as_string())();
        string->SetOffset(start_pos);

        if (tok = peek(); tok.is(Name)) {
          auto casted = RecurseExprTypeSuffix(string);
          casted->SetOffset(start_pos);

          e = casted;
        } else {
          e = string;
        }

        break;
      }

      case Char: {
        auto str_data = tok.as_string();
        if (str_data->size() != 1) [[unlikely]] {
          Log << SyntaxError << tok
              << "Expected a single byte in character literal";
          break;
        }

        auto character = make<ConstChar>(str_data->at(0))();
        character->SetOffset(start_pos);

        if (tok = peek(); tok.is(Name)) {
          auto casted = RecurseExprTypeSuffix(character);
          casted->SetOffset(start_pos);

          e = casted;
        } else {
          e = character;
        }

        break;
      }

      case MacB: {
        Log << SyntaxError << tok << "Unexpected macro block in expression";
        break;
      }

      case Macr: {
        Log << SyntaxError << tok << "Unexpected macro call in expression";
        break;
      }

      case Note: {
        Log << SyntaxError << tok << "Unexpected comment in expression";
        break;
      }
    }

    if (e.has_value()) {
      e = BindComments(e.value(), comments);
    }
  }

  return e;
}
