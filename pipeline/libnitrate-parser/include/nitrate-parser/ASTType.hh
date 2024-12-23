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
  class npar_pack NamedTy : public Type {
    string m_name;

  public:
    constexpr NamedTy(string name) : Type(QAST_NAMED), m_name(name) {}

    constexpr auto get_name() const { return m_name.get(); }
  };

  class npar_pack InferTy : public Type {
  public:
    constexpr InferTy() : Type(QAST_INFER) {}
  };

  class TemplType : public Type {
    RefNode<Type> m_template;
    std::span<CallArg> m_args;

  public:
    TemplType(RefNode<Type> templ, CallArgs args)
        : Type(QAST_TEMPLATE), m_template(templ), m_args(args) {}

    constexpr let get_template() const { return m_template; }
    constexpr let get_args() const { return m_args; }
  };

  class npar_pack U1 : public Type {
  public:
    constexpr U1() : Type(QAST_U1){};
  };

  class npar_pack U8 : public Type {
  public:
    constexpr U8() : Type(QAST_U8){};
  };

  class npar_pack U16 : public Type {
  public:
    constexpr U16() : Type(QAST_U16){};
  };

  class npar_pack U32 : public Type {
  public:
    constexpr U32() : Type(QAST_U32){};
  };

  class npar_pack U64 : public Type {
  public:
    constexpr U64() : Type(QAST_U64){};
  };

  class npar_pack U128 : public Type {
  public:
    constexpr U128() : Type(QAST_U128){};
  };

  class npar_pack I8 : public Type {
  public:
    constexpr I8() : Type(QAST_I8){};
  };

  class npar_pack I16 : public Type {
  public:
    constexpr I16() : Type(QAST_I16){};
  };

  class npar_pack I32 : public Type {
  public:
    constexpr I32() : Type(QAST_I32){};
  };

  class npar_pack I64 : public Type {
  public:
    constexpr I64() : Type(QAST_I64){};
  };

  class npar_pack I128 : public Type {
  public:
    constexpr I128() : Type(QAST_I128){};
  };

  class npar_pack F16 : public Type {
  public:
    constexpr F16() : Type(QAST_F16){};
  };

  class npar_pack F32 : public Type {
  public:
    constexpr F32() : Type(QAST_F32){};
  };

  class npar_pack F64 : public Type {
  public:
    constexpr F64() : Type(QAST_F64){};
  };

  class npar_pack F128 : public Type {
  public:
    constexpr F128() : Type(QAST_F128){};
  };

  class npar_pack VoidTy : public Type {
  public:
    constexpr VoidTy() : Type(QAST_VOID) {}
  };

  class npar_pack PtrTy : public Type {
    RefNode<Type> m_item;
    bool m_is_volatile;

  public:
    constexpr PtrTy(RefNode<Type> item, bool is_volatile = false)
        : Type(QAST_PTR), m_item(item), m_is_volatile(is_volatile) {}

    constexpr let get_item() const { return m_item; }
    constexpr bool is_volatile() const { return m_is_volatile; }
  };

  class npar_pack OpaqueTy : public Type {
    string m_name;

  public:
    OpaqueTy(string name) : Type(QAST_OPAQUE), m_name(name) {}

    constexpr auto get_name() const { return m_name.get(); }
  };

  class TupleTy : public Type {
    std::span<RefNode<Type> > m_items;

  public:
    TupleTy(TupleTyItems items) : Type(QAST_TUPLE), m_items(items) {}

    constexpr let get_items() const { return m_items; }
  };

  class npar_pack ArrayTy : public Type {
    RefNode<Type> m_item;
    RefNode<Expr> m_size;

  public:
    constexpr ArrayTy(RefNode<Type> item, RefNode<Expr> size)
        : Type(QAST_ARRAY), m_item(item), m_size(size) {}

    constexpr let get_item() const { return m_item; }
    constexpr let get_size() const { return m_size; }
  };

  class npar_pack RefTy : public Type {
    RefNode<Type> m_item;

  public:
    constexpr RefTy(RefNode<Type> item) : Type(QAST_REF), m_item(item) {}

    constexpr let get_item() const { return m_item; }
  };

  class FuncTy : public Type {
    std::span<RefNode<Expr> > m_attributes;
    FuncParams m_params;
    RefNode<Type> m_return;
    FuncPurity m_purity;

  public:
    FuncTy(RefNode<Type> return_type, FuncParams parameters, FuncPurity purity,
           std::span<RefNode<Expr> > attributes)
        : Type(QAST_FUNCTOR),
          m_attributes(attributes),
          m_params(parameters),
          m_return(return_type),
          m_purity(purity) {}

    constexpr let get_return() const { return m_return; }
    constexpr let get_purity() const { return m_purity; }
    constexpr let get_params() const { return m_params; }
    constexpr let get_attributes() const { return m_attributes; }
  };
}  // namespace ncc::parse

#endif
