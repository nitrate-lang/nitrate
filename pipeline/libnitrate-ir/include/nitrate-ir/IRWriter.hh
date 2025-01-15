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
  using WriterSourceProvider =
      std::optional<std::reference_wrapper<lex::IScanner>>;

  class NCC_EXPORT IrWriter : public IRVisitor<void> {
    using InsertString = std::function<void(std::string_view)>;
    using InsertUInt64 = std::function<void(uint64_t)>;
    using InsertDouble = std::function<void(double)>;
    using InsertBool = std::function<void(bool)>;
    using InsertNull = std::function<void()>;
    using BeginObject = std::function<void(size_t pair_count)>;
    using EndObject = std::function<void()>;
    using BeginArray = std::function<void(size_t size)>;
    using EndArray = std::function<void()>;

    InsertString m_string;
    InsertUInt64 m_uint64;
    InsertDouble m_dbl;
    InsertBool m_boolean;
    InsertNull m_null;
    BeginObject m_begin_obj;
    EndObject m_end_obj;
    BeginArray m_begin_arr;
    EndArray m_end_arr;
    WriterSourceProvider m_rd;

    void WriteSourceLocation(FlowPtr<Expr> n) const;

  public:
    IrWriter(InsertString str_impl, InsertUInt64 uint_impl,
              InsertDouble dbl_impl, InsertBool bool_impl, InsertNull null_impl,
              BeginObject begin_obj_impl, EndObject end_obj_impl,
              BeginArray begin_arr_impl, EndArray end_arr_impl,
              WriterSourceProvider rd = std::nullopt)
        : m_string(str_impl),
          m_uint64(uint_impl),
          m_dbl(dbl_impl),
          m_boolean(bool_impl),
          m_null(null_impl),
          m_begin_obj(begin_obj_impl),
          m_end_obj(end_obj_impl),
          m_begin_arr(begin_arr_impl),
          m_end_arr(end_arr_impl),
          m_rd(rd) {}
    virtual ~IrWriter() = default;

    void Visit(FlowPtr<Expr> n) ;
    void Visit(FlowPtr<BinExpr> n) ;
    void Visit(FlowPtr<Unary> n) ;
    void Visit(FlowPtr<U1Ty> n) ;
    void Visit(FlowPtr<U8Ty> n) ;
    void Visit(FlowPtr<U16Ty> n) ;
    void Visit(FlowPtr<U32Ty> n) ;
    void Visit(FlowPtr<U64Ty> n) ;
    void Visit(FlowPtr<U128Ty> n) ;
    void Visit(FlowPtr<I8Ty> n) ;
    void Visit(FlowPtr<I16Ty> n) ;
    void Visit(FlowPtr<I32Ty> n) ;
    void Visit(FlowPtr<I64Ty> n) ;
    void Visit(FlowPtr<I128Ty> n) ;
    void Visit(FlowPtr<F16Ty> n) ;
    void Visit(FlowPtr<F32Ty> n) ;
    void Visit(FlowPtr<F64Ty> n) ;
    void Visit(FlowPtr<F128Ty> n) ;
    void Visit(FlowPtr<VoidTy> n) ;
    void Visit(FlowPtr<PtrTy> n) override;
    void Visit(FlowPtr<ConstTy> n) ;
    void Visit(FlowPtr<OpaqueTy> n) ;
    void Visit(FlowPtr<StructTy> n) ;
    void Visit(FlowPtr<UnionTy> n) ;
    void Visit(FlowPtr<ArrayTy> n) ;
    void Visit(FlowPtr<FnTy> n) ;
    void Visit(FlowPtr<Int> n) ;
    void Visit(FlowPtr<Float> n) ;
    void Visit(FlowPtr<List> n) ;
    void Visit(FlowPtr<Call> n) ;
    void Visit(FlowPtr<Seq> n) ;
    void Visit(FlowPtr<Index> n) ;
    void Visit(FlowPtr<Ident> n) ;
    void Visit(FlowPtr<Extern> n) ;
    void Visit(FlowPtr<Local> n) ;
    void Visit(FlowPtr<Ret> n) ;
    void Visit(FlowPtr<Brk> n) ;
    void Visit(FlowPtr<Cont> n) ;
    void Visit(FlowPtr<If> n) override;
    void Visit(FlowPtr<While> n) ;
    void Visit(FlowPtr<For> n) ;
    void Visit(FlowPtr<Case> n) ;
    void Visit(FlowPtr<Switch> n) ;
    void Visit(FlowPtr<Function> n) ;
    void Visit(FlowPtr<Asm> n) ;
    void Visit(FlowPtr<Tmp> n) ;
  };
}  // namespace ncc::ir

#endif
