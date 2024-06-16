////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <IR/Q/Asm.h>
#include <IR/Q/Call.h>
#include <IR/Q/Cast.h>
#include <IR/Q/Control.h>
#include <IR/Q/Expr.h>
#include <IR/Q/Function.h>
#include <IR/Q/Ident.h>
#include <IR/Q/Math.h>
#include <IR/Q/Memory.h>
#include <IR/Q/OO.h>
#include <IR/Q/QIR.h>
#include <IR/Q/Type.h>
#include <IR/Q/Variable.h>

#include <iomanip>
#include <sstream>

using namespace libquixcc::ir::q;

Type *Call::infer() const { return func->type->as<FType>()->ret; }

Type *CallIndirect::infer() const { return exprfunc->return_type; }

Type *SCast::infer() const { return type; }

Type *UCast::infer() const { return type; }

Type *PtrICast::infer() const {
  switch (Ptr::create(Void::create())->size()) {
    case 1:
      return U8::create();
    case 2:
      return U16::create();
    case 4:
      return U32::create();
    case 8:
      return U64::create();
    case 16:
      return U128::create();
    default:
      throw std::runtime_error("Codegen failed: PtrICast type not supported");
  }
}

Type *IPtrCast::infer() const { return type; }

Type *Bitcast::infer() const { return type; }

Type *Segment::infer() const {
  std::vector<Type *> arg_types;
  for (auto &arg : params) {
    arg_types.push_back(arg.second);
  }

  return FType::create(arg_types, return_type, is_variadic, is_pure,
                       is_thread_safe, is_foriegn, is_no_throw);
}

Type *Ident::infer() const { return type; }

static Type *do_infer(Type *lhs, Type *rhs) {
  /// TOOD: Implement binary inference

  /*
      Wat we are doing:
      | Type A      | Type B      | Result Type |
      |-------------|-------------|-------------|
      | i1          | any         | i32         |
      | i8          | any integer | i32         |
      | u8          | any integer | i32         |
      | i16         | any integer | i32         |
      | u16         | any integer | i32         |
      | i32         | any integer | i32         |
      | u32         | any integer | u32         |
      | i64         | any integer | i64         |
      | i128        | any integer | i128        |
      | u64         | any integer | u64         |
      | u128        | any integer | u128        |
      | f32         | any integer | f32         |
      | f32         | f64         | f64         |
      | f64         | any integer | f64         |
      | *           | *           | undefined   |
  */

  if (lhs->is<F64>() || rhs->is<F64>()) return F64::create();

  if (lhs->is<F32>() || rhs->is<F32>()) return F32::create();

  if (lhs->is<I128>() || rhs->is<I128>()) return I128::create();

  if (lhs->is<U128>() || rhs->is<U128>()) return U128::create();

  if (lhs->is<I64>() || rhs->is<I64>()) return I64::create();

  if (lhs->is<U64>() || rhs->is<U64>()) return U64::create();

  if (lhs->is<I32>() || rhs->is<I32>()) return I32::create();

  if (lhs->is<U32>() || rhs->is<U32>()) return U32::create();

  if (lhs->is<I16>() || rhs->is<I16>()) return I16::create();

  if (lhs->is<U16>() || rhs->is<U16>()) return U16::create();

  if (lhs->is<I8>() || rhs->is<I8>()) return I8::create();

  if (lhs->is<U8>() || rhs->is<U8>()) return U8::create();

  if (lhs->is<I1>() || rhs->is<I1>()) return I1::create();

  throw std::runtime_error(
      "Codegen failed: Binary operation type not supported");
}

Type *Add::infer() const { return do_infer(lhs->infer(), rhs->infer()); }

Type *Sub::infer() const { return do_infer(lhs->infer(), rhs->infer()); }

Type *Mul::infer() const { return do_infer(lhs->infer(), rhs->infer()); }

Type *Div::infer() const { return do_infer(lhs->infer(), rhs->infer()); }

Type *Mod::infer() const { return do_infer(lhs->infer(), rhs->infer()); }

Type *BitAnd::infer() const { return do_infer(lhs->infer(), rhs->infer()); }

Type *BitOr::infer() const { return do_infer(lhs->infer(), rhs->infer()); }

Type *BitXor::infer() const { return do_infer(lhs->infer(), rhs->infer()); }

Type *BitNot::infer() const { return operand->infer(); }

Type *Shl::infer() const { return do_infer(lhs->infer(), rhs->infer()); }

Type *Shr::infer() const { return do_infer(lhs->infer(), rhs->infer()); }

Type *Rotl::infer() const { return do_infer(lhs->infer(), rhs->infer()); }

Type *Rotr::infer() const { return do_infer(lhs->infer(), rhs->infer()); }

Type *Eq::infer() const { return I1::create(); }

Type *Ne::infer() const { return I1::create(); }

Type *Lt::infer() const { return I1::create(); }

Type *Gt::infer() const { return I1::create(); }

Type *Le::infer() const { return I1::create(); }

Type *Ge::infer() const { return I1::create(); }

Type *And::infer() const { return I1::create(); }

Type *Or::infer() const { return I1::create(); }

Type *Not::infer() const { return I1::create(); }

Type *Xor::infer() const { return I1::create(); }

Type *Assign::infer() const { return rhs->infer(); }

Type *PostInc::infer() const { return lhs->infer(); }

Type *PostDec::infer() const { return lhs->infer(); }

Type *AddressOf::infer() const { return Ptr::create(lhs->infer()); }

Type *Deref::infer() const { return lhs->infer()->as<Ptr>()->type; }

Type *Member::infer() const { return field_type; }

Type *Index::infer() const { return type; }

uint8_t get_numbits(std::string s);

Type *Number::infer() const {
  uint8_t bits = get_numbits(value);

  if (value.find(".") != std::string::npos) {
    if (bits <= 32)
      return F32::create();
    else
      return F64::create();
  }

  switch (bits) {
    case 1:
    case 8:
    case 16:
    case 32:
      return U32::create();
    case 64:
      return U64::create();
    case 128:
      return U128::create();
    default:
      throw std::runtime_error("Codegen failed: Number type not supported");
  }
}

static std::string unqiue_typehash(const std::vector<Type *> &types) {
  auto h = libquixcc::ir::Hasher().gettag();
  for (auto &type : types) h.add(type);

  std::stringstream ss;
  ss << std::hex << std::setfill('0') << std::setw(2);
  for (auto b : h.hash().data) ss << (int)b;

  return ss.str().substr(16);
}

Type *List::infer() const {
  std::vector<Type *> types;
  for (auto &elem : values) types.push_back(elem->infer());

  if (types.empty())
    throw std::runtime_error(
        "Codegen failed: Can not perform inference on empty list");

  Type *type = types[0];

  for (size_t i = 1; i < types.size(); i++) {
    if (types[i] != type) {
      return Region::create("__t" + unqiue_typehash(types) + "_t", types);
    }
  }

  return Array::create(type, values.size());
}

Type *String::infer() const { return Ptr::create(U8::create()); }

Type *Char::infer() const { return U8::create(); }