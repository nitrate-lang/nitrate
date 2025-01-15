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

#include <cstdint>
#include <istream>
#include <nitrate-core/Macro.hh>
#include <nitrate-core/NullableFlowPtr.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/ASTBase.hh>
#include <optional>
#include <variant>

namespace ncc::parse {
  using ReaderSourceManager =
      std::optional<std::reference_wrapper<lex::IScanner>>;

  class NCC_EXPORT AST_Reader {
  public:
    using none = std::nullptr_t;

    class Value {
      using Data = std::variant<std::string, uint64_t, double, bool, none>;
      Data m_data;

    public:
      template <typename T>
      Value(T&& data) : m_data(std::forward<T>(data)) {}

      Data& operator()() { return m_data; }
      const Data& operator()() const { return m_data; }
      bool operator==(const Value& o) const { return m_data == o.m_data; }
    };

    using NextFunc = std::function<std::optional<Value>()>;

  private:
    NextFunc m_next_func;
    std::optional<Value> m_peek;
    ReaderSourceManager m_source;
    std::optional<FlowPtr<Base>> m_root;

    std::optional<Value> next_value() {
      if (m_peek.has_value()) {
        auto val = m_peek;
        m_peek.reset();
        return val;
      }

      return m_next_func();
    }

    std::optional<Value> peek_value() {
      if (!m_peek.has_value()) {
        m_peek = m_next_func();
      }

      return m_peek;
    }

    struct LocationRange {
      lex::Location start, end;
    };

    std::optional<LocationRange> Read_LocationRange();

    NullableFlowPtr<Base> deserialize_object();
    NullableFlowPtr<Stmt> deserialize_statement();
    NullableFlowPtr<Expr> deserialize_expression();
    NullableFlowPtr<Type> deserialize_type();

    struct TypeMetadata {
      NullableFlowPtr<Expr> width, min, max;
    };

    std::optional<TypeMetadata> Read_TypeMetadata();

    NullableFlowPtr<Base> ReadKind_Node();
    NullableFlowPtr<BinExpr> ReadKind_Binexpr();
    NullableFlowPtr<UnaryExpr> ReadKind_Unexpr();
    NullableFlowPtr<TernaryExpr> ReadKind_Terexpr();
    NullableFlowPtr<ConstInt> ReadKind_Int();
    NullableFlowPtr<ConstFloat> ReadKind_Float();
    NullableFlowPtr<ConstString> ReadKind_String();
    NullableFlowPtr<ConstChar> ReadKind_Char();
    NullableFlowPtr<ConstBool> ReadKind_Bool();
    NullableFlowPtr<ConstNull> ReadKind_Null();
    NullableFlowPtr<ConstUndef> ReadKind_Undef();
    NullableFlowPtr<Call> ReadKind_Call();
    NullableFlowPtr<List> ReadKind_List();
    NullableFlowPtr<Assoc> ReadKind_Assoc();
    NullableFlowPtr<Index> ReadKind_Index();
    NullableFlowPtr<Slice> ReadKind_Slice();
    NullableFlowPtr<FString> ReadKind_Fstring();
    NullableFlowPtr<Ident> ReadKind_Ident();
    NullableFlowPtr<SeqPoint> ReadKind_SeqPoint();
    NullableFlowPtr<PostUnaryExpr> ReadKind_PostUnexpr();
    NullableFlowPtr<StmtExpr> ReadKind_StmtExpr();
    NullableFlowPtr<TypeExpr> ReadKind_TypeExpr();
    NullableFlowPtr<TemplCall> ReadKind_TemplCall();
    NullableFlowPtr<RefTy> ReadKind_Ref();
    NullableFlowPtr<U1> ReadKind_U1();
    NullableFlowPtr<U8> ReadKind_U8();
    NullableFlowPtr<U16> ReadKind_U16();
    NullableFlowPtr<U32> ReadKind_U32();
    NullableFlowPtr<U64> ReadKind_U64();
    NullableFlowPtr<U128> ReadKind_U128();
    NullableFlowPtr<I8> ReadKind_I8();
    NullableFlowPtr<I16> ReadKind_I16();
    NullableFlowPtr<I32> ReadKind_I32();
    NullableFlowPtr<I64> ReadKind_I64();
    NullableFlowPtr<I128> ReadKind_I128();
    NullableFlowPtr<F16> ReadKind_F16();
    NullableFlowPtr<F32> ReadKind_F32();
    NullableFlowPtr<F64> ReadKind_F64();
    NullableFlowPtr<F128> ReadKind_F128();
    NullableFlowPtr<VoidTy> ReadKind_Void();
    NullableFlowPtr<PtrTy> ReadKind_Ptr();
    NullableFlowPtr<OpaqueTy> ReadKind_Opaque();
    NullableFlowPtr<ArrayTy> ReadKind_Array();
    NullableFlowPtr<TupleTy> ReadKind_Tuple();
    NullableFlowPtr<FuncTy> ReadKind_FuncTy();
    NullableFlowPtr<NamedTy> ReadKind_Unres();
    NullableFlowPtr<InferTy> ReadKind_Infer();
    NullableFlowPtr<TemplType> ReadKind_Templ();
    NullableFlowPtr<TypedefStmt> ReadKind_Typedef();
    NullableFlowPtr<StructDef> ReadKind_Struct();
    NullableFlowPtr<EnumDef> ReadKind_Enum();
    NullableFlowPtr<Function> ReadKind_Function();
    NullableFlowPtr<ScopeStmt> ReadKind_Scope();
    NullableFlowPtr<ExportStmt> ReadKind_Export();
    NullableFlowPtr<Block> ReadKind_Block();
    NullableFlowPtr<VarDecl> ReadKind_Let();
    NullableFlowPtr<InlineAsm> ReadKind_InlineAsm();
    NullableFlowPtr<ReturnStmt> ReadKind_Return();
    NullableFlowPtr<ReturnIfStmt> ReadKind_Retif();
    NullableFlowPtr<BreakStmt> ReadKind_Break();
    NullableFlowPtr<ContinueStmt> ReadKind_Continue();
    NullableFlowPtr<IfStmt> ReadKind_If();
    NullableFlowPtr<WhileStmt> ReadKind_While();
    NullableFlowPtr<ForStmt> ReadKind_For();
    NullableFlowPtr<ForeachStmt> ReadKind_Foreach();
    NullableFlowPtr<CaseStmt> ReadKind_Case();
    NullableFlowPtr<SwitchStmt> ReadKind_Switch();
    NullableFlowPtr<ExprStmt> ReadKind_ExprStmt();

#ifdef AST_READER_IMPL
#undef next_if

