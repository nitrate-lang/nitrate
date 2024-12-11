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

#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <nitrate-parser/Node.h>
#include <nitrate-parser/Parser.h>

#include <atomic>
#include <core/ParserStruct.hh>
#include <cstring>
#include <descent/Recurse.hh>
#include <nitrate-core/Classes.hh>
#include <nitrate-parser/Reader.hh>
#include <nitrate-parser/Writer.hh>
#include <sstream>

using namespace npar;

Stmt* npar::recurse_block(npar_t& S, qlex_t& rd, bool expect_braces,
                          bool single_stmt) {
  qlex_tok_t tok;

  Block* block = Block::get();
  block->set_offset(current().start);

  if (expect_braces) {
    tok = next();
    if (!tok.is<qPuncLCur>()) {
      diagnostic << tok << "Expected '{'";
    }
  }

  while ((tok = peek()).ty != qEofF) {
    if (single_stmt && block->get_items().size() > 0) {
      break;
    }

    if (expect_braces) {
      if (tok.is<qPuncRCur>()) {
        next();
        return block;
      }
    }

    if (tok.is<qPuncSemi>()) /* Skip excessive semicolons */
    {
      next();
      continue;
    }

    if (!tok.is(qKeyW)) {
      if (tok.is<qPuncRBrk>() || tok.is<qPuncRCur>() || tok.is<qPuncRPar>()) {
        diagnostic << tok << "Unexpected closing brace";
        return mock_stmt(QAST_NODE_BLOCK);
      }

      Expr* expr = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncSemi)});

      if (!expr) {
        diagnostic << tok << "Expected valid expression";
        return mock_stmt(QAST_NODE_BLOCK);
      }

      tok = next();
      if (!tok.is<qPuncSemi>()) {
        diagnostic << tok << "Expected ';'";
      }

      ExprStmt* stmt = ExprStmt::get(expr);
      stmt->set_offset(std::get<0>(expr->get_pos()));

      block->get_items().push_back(stmt);
      continue;
    }

    next();

    Stmt* node = nullptr;

    uint32_t loc_start = tok.start;
    switch (tok.as<qlex_key_t>()) {
      case qKVar: {
        for (auto decl : recurse_variable(S, rd, VarDeclType::Var)) {
          block->get_items().push_back(decl);
        }
        break;
      }

      case qKLet: {
        for (auto decl : recurse_variable(S, rd, VarDeclType::Let)) {
          block->get_items().push_back(decl);
        }
        break;
      }

      case qKConst: {
        for (auto decl : recurse_variable(S, rd, VarDeclType::Const)) {
          block->get_items().push_back(decl);
        }
        break;
      }

      case qKEnum: {
        node = recurse_enum(S, rd);
        break;
      }

      case qKStruct: {
        node = recurse_struct(S, rd, CompositeType::Struct);
        break;
      }

      case qKRegion: {
        node = recurse_struct(S, rd, CompositeType::Region);
        break;
      }

      case qKGroup: {
        node = recurse_struct(S, rd, CompositeType::Group);
        break;
      }

      case qKClass: {
        node = recurse_struct(S, rd, CompositeType::Class);
        break;
      }

      case qKUnion: {
        node = recurse_struct(S, rd, CompositeType::Union);
        break;
      }

      case qKType: {
        node = recurse_typedef(S, rd);
        break;
      }

      case qKScope: {
        node = recurse_scope(S, rd);
        break;
      }

      case qKFn: {
        node = recurse_function(S, rd);
        break;
      }

      case qKPub:
      case qKImport: {  // they both declare external functions
        node = recurse_pub(S, rd);
        break;
      }

      case qKSec: {
        node = recurse_sec(S, rd);
        break;
      }

      case qKPro: {
        node = recurse_pro(S, rd);
        break;
      }

      case qKReturn: {
        node = recurse_return(S, rd);
        break;
      }

      case qKRetif: {
        node = recurse_retif(S, rd);
        break;
      }

      case qKBreak: {
        node = BreakStmt::get();
        break;
      }

      case qKContinue: {
        node = ContinueStmt::get();
        break;
      }

      case qKIf: {
        node = recurse_if(S, rd);
        break;
      }

      case qKWhile: {
        node = recurse_while(S, rd);
        break;
      }

      case qKFor: {
        node = recurse_for(S, rd);
        break;
      }

      case qKForeach: {
        node = recurse_foreach(S, rd);
        break;
      }

      case qKSwitch: {
        node = recurse_switch(S, rd);
        break;
      }

      case qK__Asm__: {
        node = recurse_inline_asm(S, rd);
        break;
      }

      case qKTrue: {
        node = ExprStmt::get(ConstBool::get(true));
        break;
      }

      case qKFalse: {
        node = ExprStmt::get(ConstBool::get(false));
        break;
      }

      case qKUnsafe: {
        tok = peek();
        if (tok.is<qPuncLCur>()) {
          node = recurse_block(S, rd);
        } else {
          node = recurse_block(S, rd, false, true);
        }

        if (node->is(QAST_NODE_BLOCK)) {
          node->as<Block>()->set_safety(SafetyMode::Unsafe);
        }

        break;
      }

      case qKSafe: {
        tok = peek();
        if (tok.is<qPuncLCur>()) {
          node = recurse_block(S, rd);
        } else {
          node = recurse_block(S, rd, false, true);
        }

        if (node->is(QAST_NODE_BLOCK)) {
          node->as<Block>()->set_safety(SafetyMode::Safe);
        }

        break;
      }

      default:
        diagnostic << tok << "Unexpected keyword";
        break;
    }

    if (node) {
      node->set_offset(loc_start);
      /* End position is supplied by producer */
      block->get_items().push_back(node);
    }
  }

  if (expect_braces) {
    diagnostic << tok << "Expected '}'";
  }

  return block;
}

