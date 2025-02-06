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

#ifndef __NITRATE_IR_ENCODE_SERIALIZER_H__
#define __NITRATE_IR_ENCODE_SERIALIZER_H__

#include <cstddef>
#include <cstdint>
#include <functional>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR/Visitor.hh>
#include <nitrate-lexer/Token.hh>
#include <string_view>

namespace ncc::ir {
  using WriterSourceProvider = std::optional<std::reference_wrapper<lex::IScanner>>;

  class NCC_EXPORT IRWriter : public IRVisitor<void> {
    using InsertString = std::function<void(std::string_view)>;
    using InsertUInt64 = std::function<void(uint64_t)>;
    using InsertDouble = std::function<void(double)>;
    using InsertBool = std::function<void(bool)>;
    using InsertNull = std::function<void()>;
    using BeginObject = std::function<void(size_t pair_count)>;
    using EndObject = std::function<void()>;
    using BeginArray = std::function<void(size_t size)>;
    using EndArray = std::function<void()>;

    InsertString string;    // NOLINT
    InsertUInt64 uint64;    // NOLINT
    InsertDouble dbl;       // NOLINT
    InsertBool boolean;     // NOLINT
    InsertNull null;        // NOLINT
    BeginObject begin_obj;  // NOLINT
    EndObject end_obj;      // NOLINT
    BeginArray begin_arr;   // NOLINT
    EndArray end_arr;       // NOLINT
    WriterSourceProvider m_rd;

    void WriteSourceLocation(FlowPtr<Expr> n) const;

  public:
    IRWriter(InsertString str_impl, InsertUInt64 uint_impl, InsertDouble dbl_impl, InsertBool bool_impl,
             InsertNull null_impl, BeginObject begin_obj_impl, EndObject end_obj_impl, BeginArray begin_arr_impl,
             EndArray end_arr_impl, WriterSourceProvider rd = std::nullopt)
        : string(std::move(str_impl)),
          uint64(std::move(uint_impl)),
          dbl(std::move(dbl_impl)),
          boolean(std::move(bool_impl)),
          null(std::move(null_impl)),
          begin_obj(std::move(begin_obj_impl)),
          end_obj(std::move(end_obj_impl)),
          begin_arr(std::move(begin_arr_impl)),
          end_arr(std::move(end_arr_impl)),
          m_rd(rd) {}
    ~IRWriter() override = default;

    void Visit(FlowPtr<Expr> n) override;
    void Visit(FlowPtr<Binary> n) override;
    void Visit(FlowPtr<Unary> n) override;
    void Visit(FlowPtr<U1Ty> n) override;
    void Visit(FlowPtr<U8Ty> n) override;
    void Visit(FlowPtr<U16Ty> n) override;
    void Visit(FlowPtr<U32Ty> n) override;
    void Visit(FlowPtr<U64Ty> n) override;
    void Visit(FlowPtr<U128Ty> n) override;
    void Visit(FlowPtr<I8Ty> n) override;
    void Visit(FlowPtr<I16Ty> n) override;
    void Visit(FlowPtr<I32Ty> n) override;
    void Visit(FlowPtr<I64Ty> n) override;
    void Visit(FlowPtr<I128Ty> n) override;
    void Visit(FlowPtr<F16Ty> n) override;
    void Visit(FlowPtr<F32Ty> n) override;
    void Visit(FlowPtr<F64Ty> n) override;
    void Visit(FlowPtr<F128Ty> n) override;
    void Visit(FlowPtr<VoidTy> n) override;
    void Visit(FlowPtr<PtrTy> n) override;
    void Visit(FlowPtr<ConstTy> n) override;
    void Visit(FlowPtr<OpaqueTy> n) override;
    void Visit(FlowPtr<StructTy> n) override;
    void Visit(FlowPtr<UnionTy> n) override;
    void Visit(FlowPtr<ArrayTy> n) override;
    void Visit(FlowPtr<FnTy> n) override;
    void Visit(FlowPtr<Int> n) override;
    void Visit(FlowPtr<Float> n) override;
    void Visit(FlowPtr<List> n) override;
    void Visit(FlowPtr<Call> n) override;
    void Visit(FlowPtr<Seq> n) override;
    void Visit(FlowPtr<Index> n) override;
    void Visit(FlowPtr<Identifier> n) override;
    void Visit(FlowPtr<Extern> n) override;
    void Visit(FlowPtr<Local> n) override;
    void Visit(FlowPtr<Ret> n) override;
    void Visit(FlowPtr<Brk> n) override;
    void Visit(FlowPtr<Cont> n) override;
    void Visit(FlowPtr<If> n) override;
    void Visit(FlowPtr<While> n) override;
    void Visit(FlowPtr<For> n) override;
    void Visit(FlowPtr<Case> n) override;
    void Visit(FlowPtr<Switch> n) override;
    void Visit(FlowPtr<Function> n) override;
    void Visit(FlowPtr<Asm> n) override;
    void Visit(FlowPtr<Tmp> n) override;
  };
}  // namespace ncc::ir

#endif
