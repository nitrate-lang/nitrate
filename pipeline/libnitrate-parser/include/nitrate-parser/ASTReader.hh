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

    NullableFlowPtr<Base> ReadKind_Node();
    NullableFlowPtr<Base> ReadKind_Binexpr();
    NullableFlowPtr<Base> ReadKind_Unexpr();
    NullableFlowPtr<Base> ReadKind_Terexpr();
    NullableFlowPtr<Base> ReadKind_Int();
    NullableFlowPtr<Base> ReadKind_Float();
    NullableFlowPtr<Base> ReadKind_String();
    NullableFlowPtr<Base> ReadKind_Char();
    NullableFlowPtr<Base> ReadKind_Bool();
    NullableFlowPtr<Base> ReadKind_Null();
    NullableFlowPtr<Base> ReadKind_Undef();
    NullableFlowPtr<Base> ReadKind_Call();
    NullableFlowPtr<Base> ReadKind_List();
    NullableFlowPtr<Base> ReadKind_Assoc();
    NullableFlowPtr<Base> ReadKind_Index();
    NullableFlowPtr<Base> ReadKind_Slice();
    NullableFlowPtr<Base> ReadKind_Fstring();
    NullableFlowPtr<Base> ReadKind_Ident();
    NullableFlowPtr<Base> ReadKind_SeqPoint();
    NullableFlowPtr<Base> ReadKind_PostUnexpr();
    NullableFlowPtr<Base> ReadKind_StmtExpr();
    NullableFlowPtr<Base> ReadKind_TypeExpr();
    NullableFlowPtr<Base> ReadKind_TemplCall();
    NullableFlowPtr<Base> ReadKind_Ref();
    NullableFlowPtr<Base> ReadKind_U1();
    NullableFlowPtr<Base> ReadKind_U8();
    NullableFlowPtr<Base> ReadKind_U16();
    NullableFlowPtr<Base> ReadKind_U32();
    NullableFlowPtr<Base> ReadKind_U64();
    NullableFlowPtr<Base> ReadKind_U128();
    NullableFlowPtr<Base> ReadKind_I8();
    NullableFlowPtr<Base> ReadKind_I16();
    NullableFlowPtr<Base> ReadKind_I32();
    NullableFlowPtr<Base> ReadKind_I64();
    NullableFlowPtr<Base> ReadKind_I128();
    NullableFlowPtr<Base> ReadKind_F16();
    NullableFlowPtr<Base> ReadKind_F32();
    NullableFlowPtr<Base> ReadKind_F64();
    NullableFlowPtr<Base> ReadKind_F128();
    NullableFlowPtr<Base> ReadKind_Void();
    NullableFlowPtr<Base> ReadKind_Ptr();
    NullableFlowPtr<Base> ReadKind_Opaque();
    NullableFlowPtr<Base> ReadKind_Array();
    NullableFlowPtr<Base> ReadKind_Tuple();
    NullableFlowPtr<Base> ReadKind_FuncTy();
    NullableFlowPtr<Base> ReadKind_Unres();
    NullableFlowPtr<Base> ReadKind_Infer();
    NullableFlowPtr<Base> ReadKind_Templ();
    NullableFlowPtr<Base> ReadKind_Typedef();
    NullableFlowPtr<Base> ReadKind_Struct();
    NullableFlowPtr<Base> ReadKind_Enum();
    NullableFlowPtr<Base> ReadKind_Function();
    NullableFlowPtr<Base> ReadKind_Scope();
    NullableFlowPtr<Base> ReadKind_Export();
    NullableFlowPtr<Base> ReadKind_Block();
    NullableFlowPtr<Base> ReadKind_Let();
    NullableFlowPtr<Base> ReadKind_InlineAsm();
    NullableFlowPtr<Base> ReadKind_Return();
    NullableFlowPtr<Base> ReadKind_Retif();
    NullableFlowPtr<Base> ReadKind_Break();
    NullableFlowPtr<Base> ReadKind_Continue();
    NullableFlowPtr<Base> ReadKind_If();
    NullableFlowPtr<Base> ReadKind_While();
    NullableFlowPtr<Base> ReadKind_For();
    NullableFlowPtr<Base> ReadKind_Foreach();
    NullableFlowPtr<Base> ReadKind_Case();
    NullableFlowPtr<Base> ReadKind_Switch();
    NullableFlowPtr<Base> ReadKind_ExprStmt();

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
      if (auto n = next_value()) {
        if (std::holds_alternative<ValueType>(n->operator()()) &&
            std::get<ValueType>(n->operator()()) == v) {
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

    virtual ~AST_JsonReader();
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

    virtual ~AST_MsgPackReader();
  };
}  // namespace ncc::parse

#endif
