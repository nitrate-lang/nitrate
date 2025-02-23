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

#include <nitrate-parser/ASTFactory.hh>

using namespace ncc::parse;

auto ASTFactory::CreateReference(FlowPtr<Type> to, bool volatil, NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min,
                                 NullableFlowPtr<Expr> max, SourceLocation dbgsrc) -> FlowPtr<RefTy> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateU1(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                          SourceLocation dbgsrc) -> FlowPtr<U1> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateU8(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                          SourceLocation dbgsrc) -> FlowPtr<U8> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateU16(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation dbgsrc) -> FlowPtr<U16> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateU32(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation dbgsrc) -> FlowPtr<U32> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateU64(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation dbgsrc) -> FlowPtr<U64> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateU128(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                            SourceLocation dbgsrc) -> FlowPtr<U128> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateI8(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                          SourceLocation dbgsrc) -> FlowPtr<I8> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateI16(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation dbgsrc) -> FlowPtr<I16> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateI32(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation dbgsrc) -> FlowPtr<I32> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateI64(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation dbgsrc) -> FlowPtr<I64> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateI128(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                            SourceLocation dbgsrc) -> FlowPtr<I128> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateF16(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation dbgsrc) -> FlowPtr<F16> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateF32(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation dbgsrc) -> FlowPtr<F32> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateF64(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                           SourceLocation dbgsrc) -> FlowPtr<F64> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateF128(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                            SourceLocation dbgsrc) -> FlowPtr<F128> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateVoid(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                            SourceLocation dbgsrc) -> FlowPtr<VoidTy> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreatePointer(FlowPtr<Type> to, bool volatil, NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min,
                               NullableFlowPtr<Expr> max, SourceLocation dbgsrc) -> FlowPtr<PtrTy> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateOpaque(string name, NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min,
                              NullableFlowPtr<Expr> max, SourceLocation dbgsrc) -> FlowPtr<OpaqueTy> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateArray(FlowPtr<Type> element_type, FlowPtr<Expr> element_count, NullableFlowPtr<Expr> bits,
                             NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                             SourceLocation dbgsrc) -> FlowPtr<ArrayTy> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateTuple(std::span<FlowPtr<Type>> ele, NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min,
                             NullableFlowPtr<Expr> max, SourceLocation dbgsrc) -> FlowPtr<TupleTy> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateTuple(const std::vector<FlowPtr<Type>>& ele, NullableFlowPtr<Expr> bits,
                             NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                             SourceLocation dbgsrc) -> FlowPtr<TupleTy> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateFunc(SourceLocation dbgsrc) -> FlowPtr<FuncTy> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateNamed(string name, NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min,
                             NullableFlowPtr<Expr> max, SourceLocation dbgsrc) -> FlowPtr<NamedTy> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateUnknownType(NullableFlowPtr<Expr> bits, NullableFlowPtr<Expr> min, NullableFlowPtr<Expr> max,
                                   SourceLocation dbgsrc) -> FlowPtr<InferTy> {
  /// TODO: Implement
  qcore_implement();
}

auto ASTFactory::CreateTemplateType(SourceLocation dbgsrc) -> FlowPtr<TemplateType> {
  /// TODO: Implement
  qcore_implement();
}
