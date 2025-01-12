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

#include "nitrate-core/NewLogger.hh"
#include "nitrate-parser/EC.hh"

using namespace ncc;
using namespace ncc::parse;
using namespace ncc::lex;

FlowPtr<Stmt> Parser::recurse_block(bool expect_braces, bool single_stmt,
                                    SafetyMode safety) {
  if (expect_braces && !next().is<PuncLCur>()) {
    log << SyntaxError << current() << "Expected '{'";
  }

  auto block_start = current().get_start();
  BlockItems items;

  while (true) {
    Token tok = peek();

    if (expect_braces && next_if(PuncRCur)) {
      auto block = make<Block>(items, safety)();
      block->set_offset(tok.get_start());

      return block;
    }

    if (single_stmt && items.size() == 1) {
      break;
    }

    if (next_if(EofF)) {
      break;
    }

    /* Ignore extra semicolons */
    if (next_if(PuncSemi)) {
      continue;
    }

    if (!tok.is(KeyW)) {
      auto expr = recurse_expr({
          Token(Punc, PuncSemi),
      });

      if (!next_if(PuncSemi)) {
        log << SyntaxError << tok << "Expected ';' after expression";
      }

      auto stmt = make<ExprStmt>(expr)();
      stmt->set_offset(expr->begin());

      items.push_back(stmt);
      continue;
    }

    auto loc_start = tok.get_start();

    switch (next(), tok.as_key()) {
      case Var: {
        for (auto decl : recurse_variable(VarDeclType::Var)) {
          items.push_back(decl);
        }
        break;
      }

      case Let: {
        for (auto decl : recurse_variable(VarDeclType::Let)) {
          items.push_back(decl);
        }
        break;
      }

      case Const: {
        for (auto decl : recurse_variable(VarDeclType::Const)) {
          items.push_back(decl);
        }
        break;
      }

      case Enum: {
        auto node = recurse_enum();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Struct: {
        auto node = recurse_struct(CompositeType::Struct);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Region: {
        auto node = recurse_struct(CompositeType::Region);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Group: {
        auto node = recurse_struct(CompositeType::Group);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Class: {
        auto node = recurse_struct(CompositeType::Class);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Union: {
        auto node = recurse_struct(CompositeType::Union);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Keyword::Type: {
        auto node = recurse_typedef();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Scope: {
        auto node = recurse_scope();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Fn: {
        auto node = recurse_function(false);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Pub:
      case Import: {  // they both declare external functions
        auto node = recurse_export(Vis::Pub);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Sec: {
        auto node = recurse_export(Vis::Sec);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Pro: {
        auto node = recurse_export(Vis::Pro);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Return: {
        auto node = recurse_return();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Retif: {
        auto node = recurse_retif();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Break: {
        auto node = make<BreakStmt>()();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Continue: {
        auto node = make<ContinueStmt>()();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case If: {
        auto node = recurse_if();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case While: {
        auto node = recurse_while();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case For: {
        auto node = recurse_for();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Foreach: {
        auto node = recurse_foreach();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Switch: {
        auto node = recurse_switch();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case __Asm__: {
        auto node = recurse_inline_asm();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case True: {
        auto node = make<ExprStmt>(make<ConstBool>(true)())();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case False: {
        auto node = make<ExprStmt>(make<ConstBool>(false)())();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case __FString: {
        auto node = make<ExprStmt>(recurse_fstring())();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case Unsafe: {
        if (peek().is<PuncLCur>()) {
          auto node = recurse_block(true, false, SafetyMode::Unsafe);
          node->set_offset(loc_start);

          items.push_back(node);
        } else {
          auto node = recurse_block(false, true, SafetyMode::Unsafe);
          node->set_offset(loc_start);

          items.push_back(node);
        }

        break;
      }

      case Safe: {
        if (peek().is<PuncLCur>()) {
          auto node = recurse_block(true, false, SafetyMode::Safe);
          node->set_offset(loc_start);

          items.push_back(node);
        } else {
          auto node = recurse_block(false, true, SafetyMode::Safe);
          node->set_offset(loc_start);

          items.push_back(node);
        }

        break;
      }

      default: {
        log << SyntaxError << tok << "Unexpected keyword";
        break;
      }
    }
  }

  if (expect_braces) {
    log << SyntaxError << current() << "Expected '}'";
  }

  auto block = make<Block>(items, safety)();
  block->set_offset(block_start);

  return block;
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
