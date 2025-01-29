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

using namespace no3::lsp::fmt;
using namespace ncc::parse;

void CambrianFormatter::Visit(FlowPtr<InferTy> n) {
  PrintMultilineComments(n);

  m_line << "?";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<U1> n) {
  PrintMultilineComments(n);

  m_line << "u1";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<U8> n) {
  PrintMultilineComments(n);

  m_line << "u8";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<U16> n) {
  PrintMultilineComments(n);

  m_line << "u16";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<U32> n) {
  PrintMultilineComments(n);

  m_line << "u32";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<U64> n) {
  PrintMultilineComments(n);

  m_line << "u64";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<U128> n) {
  PrintMultilineComments(n);

  m_line << "u128";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<I8> n) {
  PrintMultilineComments(n);

  m_line << "i8";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<I16> n) {
  PrintMultilineComments(n);

  m_line << "i16";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<I32> n) {
  PrintMultilineComments(n);

  m_line << "i32";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<I64> n) {
  PrintMultilineComments(n);

  m_line << "i64";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<I128> n) {
  PrintMultilineComments(n);

  m_line << "i128";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<F16> n) {
  PrintMultilineComments(n);

  m_line << "f16";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<F32> n) {
  PrintMultilineComments(n);

  m_line << "f32";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<F64> n) {
  PrintMultilineComments(n);

  m_line << "f64";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<F128> n) {
  PrintMultilineComments(n);

  m_line << "f128";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<VoidTy> n) {
  PrintMultilineComments(n);

  m_line << "void";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<PtrTy> n) {
  PrintMultilineComments(n);

  m_line << "*";
  n->GetItem().Accept(*this);

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<RefTy> n) {
  PrintMultilineComments(n);

  m_line << "&";
  n->GetItem().Accept(*this);

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<OpaqueTy> n) {
  PrintMultilineComments(n);

  m_line << "opaque(" << n->GetName() << ")";

  FormatTypeMetadata(n);
}

void CambrianFormatter::Visit(FlowPtr<ArrayTy> n) {
  PrintMultilineComments(n);

  m_line << "[";
  n->GetItem().Accept(*this);
  m_line << "; ";
  n->GetSize().Accept(*this);
  m_line << "]";

  FormatTypeMetadata(n);
}

void CambrianFormatter::FormatTypeMetadata(const FlowPtr<parse::Type>& n) {
  auto range_start = n->GetRangeBegin();
  auto range_end = n->GetRangeEnd();

  if (range_start || range_end) {
    m_line << ": [";
    if (range_start) {
      range_start.value().Accept(*this);
    }

    m_line << ":";
    if (range_end) {
      range_end.value().Accept(*this);
    }

    m_line << "]";
  }

  if (n->GetWidth()) {
    m_line << ": ";
    n->GetWidth().value().Accept(*this);
  }
}
