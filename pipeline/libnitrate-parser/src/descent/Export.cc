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

#include <descent/Recurse.hh>

using namespace ncc::lex;
using namespace ncc::parse;

std::string_view Parser::recurse_abi_name() {
  if (let tok = next_if(qText)) {
    return tok->as_string();
  } else {
    return "";
  }
}

std::optional<ExpressionList> Parser::recurse_export_attributes() {
  ExpressionList attributes;

  if (!next_if(qPuncLBrk)) {
    return attributes;
  }

  while (true) {
    let tok = peek();

    if (!tok.is(qEofF)) {
      if (next_if(qPuncRBrk)) {
        return attributes;
      }

      let attribute =
          recurse_expr({Token(qPunc, qPuncComa), Token(qPunc, qPuncRBrk)});

      attributes.push_back(attribute);

      next_if(qPuncComa);
    } else {
      diagnostic << tok << "Encountered EOF while parsing export attributes";
      break;
    }
  }

  return std::nullopt;
}

Stmt *Parser::recurse_export_body() {
  if (peek().is<qPuncLCur>()) {
    return recurse_block(true, false, SafetyMode::Unknown);
  } else {
    return recurse_block(false, true, SafetyMode::Unknown);
  }
}

Stmt *Parser::recurse_pub() {
  let abi_id = recurse_abi_name();

  if (let attrs = recurse_export_attributes()) {
    let export_block = recurse_export_body();

    return make<ExportStmt>(export_block, SaveString(abi_id), Vis::Pub,
                            attrs.value());
  } else {
    diagnostic << current() << "Malformed export attributes";
  }

  return mock_stmt(QAST_EXPORT);
}

Stmt *Parser::recurse_sec() {
  let abi_id = recurse_abi_name();

  if (let attrs = recurse_export_attributes()) {
    let export_block = recurse_export_body();

    return make<ExportStmt>(export_block, SaveString(abi_id), Vis::Sec,
                            attrs.value());
  } else {
    diagnostic << current() << "Malformed export attributes";
  }

  return mock_stmt(QAST_EXPORT);
}

Stmt *Parser::recurse_pro() {
  let abi_id = recurse_abi_name();

  if (let attrs = recurse_export_attributes()) {
    let export_block = recurse_export_body();

    return make<ExportStmt>(export_block, SaveString(abi_id), Vis::Pro,
                            attrs.value());
  } else {
    diagnostic << current() << "Malformed export attributes";
  }

  return mock_stmt(QAST_EXPORT);
}
