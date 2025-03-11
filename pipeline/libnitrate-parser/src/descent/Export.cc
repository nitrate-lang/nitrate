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

auto GeneralParser::PImpl::RecurseAbiName() -> string {
  auto tok = NextIf<Text>();
  return tok ? tok->GetString() : "";
}

auto GeneralParser::PImpl::RecurseExportAttributes() -> std::vector<FlowPtr<Expr>> {
  std::vector<FlowPtr<Expr>> attributes;

  if (!NextIf<PuncLBrk>()) {
    return attributes;
  }

  while (true) {
    if (m_rd.IsEof()) [[unlikely]] {
      Log << ParserSignal << Current() << "Encountered EOF while parsing export attributes";
      return attributes;
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
}

auto GeneralParser::PImpl::RecurseExportBody() -> FlowPtr<Expr> {
  if (Peek().Is<PuncLCur>()) {
    return RecurseBlock(true, false, BlockMode::Unknown);
  }

  return RecurseBlock(false, true, BlockMode::Unknown);
}

auto GeneralParser::PImpl::RecurseExport(Vis vis) -> FlowPtr<Expr> {
  auto export_abi = RecurseAbiName();
  auto export_attributes = RecurseExportAttributes();
  auto export_body = RecurseExportBody();

  return m_fac.CreateExport(export_body, export_attributes, vis, export_abi);
}
