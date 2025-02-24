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

#include <cstring>
#include <descent/Recurse.hh>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Algorithm.hh>
#include <nitrate-parser/Context.hh>

using namespace ncc;
using namespace ncc::parse;
using namespace ncc::lex;

auto Parser::PImpl::RecurseName() -> string {
  enum State {
    Start,
    RequireName,
    RequireScopeOrEnd,
    Exit,
  } state = Start;

  Token last;
  std::string name;

  while (state != Exit) {
    last = Peek();

    switch (state) {
      case Start: {
        if (last.Is<PuncScope>()) {
          name += "::";
          state = RequireName;
          Next();
        } else if (last.Is(Name)) {
          name += last.GetString();
          state = RequireScopeOrEnd;
          Next();
        } else {
          /* No identifier to parse */
          state = Exit;
        }
        break;
      }

      case RequireName: {
        if (last.Is(Name)) {
          name += last.GetString();
          state = RequireScopeOrEnd;
          Next();
        } else {
          Log << SyntaxError << last << "Expected identifier after '::'";
          name.clear();
          state = Exit;
          Next();  // Prevent infinite loops elsewhere
        }
        break;
      }

      case RequireScopeOrEnd: {
        if (last.Is<PuncScope>()) {
          name += "::";
          state = RequireName;
          Next();
        } else {
          state = Exit;
        }
        break;
      }

      case Exit: {
        break;
      }
    }
  }

  return name;
}

auto Parser::PImpl::RecurseBlock(bool expect_braces, bool single_stmt, BlockMode safety) -> FlowPtr<Expr> {
  if (expect_braces && !Next().Is<PuncLCur>()) {
    Log << SyntaxError << Current() << "Expected '{'";
  }

  auto block_start = Current().GetStart();
  std::vector<FlowPtr<Expr>> statements;

  auto block_comments = m_rd.CommentBuffer();
  m_rd.ClearCommentBuffer();

  while (true) {
    /* Ignore extra semicolons */
    if (NextIf<PuncSemi>()) {
      continue;
    }

    { /* Detect exit conditon */
      bool should_break = (expect_braces && NextIf<PuncRCur>()) || (single_stmt && statements.size() == 1);

      if (!should_break && NextIf<EofF>()) {
        if (expect_braces) {
          Log << SyntaxError << Current() << "Expected '}'";
        }

        should_break = true;
      }

      if (should_break) {
        auto block = m_fac.CreateBlock(statements, safety);
        block->SetOffset(block_start);

        return BindComments(block, block_comments);
      }
    }

    if (!Peek().Is(KeyW)) {
      auto comments = m_rd.CommentBuffer();
      m_rd.ClearCommentBuffer();

      auto expr = RecurseExpr({
          Token(Punc, PuncSemi),
      });

      if (!NextIf<PuncSemi>()) {
        Log << SyntaxError << Current() << "Expected ';' after statement expression";
      }

      statements.push_back(BindComments(expr, comments));
    } else {
      auto tok = Next();
      auto loc_start = tok.GetStart();
      NullableFlowPtr<Expr> r;

      auto comments = m_rd.CommentBuffer();
      m_rd.ClearCommentBuffer();

      switch (tok.GetKeyword()) {
        case Keyword::Scope: {
          r = RecurseScope();
          break;
        }

        case Pub: {  // they both declare external functions
          r = RecurseExport(Vis::Pub);
          break;
        }

        case Sec: {
          r = RecurseExport(Vis::Sec);
          break;
        }

        case Pro: {
          r = RecurseExport(Vis::Pro);
          break;
        }

        case Import: {
          Log << SyntaxError << Current() << "Unexpected 'import' in block context";
          break;
        }

        case Keyword::Type: {
          r = RecurseTypedef();
          break;
        }

        case Let: {
          for (const auto &variable : RecurseVariable(VariableType::Let)) {
            statements.push_back(BindComments(variable, comments));
            comments.clear();
          }
          break;
        }

        case Var: {
          for (const auto &variable : RecurseVariable(VariableType::Var)) {
            statements.push_back(BindComments(variable, comments));
            comments.clear();
          }
          break;
        }

        case Const: {
          for (const auto &variable : RecurseVariable(VariableType::Const)) {
            statements.push_back(BindComments(variable, comments));
            comments.clear();
          }
          break;
        }

        case Static: {
          Log << SyntaxError << Current()
              << "Static variables are not yet "
                 "supported";
          break;
        }

        case Keyword::Struct: {
          r = RecurseStruct(CompositeType::Struct);
          break;
        }

        case Region: {
          r = RecurseStruct(CompositeType::Region);
          break;
        }

        case Group: {
          r = RecurseStruct(CompositeType::Group);
          break;
        }

        case Class: {
          r = RecurseStruct(CompositeType::Class);
          break;
        }

        case Union: {
          r = RecurseStruct(CompositeType::Union);
          break;
        }

        case Opaque: {
          Log << SyntaxError << Current() << "Unexpected 'opaque' in block context";
          break;
        }

        case Keyword::Enum: {
          r = RecurseEnum();
          break;
        }

        case __FString: {
          r = RecurseFstring();
          if (!NextIf<PuncSemi>()) {
            Log << SyntaxError << Current() << "Expected ';' after f-string expression";
          }
          break;
        }

        case Fn: {
          r = RecurseFunction(false);
          break;
        }

        case Unsafe: {
          if (Peek().Is<PuncLCur>()) {
            r = RecurseBlock(true, false, BlockMode::Unsafe);
          } else {
            r = RecurseBlock(false, true, BlockMode::Unsafe);
          }

          break;
        }

        case Safe: {
          if (Peek().Is<PuncLCur>()) {
            r = RecurseBlock(true, false, BlockMode::Safe);
          } else {
            r = RecurseBlock(false, true, BlockMode::Safe);
          }

          break;
        }

        case Promise: {
          Log << SyntaxError << Current() << "Unexpected 'promise' in block context";
          break;
        }

        case Keyword::If: {
          r = RecurseIf();
          break;
        }

        case Else: {
          Log << SyntaxError << Current() << "Unexpected 'else' in block context";
          break;
        }

        case Keyword::For: {
          r = RecurseFor();
          break;
        }

        case Keyword::While: {
          r = RecurseWhile();
          break;
        }

        case Do: {
          Log << SyntaxError << Current() << "Unexpected 'do' in block context";
          break;
        }

        case Keyword::Switch: {
          r = RecurseSwitch();
          break;
        }

        case Keyword::Break: {
          r = m_fac.CreateBreak();
          if (!NextIf<PuncSemi>()) {
            Log << SyntaxError << Current() << "Expected ';' after 'break' statement";
          }

          break;
        }

        case Keyword::Continue: {
          r = m_fac.CreateContinue();
          if (!NextIf<PuncSemi>()) {
            Log << SyntaxError << Current() << "Expected ';' after 'continue' statement";
          }
          break;
        }

        case Keyword::Return: {
          r = RecurseReturn();
          break;
        }

        case Retif: {
          r = RecurseReturnIf();
          break;
        }

        case Keyword::Foreach: {
          r = RecurseForeach();
          break;
        }

        case Try: {
          r = RecurseTry();
          break;
        }

        case Catch: {
          Log << SyntaxError << Current() << "Unexpected 'catch' in block context";
          break;
        }

        case Throw: {
          r = RecurseThrow();
          break;
        }

        case Async: {
          Log << SyntaxError << Current() << "Unexpected 'async' in block context";
          break;
        }

        case Await: {
          r = RecurseAwait();
          break;
        }

        case __Asm__: {
          r = RecurseAssembly();
          break;
        }

        case Undef: {
          r = m_fac.CreateUndefined();
          if (!NextIf<PuncSemi>()) {
            Log << SyntaxError << Current() << "Expected ';' after 'undef' statement";
          }
          break;
        }

        case Keyword::Null: {
          r = m_fac.CreateNull();
          if (!NextIf<PuncSemi>()) {
            Log << SyntaxError << Current() << "Expected ';' after 'null' statement";
          }
          break;
        }

        case True: {
          r = m_fac.CreateBoolean(true);
          if (!NextIf<PuncSemi>()) {
            Log << SyntaxError << Current() << "Expected ';' after 'true' statement";
          }
          break;
        }

        case False: {
          r = m_fac.CreateBoolean(false);
          if (!NextIf<PuncSemi>()) {
            Log << SyntaxError << Current() << "Expected ';' after 'false' statement";
          }
          break;
        }
      }

      if (r.has_value()) {
        r.value()->SetOffset(loc_start);
        r = BindComments(r.value(), comments);
        statements.push_back(r.value());
      }
    }
  }
}

