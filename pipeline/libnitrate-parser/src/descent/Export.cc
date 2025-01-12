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

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

string Parser::recurse_abi_name() {
  auto tok = next_if(Text);
  return tok ? tok->as_string() : "";
}

std::optional<ExpressionList> Parser::recurse_export_attributes() {
  ExpressionList attributes;

  if (!next_if(PuncLBrk)) {
    return attributes;
  }

  while (true) {
    if (next_if(EofF)) [[unlikely]] {
      log << SyntaxError << current()
          << "Encountered EOF while parsing export attributes";
      break;
    }

    if (next_if(PuncRBrk)) {
      return attributes;
    }

    auto attribute = recurse_expr({
        Token(Punc, PuncComa),
        Token(Punc, PuncRBrk),
    });

    attributes.push_back(attribute);

    next_if(PuncComa);
  }

  return std::nullopt;
}

FlowPtr<Stmt> Parser::recurse_export_body() {
  if (peek().is<PuncLCur>()) {
    return recurse_block(true, false, SafetyMode::Unknown);
  } else {
    return recurse_block(false, true, SafetyMode::Unknown);
  }
}

FlowPtr<Stmt> Parser::recurse_export(Vis vis) {
  auto export_abi = recurse_abi_name();

  if (auto export_attributes = recurse_export_attributes()) {
    auto export_body = recurse_export_body();

    return make<ExportStmt>(export_body, export_abi, vis,
                            export_attributes.value())();
  } else {
    log << SyntaxError << current() << "Malformed export attributes";
    return mock_stmt(QAST_EXPORT);
  }
}
