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

#ifndef __NITRATE_PARSER_WRITER_H__
#define __NITRATE_PARSER_WRITER_H__

#include <cstddef>
#include <cstdint>
#include <functional>
#include <nitrate-parser/Vistor.hh>
#include <ostream>
#include <stack>
#include <string_view>

namespace npar {
  class AST_Writer : public ASTVisitor {
    using InsertString = std::function<void(std::string_view)>;
    using InsertUInt64 = std::function<void(uint64_t)>;
    using InsertDouble = std::function<void(double)>;
    using InsertBool = std::function<void(bool)>;
    using InsertNull = std::function<void()>;
    using BeginObject = std::function<void()>;
    using EndObject = std::function<void()>;
    using BeginArray = std::function<void(size_t max_size)>;
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

  public:
    AST_Writer(InsertString str_impl, InsertUInt64 uint_impl,
               InsertDouble dbl_impl, InsertBool bool_impl,
               InsertNull null_impl, BeginObject begin_obj_impl,
               EndObject end_obj_impl, BeginArray begin_arr_impl,
               EndArray end_arr_impl)
        : string(str_impl),
          uint64(uint_impl),
          dbl(dbl_impl),
          boolean(bool_impl),
          null(null_impl),
          begin_obj(begin_obj_impl),
          end_obj(end_obj_impl),
          begin_arr(begin_arr_impl),
          end_arr(end_arr_impl) {}

    void visit(npar_node_t& n) override;
    void visit(ExprStmt&) override;
    void visit(StmtExpr&) override;
    void visit(TypeExpr&) override;
    void visit(NamedTy&) override;
    void visit(InferTy&) override;
    void visit(TemplType&) override;
    void visit(U1&) override;
    void visit(U8&) override;
    void visit(U16&) override;
    void visit(U32&) override;
    void visit(U64&) override;
    void visit(U128&) override;
    void visit(I8&) override;
    void visit(I16&) override;
    void visit(I32&) override;
    void visit(I64&) override;
    void visit(I128&) override;
    void visit(F16&) override;
    void visit(F32&) override;
    void visit(F64&) override;
    void visit(F128&) override;
    void visit(VoidTy&) override;
    void visit(PtrTy&) override;
    void visit(OpaqueTy&) override;
    void visit(TupleTy&) override;
    void visit(ArrayTy&) override;
    void visit(RefTy&) override;
    void visit(StructTy&) override;
    void visit(FuncTy&) override;
    void visit(UnaryExpr&) override;
    void visit(BinExpr&) override;
    void visit(PostUnaryExpr&) override;
    void visit(TernaryExpr&) override;
    void visit(ConstInt&) override;
    void visit(ConstFloat&) override;
    void visit(ConstBool&) override;
    void visit(ConstString&) override;
    void visit(ConstChar&) override;
    void visit(ConstNull&) override;
    void visit(ConstUndef&) override;
    void visit(Call&) override;
    void visit(TemplCall&) override;
    void visit(List&) override;
    void visit(Assoc&) override;
    void visit(Field&) override;
    void visit(Index&) override;
    void visit(Slice&) override;
    void visit(FString&) override;
    void visit(Ident&) override;
    void visit(SeqPoint&) override;
    void visit(Block&) override;
    void visit(VarDecl&) override;
    void visit(InlineAsm&) override;
    void visit(IfStmt&) override;
    void visit(WhileStmt&) override;
    void visit(ForStmt&) override;
    void visit(ForeachStmt&) override;
    void visit(BreakStmt&) override;
    void visit(ContinueStmt&) override;
    void visit(ReturnStmt&) override;
    void visit(ReturnIfStmt&) override;
    void visit(CaseStmt&) override;
    void visit(SwitchStmt&) override;
    void visit(TypedefDecl&) override;
    void visit(FnDecl&) override;
    void visit(FnDef&) override;
    void visit(StructField&) override;
    void visit(StructDef&) override;
    void visit(EnumDef&) override;
    void visit(ScopeDecl&) override;
    void visit(ExportDecl&) override;
  };

  class AST_JsonWriter : public AST_Writer {
    std::ostream& m_os;
    std::stack<bool> m_comma;
    std::stack<size_t> m_count;

    void delim();

    void str_impl(std::string_view str);
    void uint_impl(uint64_t val);
    void double_impl(double val);
    void bool_impl(bool val);
    void null_impl();
    void begin_obj_impl();
    void end_obj_impl();
    void begin_arr_impl(size_t max_size);
    void end_arr_impl();

  public:
    AST_JsonWriter(std::ostream& os)
        : AST_Writer(
              std::bind(&AST_JsonWriter::str_impl, this, std::placeholders::_1),
              std::bind(&AST_JsonWriter::uint_impl, this,
                        std::placeholders::_1),
              std::bind(&AST_JsonWriter::double_impl, this,
                        std::placeholders::_1),
              std::bind(&AST_JsonWriter::bool_impl, this,
                        std::placeholders::_1),
              std::bind(&AST_JsonWriter::null_impl, this),
              std::bind(&AST_JsonWriter::begin_obj_impl, this),
              std::bind(&AST_JsonWriter::end_obj_impl, this),
              std::bind(&AST_JsonWriter::begin_arr_impl, this,
                        std::placeholders::_1),
              std::bind(&AST_JsonWriter::end_arr_impl, this)),
          m_os(os) {
      m_comma.push(false);
      m_count.push(0);
    }
  };

  class AST_MsgPackWriter : public AST_Writer {
    std::ostream& m_os;

    void str_impl(std::string_view str);
    void uint_impl(uint64_t val);
    void double_impl(double val);
    void bool_impl(bool val);
    void null_impl();
    void begin_obj_impl();
    void end_obj_impl();
    void begin_arr_impl(size_t max_size);
    void end_arr_impl();

  public:
    AST_MsgPackWriter(std::ostream& os)
        : AST_Writer(std::bind(&AST_MsgPackWriter::str_impl, this,
                               std::placeholders::_1),
                     std::bind(&AST_MsgPackWriter::uint_impl, this,
                               std::placeholders::_1),
                     std::bind(&AST_MsgPackWriter::double_impl, this,
                               std::placeholders::_1),
                     std::bind(&AST_MsgPackWriter::bool_impl, this,
                               std::placeholders::_1),
                     std::bind(&AST_MsgPackWriter::null_impl, this),
                     std::bind(&AST_MsgPackWriter::begin_obj_impl, this),
                     std::bind(&AST_MsgPackWriter::end_obj_impl, this),
                     std::bind(&AST_MsgPackWriter::begin_arr_impl, this,
                               std::placeholders::_1),
                     std::bind(&AST_MsgPackWriter::end_arr_impl, this)),
          m_os(os) {}
  };
}  // namespace npar

#endif
