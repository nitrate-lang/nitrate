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
#include <nitrate-parser/ASTReader.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>

#include "nitrate-lexer/Token.hh"

using namespace ncc;
using namespace ncc::parse;
using namespace ncc::lex;

FlowPtr<Stmt> Parser::recurse_block(bool expect_braces, bool single_stmt,
                                    SafetyMode safety) {
  if (expect_braces && !next().is<PuncLCur>()) {
    log << SyntaxError << current() << "Expected '{'";
  }

  auto block_start = current().get_start();
  BlockItems statements;

  auto block_comments = rd.CommentBuffer();
  rd.ClearCommentBuffer();

  while (true) {
    /* Ignore extra semicolons */
    if (next_if(PuncSemi)) {
      continue;
    }

    { /* Detect exit conditon */
      bool should_break = (expect_braces && next_if(PuncRCur)) ||
                          (single_stmt && statements.size() == 1);

      if (!should_break && next_if(EofF)) {
        if (expect_braces) {
          log << SyntaxError << current() << "Expected '}'";
        }

        should_break = true;
      }

      if (should_break) {
        auto block = make<Block>(statements, safety)();
        block->set_offset(block_start);

        return BIND_COMMENTS(block, block_comments);
      }
    }

    if (!peek().is(KeyW)) {
      auto comments = rd.CommentBuffer();
      rd.ClearCommentBuffer();

      auto expr = recurse_expr({
          Token(Punc, PuncSemi),
      });

      if (!next_if(PuncSemi)) {
        log << SyntaxError << current()
            << "Expected ';' after statement expression";
      }

      auto stmt = make<ExprStmt>(expr)();
      stmt->set_offset(expr->begin());

      statements.push_back(BIND_COMMENTS(stmt, comments));
    } else {
      auto tok = next();
      auto loc_start = tok.get_start();
      NullableFlowPtr<Stmt> R;

      auto comments = rd.CommentBuffer();
      rd.ClearCommentBuffer();

      switch (tok.as_key()) {
        case Scope: {
          R = recurse_scope();
          break;
        }

        case Import:
        case Pub: {  // they both declare external functions
          R = recurse_export(Vis::Pub);
          break;
        }

        case Sec: {
          R = recurse_export(Vis::Sec);
          break;
        }

        case Pro: {
          R = recurse_export(Vis::Pro);
          break;
        }

        case Keyword::Type: {
          R = recurse_typedef();
          break;
        }

        case Let: {
          for (auto decl : recurse_variable(VarDeclType::Let)) {
            statements.push_back(BIND_COMMENTS(decl, comments));
          }
          break;
        }

        case Var: {
          for (auto decl : recurse_variable(VarDeclType::Var)) {
            statements.push_back(BIND_COMMENTS(decl, comments));
          }
          break;
        }

        case Const: {
          for (auto decl : recurse_variable(VarDeclType::Const)) {
            statements.push_back(BIND_COMMENTS(decl, comments));
          }
          break;
        }

        case Static: {
          log << SyntaxError << current()
              << "Static variables are not yet "
                 "supported";
          break;
        }

        case Struct: {
          R = recurse_struct(CompositeType::Struct);
          break;
        }

        case Region: {
          R = recurse_struct(CompositeType::Region);
          break;
        }

        case Group: {
          R = recurse_struct(CompositeType::Group);
          break;
        }

        case Class: {
          R = recurse_struct(CompositeType::Class);
          break;
        }

        case Union: {
          R = recurse_struct(CompositeType::Union);
          break;
        }

        case Opaque: {
          log << SyntaxError << current()
              << "Unexpected 'opaque' in block context";
          break;
        }

        case Enum: {
          R = recurse_enum();
          break;
        }

        case __FString: {
          R = make<ExprStmt>(recurse_fstring())();
          break;
        }

        case Fn: {
          R = recurse_function(false);
          break;
        }

        case Unsafe: {
          if (peek().is<PuncLCur>()) {
            R = recurse_block(true, false, SafetyMode::Unsafe);
          } else {
            R = recurse_block(false, true, SafetyMode::Unsafe);
          }

          break;
        }

        case Safe: {
          if (peek().is<PuncLCur>()) {
            R = recurse_block(true, false, SafetyMode::Safe);
          } else {
            R = recurse_block(false, true, SafetyMode::Safe);
          }

          break;
        }

        case Promise: {
          log << SyntaxError << current()
              << "Unexpected 'promise' in block context";
          break;
        }

        case If: {
          R = recurse_if();
          break;
        }

        case Else: {
          log << SyntaxError << current()
              << "Unexpected 'else' in block context";
          break;
        }

        case For: {
          R = recurse_for();
          break;
        }

        case While: {
          R = recurse_while();
          break;
        }

        case Do: {
          log << SyntaxError << current() << "Unexpected 'do' in block context";
          break;
        }

        case Switch: {
          R = recurse_switch();
          break;
        }

        case Break: {
          R = make<BreakStmt>()();
          break;
        }

        case Continue: {
          R = make<ContinueStmt>()();
          break;
        }

        case Return: {
          R = recurse_return();
          break;
        }

        case Retif: {
          R = recurse_retif();
          break;
        }

        case Foreach: {
          R = recurse_foreach();
          break;
        }

        case Try: {
          R = recurse_try();
          break;
        }

        case Catch: {
          log << SyntaxError << current()
              << "Unexpected 'catch' in block context";
          break;
        }

        case Throw: {
          R = recurse_throw();
          break;
        }

        case Async: {
          log << SyntaxError << current()
              << "Unexpected 'async' in block context";
          break;
        }

        case Await: {
          R = recurse_await();
          break;
        }

        case __Asm__: {
          R = recurse_inline_asm();
          break;
        }

        case Undef: {
          R = make<ExprStmt>(make<ConstUndef>()())();
          break;
        }

        case Null: {
          R = make<ExprStmt>(make<ConstNull>()())();
          break;
        }

        case True: {
          R = make<ExprStmt>(make<ConstBool>(true)())();
          break;
        }

        case False: {
          R = make<ExprStmt>(make<ConstBool>(false)())();
          break;
        }
      }

      if (R.has_value()) {
        R.value()->set_offset(loc_start);
        R = BIND_COMMENTS(R.value(), comments);
        statements.push_back(R.value());
      }
    }
  }
}

