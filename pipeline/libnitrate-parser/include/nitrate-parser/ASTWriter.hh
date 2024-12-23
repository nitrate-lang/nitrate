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

#ifndef __NITRATE_AST_WRITER_H__
#define __NITRATE_AST_WRITER_H__

#include <cstddef>
#include <cstdint>
#include <functional>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-lexer/Token.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <ostream>
#include <stack>
#include <string_view>

namespace ncc::parse {
  using WriterSourceProvider =
      std::optional<std::reference_wrapper<lex::IScanner>>;

  class CPP_EXPORT AST_Writer : public ASTVisitor {
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
    WriterSourceProvider m_rd;

    void write_source_location(Base const& n) const;
    void write_type_metadata(Type const& n);

    std::string_view vis_str(Vis vis) const;

  public:
    AST_Writer(InsertString str_impl, InsertUInt64 uint_impl,
               InsertDouble dbl_impl, InsertBool bool_impl,
               InsertNull null_impl, BeginObject begin_obj_impl,
               EndObject end_obj_impl, BeginArray begin_arr_impl,
               EndArray end_arr_impl, WriterSourceProvider rd = std::nullopt)
        : string(str_impl),
          uint64(uint_impl),
          dbl(dbl_impl),
          boolean(bool_impl),
          null(null_impl),
          begin_obj(begin_obj_impl),
          end_obj(end_obj_impl),
          begin_arr(begin_arr_impl),
          end_arr(end_arr_impl),
          m_rd(rd) {}
    virtual ~AST_Writer() = default;

    void visit(Base const& n) override;
    void visit(ExprStmt const& n) override;
    void visit(StmtExpr const& n) override;
    void visit(TypeExpr const& n) override;
    void visit(NamedTy const& n) override;
    void visit(InferTy const& n) override;
    void visit(TemplType const& n) override;
    void visit(U1 const& n) override;
    void visit(U8 const& n) override;
    void visit(U16 const& n) override;
    void visit(U32 const& n) override;
    void visit(U64 const& n) override;
    void visit(U128 const& n) override;
    void visit(I8 const& n) override;
    void visit(I16 const& n) override;
    void visit(I32 const& n) override;
    void visit(I64 const& n) override;
    void visit(I128 const& n) override;
    void visit(F16 const& n) override;
    void visit(F32 const& n) override;
    void visit(F64 const& n) override;
    void visit(F128 const& n) override;
    void visit(VoidTy const& n) override;
    void visit(PtrTy const& n) override;
    void visit(OpaqueTy const& n) override;
    void visit(TupleTy const& n) override;
    void visit(ArrayTy const& n) override;
    void visit(RefTy const& n) override;
    void visit(FuncTy const& n) override;
    void visit(UnaryExpr const& n) override;
    void visit(BinExpr const& n) override;
    void visit(PostUnaryExpr const& n) override;
    void visit(TernaryExpr const& n) override;
    void visit(ConstInt const& n) override;
    void visit(ConstFloat const& n) override;
    void visit(ConstBool const& n) override;
    void visit(ConstString const& n) override;
    void visit(ConstChar const& n) override;
    void visit(ConstNull const& n) override;
    void visit(ConstUndef const& n) override;
    void visit(Call const& n) override;
    void visit(TemplCall const& n) override;
    void visit(List const& n) override;
    void visit(Assoc const& n) override;
    void visit(Index const& n) override;
    void visit(Slice const& n) override;
    void visit(FString const& n) override;
    void visit(Ident const& n) override;
    void visit(SeqPoint const& n) override;
    void visit(Block const& n) override;
    void visit(VarDecl const& n) override;
    void visit(InlineAsm const& n) override;
    void visit(IfStmt const& n) override;
    void visit(WhileStmt const& n) override;
    void visit(ForStmt const& n) override;
    void visit(ForeachStmt const& n) override;
    void visit(BreakStmt const& n) override;
    void visit(ContinueStmt const& n) override;
    void visit(ReturnStmt const& n) override;
    void visit(ReturnIfStmt const& n) override;
    void visit(CaseStmt const& n) override;
    void visit(SwitchStmt const& n) override;
    void visit(TypedefStmt const& n) override;
    void visit(Function const& n) override;
    void visit(StructDef const& n) override;
    void visit(EnumDef const& n) override;
    void visit(ScopeStmt const& n) override;
    void visit(ExportStmt const& n) override;
  };

  class CPP_EXPORT AST_JsonWriter : public AST_Writer {
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
    AST_JsonWriter(std::ostream& os, WriterSourceProvider rd = std::nullopt)
        : AST_Writer(
              std::bind(&AST_JsonWriter::str_impl, this, std::placeholders::_1),
              std::bind(&AST_JsonWriter::uint_impl, this,
                        std::placeholders::_1),
              std::bind(&AST_JsonWriter::double_impl, this,
                        std::placeholders::_1),
              std::bind(&AST_JsonWriter::bool_impl, this,
                        std::placeholders::_1),
              std::bind(&AST_JsonWriter::null_impl, this),
              std::bind(&AST_JsonWriter::begin_obj_impl, this,
                        std::placeholders::_1),
              std::bind(&AST_JsonWriter::end_obj_impl, this),
              std::bind(&AST_JsonWriter::begin_arr_impl, this,
                        std::placeholders::_1),
              std::bind(&AST_JsonWriter::end_arr_impl, this), rd),
          m_os(os) {
      m_comma.push(false);
      m_count.push(0);
    }
    virtual ~AST_JsonWriter() = default;
  };

  class CPP_EXPORT AST_MsgPackWriter : public AST_Writer {
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
    AST_MsgPackWriter(std::ostream& os, WriterSourceProvider rd = std::nullopt)
        : AST_Writer(std::bind(&AST_MsgPackWriter::str_impl, this,
                               std::placeholders::_1),
                     std::bind(&AST_MsgPackWriter::uint_impl, this,
                               std::placeholders::_1),
                     std::bind(&AST_MsgPackWriter::double_impl, this,
                               std::placeholders::_1),
                     std::bind(&AST_MsgPackWriter::bool_impl, this,
                               std::placeholders::_1),
                     std::bind(&AST_MsgPackWriter::null_impl, this),
                     std::bind(&AST_MsgPackWriter::begin_obj_impl, this,
                               std::placeholders::_1),
                     std::bind(&AST_MsgPackWriter::end_obj_impl, this),
                     std::bind(&AST_MsgPackWriter::begin_arr_impl, this,
                               std::placeholders::_1),
                     std::bind(&AST_MsgPackWriter::end_arr_impl, this), rd),
          m_os(os) {}
    virtual ~AST_MsgPackWriter() = default;
  };
}  // namespace ncc::parse

#endif
