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
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTType.hh>
#include <stack>
#include <utility>

static constexpr size_t kMaxRecursionDepth = 4096;
static constexpr size_t kMaxListRepeatCount = 4096;

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;
using namespace ncc;

auto GeneralParser::Context::RecurseAttributes(string kind) -> std::vector<FlowPtr<Expr>> {
  std::vector<FlowPtr<Expr>> attributes;

  if (!NextIf<PuncLBrk>()) {
    return attributes;
  }

  while (true) {
    if (m.IsEof()) [[unlikely]] {
      Log << ParserSignal << Current() << "Encountered EOF while parsing " << kind << " attributes";
      return attributes;
    }

    if (NextIf<PuncRBrk>()) {
      return attributes;
    }

    auto attribute = RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncRBrk),
    });

    attributes.push_back(attribute);

    NextIf<PuncComa>();
  }
}

auto GeneralParser::Context::RecurseCallArguments(const std::set<lex::Token> &end,
                                                  bool type_by_default) -> std::vector<CallArg> {
  std::vector<CallArg> call_args;
  size_t positional_index = 0;
  string argument_name;

  while (true) {
    if (m.IsEof()) [[unlikely]] {
      Log << ParserSignal << Current() << "Unexpected end of file while parsing call expression";
      return call_args;
    }

    if (end.contains(Peek())) {
      break;
    }

    auto some_identifier = NextIf<Name>();
    auto next_is_colon = some_identifier && NextIf<PuncColn>().has_value();

    if (some_identifier && next_is_colon) {
      argument_name = some_identifier->GetString();
    } else {
      if (some_identifier && !next_is_colon) {
        m.GetScanner().Insert(some_identifier.value());
      }

      argument_name = string(std::to_string(positional_index++));
    }

    if (type_by_default) {
      auto argument_value = RecurseType();
      call_args.emplace_back(argument_name, argument_value);
    } else {
      std::set<Token> end_copy(end);
      end_copy.insert(Token(Punc, PuncComa));
      auto argument_value = RecurseExpr(end_copy);
      call_args.emplace_back(argument_name, argument_value);
    }

    NextIf<PuncComa>();
  }

  return call_args;
}

static auto ParseFStringExpression(GeneralParser::Context &m, std::string_view source) -> FlowPtr<Expr> {
  std::vector<std::variant<string, FlowPtr<Expr>>> sections;
  std::string buf;
  size_t rank = 0;
  bool in_escape = false;

  buf.reserve(source.size());

  for (char ch : source) {
    if (ch == '{') {
      if (!in_escape && !buf.empty()) {
        sections.emplace_back(string(std::move(buf)));
        buf.clear();
      }

      rank++;
      in_escape = true;
    } else if (ch == '}') {
      rank--;
    }

    buf.push_back(ch);

    if (rank == 0 && in_escape) [[unlikely]] {
      in_escape = false;

      qcore_assert(buf.starts_with("{"));
      qcore_assert(buf.ends_with("}"));

      auto buf_view = buf.substr(1, buf.size() - 1);

      auto in_src = boost::iostreams::stream<boost::iostreams::array_source>(buf_view.data(), buf_view.size());
      auto scanner = Tokenizer(in_src, m.GetEnvironment());
      auto sub_parser = m.CreateSubParser(scanner);
      auto subnode = m.GetPImplPtr(sub_parser)
                         ->RecurseExpr({
                             Token(Punc, PuncRCur),
                         });

      sections.emplace_back(subnode);

      buf.clear();
    }
  }

  if (!buf.empty()) {
    sections.emplace_back(string(std::move(buf)));
    buf.clear();
  }

  if (rank != 0) [[unlikely]] {
    Log << ParserSignal << m.Current() << "F-string expression has mismatched braces";
  }

  return m.CreateFormatString(sections);
}

