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

    void write_source_location(RefNode<const Base> n) const;
    void write_type_metadata(RefNode<const Type> n);

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

    void visit(RefNode<const Base> n) override;
    void visit(RefNode<const ExprStmt> n) override;
    void visit(RefNode<const StmtExpr> n) override;
    void visit(RefNode<const TypeExpr> n) override;
    void visit(RefNode<const NamedTy> n) override;
    void visit(RefNode<const InferTy> n) override;
    void visit(RefNode<const TemplType> n) override;
    void visit(RefNode<const U1> n) override;
    void visit(RefNode<const U8> n) override;
    void visit(RefNode<const U16> n) override;
    void visit(RefNode<const U32> n) override;
    void visit(RefNode<const U64> n) override;
    void visit(RefNode<const U128> n) override;
    void visit(RefNode<const I8> n) override;
    void visit(RefNode<const I16> n) override;
    void visit(RefNode<const I32> n) override;
    void visit(RefNode<const I64> n) override;
    void visit(RefNode<const I128> n) override;
    void visit(RefNode<const F16> n) override;
    void visit(RefNode<const F32> n) override;
    void visit(RefNode<const F64> n) override;
    void visit(RefNode<const F128> n) override;
    void visit(RefNode<const VoidTy> n) override;
    void visit(RefNode<const PtrTy> n) override;
    void visit(RefNode<const OpaqueTy> n) override;
    void visit(RefNode<const TupleTy> n) override;
    void visit(RefNode<const ArrayTy> n) override;
    void visit(RefNode<const RefTy> n) override;
    void visit(RefNode<const FuncTy> n) override;
    void visit(RefNode<const UnaryExpr> n) override;
    void visit(RefNode<const BinExpr> n) override;
    void visit(RefNode<const PostUnaryExpr> n) override;
    void visit(RefNode<const TernaryExpr> n) override;
    void visit(RefNode<const ConstInt> n) override;
    void visit(RefNode<const ConstFloat> n) override;
    void visit(RefNode<const ConstBool> n) override;
    void visit(RefNode<const ConstString> n) override;
    void visit(RefNode<const ConstChar> n) override;
    void visit(RefNode<const ConstNull> n) override;
    void visit(RefNode<const ConstUndef> n) override;
    void visit(RefNode<const Call> n) override;
    void visit(RefNode<const TemplCall> n) override;
    void visit(RefNode<const List> n) override;
    void visit(RefNode<const Assoc> n) override;
    void visit(RefNode<const Index> n) override;
    void visit(RefNode<const Slice> n) override;
    void visit(RefNode<const FString> n) override;
    void visit(RefNode<const Ident> n) override;
    void visit(RefNode<const SeqPoint> n) override;
    void visit(RefNode<const Block> n) override;
    void visit(RefNode<const VarDecl> n) override;
    void visit(RefNode<const InlineAsm> n) override;
    void visit(RefNode<const IfStmt> n) override;
    void visit(RefNode<const WhileStmt> n) override;
    void visit(RefNode<const ForStmt> n) override;
    void visit(RefNode<const ForeachStmt> n) override;
    void visit(RefNode<const BreakStmt> n) override;
    void visit(RefNode<const ContinueStmt> n) override;
    void visit(RefNode<const ReturnStmt> n) override;
    void visit(RefNode<const ReturnIfStmt> n) override;
    void visit(RefNode<const CaseStmt> n) override;
    void visit(RefNode<const SwitchStmt> n) override;
    void visit(RefNode<const TypedefStmt> n) override;
    void visit(RefNode<const Function> n) override;
    void visit(RefNode<const StructDef> n) override;
    void visit(RefNode<const EnumDef> n) override;
    void visit(RefNode<const ScopeStmt> n) override;
    void visit(RefNode<const ExportStmt> n) override;
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