Parser::Parser(ncc::lex::IScanner &lexer, std::shared_ptr<ncc::IEnvironment> env, std::shared_ptr<void> lifetime)
    : m_impl(std::make_unique<Parser::PImpl>(lexer, std::move(env), std::move(lifetime))) {}

Parser::~Parser() = default;

void ParserSetCurrentScanner(IScanner *scanner);

void Parser::SetFailBit() { m_impl->m_failed = true; }

auto Parser::GetLexer() -> lex::IScanner & { return m_impl->m_rd; }

auto Parser::Parse() -> ASTRoot {
  std::optional<ASTRoot> ast;

  { /* Assign the current context to thread-local global state */
    ParserSetCurrentScanner(&m_impl->m_rd);

    { /* Subscribe to events emitted by the parser */
      auto sub_id = Log.Subscribe([&](auto, auto, const auto &ec) {
        if (ec.GetKind() == SyntaxError.GetKind()) {
          SetFailBit();
        }
      });

      { /* Configure the scanner to ignore comments */
        auto old_state = m_impl->m_rd.GetSkipCommentsState();
        m_impl->m_rd.SkipCommentsState(true);

        {   /* Parse the input */
          { /* Swap in an arena allocator */
            std::swap(MainAllocator, m_impl->m_allocator);

            /* Recursive descent parsing */
            auto node = m_impl->RecurseBlock(false, false, BlockMode::Unknown);

            std::swap(MainAllocator, m_impl->m_allocator);

            if (m_impl->m_rd.HasError()) {
              Log << SyntaxError << "Some lexical errors have occurred";
            }

            ast = ASTRoot(node, std::move(m_impl->m_allocator), m_impl->m_failed);
          }

          /* Recreate the thread-local allocator */
          m_impl->m_allocator = std::make_unique<ncc::DynamicArena>();
        }

        m_impl->m_rd.SkipCommentsState(old_state);
      }

      Log.Unsubscribe(sub_id);
    }

    ParserSetCurrentScanner(nullptr);
  }

  return ast.value();
}

auto ASTRoot::Check() const -> bool {
  if (m_failed) {
    return false;
  }

  bool failed = false;
  iterate<dfs_pre>(m_base, [&](auto, auto c) {
    failed |= !c || c->IsMock();

    return failed ? IterOp::Abort : IterOp::Proceed;
  });

  return !failed;
}