auto GeneralParser::Context::RecurseFString() -> FlowPtr<Expr> {
  const auto fstring_raw_text = NextIf<Text>();
  if (!fstring_raw_text) [[unlikely]] {
    Log << ParserSignal << Current() << "Expected a string literal token for F-string expression";
    return m.CreateMockInstance<FString>();
  }

  return ParseFStringExpression(m, fstring_raw_text->GetString());
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

static auto RecurseExprKeyword(GeneralParser::Context &m, lex::Keyword key) -> NullableFlowPtr<Expr> {
  NullableFlowPtr<Expr> e;

  switch (key) {
    case Keyword::Type: {
      auto start_pos = m.Current().GetStart();
      auto type = m.RecurseType();
      type->SetOffset(start_pos);

      e = type;
      break;
    }

    case __FString: {
      e = m.RecurseFString();
      break;
    }

    case Fn: {
      auto start_pos = m.Current().GetStart();
      auto function = m.RecurseFunction(false);
      function->SetOffset(start_pos);

      if (m.NextIf<PuncLPar>()) {
        auto args = m.RecurseCallArguments({Token(Punc, PuncRPar)}, false);

        if (m.NextIf<PuncRPar>()) {
          e = m.CreateCall(args, function);
        } else {
          Log << ParserSignal << m.Current() << "Expected ')' to close the function call";
          e = m.CreateMockInstance<Call>();
        }
      } else {
        e = function;
      }

      break;
    }

    case Keyword::Null: {
      e = m.CreateNull();
      break;
    }

    case True: {
      e = m.CreateBoolean(true);
      break;
    }

    case False: {
      e = m.CreateBoolean(false);
      break;
    }

    case EscapeBlock: {
      m.RecurseEscapeBlock();  // We discard the block
      break;
    }

    case lex::Import: {
      e = m.RecurseImport();
      break;
    }

    default: {
      Log << ParserSignal << m.Current() << "Unexpected '" << key << "' in expression context";
      break;
    }
  }

  return e;
}

static auto RecurseExprPunctor(GeneralParser::Context &m, lex::Punctor punc) -> NullableFlowPtr<Expr> {
  NullableFlowPtr<Expr> e;

  switch (punc) {
    case PuncLPar: {
      e = m.RecurseExpr({
          Token(Punc, PuncRPar),
      });
      auto depth = e.value()->GetParenthesisDepth();
      e.value()->SetParenthesisDepth(depth + 1);

      if (!m.NextIf<PuncRPar>()) {
        Log << ParserSignal << m.Current() << "Expected ')' to close the expression";
      }

      break;
    }

    case PuncRPar: {
      Log << ParserSignal << m.Current() << "Unexpected right parenthesis in expression";
      break;
    }

    case PuncLBrk: {
      std::vector<FlowPtr<Expr>> items;

      while (true) {
        if (m.IsEof()) [[unlikely]] {
          Log << ParserSignal << m.Current() << "Unexpected end of file while parsing expression";
          break;
        }

        if (m.NextIf<PuncRBrk>()) {
          break;
        }

        auto expr = m.RecurseExpr({
            Token(Punc, PuncComa),
            Token(Punc, PuncRBrk),
            Token(Punc, PuncSemi),
        });

        if (m.NextIf<PuncSemi>()) {
          if (auto count_tok = m.NextIf<IntL>()) {
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
                Log << ParserSignal << m.Current() << "Compressed list size exceeds maximum limit";
              }

            } else {
              Log << ParserSignal << m.Current()
                  << "Expected an integer literal for the compressed "
                     "list size";
            }
          } else {
            Log << ParserSignal << m.Current()
                << "Expected an integer literal for the compressed list "
                   "size";
          }
        } else {
          items.push_back(expr);
        }

        m.NextIf<PuncComa>();
      }

      e = m.CreateList(items);
      break;
    }

    case PuncRBrk: {
      Log << ParserSignal << m.Current() << "Unexpected right bracket in expression";
      break;
    }

    case PuncLCur: {
      // std::vector<FlowPtr<Expr>> items;
      std::vector<FlowPtr<Assoc>> items;

      while (true) {
        if (m.IsEof()) [[unlikely]] {
          Log << ParserSignal << m.Current() << "Unexpected end of file while parsing dictionary";
          break;
        }

        if (m.NextIf<PuncRCur>()) {
          break;
        }

        auto start_pos = m.Peek().GetStart();
        auto key = m.RecurseExpr({
            Token(Punc, PuncColn),
        });

        if (!m.NextIf<PuncColn>()) {
          Log << ParserSignal << m.Current() << "Expected colon after key in dictionary";
          break;
        }

        auto value = m.RecurseExpr({
            Token(Punc, PuncRCur),
            Token(Punc, PuncComa),
        });

        m.NextIf<PuncComa>();

        auto assoc = m.CreateAssociation(key, value);
        assoc->SetOffset(start_pos);

        items.push_back(assoc);
      }

      if (items.size() == 1) {
        e = items[0];
      } else {
        std::vector<FlowPtr<Expr>> items_copy(items.begin(), items.end());
        e = m.CreateList(items_copy);
      }

      break;
    }

    case PuncRCur: {
      Log << ParserSignal << m.Current() << "Unexpected right curly brace in expression";
      break;
    }

    case PuncComa: {
      Log << ParserSignal << m.Current() << "Unexpected comma in expression context";
      break;
    }

    case PuncColn: {
      Log << ParserSignal << m.Current() << "Unexpected colon in expression context";
      break;
    }

    case PuncSemi: {
      Log << ParserSignal << m.Current() << "Unexpected semicolon in expression context";
      break;
    }

    case PuncScope: {
      e = m.CreateIdentifier("::" + *m.RecurseName());
      break;
    }
  }

  return e;
}

