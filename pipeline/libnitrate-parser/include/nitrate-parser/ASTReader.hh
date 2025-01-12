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
#include <nitrate-parser/ASTVisitor.hh>
#include <optional>
#include <queue>
#include <stack>
#include <variant>

namespace ncc::parse {
  class NCC_EXPORT AST_Reader {
    class Value {
      using Data =
          std::variant<std::string, uint64_t, double, bool, std::nullptr_t,
                       std::vector<Value>, FlowPtr<Base>>;
      Data m_data;

    public:
      template <typename T>
      Value(T&& data) : m_data(std::forward<T>(data)) {}

      Data& operator()() { return m_data; }
      const Data& operator()() const { return m_data; }

      bool operator==(const Value& o) const { return m_data == o.m_data; }
    };

    enum class Mode : uint8_t { InsideArray, NotInsideArray };
    using State = std::pair<Mode, std::queue<Value>>;
    static inline State NewObjectState = {Mode::NotInsideArray,
                                          std::queue<Value>()};

    std::stack<State> m_stack;

    FlowPtr<Base> deserialize_object();

    FlowPtr<Base> ReadKind_Node();
    FlowPtr<Base> ReadKind_Binexpr();
    FlowPtr<Base> ReadKind_Unexpr();
    FlowPtr<Base> ReadKind_Terexpr();
    FlowPtr<Base> ReadKind_Int();
    FlowPtr<Base> ReadKind_Float();
    FlowPtr<Base> ReadKind_String();
    FlowPtr<Base> ReadKind_Char();
    FlowPtr<Base> ReadKind_Bool();
    FlowPtr<Base> ReadKind_Null();
    FlowPtr<Base> ReadKind_Undef();
    FlowPtr<Base> ReadKind_Call();
    FlowPtr<Base> ReadKind_List();
    FlowPtr<Base> ReadKind_Assoc();
    FlowPtr<Base> ReadKind_Index();
    FlowPtr<Base> ReadKind_Slice();
    FlowPtr<Base> ReadKind_Fstring();
    FlowPtr<Base> ReadKind_Ident();
    FlowPtr<Base> ReadKind_SeqPoint();
    FlowPtr<Base> ReadKind_PostUnexpr();
    FlowPtr<Base> ReadKind_StmtExpr();
    FlowPtr<Base> ReadKind_TypeExpr();
    FlowPtr<Base> ReadKind_TemplCall();
    FlowPtr<Base> ReadKind_Ref();
    FlowPtr<Base> ReadKind_U1();
    FlowPtr<Base> ReadKind_U8();
    FlowPtr<Base> ReadKind_U16();
    FlowPtr<Base> ReadKind_U32();
    FlowPtr<Base> ReadKind_U64();
    FlowPtr<Base> ReadKind_U128();
    FlowPtr<Base> ReadKind_I8();
    FlowPtr<Base> ReadKind_I16();
    FlowPtr<Base> ReadKind_I32();
    FlowPtr<Base> ReadKind_I64();
    FlowPtr<Base> ReadKind_I128();
    FlowPtr<Base> ReadKind_F16();
    FlowPtr<Base> ReadKind_F32();
    FlowPtr<Base> ReadKind_F64();
    FlowPtr<Base> ReadKind_F128();
    FlowPtr<Base> ReadKind_Void();
    FlowPtr<Base> ReadKind_Ptr();
    FlowPtr<Base> ReadKind_Opaque();
    FlowPtr<Base> ReadKind_Array();
    FlowPtr<Base> ReadKind_Tuple();
    FlowPtr<Base> ReadKind_FuncTy();
    FlowPtr<Base> ReadKind_Unres();
    FlowPtr<Base> ReadKind_Infer();
    FlowPtr<Base> ReadKind_Templ();
    FlowPtr<Base> ReadKind_Typedef();
    FlowPtr<Base> ReadKind_Struct();
    FlowPtr<Base> ReadKind_Enum();
    FlowPtr<Base> ReadKind_Function();
    FlowPtr<Base> ReadKind_Scope();
    FlowPtr<Base> ReadKind_Export();
    FlowPtr<Base> ReadKind_Block();
    FlowPtr<Base> ReadKind_Let();
    FlowPtr<Base> ReadKind_InlineAsm();
    FlowPtr<Base> ReadKind_Return();
    FlowPtr<Base> ReadKind_Retif();
    FlowPtr<Base> ReadKind_Break();
    FlowPtr<Base> ReadKind_Continue();
    FlowPtr<Base> ReadKind_If();
    FlowPtr<Base> ReadKind_While();
    FlowPtr<Base> ReadKind_For();
    FlowPtr<Base> ReadKind_Foreach();
    FlowPtr<Base> ReadKind_Case();
    FlowPtr<Base> ReadKind_Switch();
    FlowPtr<Base> ReadKind_ExprStmt();

    void push_value(Value&& value);

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
    constexpr StrongBool next_if(const ValueType& v) {
      auto& tokens = m_stack.top().second;

      if (!tokens.empty() &&
          std::holds_alternative<ValueType>(tokens.front()()) &&
          tokens.front() == v) {
        tokens.pop();
        return StrongBool(true);
      }

      return StrongBool(false);
    }

    template <typename ValueType>
    constexpr StrongBool next_is() const {
      const auto& tokens = m_stack.top().second;
      return StrongBool{std::holds_alternative<ValueType>(tokens.front()())};
    }

    template <typename ValueType>
    constexpr ValueType next() {
      auto& tokens = m_stack.top().second;
      auto value = std::get<ValueType>(tokens.front()());
      tokens.pop();

      return value;
    }

#endif

  protected:
    void str(std::string_view str);
    void uint(uint64_t val);
    void dbl(double val);
    void boolean(bool val);
    void null();
    void begin_obj(size_t pair_count);
    void end_obj();
    void begin_arr(size_t size);
    void end_arr();

  public:
    AST_Reader() { m_stack.push(NewObjectState); }
    virtual ~AST_Reader() = default;

    std::optional<FlowPtr<Base>> get();
  };

  class NCC_EXPORT AST_JsonReader final : public AST_Reader {
    void parse_stream(std::istream& is);

  public:
    AST_JsonReader(std::istream& is) { parse_stream(is); }
    virtual ~AST_JsonReader() = default;
  };

  class NCC_EXPORT AST_MsgPackReader final : public AST_Reader {
    void parse_stream(std::istream& is);

  public:
    AST_MsgPackReader(std::istream& is) { parse_stream(is); }
    virtual ~AST_MsgPackReader() = default;
  };
}  // namespace ncc::parse

#endif
