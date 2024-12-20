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

#ifndef __NITRATE_NR_WRITER_H__
#define __NITRATE_NR_WRITER_H__

#include <cstddef>
#include <cstdint>
#include <functional>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/Visitor.hh>
#include <nitrate-lexer/Token.hh>
#include <ostream>
#include <stack>
#include <string_view>

namespace nr {
  class CPP_EXPORT NR_Writer : public NRVisitor {
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

  public:
    NR_Writer(InsertString str_impl, InsertUInt64 uint_impl,
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
    virtual ~NR_Writer() = default;

    void visit(Expr& n) override;
    void visit(Type& n) override;
    void visit(BinExpr& n) override;
    void visit(UnExpr& n) override;
    void visit(PostUnExpr& n) override;
    void visit(U1Ty& n) override;
    void visit(U8Ty& n) override;
    void visit(U16Ty& n) override;
    void visit(U32Ty& n) override;
    void visit(U64Ty& n) override;
    void visit(U128Ty& n) override;
    void visit(I8Ty& n) override;
    void visit(I16Ty& n) override;
    void visit(I32Ty& n) override;
    void visit(I64Ty& n) override;
    void visit(I128Ty& n) override;
    void visit(F16Ty& n) override;
    void visit(F32Ty& n) override;
    void visit(F64Ty& n) override;
    void visit(F128Ty& n) override;
    void visit(VoidTy& n) override;
    void visit(PtrTy& n) override;
    void visit(ConstTy& n) override;
    void visit(OpaqueTy& n) override;
    void visit(StructTy& n) override;
    void visit(UnionTy& n) override;
    void visit(ArrayTy& n) override;
    void visit(FnTy& n) override;
    void visit(Int& n) override;
    void visit(Float& n) override;
    void visit(List& n) override;
    void visit(Call& n) override;
    void visit(Seq& n) override;
    void visit(Index& n) override;
    void visit(Ident& n) override;
    void visit(Extern& n) override;
    void visit(Local& n) override;
    void visit(Ret& n) override;
    void visit(Brk& n) override;
    void visit(Cont& n) override;
    void visit(If& n) override;
    void visit(While& n) override;
    void visit(For& n) override;
    void visit(Case& n) override;
    void visit(Switch& n) override;
    void visit(Fn& n) override;
    void visit(Asm& n) override;
    void visit(Tmp& n) override;
  };

  class CPP_EXPORT NR_JsonWriter : public NR_Writer {
    std::ostream& m_os;
    std::stack<bool> m_comma;
    std::stack<size_t> m_count;

    void delim();

    void str_impl(std::string_view str);
    void uint_impl(uint64_t val);
    void double_impl(double val);
    void bool_impl(bool val);
    void null_impl();
    void begin_obj_impl(size_t pair_count);
    void end_obj_impl();
    void begin_arr_impl(size_t size);
    void end_arr_impl();

  public:
    NR_JsonWriter(std::ostream& os)
        : NR_Writer(
              std::bind(&NR_JsonWriter::str_impl, this, std::placeholders::_1),
              std::bind(&NR_JsonWriter::uint_impl, this, std::placeholders::_1),
              std::bind(&NR_JsonWriter::double_impl, this,
                        std::placeholders::_1),
              std::bind(&NR_JsonWriter::bool_impl, this, std::placeholders::_1),
              std::bind(&NR_JsonWriter::null_impl, this),
              std::bind(&NR_JsonWriter::begin_obj_impl, this,
                        std::placeholders::_1),
              std::bind(&NR_JsonWriter::end_obj_impl, this),
              std::bind(&NR_JsonWriter::begin_arr_impl, this,
                        std::placeholders::_1),
              std::bind(&NR_JsonWriter::end_arr_impl, this)),
          m_os(os) {
      m_comma.push(false);
      m_count.push(0);
    }
    virtual ~NR_JsonWriter() = default;
  };

  class CPP_EXPORT NR_MsgPackWriter : public NR_Writer {
    std::ostream& m_os;

    void str_impl(std::string_view str);
    void uint_impl(uint64_t val);
    void double_impl(double val);
    void bool_impl(bool val);
    void null_impl();
    void begin_obj_impl(size_t pair_count);
    void end_obj_impl();
    void begin_arr_impl(size_t size);
    void end_arr_impl();

  public:
    NR_MsgPackWriter(std::ostream& os)
        : NR_Writer(std::bind(&NR_MsgPackWriter::str_impl, this,
                              std::placeholders::_1),
                    std::bind(&NR_MsgPackWriter::uint_impl, this,
                              std::placeholders::_1),
                    std::bind(&NR_MsgPackWriter::double_impl, this,
                              std::placeholders::_1),
                    std::bind(&NR_MsgPackWriter::bool_impl, this,
                              std::placeholders::_1),
                    std::bind(&NR_MsgPackWriter::null_impl, this),
                    std::bind(&NR_MsgPackWriter::begin_obj_impl, this,
                              std::placeholders::_1),
                    std::bind(&NR_MsgPackWriter::end_obj_impl, this),
                    std::bind(&NR_MsgPackWriter::begin_arr_impl, this,
                              std::placeholders::_1),
                    std::bind(&NR_MsgPackWriter::end_arr_impl, this)),
          m_os(os) {}
    virtual ~NR_MsgPackWriter() = default;
  };
}  // namespace nr

#endif
