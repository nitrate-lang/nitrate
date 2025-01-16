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

#ifndef __NITRATE_AST_ASTTYPE_H__
#define __NITRATE_AST_ASTTYPE_H__

#include <nitrate-parser/ASTBase.hh>
#include <span>

namespace ncc::parse {
  class NamedTy : public Type {
    string m_name;

  public:
    constexpr NamedTy(auto name) : Type(QAST_NAMED), m_name(name) {}

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
  };

  class InferTy : public Type {
  public:
    constexpr InferTy() : Type(QAST_INFER) {}
  };

  class TemplType : public Type {
    FlowPtr<Type> m_template;
    std::span<CallArg> m_args;

  public:
    constexpr TemplType(auto templ, auto args)
        : Type(QAST_TEMPLATE), m_template(templ), m_args(args) {}

    [[nodiscard]] constexpr auto GetTemplate() const { return m_template; }
    [[nodiscard]] constexpr auto GetArgs() const { return m_args; }
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
    bool m_is_volatile;

  public:
    constexpr PtrTy(auto item, auto is_volatile)
        : Type(QAST_PTR), m_item(item), m_is_volatile(is_volatile) {}

    [[nodiscard]] constexpr auto GetItem() const { return m_item; }
    [[nodiscard]] constexpr bool IsVolatile() const { return m_is_volatile; }
  };

  class OpaqueTy : public Type {
    string m_name;

  public:
    constexpr OpaqueTy(auto name) : Type(QAST_OPAQUE), m_name(name) {}

    [[nodiscard]] constexpr auto GetName() const { return m_name; }
  };

  class TupleTy : public Type {
    std::span<FlowPtr<Type>> m_items;

  public:
    constexpr TupleTy(auto items) : Type(QAST_TUPLE), m_items(items) {}

    [[nodiscard]] constexpr auto GetItems() const { return m_items; }
  };

  class ArrayTy : public Type {
    FlowPtr<Type> m_item;
    FlowPtr<Expr> m_size;

  public:
    constexpr ArrayTy(auto item, auto size)
        : Type(QAST_ARRAY), m_item(item), m_size(size) {}

    [[nodiscard]] constexpr auto GetItem() const { return m_item; }
    [[nodiscard]] constexpr auto GetSize() const { return m_size; }
  };

  class RefTy : public Type {
    FlowPtr<Type> m_item;

  public:
    constexpr RefTy(auto item) : Type(QAST_REF), m_item(item) {}

    [[nodiscard]] constexpr auto GetItem() const { return m_item; }
  };

  class FuncTy : public Type {
    std::span<FlowPtr<Expr>> m_attributes;
    std::span<FuncParam> m_params;
    FlowPtr<Type> m_return;
    Purity m_purity;
    bool m_variadic;

  public:
    constexpr FuncTy(auto return_type, auto parameters, auto variadic,
                     auto purity, auto attributes)
        : Type(QAST_FUNCTOR),
          m_attributes(attributes),
          m_params(parameters),
          m_return(return_type),
          m_purity(purity),
          m_variadic(variadic) {}

    [[nodiscard]] constexpr auto GetReturn() const { return m_return; }
    [[nodiscard]] constexpr auto GetPurity() const { return m_purity; }
    [[nodiscard]] constexpr auto GetParams() const { return m_params; }
    [[nodiscard]] constexpr auto IsVariadic() const { return m_variadic; }
    [[nodiscard]] constexpr auto GetAttributes() const { return m_attributes; }
  };
}  // namespace ncc::parse

#endif
