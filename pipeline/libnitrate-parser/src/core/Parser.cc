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
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTReader.hh>
#include <nitrate-parser/ASTWriter.hh>
#include <nitrate-parser/Context.hh>

#include "nitrate-core/Environment.hh"

using namespace ncc::parse;

Stmt *Parser::recurse_block(bool expect_braces, bool single_stmt,
                            SafetyMode safety) {
  if (expect_braces && !next().is<qPuncLCur>()) {
    diagnostic << current() << "Expected '{'";
  }

  let block_start = current().start;
  BlockItems items;

  while (true) {
    NCCToken tok = peek();

    if (expect_braces && next_if(qPuncRCur)) {
      let block = make<Block>(items, safety);
      block->set_offset(tok.start);

      return block;
    }

    if (single_stmt && items.size() == 1) {
      break;
    }

    if (next_if(qEofF)) {
      break;
    }

    /* Ignore extra semicolons */
    if (next_if(qPuncSemi)) {
      continue;
    }

    if (!tok.is(qKeyW)) {
      let expr = recurse_expr({NCCToken(qPunc, qPuncSemi)});

      if (!next_if(qPuncSemi)) {
        diagnostic << tok << "Expected ';' after expression";
      }

      let stmt = make<ExprStmt>(expr);
      stmt->set_offset(expr->get_offset());

      items.push_back(stmt);
      continue;
    }

    let loc_start = tok.start;

    switch (next(), tok.as<qlex_key_t>()) {
      case qKVar: {
        for (let decl : recurse_variable(VarDeclType::Var)) {
          items.push_back(decl);
        }
        break;
      }

      case qKLet: {
        for (let decl : recurse_variable(VarDeclType::Let)) {
          items.push_back(decl);
        }
        break;
      }

      case qKConst: {
        for (let decl : recurse_variable(VarDeclType::Const)) {
          items.push_back(decl);
        }
        break;
      }

      case qKEnum: {
        let node = recurse_enum();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKStruct: {
        let node = recurse_struct(CompositeType::Struct);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKRegion: {
        let node = recurse_struct(CompositeType::Region);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKGroup: {
        let node = recurse_struct(CompositeType::Group);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKClass: {
        let node = recurse_struct(CompositeType::Class);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKUnion: {
        let node = recurse_struct(CompositeType::Union);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKType: {
        let node = recurse_typedef();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKScope: {
        let node = recurse_scope();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKFn: {
        let node = recurse_function(false);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKPub:
      case qKImport: {  // they both declare external functions
        let node = recurse_pub();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKSec: {
        let node = recurse_sec();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKPro: {
        let node = recurse_pro();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKReturn: {
        let node = recurse_return();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKRetif: {
        let node = recurse_retif();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKBreak: {
        let node = make<BreakStmt>();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKContinue: {
        let node = make<ContinueStmt>();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKIf: {
        let node = recurse_if();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKWhile: {
        let node = recurse_while();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKFor: {
        let node = recurse_for();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKForeach: {
        let node = recurse_foreach();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKSwitch: {
        let node = recurse_switch();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qK__Asm__: {
        let node = recurse_inline_asm();
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKTrue: {
        let node = make<ExprStmt>(make<ConstBool>(true));
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKFalse: {
        let node = make<ExprStmt>(make<ConstBool>(false));
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKUnsafe: {
        if (peek().is<qPuncLCur>()) {
          let node = recurse_block(true, false, SafetyMode::Unsafe);
          node->set_offset(loc_start);

          items.push_back(node);
        } else {
          let node = recurse_block(false, true, SafetyMode::Unsafe);
          node->set_offset(loc_start);

          items.push_back(node);
        }

        break;
      }

      case qKSafe: {
        if (peek().is<qPuncLCur>()) {
          let node = recurse_block(true, false, SafetyMode::Safe);
          node->set_offset(loc_start);

          items.push_back(node);
        } else {
          let node = recurse_block(false, true, SafetyMode::Safe);
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

  let block = make<Block>(items, safety);
  block->set_offset(block_start);

  return block;
}

CPP_EXPORT Parser::Parser(ncc::lex::IScanner &lexer,
                          std::shared_ptr<ncc::core::Environment> env)
    : rd(lexer) {
  m_env = env;
  m_allocator = std::make_unique<ncc::core::dyn_arena>();
  m_failed = false;

  env->set("this.lexer.emit_comments", "false", true);
}

CPP_EXPORT Parser::~Parser() {}

CPP_EXPORT ASTRoot Parser::parse() {
  /*=============== Swap in their arena  ===============*/
  std::swap(ncc::parse::npar_allocator, m_allocator);

  /*== Install thread-local references to the parser ==*/
  ncc::parse::diagnostic = this;

  auto node = recurse_block(false, false, SafetyMode::Unknown);
  ASTRoot ast(shared_from_this(), node);

  /*== Uninstall thread-local references to the parser ==*/
  ncc::parse::diagnostic = nullptr;

  /*=============== Swap out their arena ===============*/
  std::swap(ncc::parse::npar_allocator, m_allocator);

  return ast;
}

CPP_EXPORT bool ASTRoot::check() const {
  bool failed = false;
  ncc::parse::iterate<dfs_pre>(const_cast<Base *&>(m_base), [&](auto, auto c) {
    failed |= !c || !*c || (*c)->is_mock();

    return failed ? IterOp::Abort : IterOp::Proceed;
  });

  return !failed;
}

std::string ncc::parse::mint_clang16_message(ncc::lex::IScanner &rd,
                                             const DiagMessage &msg) {
  /// TODO: Implement this
  qcore_implement();

  // std::stringstream ss;
  // ss << "\x1b[37;1m" << qlex_filename(&rd) << ":";
  // uint32_t line = qlex_line(&rd, qlex_begin(&msg.tok));
  // uint32_t col = qlex_col(&rd, qlex_begin(&msg.tok));

  // if (line != QLEX_EOFF) {
  //   ss << line << ":";
  // } else {
  //   ss << "?:";
  // }

  // if (col != QLEX_EOFF) {
  //   ss << col << ":\x1b[0m ";
  // } else {
  //   ss << "?:\x1b[0m ";
  // }

  // ss << "\x1b[37;1m" << msg.msg << " [";

  // ss << "SyntaxError";

  // ss << "]\x1b[0m";

  // uint32_t offset;
  // char *snippet = qlex_snippet(&rd, msg.tok, &offset);
  // if (!snippet) {
  //   return ss.str();
  // }

  // ss << "\n" << snippet << "\n";
  // for (uint32_t i = 0; i < offset; i++) {
  //   ss << " ";
  // }
  // ss << "\x1b[32;1m^\x1b[0m";
  // free(snippet);

  // return ss.str();
}