    class StrongBool {
    public:
      bool m_val;

      constexpr StrongBool operator!() const { return {!m_val}; }
      constexpr StrongBool operator||(StrongBool o) const {
        return {m_val || o.m_val};
      }
      constexpr operator bool() { return m_val; }
    };

    template <typename ValueType>
    constexpr StrongBool next_if(const ValueType& v = ValueType()) {
      if (auto n = peek_value()) {
        if (std::holds_alternative<ValueType>(n->operator()()) &&
            std::get<ValueType>(n->operator()()) == v) {
          next_value();
          return StrongBool(true);
        }
      }

      return StrongBool(false);
    }

    template <typename ValueType>
    constexpr StrongBool next_is() {
      if (auto n = peek_value()) {
        if (std::holds_alternative<ValueType>(n->operator()())) {
          return StrongBool(true);
        }
      }

      return StrongBool(false);
    }

    template <typename ValueType>
    constexpr ValueType next() {
      if (auto n = next_value()) {
        if (std::holds_alternative<ValueType>(n->operator()())) {
          return std::get<ValueType>(n->operator()());
        }
      }

      qcore_panic("Attempted to read value of incorrect type");
    }

#endif

  public:
    AST_Reader(NextFunc data_source, ReaderSourceManager source_manager)
        : m_next_func(data_source), m_source(source_manager) {}
    virtual ~AST_Reader() = default;

    std::optional<FlowPtr<Base>> get();
  };

  class NCC_EXPORT AST_JsonReader final : public AST_Reader {
    std::optional<Value> ReadValue();
    std::istream& m_is;

    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;

  public:
    AST_JsonReader(std::istream& is,
                   ReaderSourceManager source_manager = std::nullopt);

    static std::optional<FlowPtr<Base>> FromString(
        const std::string& json,
        ReaderSourceManager source_manager = std::nullopt) {
      std::istringstream is(json);
      AST_JsonReader reader(is, source_manager);
      return reader.get();
    }

    ~AST_JsonReader() override;
  };

  class NCC_EXPORT AST_MsgPackReader final : public AST_Reader {
    std::optional<Value> ReadValue();
    std::istream& m_is;

    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;

  public:
    AST_MsgPackReader(std::istream& is,
                      ReaderSourceManager source_manager = std::nullopt);

    static std::optional<FlowPtr<Base>> FromString(
        const std::string& msgpack,
        ReaderSourceManager source_manager = std::nullopt) {
      std::istringstream is(msgpack);
      AST_MsgPackReader reader(is, source_manager);
      return reader.get();
    }

    ~AST_MsgPackReader() override;
  };
}  // namespace ncc::parse

#endif
