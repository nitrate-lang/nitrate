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

#include <lsp/lang/format/Formatter.hh>
#include <unordered_set>

using namespace no3::lsp::fmt;
using namespace ncc::parse;
using namespace ncc::lex;

void CambrianFormatter::WrapStmtBody(FlowPtr<parse::Stmt> n, size_t size_threshold, bool use_arrow_if_wrapped) {
  if (n->Is(QAST_BLOCK)) {
    auto block = n.As<Block>();
    bool single_stmt = block->GetStatements().size() == 1;
    bool few_children = single_stmt && block->RecursiveChildCount() <= size_threshold;

    if (single_stmt && few_children) {
      if (use_arrow_if_wrapped) {
        m_line << "=> ";
      }

      block->GetStatements().front().Accept(*this);
      return;
    }
  }

  n.Accept(*this);
}

void CambrianFormatter::Visit(FlowPtr<Base> n) {
  PrintMultilineComments(n);

  /** This node symbolizes a placeholder value in the event of an error. */
  m_failed = true;

  m_line << "/* !!! */";
}

void CambrianFormatter::Visit(FlowPtr<ExprStmt> n) {
  PrintLineComments(n);

  n->GetExpr().Accept(*this);
  m_line << ";";
}

void CambrianFormatter::Visit(FlowPtr<StmtExpr> n) {
  PrintMultilineComments(n);

  n->GetStmt().Accept(*this);
}

void CambrianFormatter::Visit(FlowPtr<TypeExpr> n) {
  PrintMultilineComments(n);

  m_line << "type ";
  n->GetType().Accept(*this);
}

void CambrianFormatter::Visit(FlowPtr<Unary> n) {
  static const std::unordered_set<Operator> word_ops = {OpAs,        OpBitcastAs, OpIn,     OpOut,     OpSizeof,
                                                        OpBitsizeof, OpAlignof,   OpTypeof, OpComptime};

  PrintMultilineComments(n);

  m_line << "(" << n->GetOp();
  if (word_ops.contains(n->GetOp())) {
    m_line << " ";
  }
  n->GetRHS().Accept(*this);
  m_line << ")";
}

void CambrianFormatter::Visit(FlowPtr<PostUnary> n) {
  PrintMultilineComments(n);

  m_line << "(";
  n->GetLHS().Accept(*this);
  m_line << n->GetOp() << ")";
}

void CambrianFormatter::Visit(FlowPtr<Binary> n) {
  PrintMultilineComments(n);

  if (n->GetOp() == OpDot) {
    n->GetLHS().Accept(*this);
    m_line << ".";
    n->GetRHS().Accept(*this);
  } else {
    m_line << "(";
    n->GetLHS().Accept(*this);
    m_line << " " << n->GetOp() << " ";
    n->GetRHS().Accept(*this);
    m_line << ")";
  }
}

void CambrianFormatter::Visit(FlowPtr<Ternary> n) {
  PrintMultilineComments(n);

  m_line << "(";
  n->GetCond().Accept(*this);
  m_line << " ? ";
  n->GetLHS().Accept(*this);
  m_line << " : ";
  n->GetRHS().Accept(*this);
  m_line << ")";
}
