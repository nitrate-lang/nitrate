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

#ifndef __NITRATE_PARSER_ASTTYPE_H__
#define __NITRATE_PARSER_ASTTYPE_H__

#ifndef __cplusplus
#error "This code requires c++"
#endif

#include <nitrate-parser/ASTBase.hh>

namespace npar {
  class NamedTy : public Type {
    SmallString m_name;

  public:
    NamedTy(SmallString name) : Type(QAST_NAMED), m_name(name) {}

    let get_name() const { return m_name; }
  };

  class InferTy : public Type {
  public:
    constexpr InferTy() : Type(QAST_INFER) {}
  };

  class TemplType : public Type {
    Type *m_template;
    CallArgs m_args;

  public:
    TemplType(Type *templ, const CallArgs &args)
        : Type(QAST_TEMPLATE), m_template(templ), m_args(args) {}

    let get_template() const { return m_template; }
    let get_args() const { return m_args; }
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
    Type *m_item;
    bool m_is_volatile;

  public:
    constexpr PtrTy(Type *item, bool is_volatile = false)
        : Type(QAST_PTR), m_item(item), m_is_volatile(is_volatile) {}

    let get_item() const { return m_item; }
    bool is_volatile() const { return m_is_volatile; }
  };

  class OpaqueTy : public Type {
    SmallString m_name;

  public:
    OpaqueTy(SmallString name) : Type(QAST_OPAQUE), m_name(name) {}

    let get_name() const { return m_name; }
  };

  class TupleTy : public Type {
    TupleTyItems m_items;

  public:
    TupleTy(const TupleTyItems &items) : Type(QAST_TUPLE), m_items(items) {}

    let get_items() const { return m_items; }
  };

  class ArrayTy : public Type {
    Type *m_item;
    Expr *m_size;

  public:
    constexpr ArrayTy(Type *item, Expr *size)
        : Type(QAST_ARRAY), m_item(item), m_size(size) {}

    let get_item() const { return m_item; }
    let get_size() const { return m_size; }
  };

  class RefTy : public Type {
    Type *m_item;

  public:
    constexpr RefTy(Type *item) : Type(QAST_REF), m_item(item) {}

    let get_item() const { return m_item; }
  };

  class FuncTy : public Type {
    ExpressionList m_attributes;
    FuncParams m_params;
    Type *m_return;
    FuncPurity m_purity;

  public:
    FuncTy(Type *return_type, FuncParams parameters, FuncPurity purity,
           ExpressionList attributes)
        : Type(QAST_FUNCTOR),
          m_attributes(attributes),
          m_params(parameters),
          m_return(return_type),
          m_purity(purity) {}

    let get_return() const { return m_return; }
    let get_params() const { return m_params; }
    let get_purity() const { return m_purity; }
    let get_attributes() const { return m_attributes; }
  };
}  // namespace npar

#endif
