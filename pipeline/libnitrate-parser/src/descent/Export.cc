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

auto Parser::PImpl::RecurseAbiName() -> string {
  auto tok = NextIf<Text>();

  return tok ? tok->GetString() : "";
}

auto Parser::PImpl::RecurseExportAttributes() -> std::optional<ExpressionList> {
  ExpressionList attributes;

  if (!NextIf<PuncLBrk>()) {
    return attributes;
  }

  while (true) {
    if (NextIf<EofF>()) [[unlikely]] {
      Log << SyntaxError << Current() << "Encountered EOF while parsing export attributes";
      break;
    }

    if (NextIf<PuncRBrk>()) {
      return attributes;
    }

    auto attribute = RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncRBrk),
    });

    attributes.push_back(attribute);

    NextIf<PuncComa>();
  }

  return std::nullopt;
}

auto Parser::PImpl::RecurseExportBody() -> FlowPtr<Stmt> {
  if (Peek().Is<PuncLCur>()) {
    return RecurseBlock(true, false, SafetyMode::Unknown);
  }

  return RecurseBlock(false, true, SafetyMode::Unknown);
}

auto Parser::PImpl::RecurseExport(Vis vis) -> FlowPtr<Stmt> {
  auto export_abi = RecurseAbiName();

  if (auto export_attributes = RecurseExportAttributes()) {
    auto export_body = RecurseExportBody();

    return CreateNode<Export>(export_body, export_abi, vis, export_attributes.value())();
  }

  Log << SyntaxError << Current() << "Malformed export attributes";

  return MockStmt(QAST_EXPORT);
}