static auto RecurseExprTypeSuffix(GeneralParser::Context &m, FlowPtr<Expr> base) -> FlowPtr<Expr> {
  auto tok = m.Current();
  auto suffix = m.RecurseType();
  suffix->SetOffset(tok.GetStart());

  return m.CreateBinary(std::move(base), OpAs, suffix);
}

static auto RecurseExprPrimary(GeneralParser::Context &m, bool is_type) -> NullableFlowPtr<Expr> {
  auto start_pos = m.Peek().GetStart();
  NullableFlowPtr<Expr> e;

  if (is_type) {
    auto type = m.RecurseType();
    type->SetOffset(start_pos);
    e = type;
  } else {
    auto tok = m.Peek();

    switch (tok.GetKind()) {
      case EofF: {
        break;
      }

      case KeyW: {
        m.Next();
        if ((e = RecurseExprKeyword(m, tok.GetKeyword())).has_value()) {
          e.value()->SetOffset(start_pos);
        }

        break;
      }

      case Oper: {
        Log << ParserSignal << m.Next() << "Unexpected operator in expression";
        break;
      }

      case Punc: {
        m.Next();

        if ((e = RecurseExprPunctor(m, tok.GetPunctor())).has_value()) {
          e.value()->SetOffset(start_pos);
        }

        break;
      }

      case Name: {
        auto name = m.RecurseName();
        if (!name) [[unlikely]] {
          Log << ParserSignal << m.Next() << "Expected a name in expression";
          break;
        }

        auto identifier = m.CreateIdentifier(name);
        identifier->SetOffset(start_pos);

        e = identifier;
        break;
      }

      case IntL: {
        m.Next();

        auto integer = m.CreateIntegerUnchecked(tok.GetString());
        integer->SetOffset(start_pos);

        if (m.Peek().Is(Name)) {
          auto casted = RecurseExprTypeSuffix(m, integer);
          casted->SetOffset(start_pos);

          e = casted;
        } else {
          e = integer;
        }

        break;
      }

      case NumL: {
        m.Next();

        auto decimal = m.CreateFloatUnchecked(tok.GetString());
        decimal->SetOffset(start_pos);

        if (m.Peek().Is(Name)) {
          auto casted = RecurseExprTypeSuffix(m, decimal);
          casted->SetOffset(start_pos);

          e = casted;
        } else {
          e = decimal;
        }

        break;
      }

      case Text: {
        m.Next();

        auto string = m.CreateString(tok.GetString());
        string->SetOffset(start_pos);

        if (m.Peek().Is(Name)) {
          auto casted = RecurseExprTypeSuffix(m, string);
          casted->SetOffset(start_pos);

          e = casted;
        } else {
          e = string;
        }

        break;
      }

      case Char: {
        m.Next();

        auto str_data = tok.GetString();
        if (str_data->size() != 1) [[unlikely]] {
          Log << ParserSignal << tok << "Expected a single byte in character literal";
          break;
        }

        auto character = m.CreateCharacter(str_data->at(0));
        character->SetOffset(start_pos);

        if (m.Peek().Is(Name)) {
          auto casted = RecurseExprTypeSuffix(m, character);
          casted->SetOffset(start_pos);

          e = casted;
        } else {
          e = character;
        }

        break;
      }

      case MacB: {
        Log << ParserSignal << m.Next() << "Unexpected macro block in expression";
        break;
      }

      case Macr: {
        Log << ParserSignal << m.Next() << "Unexpected macro call in expression";
        break;
      }

      case Note: {
        Log << ParserSignal << m.Next() << "Unexpected comment in expression";
        break;
      }
    }
  }

  return e;
}

