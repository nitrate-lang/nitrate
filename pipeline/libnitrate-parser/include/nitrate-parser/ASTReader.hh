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

#ifndef __NITRATE_AST_READER_H__
#define __NITRATE_AST_READER_H__

#include <istream>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/NullableFlowPtr.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/ASTBase.hh>
#include <optional>

namespace ncc::parse {
  using ReaderSourceManager =
      std::optional<std::reference_wrapper<lex::IScanner>>;

  class NCC_EXPORT AstReader {
    std::istream& m_is;
    ReaderSourceManager m_source_manager;

    auto ReadKindNode() -> NullableFlowPtr<Base>;
    auto ReadKindBinexpr() -> NullableFlowPtr<Binary>;
    auto ReadKindUnexpr() -> NullableFlowPtr<Unary>;
    auto ReadKindTerexpr() -> NullableFlowPtr<Ternary>;
    auto ReadKindInt() -> NullableFlowPtr<Integer>;
    auto ReadKindFloat() -> NullableFlowPtr<Float>;
    auto ReadKindString() -> NullableFlowPtr<String>;
    auto ReadKindChar() -> NullableFlowPtr<Character>;
    auto ReadKindBool() -> NullableFlowPtr<Boolean>;
    auto ReadKindNull() -> NullableFlowPtr<Null>;
    auto ReadKindUndef() -> NullableFlowPtr<Undefined>;
    auto ReadKindCall() -> NullableFlowPtr<Call>;
    auto ReadKindList() -> NullableFlowPtr<List>;
    auto ReadKindAssoc() -> NullableFlowPtr<Assoc>;
    auto ReadKindIndex() -> NullableFlowPtr<Index>;
    auto ReadKindSlice() -> NullableFlowPtr<Slice>;
    auto ReadKindFstring() -> NullableFlowPtr<FString>;
    auto ReadKindIdentifier() -> NullableFlowPtr<Identifier>;
    auto ReadKindSequence() -> NullableFlowPtr<Sequence>;
    auto ReadKindPostUnexpr() -> NullableFlowPtr<PostUnary>;
    auto ReadKindStmtExpr() -> NullableFlowPtr<StmtExpr>;
    auto ReadKindTypeExpr() -> NullableFlowPtr<TypeExpr>;
    auto ReadKindTemplateCall() -> NullableFlowPtr<TemplateCall>;
    auto ReadKindRef() -> NullableFlowPtr<RefTy>;
    auto ReadKindU1() -> NullableFlowPtr<U1>;
    auto ReadKindU8() -> NullableFlowPtr<U8>;
    auto ReadKindU16() -> NullableFlowPtr<U16>;
    auto ReadKindU32() -> NullableFlowPtr<U32>;
    auto ReadKindU64() -> NullableFlowPtr<U64>;
    auto ReadKindU128() -> NullableFlowPtr<U128>;
    auto ReadKindI8() -> NullableFlowPtr<I8>;
    auto ReadKindI16() -> NullableFlowPtr<I16>;
    auto ReadKindI32() -> NullableFlowPtr<I32>;
    auto ReadKindI64() -> NullableFlowPtr<I64>;
    auto ReadKindI128() -> NullableFlowPtr<I128>;
    auto ReadKindF16() -> NullableFlowPtr<F16>;
    auto ReadKindF32() -> NullableFlowPtr<F32>;
    auto ReadKindF64() -> NullableFlowPtr<F64>;
    auto ReadKindF128() -> NullableFlowPtr<F128>;
    auto ReadKindVoid() -> NullableFlowPtr<VoidTy>;
    auto ReadKindPtr() -> NullableFlowPtr<PtrTy>;
    auto ReadKindOpaque() -> NullableFlowPtr<OpaqueTy>;
    auto ReadKindArray() -> NullableFlowPtr<ArrayTy>;
    auto ReadKindTuple() -> NullableFlowPtr<TupleTy>;
    auto ReadKindFuncTy() -> NullableFlowPtr<FuncTy>;
    auto ReadKindUnres() -> NullableFlowPtr<NamedTy>;
    auto ReadKindInfer() -> NullableFlowPtr<InferTy>;
    auto ReadKindTempl() -> NullableFlowPtr<TemplateType>;
    auto ReadKindTypedef() -> NullableFlowPtr<Typedef>;
    auto ReadKindStruct() -> NullableFlowPtr<Struct>;
    auto ReadKindEnum() -> NullableFlowPtr<Enum>;
    auto ReadKindFunction() -> NullableFlowPtr<Function>;
    auto ReadKindScope() -> NullableFlowPtr<Scope>;
    auto ReadKindExport() -> NullableFlowPtr<Export>;
    auto ReadKindBlock() -> NullableFlowPtr<Block>;
    auto ReadKindLet() -> NullableFlowPtr<Variable>;
    auto ReadKindAssembly() -> NullableFlowPtr<Assembly>;
    auto ReadKindReturn() -> NullableFlowPtr<Return>;
    auto ReadKindRetif() -> NullableFlowPtr<ReturnIf>;
    auto ReadKindBreak() -> NullableFlowPtr<Break>;
    auto ReadKindContinue() -> NullableFlowPtr<Continue>;
    auto ReadKindIf() -> NullableFlowPtr<If>;
    auto ReadKindWhile() -> NullableFlowPtr<While>;
    auto ReadKindFor() -> NullableFlowPtr<For>;
    auto ReadKindForeach() -> NullableFlowPtr<Foreach>;
    auto ReadKindCase() -> NullableFlowPtr<Case>;
    auto ReadKindSwitch() -> NullableFlowPtr<Switch>;
    auto ReadKindExprStmt() -> NullableFlowPtr<ExprStmt>;

  public:
    AstReader(std::istream& in,
              ReaderSourceManager source_manager = std::nullopt)
        : m_is(in), m_source_manager(source_manager) {}
    virtual ~AstReader() = default;

    auto Get() -> std::optional<FlowPtr<Base>>;

    static auto FromString(std::string_view json,
                           ReaderSourceManager source_manager = std::nullopt)
        -> std::optional<FlowPtr<Base>>;
  };
}  // namespace ncc::parse

#endif
