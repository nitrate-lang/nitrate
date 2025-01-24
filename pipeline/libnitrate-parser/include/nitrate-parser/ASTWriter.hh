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
#include <nitrate-lexer/Scanner.hh>
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

    InsertString string;        // NOLINT
    InsertUInt64 uint64;        // NOLINT
    InsertDouble dbl;           // NOLINT
    InsertBool boolean;         // NOLINT
    InsertNull null;            // NOLINT
    BeginObject begin_obj;      // NOLINT
    EndObject end_obj;          // NOLINT
    BeginArray begin_arr;       // NOLINT
    EndArray end_arr;           // NOLINT
    WriterSourceProvider m_rd;  // NOLINT

    void WriteSourceLocation(const FlowPtr<Base>& n) const;
    void WriteTypeMetadata(const FlowPtr<Type>& n);

    [[nodiscard]] static auto VisStr(Vis vis) -> std::string_view;

  public:
    AstWriter(auto str_impl, auto uint_impl, auto dbl_impl, auto bool_impl,
              auto null_impl, auto begin_obj_impl, auto end_obj_impl,
              auto begin_arr_impl, auto end_arr_impl,
              WriterSourceProvider rd = std::nullopt)
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
    ~AstWriter() override = default;

    void Visit(FlowPtr<Base> n) override;
    void Visit(FlowPtr<ExprStmt> n) override;
    void Visit(FlowPtr<StmtExpr> n) override;
    void Visit(FlowPtr<TypeExpr> n) override;
    void Visit(FlowPtr<NamedTy> n) override;
    void Visit(FlowPtr<InferTy> n) override;
    void Visit(FlowPtr<TemplateType> n) override;
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
    void Visit(FlowPtr<UnaryExpression> n) override;
    void Visit(FlowPtr<BinaryExpression> n) override;
    void Visit(FlowPtr<PostUnaryExpression> n) override;
    void Visit(FlowPtr<TernaryExpression> n) override;
    void Visit(FlowPtr<Integer> n) override;
    void Visit(FlowPtr<Float> n) override;
    void Visit(FlowPtr<Boolean> n) override;
    void Visit(FlowPtr<String> n) override;
    void Visit(FlowPtr<Character> n) override;
    void Visit(FlowPtr<Null> n) override;
    void Visit(FlowPtr<Undefined> n) override;
    void Visit(FlowPtr<Call> n) override;
    void Visit(FlowPtr<TemplateCall> n) override;
    void Visit(FlowPtr<List> n) override;
    void Visit(FlowPtr<Assoc> n) override;
    void Visit(FlowPtr<Index> n) override;
    void Visit(FlowPtr<Slice> n) override;
    void Visit(FlowPtr<FString> n) override;
    void Visit(FlowPtr<Identifier> n) override;
    void Visit(FlowPtr<Sequence> n) override;
    void Visit(FlowPtr<Block> n) override;
    void Visit(FlowPtr<Variable> n) override;
    void Visit(FlowPtr<Assembly> n) override;
    void Visit(FlowPtr<If> n) override;
    void Visit(FlowPtr<While> n) override;
    void Visit(FlowPtr<For> n) override;
    void Visit(FlowPtr<Foreach> n) override;
    void Visit(FlowPtr<Break> n) override;
    void Visit(FlowPtr<Continue> n) override;
    void Visit(FlowPtr<Return> n) override;
    void Visit(FlowPtr<ReturnIf> n) override;
    void Visit(FlowPtr<Case> n) override;
    void Visit(FlowPtr<Switch> n) override;
    void Visit(FlowPtr<Typedef> n) override;
    void Visit(FlowPtr<Function> n) override;
    void Visit(FlowPtr<Struct> n) override;
    void Visit(FlowPtr<Enum> n) override;
    void Visit(FlowPtr<Scope> n) override;
    void Visit(FlowPtr<Export> n) override;
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
              [this](auto&& x) { StrImpl(std::forward<decltype(x)>(x)); },
              [this](auto&& x) { UintImpl(std::forward<decltype(x)>(x)); },
              [this](auto&& x) { DoubleImpl(std::forward<decltype(x)>(x)); },
              [this](auto&& x) { BoolImpl(std::forward<decltype(x)>(x)); },
              [this] { NullImpl(); },
              [this](auto&& x) { BeginObjImpl(std::forward<decltype(x)>(x)); },
              [this] { EndObjImpl(); },
              [this](auto&& x) { BeginArrImpl(std::forward<decltype(x)>(x)); },
              [this] { EndArrImpl(); }, rd),
          m_os(os) {
      m_comma.push(false);
      m_count.push(0);
    }
    ~AstJsonWriter() override = default;
  };

  class NCC_EXPORT AstMsgPackWriter : public AstWriter {
    std::ostream& m_os;

    void StrImpl(std::string_view str);
    void UintImpl(uint64_t x);
    void DoubleImpl(double x);
    void BoolImpl(bool x);
    void NullImpl();
    void BeginObjImpl(size_t pair_count);
    void EndObjImpl();
    void BeginArrImpl(size_t size);
    void EndArrImpl();

  public:
    AstMsgPackWriter(std::ostream& os, WriterSourceProvider rd = std::nullopt)
        : AstWriter(
              [this](auto&& x) { StrImpl(std::forward<decltype(x)>(x)); },
              [this](auto&& x) { UintImpl(std::forward<decltype(x)>(x)); },
              [this](auto&& x) { DoubleImpl(std::forward<decltype(x)>(x)); },
              [this](auto&& x) { BoolImpl(std::forward<decltype(x)>(x)); },
              [this] { NullImpl(); },
              [this](auto&& x) { BeginObjImpl(std::forward<decltype(x)>(x)); },
              [this] { EndObjImpl(); },
              [this](auto&& x) { BeginArrImpl(std::forward<decltype(x)>(x)); },
              [this] { EndArrImpl(); }, rd),
          m_os(os) {}
    ~AstMsgPackWriter() override = default;
  };
}  // namespace ncc::parse

#endif
