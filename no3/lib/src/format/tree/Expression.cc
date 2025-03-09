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

#include <format/tree/Visitor.hh>
#include <unordered_set>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;
using namespace no3::format;

void CambrianFormatter::Visit(FlowPtr<Unary> n) {
  static const std::unordered_set<Operator> word_ops = {OpAs,        OpBitcastAs, OpIn,     OpOut,     OpSizeof,
                                                        OpBitsizeof, OpAlignof,   OpTypeof, OpComptime};

  PrintMultilineComments(n);

  m_line << n->GetOp();
  if (word_ops.contains(n->GetOp())) {
    m_line << " ";
  }
  n->GetRHS().Accept(*this);
}

void CambrianFormatter::Visit(FlowPtr<PostUnary> n) {
  PrintMultilineComments(n);

  n->GetLHS().Accept(*this);
  m_line << n->GetOp();
}

void CambrianFormatter::Visit(FlowPtr<Binary> n) {
  PrintMultilineComments(n);

  if (n->GetOp() == OpDot) {
    n->GetLHS().Accept(*this);
    m_line << ".";
    n->GetRHS().Accept(*this);
  } else {
    n->GetLHS().Accept(*this);
    m_line << " " << n->GetOp() << " ";
    n->GetRHS().Accept(*this);
  }
}

void CambrianFormatter::Visit(FlowPtr<Ternary> n) {
  PrintMultilineComments(n);

  n->GetCond().Accept(*this);
  m_line << " ? ";
  n->GetLHS().Accept(*this);
  m_line << " : ";
  n->GetRHS().Accept(*this);
}