NCC_EXPORT Parser::Parser(ncc::lex::IScanner &lexer,
                          std::shared_ptr<ncc::Environment> env,
                          std::shared_ptr<void> lifetime)
    : rd(lexer), m_lifetime(lifetime) {
  m_env = env;
  m_allocator = std::make_unique<ncc::dyn_arena>();
  m_failed = false;
}

void Parser_SetCurrentScanner(IScanner *scanner);

NCC_EXPORT ASTRoot Parser::parse() {
  std::optional<ASTRoot> ast;

  { /* Assign the current context to thread-local global state */
    Parser_SetCurrentScanner(&rd);

    { /* Subscribe to events emitted by the parser */
      auto sub_id = log.subscribe([&](auto, auto, const auto &ec) {
        if (ec.getKind() == SyntaxError.getKind()) {
          SetFailBit();
        }
      });

      { /* Configure the scanner to ignore comments */
        auto old_state = rd.GetSkipCommentsState();
        rd.SkipCommentsState(true);

        {   /* Parse the input */
          { /* Swap in an arena allocator */
            std::swap(npar_allocator, m_allocator);

            /* Recursive descent parsing */
            auto node = recurse_block(false, false, SafetyMode::Unknown);

            std::swap(npar_allocator, m_allocator);

            ast = ASTRoot(node, std::move(m_allocator), m_failed);
          }

          /* Create a new allocator */
          m_allocator = std::make_unique<ncc::dyn_arena>();
        }

        rd.SkipCommentsState(old_state);
      }

      log.unsubscribe(sub_id);
    }

    Parser_SetCurrentScanner(nullptr);
  }

  return ast.value();
}

NCC_EXPORT bool ASTRoot::check() const {
  if (m_failed) {
    return false;
  }

  bool failed = false;
  iterate<dfs_pre>(m_base, [&](auto, auto c) {
    failed |= !c || c->is_mock();

    return failed ? IterOp::Abort : IterOp::Proceed;
  });

  return !failed;
}
