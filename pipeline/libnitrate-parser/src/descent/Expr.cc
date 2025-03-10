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
#include <nitrate-lexer/Grammar.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTBase.hh>
#include <stack>
#include <utility>

static constexpr size_t kMaxRecursionDepth = 4096;
static constexpr size_t kMaxListRepeatCount = 4096;

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;
using namespace ncc;

auto GeneralParser::PImpl::RecurseCallArguments(const std::set<lex::Token> &terminators,
                                                bool type_by_default) -> std::vector<CallArg> {
  std::vector<CallArg> call_args;
  size_t positional_index = 0;
  string argument_name;

  while (true) {
    if (m_rd.IsEof()) [[unlikely]] {
      Log << SyntaxError << Current() << "Unexpected end of file while parsing call expression";
      return call_args;
    }

    if (terminators.contains(Peek())) {
      break;
    }

    auto some_identifier = NextIf<Name>();
    auto next_is_colon = some_identifier && NextIf<PuncColn>().has_value();

    if (some_identifier && next_is_colon) {
      argument_name = some_identifier->GetString();
    } else {
      if (some_identifier && !next_is_colon) {
        m_rd.Insert(some_identifier.value());
      }

      argument_name = string(std::to_string(positional_index++));
    }

    if (type_by_default) {
      auto argument_value = RecurseType();
      call_args.emplace_back(argument_name, argument_value);
    } else {
      std::set<Token> terminators_copy(terminators);
      terminators_copy.insert(Token(Punc, PuncComa));
      auto argument_value = RecurseExpr(terminators_copy);
      call_args.emplace_back(argument_name, argument_value);
    }

    NextIf<PuncComa>();
  }

  return call_args;
}

auto GeneralParser::PImpl::RecurseFstring() -> FlowPtr<Expr> {
  std::vector<std::variant<string, FlowPtr<Expr>>> items;

  if (auto tok = NextIf<Text>()) {
    size_t state = 0;
    size_t w_beg = 0;
    size_t w_end{};
    auto fstring_raw = tok->GetString().Get();

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
          items.emplace_back(string(std::move(buf)));
          buf.clear();
        }

        auto source = fstring_raw.substr(w_beg, w_end - w_beg);
        auto in_src = boost::iostreams::stream<boost::iostreams::array_source>(source.data(), source.size());
        auto scanner = Tokenizer(in_src, m_env);
        auto subnode = GeneralParser::Create(scanner, m_env, m_pool)
                           ->m_impl->RecurseExpr({
                               Token(Punc, PuncRCur),
                           });

        items.emplace_back(subnode);
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
      items.emplace_back(string(std::move(buf)));
      buf.clear();
    }

    if (state != 0) {
      Log << SyntaxError << Current() << "F-string expression has mismatched braces";
    }

    return m_fac.CreateFormatString(items);
  }
  Log << SyntaxError << Current() << "Expected a string literal token for F-string expression";
  return m_fac.CreateMockInstance<FString>();
}

static auto IsPostUnaryOp(Operator o) -> bool { return o == OpInc || o == OpDec; }

static auto IsOnlyUnaryOp(Operator o) -> bool {
  switch (o) {
    case OpBitNot:
    case OpLogicNot:
    case OpInc:
    case OpDec:
    case OpSizeof:
    case OpBitsizeof:
    case OpAlignof:
    case OpTypeof:
    case OpComptime:
      return true;

    default:
      return false;
  }
}

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

  Frame(FlowPtr<Expr> base, LocationID start_pos, short min_precedence, FrameType type, Operator op)
      : m_base(std::move(base)), m_minPrecedence(min_precedence), m_start_pos(start_pos), m_type(type), m_op(op) {}
};

static NCC_FORCE_INLINE auto UnwindStack(ASTFactory &fac, std::stack<Frame> &stack, FlowPtr<Expr> base,
                                         short min_precedence) -> FlowPtr<Expr> {
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
        base = fac.CreateUnary(frame.m_op, base);
        break;
      }

      case FrameType::PostUnary: {
        base = fac.CreateUnary(frame.m_op, base, true);
        break;
      }

      case FrameType::Binary: {
        base = fac.CreateBinary(frame.m_base, frame.m_op, base);
        break;
      }
    }

    base->SetOffset(frame.m_start_pos);
    stack.pop();
  }

  return base;
}

