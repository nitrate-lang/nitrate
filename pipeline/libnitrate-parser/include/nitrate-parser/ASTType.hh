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

#ifndef __NITRATE_AST_TYPE_H__
#define __NITRATE_AST_TYPE_H__

#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTData.hh>
#include <span>

namespace ncc::parse {
  class NamedTy : public Type {
    string m_name;

  public:
    constexpr NamedTy(auto name) : Type(QAST_NAMED), m_name(name) {}

    [[nodiscard, gnu::pure]] constexpr auto GetName() const { return m_name; }
  };

  class InferTy : public Type {
  public:
    constexpr InferTy() : Type(QAST_INFER) {}
  };

  class TemplateType : public Type {
    FlowPtr<Type> m_template;
    std::span<CallArg> m_args;

  public:
    constexpr TemplateType(auto templ, auto args) : Type(QAST_TEMPLATE), m_template(std::move(templ)), m_args(args) {}

    [[nodiscard, gnu::pure]] constexpr auto GetTemplate() const { return m_template; }
    [[nodiscard, gnu::pure]] constexpr auto GetArgs() const { return m_args; }
  };

  class U1 : public Type {
  public:
    constexpr U1() : Type(QAST_U1){};
  };

  class U8 : public Type {
  public:
    constexpr U8() : Type(QAST_U8){};
  };

  class U16 : public Type {
  public:
    constexpr U16() : Type(QAST_U16){};
  };

  class U32 : public Type {
  public:
    constexpr U32() : Type(QAST_U32){};
  };

  class U64 : public Type {
  public:
    constexpr U64() : Type(QAST_U64){};
  };

  class U128 : public Type {
  public:
    constexpr U128() : Type(QAST_U128){};
  };

  class I8 : public Type {
  public:
    constexpr I8() : Type(QAST_I8){};
  };

  class I16 : public Type {
  public:
    constexpr I16() : Type(QAST_I16){};
  };

  class I32 : public Type {
  public:
    constexpr I32() : Type(QAST_I32){};
  };

  class I64 : public Type {
  public:
    constexpr I64() : Type(QAST_I64){};
  };

  class I128 : public Type {
  public:
    constexpr I128() : Type(QAST_I128){};
  };

  class F16 : public Type {
  public:
    constexpr F16() : Type(QAST_F16){};
  };

  class F32 : public Type {
  public:
    constexpr F32() : Type(QAST_F32){};
  };

  class F64 : public Type {
  public:
    constexpr F64() : Type(QAST_F64){};
  };

  class F128 : public Type {
  public:
    constexpr F128() : Type(QAST_F128){};
  };

  class VoidTy : public Type {
  public:
    constexpr VoidTy() : Type(QAST_VOID) {}
  };

  class PtrTy : public Type {
    FlowPtr<Type> m_item;
    bool m_volatil;

  public:
    constexpr PtrTy(auto item, auto volatil) : Type(QAST_PTR), m_item(std::move(item)), m_volatil(volatil) {}

    [[nodiscard, gnu::pure]] constexpr auto GetItem() const { return m_item; }
    [[nodiscard, gnu::pure]] constexpr auto IsVolatile() const -> bool { return m_volatil; }
  };

  class OpaqueTy : public Type {
    string m_name;

  public:
    constexpr OpaqueTy(auto name) : Type(QAST_OPAQUE), m_name(name) {}

    [[nodiscard, gnu::pure]] constexpr auto GetName() const { return m_name; }
  };

  class TupleTy : public Type {
    std::span<FlowPtr<Type>> m_items;

  public:
    constexpr TupleTy(auto items) : Type(QAST_TUPLE), m_items(items) {}

    [[nodiscard, gnu::pure]] constexpr auto GetItems() const { return m_items; }
  };

  class ArrayTy : public Type {
    FlowPtr<Type> m_item;
    FlowPtr<Expr> m_size;

  public:
    constexpr ArrayTy(auto item, auto size) : Type(QAST_ARRAY), m_item(std::move(item)), m_size(std::move(size)) {}

    [[nodiscard, gnu::pure]] constexpr auto GetItem() const { return m_item; }
    [[nodiscard, gnu::pure]] constexpr auto GetSize() const { return m_size; }
  };

  class RefTy : public Type {
    FlowPtr<Type> m_item;
    bool m_volatil;

  public:
    constexpr RefTy(auto item, bool volatil) : Type(QAST_REF), m_item(std::move(item)), m_volatil(volatil) {}

    [[nodiscard, gnu::pure]] constexpr auto GetItem() const { return m_item; }
    [[nodiscard, gnu::pure]] constexpr auto IsVolatile() const { return m_volatil; }
  };

  class FuncTy : public Type {
    std::span<FlowPtr<Expr>> m_attributes;
    std::span<FuncParam> m_params;
    FlowPtr<Type> m_return;
    bool m_variadic;

  public:
    constexpr FuncTy(auto return_type, auto parameters, auto variadic, auto attributes)
        : Type(QAST_FUNCTOR),
          m_attributes(attributes),
          m_params(parameters),
          m_return(std::move(return_type)),
          m_variadic(variadic) {}

    [[nodiscard, gnu::pure]] constexpr auto GetReturn() const { return m_return; }
    [[nodiscard, gnu::pure]] constexpr auto GetParams() const { return m_params; }
    [[nodiscard, gnu::pure]] constexpr auto IsVariadic() const { return m_variadic; }
    [[nodiscard, gnu::pure]] constexpr auto GetAttributes() const { return m_attributes; }
  };
}  // namespace ncc::parse

#endif
