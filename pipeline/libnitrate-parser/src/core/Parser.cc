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

#include <core/Context.hh>
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

using namespace ncc;
using namespace ncc::parse;
using namespace ncc::lex;

FlowPtr<Stmt> Parser::recurse_block(bool expect_braces, bool single_stmt,
                                    SafetyMode safety) {
  if (expect_braces && !next().is<PuncLCur>()) {
    diagnostic << current() << "Expected '{'";
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
        diagnostic << tok << "Expected ';' after expression";
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
        diagnostic << tok << "Unexpected keyword";
        break;
      }
    }
  }

  if (expect_braces) {
    diagnostic << current() << "Expected '}'";
  }

  auto block = make<Block>(items, safety)();
  block->set_offset(block_start);

  return block;
}

CPP_EXPORT Parser::Parser(ncc::lex::IScanner &lexer,
                          std::shared_ptr<ncc::Environment> env,
                          std::shared_ptr<void> lifetime)
    : rd(lexer), m_lifetime(lifetime) {
  m_env = env;
  m_allocator = std::make_unique<ncc::dyn_arena>();
  m_failed = false;
}

CPP_EXPORT ASTRoot Parser::parse() {
  auto old_state = rd.GetSkipCommentsState();
  rd.SkipCommentsState(true);

  /*== Install thread-local references to the parser ==*/
  auto old = diagnostic;
  diagnostic = this;

  std::swap(npar_allocator, m_allocator);
  auto node = recurse_block(false, false, SafetyMode::Unknown);
  std::swap(npar_allocator, m_allocator);

  ASTRoot ast(node, std::move(m_allocator), m_failed);
  m_allocator = std::make_unique<ncc::dyn_arena>();

  /*== Uninstall thread-local references to the parser ==*/
  diagnostic = old;

  rd.SkipCommentsState(old_state);

  return ast;
}

CPP_EXPORT bool ASTRoot::check() const {
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

std::string parse::mint_clang16_message(ncc::lex::IScanner &rd,
                                        std::string_view message,
                                        ncc::lex::Token tok) {
  auto token_start = rd.Start(tok), token_end = rd.End(tok);
  auto start_filename = token_start.GetFilename();
  auto start_line = token_start.GetRow(), start_col = token_start.GetCol();
  auto end_line = token_end.GetRow();

  std::stringstream ss;
  ss << "\x1b[37;1m" << (start_filename->empty() ? "?" : start_filename) << ":";
  ss << (start_line == QLEX_EOFF ? "?" : std::to_string(start_line)) << ":";
  ss << (start_col == QLEX_EOFF ? "?" : std::to_string(start_col))
     << ":\x1b[0m ";
  ss << "\x1b[37;1m" << message << " [SyntaxError]\x1b[0m";

  if (start_line != QLEX_EOFF) {
    IScanner::Point start_pos(start_line == 0 ? 0 : start_line - 1, 0), end_pos;

    if (end_line != QLEX_EOFF) {
      end_pos = IScanner::Point(end_line + 1, -1);
    } else {
      end_pos = IScanner::Point(start_line + 1, -1);
    }

    if (auto window = rd.GetSourceWindow(start_pos, end_pos, ' ')) {
      ss << "\n";

      for (auto &line : window.value()) {
        ss << line << "\n";
      }

      for (uint32_t i = 0; i < start_col; i++) {
        ss << " ";
      }
      ss << "\x1b[32;1m^\x1b[0m";
    }
  }

  return ss.str();
}