C_EXPORT npar_t* npar_new(qlex_t* lexer, qcore_env_t env) {
  if (!lexer) {
    return nullptr;
  }
  static std::atomic<uint64_t> job_id = 1;  // 0 is reserved for invalid job

  npar_t* parser = new npar_t();

  parser->env = env;
  parser->id = job_id++;
  parser->lexer = lexer;
  parser->failed = false;
  parser->diag.set_ctx(parser);

  qlex_set_flags(lexer, qlex_get_flags(lexer) | QLEX_NO_COMMENTS);

  return parser;
}

C_EXPORT void npar_free(npar_t* parser) {
  if (!parser) {
    return;
  }

  parser->lexer = nullptr;

  delete parser;
}

static thread_local npar_t* parser_ctx;

C_EXPORT bool npar_do(npar_t* L, npar_node_t** out) {
  if (!L || !out) {
    return false;
  }
  *out = nullptr;

  /*=============== Swap in their arena  ===============*/
  npar::npar_arena.swap(*L->arena.get());

  /*== Install thread-local references to the parser ==*/
  npar::install_reference(L);

  parser_ctx = L;
  *out = npar::recurse_block(*L, *L->lexer, false, false);
  parser_ctx = nullptr;

  /*== Uninstall thread-local references to the parser ==*/
  npar::install_reference(nullptr);

  /*=============== Swap out their arena ===============*/
  npar::npar_arena.swap(*L->arena.get());

  /*==================== Return status ====================*/
  return !L->failed;
}

C_EXPORT bool npar_check(npar_t* parser, const npar_node_t* base) {
  if (!parser || !base) {
    return false;
  }

  if (parser->failed) {
    return false;
  }

  /* TODO: Implement checks */
  return true;
}

C_EXPORT void npar_dumps(npar_t* parser, bool no_ansi, npar_dump_cb cb,
                         uintptr_t data) {
  if (!parser || !cb) {
    return;
  }

  auto adapter = [&](const char* msg) { cb(msg, std::strlen(msg), data); };

  if (no_ansi) {
    parser->diag.render(adapter, npar::FormatStyle::ClangPlain);
  } else {
    parser->diag.render(adapter, npar::FormatStyle::Clang16Color);
  }
}