auto GeneralParser::Context::RecurseExpr(const std::set<Token> &end) -> FlowPtr<Expr> {
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

  if (auto left_side_opt = RecurseExprPrimary(*this, false)) {
    auto left_side = left_side_opt.value();
    auto spinning = true;

    /****************************************
     * Combine pre-unary operators
     ****************************************/
    while (!pre_unary_ops.empty()) {
      auto [Op, Offset] = pre_unary_ops.top();
      pre_unary_ops.pop();

      left_side = CreateUnary(Op, left_side);
      left_side->SetOffset(Offset);
    }

    while (!stack.empty() && spinning) {
      auto tok = Peek();

      if (end.contains(tok)) {
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
              left_side = CreateUnary(op, left_side, true);
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
            if (auto right_side = RecurseExprPrimary(*this, is_type)) {
              /****************************************
               * Combine pre-unary operators
               ****************************************/
              while (!pre_unary_ops.empty()) {
                auto [op, Offset] = pre_unary_ops.top();
                pre_unary_ops.pop();

                auto pre_unary_expr = CreateUnary(op, right_side.value());
                pre_unary_expr->SetOffset(Offset);

                right_side = pre_unary_expr;
              }

              if (stack.size() + 1 > kMaxRecursionDepth) {
                Log << ParserSignal << Current() << "Recursion depth exceeds maximum limit";
                return CreateMockInstance<VoidTy>();
              }

              stack.emplace(left_side, source_offset, next_min_precedence, FrameType::Binary, op);
              left_side = right_side.value();
            } else {
              Log << ParserSignal << Current() << "Failed to parse right-hand side of binary expression";
            }
          } else {
            if (IsOnlyUnaryOp(op)) {
              spinning = false;
            }

            left_side = UnwindStack(*this, stack, left_side, 0);

            continue;
          }

          break;
        }

        case Punc: {
          // Based on the assumption that function calls have the same
          // precedence as the dot operator (member access)
          static auto suffix_op_precedence = GetOperatorPrecedence(OpDot, OpMode::Binary);

          left_side = UnwindStack(*this, stack, left_side, suffix_op_precedence);

          if (NextIf<PuncLPar>()) {
            auto arguments = RecurseCallArguments({Token(Punc, PuncRPar)}, false);
            if (!NextIf<PuncRPar>()) {
              Log << ParserSignal << Current() << "Expected ')' to close the function call";
            }

            left_side = CreateCall(arguments, left_side);
            left_side->SetOffset(source_offset);
          } else if (NextIf<PuncLBrk>()) {
            auto first = RecurseExpr({
                Token(Punc, PuncRBrk),
                Token(Punc, PuncColn),
            });

            if (NextIf<PuncColn>()) {
              auto second = RecurseExpr({Token(Punc, PuncRBrk)});
              if (!NextIf<PuncRBrk>()) {
                Log << ParserSignal << Current() << "Expected ']' to close the slice";
              }

              left_side = CreateSlice(left_side, first, second);
              left_side->SetOffset(source_offset);
            } else {
              if (!NextIf<PuncRBrk>()) {
                Log << ParserSignal << Current() << "Expected ']' to close the index expression";
              }

              left_side = CreateIndex(left_side, first);
              left_side->SetOffset(source_offset);
            }
          } else if (NextIf<PuncLCur>()) {
            auto template_arguments = RecurseCallArguments({Token(Punc, PuncRCur)}, true);
            if (!NextIf<PuncRCur>()) {
              Log << ParserSignal << Current() << "Expected '}' to close the template arguments";
            }

            if (!NextIf<PuncLPar>()) {
              Log << ParserSignal << Current() << "Expected '(' to open the call arguments";
            }

            auto call_arguments = RecurseCallArguments({Token(Punc, PuncRPar)}, false);
            if (!NextIf<PuncRPar>()) {
              Log << ParserSignal << Current() << "Expected ')' to close the call arguments";
            }

            left_side = CreateTemplateCall(std::move(template_arguments), std::move(call_arguments), left_side);
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

    return UnwindStack(*this, stack, left_side, 0);
  }
  Log << ParserSignal << Current() << "Expected an expression";

  return CreateMockInstance<VoidTy>();
}
