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

#include <decent/Recurse.hh>

using namespace npar;

static std::optional<SymbolAttributes> recurse_export_attributes(npar_t &S,
                                                                 qlex_t &rd) {
  SymbolAttributes attributes;

  if (!next_if(qPuncLBrk)) {
    return attributes;
  }

  while (true) {
    let tok = peek();

    if (!tok.is(qEofF)) {
      if (next_if(qPuncRBrk)) {
        return attributes;
      }

      let attribute = recurse_expr(
          S, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRBrk)});

      attributes.insert(attribute);

      next_if(qPuncComa);
    } else {
      diagnostic << tok << "Encountered EOF while parsing export attributes";
      break;
    }
  }

  return std::nullopt;
}

static Stmt *recurse_export_body(npar_t &S, qlex_t &rd) {
  if (peek().is<qPuncLCur>()) {
    return recurse_block(S, rd, true);
  } else {
    return recurse_block(S, rd, false, true);
  }
}

npar::Stmt *npar::recurse_pub(npar_t &S, qlex_t &rd) {
  let abi_id = peek().is(qText) ? next().as_string(&rd) : "";

  if (let attrs = recurse_export_attributes(S, rd)) {
    let body = recurse_export_body(S, rd);

    let stmt = ExportDecl::get(body, abi_id, Vis::PUBLIC, attrs.value());
    stmt->set_end_pos(body->get_end_pos());

    return stmt;
  } else {
    diagnostic << current() << "Malformed export attributes";
  }

  return mock_stmt(QAST_NODE_EXPORT);
}

npar::Stmt *npar::recurse_sec(npar_t &S, qlex_t &rd) {
  let abi_id = peek().is(qText) ? next().as_string(&rd) : "";

  if (let attrs = recurse_export_attributes(S, rd)) {
    let body = recurse_export_body(S, rd);

    let stmt = ExportDecl::get(body, abi_id, Vis::PRIVATE, attrs.value());
    stmt->set_end_pos(body->get_end_pos());

    return stmt;
  } else {
    diagnostic << current() << "Malformed export attributes";
  }

  return mock_stmt(QAST_NODE_EXPORT);
}

npar::Stmt *npar::recurse_pro(npar_t &S, qlex_t &rd) {
  let abi_id = peek().is(qText) ? next().as_string(&rd) : "";

  if (let attrs = recurse_export_attributes(S, rd)) {
    let body = recurse_export_body(S, rd);

    let stmt = ExportDecl::get(body, abi_id, Vis::PROTECTED, attrs.value());
    stmt->set_end_pos(body->get_end_pos());

    return stmt;
  } else {
    diagnostic << current() << "Malformed export attributes";
  }

  return mock_stmt(QAST_NODE_EXPORT);
}