auto GeneralParser::PImpl::RecurseExpr(const std::set<Token> &terminators) -> FlowPtr<Expr> {
  auto source_offset = Peek().GetStart();

  std::stack<Frame> stack;
  Boolean start_node(false);
  stack.emplace(&start_node, source_offset, 0, FrameType::Start, OpPlus);

  /****************************************
   * Parse pre-unary operators
   ****************************************/
  std::stack<std::pair<Operator, LocationID>> pre_unary_ops;
  while (auto tok = NextIf<Oper>()) {
    pre_unary_ops.emplace(tok->GetOperator(), tok->GetStart());
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

      left_side = m_fac.CreateUnary(Op, left_side);
      left_side->SetOffset(Offset);
    }

    while (!stack.empty() && spinning) {
      auto tok = Peek();

      if (terminators.contains(tok)) {
        break;
      }

      switch (tok.GetKind()) {
        case Oper: {
          const auto op = tok.GetOperator();
          const auto op_type = IsPostUnaryOp(op) ? OpMode::PostUnary : OpMode::Binary;
          const auto op_precedence = GetOperatorPrecedence(op, op_type);

          if (op_precedence >= stack.top().m_minPrecedence) {
            Next();

            /****************************************
             * Handle post-unary operators
             ****************************************/
            if (op_type == OpMode::PostUnary) {
              left_side = m_fac.CreateUnary(op, left_side, true);
              left_side->SetOffset(source_offset);

              continue;
            }

            auto is_left_assoc = GetOperatorAssociativity(op, op_type) == Associativity::Left;
            auto next_min_precedence = is_left_assoc ? op_precedence + 1 : op_precedence;
            auto is_type = op == OpAs || op == OpBitcastAs;

            if (!is_type) {
              /****************************************
               * Parse pre-unary operators
               ****************************************/
              while (auto tok_op = NextIf<Oper>()) {
                pre_unary_ops.emplace(tok_op->GetOperator(), tok_op->GetStart());
              }
            } else {
              NextIf<lex::Type>();
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

                auto pre_unary_expr = m_fac.CreateUnary(op, right_side.value());
                pre_unary_expr->SetOffset(Offset);

                right_side = pre_unary_expr;
              }

              if (stack.size() + 1 > kMaxRecursionDepth) {
                Log << SyntaxError << Current() << "Recursion depth exceeds maximum limit";
                return m_fac.CreateMockInstance<VoidTy>();
              }

              stack.emplace(left_side, source_offset, next_min_precedence, FrameType::Binary, op);
              left_side = right_side.value();
            } else {
              Log << SyntaxError << Current() << "Failed to parse right-hand side of binary expression";
            }
          } else {
            if (IsOnlyUnaryOp(op)) {
              spinning = false;
            }

            left_side = UnwindStack(m_fac, stack, left_side, 0);

            continue;
          }

          break;
        }

        case Punc: {
          // Based on the assumption that function calls have the same
          // precedence as the dot operator (member access)
          static auto suffix_op_precedence = GetOperatorPrecedence(OpDot, OpMode::Binary);

          left_side = UnwindStack(m_fac, stack, left_side, suffix_op_precedence);

          if (NextIf<PuncLPar>()) {
            auto arguments = RecurseCallArguments({Token(Punc, PuncRPar)}, false);
            if (!NextIf<PuncRPar>()) {
              Log << SyntaxError << Current() << "Expected ')' to close the function call";
            }

            left_side = m_fac.CreateCall(arguments, left_side);
            left_side->SetOffset(source_offset);
          } else if (NextIf<PuncLBrk>()) {
            auto first = RecurseExpr({
                Token(Punc, PuncRBrk),
                Token(Punc, PuncColn),
            });

            if (NextIf<PuncColn>()) {
              auto second = RecurseExpr({Token(Punc, PuncRBrk)});
              if (!NextIf<PuncRBrk>()) {
                Log << SyntaxError << Current() << "Expected ']' to close the slice";
              }

              left_side = m_fac.CreateSlice(left_side, first, second);
              left_side->SetOffset(source_offset);
            } else {
              if (!NextIf<PuncRBrk>()) {
                Log << SyntaxError << Current() << "Expected ']' to close the index expression";
              }

              left_side = m_fac.CreateIndex(left_side, first);
              left_side->SetOffset(source_offset);
            }
          } else if (NextIf<PuncLCur>()) {
            auto template_arguments = RecurseCallArguments({Token(Punc, PuncRCur)}, true);
            if (!NextIf<PuncRCur>()) {
              Log << SyntaxError << Current() << "Expected '}' to close the template arguments";
            }

            if (!NextIf<PuncLPar>()) {
              Log << SyntaxError << Current() << "Expected '(' to open the call arguments";
            }

            auto call_arguments = RecurseCallArguments({Token(Punc, PuncRPar)}, false);
            if (!NextIf<PuncRPar>()) {
              Log << SyntaxError << Current() << "Expected ')' to close the call arguments";
            }

            left_side = m_fac.CreateTemplateCall(std::move(template_arguments), std::move(call_arguments), left_side);
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

    return UnwindStack(m_fac, stack, left_side, 0);
  }
  Log << SyntaxError << Current() << "Expected an expression";

  return m_fac.CreateMockInstance<VoidTy>();
}

auto GeneralParser::PImpl::RecurseExprKeyword(lex::Keyword key) -> NullableFlowPtr<Expr> {
  NullableFlowPtr<Expr> e;

  switch (key) {
    case Keyword::Type: {
      auto start_pos = Current().GetStart();
      auto type = RecurseType();
      type->SetOffset(start_pos);

      e = type;
      break;
    }

    case __FString: {
      e = RecurseFstring();
      break;
    }

    case Fn: {
      auto start_pos = Current().GetStart();
      auto function = RecurseFunction(false);
      function->SetOffset(start_pos);

      if (NextIf<PuncLPar>()) {
        auto args = RecurseCallArguments({Token(Punc, PuncRPar)}, false);

        if (NextIf<PuncRPar>()) {
          e = m_fac.CreateCall(args, function);
        } else {
          Log << SyntaxError << Current() << "Expected ')' to close the function call";
          e = m_fac.CreateMockInstance<Call>();
        }
      } else {
        e = function;
      }

      break;
    }

    case Undef: {
      e = m_fac.CreateUndefined();
      break;
    }

    case Keyword::Null: {
      e = m_fac.CreateNull();
      break;
    }

    case True: {
      e = m_fac.CreateBoolean(true);
      break;
    }

    case False: {
      e = m_fac.CreateBoolean(false);
      break;
    }

    case EscapeBlock: {
      RecurseEscapeBlock();  // We discard the block
      break;
    }

    default: {
      Log << SyntaxError << Current() << "Unexpected '" << key << "' in expression context";
      break;
    }
  }

  return e;
}

auto GeneralParser::PImpl::RecurseExprPunctor(lex::Punctor punc) -> NullableFlowPtr<Expr> {
  NullableFlowPtr<Expr> e;

  switch (punc) {
    case PuncLPar: {
      e = RecurseExpr({
          Token(Punc, PuncRPar),
      });
      auto depth = e.value()->GetParenthesisDepth();
      e.value()->SetParenthesisDepth(depth + 1);

      if (!NextIf<PuncRPar>()) {
        Log << SyntaxError << Current() << "Expected ')' to close the expression";
      }

      break;
    }

    case PuncRPar: {
      Log << SyntaxError << Current() << "Unexpected right parenthesis in expression";
      break;
    }

    case PuncLBrk: {
      std::vector<FlowPtr<Expr>> items;

      while (true) {
        if (m_rd.IsEof()) [[unlikely]] {
          Log << SyntaxError << Current() << "Unexpected end of file while parsing expression";
          break;
        }

        if (NextIf<PuncRBrk>()) {
          break;
        }

        auto expr = RecurseExpr({
            Token(Punc, PuncComa),
            Token(Punc, PuncRBrk),
            Token(Punc, PuncSemi),
        });

        if (NextIf<PuncSemi>()) {
          if (auto count_tok = NextIf<IntL>()) {
            auto item_repeat_str = count_tok->GetString();
            size_t item_repeat_count = 0;

            if (std::from_chars(item_repeat_str->c_str(), item_repeat_str->c_str() + item_repeat_str->size(),
                                item_repeat_count)
                    .ec == std::errc()) {
              if (item_repeat_count <= kMaxListRepeatCount) {
                for (size_t i = 0; i < item_repeat_count; i++) {
                  items.push_back(expr);
                }
              } else {
                Log << SyntaxError << Current() << "Compressed list size exceeds maximum limit";
              }

            } else {
              Log << SyntaxError << Current()
                  << "Expected an integer literal for the compressed "
                     "list size";
            }
          } else {
            Log << SyntaxError << Current()
                << "Expected an integer literal for the compressed list "
                   "size";
          }
        } else {
          items.push_back(expr);
        }

        NextIf<PuncComa>();
      }

      e = m_fac.CreateList(items);
      break;
    }

    case PuncRBrk: {
      Log << SyntaxError << Current() << "Unexpected right bracket in expression";
      break;
    }

    case PuncLCur: {
      // std::vector<FlowPtr<Expr>> items;
      std::vector<FlowPtr<Assoc>> items;

      while (true) {
        if (m_rd.IsEof()) [[unlikely]] {
          Log << SyntaxError << Current() << "Unexpected end of file while parsing dictionary";
          break;
        }

        if (NextIf<PuncRCur>()) {
          break;
        }

        auto start_pos = Peek().GetStart();
        auto key = RecurseExpr({
            Token(Punc, PuncColn),
        });

        if (!NextIf<PuncColn>()) {
          Log << SyntaxError << Current() << "Expected colon after key in dictionary";
          break;
        }

        auto value = RecurseExpr({
            Token(Punc, PuncRCur),
            Token(Punc, PuncComa),
        });

        NextIf<PuncComa>();

        auto assoc = m_fac.CreateAssociation(key, value);
        assoc->SetOffset(start_pos);

        items.push_back(assoc);
      }

      if (items.size() == 1) {
        e = items[0];
      } else {
        std::vector<FlowPtr<Expr>> items_copy(items.begin(), items.end());
        e = m_fac.CreateList(items_copy);
      }

      break;
    }

    case PuncRCur: {
      Log << SyntaxError << Current() << "Unexpected right curly brace in expression";
      break;
    }

    case PuncComa: {
      Log << SyntaxError << Current() << "Unexpected comma in expression context";
      break;
    }

    case PuncColn: {
      Log << SyntaxError << Current() << "Unexpected colon in expression context";
      break;
    }

    case PuncSemi: {
      Log << SyntaxError << Current() << "Unexpected semicolon in expression context";
      break;
    }

    case PuncScope: {
      e = m_fac.CreateIdentifier("::" + *RecurseName());
      break;
    }
  }

  return e;
}

auto GeneralParser::PImpl::RecurseExprTypeSuffix(FlowPtr<Expr> base) -> FlowPtr<Expr> {
  auto tok = Current();

  auto suffix = RecurseType();
  suffix->SetOffset(tok.GetStart());

  return m_fac.CreateBinary(std::move(base), OpAs, suffix);
}

auto GeneralParser::PImpl::RecurseExprPrimary(bool is_type) -> NullableFlowPtr<Expr> {
  auto start_pos = Peek().GetStart();

  NullableFlowPtr<Expr> e;

  if (is_type) {
    auto comments = m_rd.CommentBuffer();
    m_rd.ClearCommentBuffer();

    auto type = RecurseType();
    type->SetOffset(start_pos);

    e = BindComments(type, comments);
  } else {
    auto tok = Peek();

    auto comments = m_rd.CommentBuffer();
    m_rd.ClearCommentBuffer();

    switch (tok.GetKind()) {
      case EofF: {
        break;
      }

      case KeyW: {
        Next();
        if ((e = RecurseExprKeyword(tok.GetKeyword())).has_value()) {
          e.value()->SetOffset(start_pos);
        }

        break;
      }

      case Oper: {
        Log << SyntaxError << Next() << "Unexpected operator in expression";
        break;
      }

      case Punc: {
        Next();

        if ((e = RecurseExprPunctor(tok.GetPunctor())).has_value()) {
          e.value()->SetOffset(start_pos);
        }

        break;
      }

      case Name: {
        auto identifier = m_fac.CreateIdentifier(RecurseName());
        identifier->SetOffset(start_pos);

        e = identifier;
        break;
      }

      case IntL: {
        Next();

        auto integer = m_fac.CreateIntegerUnchecked(tok.GetString());
        integer->SetOffset(start_pos);

        if (Peek().Is(Name)) {
          auto casted = RecurseExprTypeSuffix(integer);
          casted->SetOffset(start_pos);

          e = casted;
        } else {
          e = integer;
        }

        break;
      }

      case NumL: {
        Next();

        auto decimal = m_fac.CreateFloatUnchecked(tok.GetString());
        decimal->SetOffset(start_pos);

        if (Peek().Is(Name)) {
          auto casted = RecurseExprTypeSuffix(decimal);
          casted->SetOffset(start_pos);

          e = casted;
        } else {
          e = decimal;
        }

        break;
      }

      case Text: {
        Next();

        auto string = m_fac.CreateString(tok.GetString());
        string->SetOffset(start_pos);

        if (Peek().Is(Name)) {
          auto casted = RecurseExprTypeSuffix(string);
          casted->SetOffset(start_pos);

          e = casted;
        } else {
          e = string;
        }

        break;
      }

      case Char: {
        Next();

        auto str_data = tok.GetString();
        if (str_data->size() != 1) [[unlikely]] {
          Log << SyntaxError << tok << "Expected a single byte in character literal";
          break;
        }

        auto character = m_fac.CreateCharacter(str_data->at(0));
        character->SetOffset(start_pos);

        if (Peek().Is(Name)) {
          auto casted = RecurseExprTypeSuffix(character);
          casted->SetOffset(start_pos);

          e = casted;
        } else {
          e = character;
        }

        break;
      }

      case MacB: {
        Log << SyntaxError << Next() << "Unexpected macro block in expression";
        break;
      }

      case Macr: {
        Log << SyntaxError << Next() << "Unexpected macro call in expression";
        break;
      }

      case Note: {
        Log << SyntaxError << Next() << "Unexpected comment in expression";
        break;
      }
    }

    if (e.has_value()) {
      e = BindComments(e.value(), comments);
    }
  }

  return e;
}
