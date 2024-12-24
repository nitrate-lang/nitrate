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
      auto expr = recurse_expr({Token(Punc, PuncSemi)});

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
      case qKVar: {
        for (auto decl : recurse_variable(VarDeclType::Var)) {
          items.push_back(decl);
        }
        break;
      }

      case qKLet: {
        for (auto decl : recurse_variable(VarDeclType::Let)) {
          items.push_back(decl);
        }
        break;
      }

      case qKConst: {
        for (auto decl : recurse_variable(VarDeclType::Const)) {
          items.push_back(decl);
        }
        break;
      }

      case qKEnum: {
        auto node = recurse_enum();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKStruct: {
        auto node = recurse_struct(CompositeType::Struct);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKRegion: {
        auto node = recurse_struct(CompositeType::Region);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKGroup: {
        auto node = recurse_struct(CompositeType::Group);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKClass: {
        auto node = recurse_struct(CompositeType::Class);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKUnion: {
        auto node = recurse_struct(CompositeType::Union);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKType: {
        auto node = recurse_typedef();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKScope: {
        auto node = recurse_scope();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKFn: {
        auto node = recurse_function(false);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKPub:
      case qKImport: {  // they both declare external functions
        auto node = recurse_pub();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKSec: {
        auto node = recurse_sec();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKPro: {
        auto node = recurse_pro();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKReturn: {
        auto node = recurse_return();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKRetif: {
        auto node = recurse_retif();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKBreak: {
        auto node = make<BreakStmt>()();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKContinue: {
        auto node = make<ContinueStmt>()();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKIf: {
        auto node = recurse_if();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKWhile: {
        auto node = recurse_while();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKFor: {
        auto node = recurse_for();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKForeach: {
        auto node = recurse_foreach();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKSwitch: {
        auto node = recurse_switch();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qK__Asm__: {
        auto node = recurse_inline_asm();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKTrue: {
        auto node = make<ExprStmt>(make<ConstBool>(true)())();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKFalse: {
        auto node = make<ExprStmt>(make<ConstBool>(false)())();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qK__FString: {
        auto node = make<ExprStmt>(recurse_fstring())();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKUnsafe: {
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

      case qKSafe: {
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
  auto old = ncc::parse::diagnostic;
  ncc::parse::diagnostic = this;

  std::swap(ncc::parse::npar_allocator, m_allocator);
  auto node = recurse_block(false, false, SafetyMode::Unknown);
  std::swap(ncc::parse::npar_allocator, m_allocator);

  ASTRoot ast(node, std::move(m_allocator), m_failed);
  m_allocator = std::make_unique<ncc::dyn_arena>();

  /*== Uninstall thread-local references to the parser ==*/
  ncc::parse::diagnostic = old;

  rd.SkipCommentsState(old_state);

  return ast;
}

CPP_EXPORT bool ASTRoot::check() const {
  if (m_failed) {
    return false;
  }

  bool failed = false;
  ncc::parse::iterate<dfs_pre>(m_base, [&](auto, auto c) {
    failed |= !c || c->is_mock();

    return failed ? IterOp::Abort : IterOp::Proceed;
  });

  return !failed;
}

std::string ncc::parse::mint_clang16_message(ncc::lex::IScanner &rd,
                                             const DiagMessage &msg) {
  auto start = msg.tok.get_start().Get(rd);

  auto filename = start.GetFilename();
  auto line = start.GetRow(), col = start.GetCol();

  std::stringstream ss;
  ss << "\x1b[37;1m" << filename << ":";

  if (line != QLEX_EOFF) {
    ss << line << ":";
  } else {
    ss << "?:";
  }

  if (col != QLEX_EOFF) {
    ss << col << ":\x1b[0m ";
  } else {
    ss << "?:\x1b[0m ";
  }

  ss << "\x1b[37;1m" << msg.msg << " [";

  ss << "SyntaxError";

  ss << "]\x1b[0m";

  uint32_t offset = 0;
  /// TODO: Implement source snippet
  // char *snippet = qlex_snippet(&rd, msg.tok, &offset);
  char *snippet = nullptr;
  if (!snippet) {
    return ss.str();
  }

  ss << "\n" << snippet << "\n";
  for (uint32_t i = 0; i < offset; i++) {
    ss << " ";
  }
  ss << "\x1b[32;1m^\x1b[0m";
  free(snippet);

  return ss.str();
}
