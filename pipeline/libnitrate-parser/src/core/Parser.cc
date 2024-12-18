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
#include <nitrate-parser/Parser.hh>

#include "nitrate-core/Environment.hh"

using namespace npar;

Stmt* npar::recurse_block(npar_t& S, qlex_t& rd, bool expect_braces,
                          bool single_stmt, SafetyMode safety) {
  if (expect_braces && !next().is<qPuncLCur>()) {
    diagnostic << current() << "Expected '{'";
  }

  let block_start = current().start;
  BlockItems items;

  while (true) {
    qlex_tok_t tok = peek();

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
      let expr = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

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
        for (let decl : recurse_variable(S, rd, VarDeclType::Var)) {
          items.push_back(decl);
        }
        break;
      }

      case qKLet: {
        for (let decl : recurse_variable(S, rd, VarDeclType::Let)) {
          items.push_back(decl);
        }
        break;
      }

      case qKConst: {
        for (let decl : recurse_variable(S, rd, VarDeclType::Const)) {
          items.push_back(decl);
        }
        break;
      }

      case qKEnum: {
        let node = recurse_enum(S, rd);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKStruct: {
        let node = recurse_struct(S, rd, CompositeType::Struct);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKRegion: {
        let node = recurse_struct(S, rd, CompositeType::Region);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKGroup: {
        let node = recurse_struct(S, rd, CompositeType::Group);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKClass: {
        let node = recurse_struct(S, rd, CompositeType::Class);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKUnion: {
        let node = recurse_struct(S, rd, CompositeType::Union);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKType: {
        let node = recurse_typedef(S, rd);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKScope: {
        let node = recurse_scope(S, rd);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKFn: {
        let node = recurse_function(S, rd, false);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKPub:
      case qKImport: {  // they both declare external functions
        let node = recurse_pub(S, rd);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKSec: {
        let node = recurse_sec(S, rd);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKPro: {
        let node = recurse_pro(S, rd);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKReturn: {
        let node = recurse_return(S, rd);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKRetif: {
        let node = recurse_retif(S, rd);
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
        let node = recurse_if(S, rd);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKWhile: {
        let node = recurse_while(S, rd);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKFor: {
        let node = recurse_for(S, rd);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKForeach: {
        let node = recurse_foreach(S, rd);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qKSwitch: {
        let node = recurse_switch(S, rd);
        node->set_offset(loc_start);

        items.push_back(node);
        break;
      }

      case qK__Asm__: {
        let node = recurse_inline_asm(S, rd);
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
          let node = recurse_block(S, rd, true, false, SafetyMode::Unsafe);
          node->set_offset(loc_start);

          items.push_back(node);
        } else {
          let node = recurse_block(S, rd, false, true, SafetyMode::Unsafe);
          node->set_offset(loc_start);

          items.push_back(node);
        }

        break;
      }

      case qKSafe: {
        if (peek().is<qPuncLCur>()) {
          let node = recurse_block(S, rd, true, false, SafetyMode::Safe);
          node->set_offset(loc_start);

          items.push_back(node);
        } else {
          let node = recurse_block(S, rd, false, true, SafetyMode::Safe);
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

CPP_EXPORT npar_t* npar_new(qlex_t* lexer,
                            std::shared_ptr<ncc::core::Environment> env) {
  if (!lexer) {
    return nullptr;
  }

  npar_t* parser = new npar_t();

  parser->allocator = std::make_unique<ncc::core::dyn_arena>();
  parser->env = env;
  parser->lexer = lexer;
  parser->failed = false;
  parser->diag.set_ctx(parser);

  qlex_set_flags(lexer, qlex_get_flags(lexer) | QLEX_NO_COMMENTS);

  return parser;
}

CPP_EXPORT void npar_free(npar_t* parser) {
  if (!parser) {
    return;
  }

  parser->lexer = nullptr;

  delete parser;
}

static thread_local npar_t* parser_ctx;

CPP_EXPORT bool npar_do(npar_t* L, npar_node_t** out) {
  if (!L || !out) {
    return false;
  }
  *out = nullptr;

  /*=============== Swap in their arena  ===============*/
  std::swap(npar::npar_allocator, L->allocator);

  /*== Install thread-local references to the parser ==*/
  npar::install_reference(L);

  parser_ctx = L;
  *out = npar::recurse_block(*L, *L->lexer, false, false, SafetyMode::Unknown);
  parser_ctx = nullptr;

  /*== Uninstall thread-local references to the parser ==*/
  npar::install_reference(nullptr);

  /*=============== Swap out their arena ===============*/
  std::swap(npar::npar_allocator, L->allocator);

  /*==================== Return status ====================*/
  return !L->failed;
}

CPP_EXPORT bool npar_check(npar_t* parser, const npar_node_t* base) {
  if (!base) {
    return false;
  }

  if (parser && parser->failed) {
    return false;
  }

  bool failed = false;
  npar::iterate<dfs_pre>(base, [&](auto, auto c) {
    failed |= !c || !*c || (*c)->is_mock();

    return failed ? IterOp::Abort : IterOp::Proceed;
  });

  return !failed;
}

CPP_EXPORT void npar_dumps(npar_t* parser, bool no_ansi, npar_dump_cb cb,
                           uintptr_t data) {
  if (!parser || !cb) {
    return;
  }

  let adapter = [&](const char* msg) { cb(msg, std::strlen(msg), data); };

  if (no_ansi) {
    parser->diag.render(adapter, npar::FormatStyle::ClangPlain);
  } else {
    parser->diag.render(adapter, npar::FormatStyle::Clang16Color);
  }
}
