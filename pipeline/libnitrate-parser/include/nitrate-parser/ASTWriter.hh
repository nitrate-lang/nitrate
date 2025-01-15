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

  class NCC_EXPORT AstWriter : public ASTVisitor {
    using InsertString = std::function<void(std::string_view)>;
    using InsertUInt64 = std::function<void(uint64_t)>;
    using InsertDouble = std::function<void(double)>;
    using InsertBool = std::function<void(bool)>;
    using InsertNull = std::function<void()>;
    using BeginObject = std::function<void(size_t pair_count)>;
    using EndObject = std::function<void()>;
    using BeginArray = std::function<void(size_t size)>;
    using EndArray = std::function<void()>;

    InsertString string;        /// NOLINT
    InsertUInt64 uint64;        /// NOLINT
    InsertDouble dbl;           /// NOLINT
    InsertBool boolean;         /// NOLINT
    InsertNull null;            /// NOLINT
    BeginObject begin_obj;      /// NOLINT
    EndObject end_obj;          /// NOLINT
    BeginArray begin_arr;       /// NOLINT
    EndArray end_arr;           /// NOLINT
    WriterSourceProvider m_rd;  /// NOLINT

    void WriteSourceLocation(FlowPtr<Base> n) const;
    void WriteTypeMetadata(FlowPtr<Type> n);

    std::string_view VisStr(Vis vis) const;

  public:
    AstWriter(InsertString str_impl, InsertUInt64 uint_impl,
              InsertDouble dbl_impl, InsertBool bool_impl, InsertNull null_impl,
              BeginObject begin_obj_impl, EndObject end_obj_impl,
              BeginArray begin_arr_impl, EndArray end_arr_impl,
              WriterSourceProvider rd = std::nullopt)
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
    virtual ~AstWriter() = default;

    void Visit(FlowPtr<Base> n) override;
    void Visit(FlowPtr<ExprStmt> n) override;
    void Visit(FlowPtr<StmtExpr> n) override;
    void Visit(FlowPtr<TypeExpr> n) override;
    void Visit(FlowPtr<NamedTy> n) override;
    void Visit(FlowPtr<InferTy> n) override;
    void Visit(FlowPtr<TemplType> n) override;
    void Visit(FlowPtr<U1> n) override;
    void Visit(FlowPtr<U8> n) override;
    void Visit(FlowPtr<U16> n) override;
    void Visit(FlowPtr<U32> n) override;
    void Visit(FlowPtr<U64> n) override;
    void Visit(FlowPtr<U128> n) override;
    void Visit(FlowPtr<I8> n) override;
    void Visit(FlowPtr<I16> n) override;
    void Visit(FlowPtr<I32> n) override;
    void Visit(FlowPtr<I64> n) override;
    void Visit(FlowPtr<I128> n) override;
    void Visit(FlowPtr<F16> n) override;
    void Visit(FlowPtr<F32> n) override;
    void Visit(FlowPtr<F64> n) override;
    void Visit(FlowPtr<F128> n) override;
    void Visit(FlowPtr<VoidTy> n) override;
    void Visit(FlowPtr<PtrTy> n) override;
    void Visit(FlowPtr<OpaqueTy> n) override;
    void Visit(FlowPtr<TupleTy> n) override;
    void Visit(FlowPtr<ArrayTy> n) override;
    void Visit(FlowPtr<RefTy> n) override;
    void Visit(FlowPtr<FuncTy> n) override;
    void Visit(FlowPtr<UnaryExpr> n) override;
    void Visit(FlowPtr<BinExpr> n) override;
    void Visit(FlowPtr<PostUnaryExpr> n) override;
    void Visit(FlowPtr<TernaryExpr> n) override;
    void Visit(FlowPtr<ConstInt> n) override;
    void Visit(FlowPtr<ConstFloat> n) override;
    void Visit(FlowPtr<ConstBool> n) override;
    void Visit(FlowPtr<ConstString> n) override;
    void Visit(FlowPtr<ConstChar> n) override;
    void Visit(FlowPtr<ConstNull> n) override;
    void Visit(FlowPtr<ConstUndef> n) override;
    void Visit(FlowPtr<Call> n) override;
    void Visit(FlowPtr<TemplCall> n) override;
    void Visit(FlowPtr<List> n) override;
    void Visit(FlowPtr<Assoc> n) override;
    void Visit(FlowPtr<Index> n) override;
    void Visit(FlowPtr<Slice> n) override;
    void Visit(FlowPtr<FString> n) override;
    void Visit(FlowPtr<Ident> n) override;
    void Visit(FlowPtr<SeqPoint> n) override;
    void Visit(FlowPtr<Block> n) override;
    void Visit(FlowPtr<VarDecl> n) override;
    void Visit(FlowPtr<InlineAsm> n) override;
    void Visit(FlowPtr<IfStmt> n) override;
    void Visit(FlowPtr<WhileStmt> n) override;
    void Visit(FlowPtr<ForStmt> n) override;
    void Visit(FlowPtr<ForeachStmt> n) override;
    void Visit(FlowPtr<BreakStmt> n) override;
    void Visit(FlowPtr<ContinueStmt> n) override;
    void Visit(FlowPtr<ReturnStmt> n) override;
    void Visit(FlowPtr<ReturnIfStmt> n) override;
    void Visit(FlowPtr<CaseStmt> n) override;
    void Visit(FlowPtr<SwitchStmt> n) override;
    void Visit(FlowPtr<TypedefStmt> n) override;
    void Visit(FlowPtr<Function> n) override;
    void Visit(FlowPtr<StructDef> n) override;
    void Visit(FlowPtr<EnumDef> n) override;
    void Visit(FlowPtr<ScopeStmt> n) override;
    void Visit(FlowPtr<ExportStmt> n) override;
  };

  class NCC_EXPORT AstJsonWriter : public AstWriter {
    std::ostream& m_os;
    std::stack<bool> m_comma;
    std::stack<size_t> m_count;

    void Delim();

    void StrImpl(std::string_view str);
    void UintImpl(uint64_t val);
    void DoubleImpl(double val);
    void BoolImpl(bool val);
    void NullImpl();
    void BeginObjImpl(size_t pair_count);
    void EndObjImpl();
    void BeginArrImpl(size_t size);
    void EndArrImpl();

  public:
    AstJsonWriter(std::ostream& os, WriterSourceProvider rd = std::nullopt)
        : AstWriter(
              std::bind(&AstJsonWriter::StrImpl, this, std::placeholders::_1),
              std::bind(&AstJsonWriter::UintImpl, this, std::placeholders::_1),
              std::bind(&AstJsonWriter::DoubleImpl, this,
                        std::placeholders::_1),
              std::bind(&AstJsonWriter::BoolImpl, this, std::placeholders::_1),
              std::bind(&AstJsonWriter::NullImpl, this),
              std::bind(&AstJsonWriter::BeginObjImpl, this,
                        std::placeholders::_1),
              std::bind(&AstJsonWriter::EndObjImpl, this),
              std::bind(&AstJsonWriter::BeginArrImpl, this,
                        std::placeholders::_1),
              std::bind(&AstJsonWriter::EndArrImpl, this), rd),
          m_os(os) {
      m_comma.push(false);
      m_count.push(0);
    }
    virtual ~AstJsonWriter() = default;
  };

  class NCC_EXPORT AstMsgPackWriter : public AstWriter {
    std::ostream& m_os;

    void StrImpl(std::string_view str);
    void UintImpl(uint64_t val);
    void DoubleImpl(double val);
    void BoolImpl(bool val);
    void NullImpl();
    void BeginObjImpl(size_t pair_count);
    void EndObjImpl();
    void BeginArrImpl(size_t size);
    void EndArrImpl();

  public:
    AstMsgPackWriter(std::ostream& os, WriterSourceProvider rd = std::nullopt)
        : AstWriter(std::bind(&AstMsgPackWriter::StrImpl, this,
                              std::placeholders::_1),
                    std::bind(&AstMsgPackWriter::UintImpl, this,
                              std::placeholders::_1),
                    std::bind(&AstMsgPackWriter::DoubleImpl, this,
                              std::placeholders::_1),
                    std::bind(&AstMsgPackWriter::BoolImpl, this,
                              std::placeholders::_1),
                    std::bind(&AstMsgPackWriter::NullImpl, this),
                    std::bind(&AstMsgPackWriter::BeginObjImpl, this,
                              std::placeholders::_1),
                    std::bind(&AstMsgPackWriter::EndObjImpl, this),
                    std::bind(&AstMsgPackWriter::BeginArrImpl, this,
                              std::placeholders::_1),
                    std::bind(&AstMsgPackWriter::EndArrImpl, this), rd),
          m_os(os) {}
    virtual ~AstMsgPackWriter() = default;
  };
}  // namespace ncc::parse

#endif
