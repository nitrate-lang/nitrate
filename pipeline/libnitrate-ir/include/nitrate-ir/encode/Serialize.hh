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

namespace ncc::ir::encode {
  class CPP_EXPORT IR_Writer : public IRVisitor<void> {
    using InsertString = std::function<void(std::string_view)>;
    using InsertUInt64 = std::function<void(uint64_t)>;
    using InsertDouble = std::function<void(double)>;
    using InsertBool = std::function<void(bool)>;
    using InsertNull = std::function<void()>;
    using BeginObject = std::function<void(size_t pair_count)>;
    using EndObject = std::function<void()>;
    using BeginArray = std::function<void(size_t size)>;
    using EndArray = std::function<void()>;

    InsertString string;
    InsertUInt64 uint64;
    InsertDouble dbl;
    InsertBool boolean;
    InsertNull null;
    BeginObject begin_obj;
    EndObject end_obj;
    BeginArray begin_arr;
    EndArray end_arr;

    bool m_include_source_location;

    void write_source_location(FlowPtr<Expr> n) const;

  public:
    IR_Writer(InsertString str_impl, InsertUInt64 uint_impl,
              InsertDouble dbl_impl, InsertBool bool_impl, InsertNull null_impl,
              BeginObject begin_obj_impl, EndObject end_obj_impl,
              BeginArray begin_arr_impl, EndArray end_arr_impl,
              bool include_source_location = true)
        : string(str_impl),
          uint64(uint_impl),
          dbl(dbl_impl),
          boolean(bool_impl),
          null(null_impl),
          begin_obj(begin_obj_impl),
          end_obj(end_obj_impl),
          begin_arr(begin_arr_impl),
          end_arr(end_arr_impl),
          m_include_source_location(include_source_location) {}
    virtual ~IR_Writer() = default;

    void visit(FlowPtr<Expr> n) override;
    void visit(FlowPtr<BinExpr> n) override;
    void visit(FlowPtr<Unary> n) override;
    void visit(FlowPtr<U1Ty> n) override;
    void visit(FlowPtr<U8Ty> n) override;
    void visit(FlowPtr<U16Ty> n) override;
    void visit(FlowPtr<U32Ty> n) override;
    void visit(FlowPtr<U64Ty> n) override;
    void visit(FlowPtr<U128Ty> n) override;
    void visit(FlowPtr<I8Ty> n) override;
    void visit(FlowPtr<I16Ty> n) override;
    void visit(FlowPtr<I32Ty> n) override;
    void visit(FlowPtr<I64Ty> n) override;
    void visit(FlowPtr<I128Ty> n) override;
    void visit(FlowPtr<F16Ty> n) override;
    void visit(FlowPtr<F32Ty> n) override;
    void visit(FlowPtr<F64Ty> n) override;
    void visit(FlowPtr<F128Ty> n) override;
    void visit(FlowPtr<VoidTy> n) override;
    void visit(FlowPtr<PtrTy> n) override;
    void visit(FlowPtr<ConstTy> n) override;
    void visit(FlowPtr<OpaqueTy> n) override;
    void visit(FlowPtr<StructTy> n) override;
    void visit(FlowPtr<UnionTy> n) override;
    void visit(FlowPtr<ArrayTy> n) override;
    void visit(FlowPtr<FnTy> n) override;
    void visit(FlowPtr<Int> n) override;
    void visit(FlowPtr<Float> n) override;
    void visit(FlowPtr<List> n) override;
    void visit(FlowPtr<Call> n) override;
    void visit(FlowPtr<Seq> n) override;
    void visit(FlowPtr<Index> n) override;
    void visit(FlowPtr<Ident> n) override;
    void visit(FlowPtr<Extern> n) override;
    void visit(FlowPtr<Local> n) override;
    void visit(FlowPtr<Ret> n) override;
    void visit(FlowPtr<Brk> n) override;
    void visit(FlowPtr<Cont> n) override;
    void visit(FlowPtr<If> n) override;
    void visit(FlowPtr<While> n) override;
    void visit(FlowPtr<For> n) override;
    void visit(FlowPtr<Case> n) override;
    void visit(FlowPtr<Switch> n) override;
    void visit(FlowPtr<Function> n) override;
    void visit(FlowPtr<Asm> n) override;
    void visit(FlowPtr<Tmp> n) override;
  };
}  // namespace ncc::ir::encode

#endif
