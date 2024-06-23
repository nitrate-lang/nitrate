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
#include <core/Logger.h>

#include <map>
#include <unordered_map>

using namespace libquixcc::ir::q;

template <typename T>
using vec = std::vector<T>;

template <typename K, typename V>
using map = std::map<K, V>;

template <typename X, typename Y>
using pair = std::pair<X, Y>;

using str = std::string;

class ThreadGarbageCollecter {
  std::vector<Value *> m_ptrs;

 public:
  ThreadGarbageCollecter() {}

  ~ThreadGarbageCollecter() {
    for (auto ptr : m_ptrs) {
      delete ptr;
    }
  }

  template <typename T>
  inline void adopt(Value *ptr) {
    m_ptrs.push_back(ptr);
  }
};

static I1 *g_i1 = new I1();
static I8 *g_i8 = new I8();
static I16 *g_i16 = new I16();
static I32 *g_i32 = new I32();
static I64 *g_i64 = new I64();
static I128 *g_i128 = new I128();
static U8 *g_u8 = new U8();
static U16 *g_u16 = new U16();
static U32 *g_u32 = new U32();
static U64 *g_u64 = new U64();
static U128 *g_u128 = new U128();
static F32 *g_f32 = new F32();
static F64 *g_f64 = new F64();
static Void *g_void = new Void();

static thread_local ThreadGarbageCollecter thread_gc;

#define MAKE_GC(T, ...)          \
  {                              \
    T *ptr = new T(__VA_ARGS__); \
    thread_gc.adopt<T>(ptr);     \
    return ptr;                  \
  }

RootNode *RootNode::create(vec<Value *> children) {
  MAKE_GC(RootNode, children);
}

SCast *SCast::create(Type *type, Expr *value) { MAKE_GC(SCast, type, value); }

UCast *UCast::create(Type *type, Expr *value) { MAKE_GC(UCast, type, value); }

PtrICast *PtrICast::create(Expr *value) { MAKE_GC(PtrICast, value); }

IPtrCast *IPtrCast::create(Type *type, Expr *value) {
  MAKE_GC(IPtrCast, type, value);
}

Bitcast *Bitcast::create(Type *type, Expr *value) {
  MAKE_GC(Bitcast, type, value);
}

IfElse *IfElse::create(Expr *cond, Value *then, Value *els) {
  MAKE_GC(IfElse, cond, then, els);
}

While *While::create(Expr *cond, Value *body) { MAKE_GC(While, cond, body); }

For *For::create(Value *init, Expr *cond, Expr *step, Value *body) {
  MAKE_GC(For, init, cond, step, body);
}

Loop *Loop::create(Value *body) { MAKE_GC(Loop, body); }

Break *Break::create() { MAKE_GC(Break, ); }

Continue *Continue::create() { MAKE_GC(Continue, ); }

Ret *Ret::create(Expr *value) { MAKE_GC(Ret, value); }

Throw *Throw::create(Expr *value) { MAKE_GC(Throw, value); }

TryCatchFinally *TryCatchFinally::create(
    Value *tryblock, vec<pair<Value *, Value *>> catchblocks,
    Value *finallyblock) {
  MAKE_GC(TryCatchFinally, tryblock, catchblocks, finallyblock);
}

Case *Case::create(Expr *value, Value *body) { MAKE_GC(Case, value, body); }

Switch *Switch::create(Expr *value,  std::vector<Case *> &cases,
                       Value *defaultcase) {
  MAKE_GC(Switch, value, cases, defaultcase);
}

Call *Call::create(Global *func, vec<Expr *> args) {
  MAKE_GC(Call, func, args);
}

CallIndirect *CallIndirect::create(Segment *callee, vec<Expr *> args) {
  MAKE_GC(CallIndirect, callee, args);
}

IntrinsicCall *IntrinsicCall::create(QIntrinsic name, vec<Expr *> args) {
  MAKE_GC(IntrinsicCall, name, args);
}

Ident *Ident::create(str name, Type *type) { MAKE_GC(Ident, name, type); }

Asm *Asm::create(str asm_str, vec<pair<str, Value *>> inputs,
                 vec<pair<str, Value *>> outputs, vec<str> clobbers) {
  MAKE_GC(Asm, asm_str, inputs, outputs, clobbers);
}

Block *Block::create(vec<Value *> stmts) { MAKE_GC(Block, stmts); }

Segment *Segment::create(vec<pair<str, Type *>> params, Type *return_type,
                         Block *block, bool is_variadic, bool is_pure,
                         bool is_thread_safe, bool is_no_throw,
                         bool is_no_return, bool is_foriegn) {
  MAKE_GC(Segment, params, return_type, block, is_variadic, is_pure,
          is_thread_safe, is_no_throw, is_no_return, is_foriegn);
}

Add *Add::create(Expr *lhs, Expr *rhs) { MAKE_GC(Add, lhs, rhs); }

Sub *Sub::create(Expr *lhs, Expr *rhs) { MAKE_GC(Sub, lhs, rhs); }

Mul *Mul::create(Expr *lhs, Expr *rhs) { MAKE_GC(Mul, lhs, rhs); }

Div *Div::create(Expr *lhs, Expr *rhs) { MAKE_GC(Div, lhs, rhs); }

Mod *Mod::create(Expr *lhs, Expr *rhs) { MAKE_GC(Mod, lhs, rhs); }

BitAnd *BitAnd::create(Expr *lhs, Expr *rhs) { MAKE_GC(BitAnd, lhs, rhs); }

BitOr *BitOr::create(Expr *lhs, Expr *rhs) { MAKE_GC(BitOr, lhs, rhs); }

BitXor *BitXor::create(Expr *lhs, Expr *rhs) { MAKE_GC(BitXor, lhs, rhs); }

BitNot *BitNot::create(Expr *value) { MAKE_GC(BitNot, value); }

Shl *Shl::create(Expr *lhs, Expr *rhs) { MAKE_GC(Shl, lhs, rhs); }

Shr *Shr::create(Expr *lhs, Expr *rhs) { MAKE_GC(Shr, lhs, rhs); }

Rotl *Rotl::create(Expr *lhs, Expr *rhs) { MAKE_GC(Rotl, lhs, rhs); }

Rotr *Rotr::create(Expr *lhs, Expr *rhs) { MAKE_GC(Rotr, lhs, rhs); }

Eq *Eq::create(Expr *lhs, Expr *rhs) { MAKE_GC(Eq, lhs, rhs); }

Ne *Ne::create(Expr *lhs, Expr *rhs) { MAKE_GC(Ne, lhs, rhs); }

Lt *Lt::create(Expr *lhs, Expr *rhs) { MAKE_GC(Lt, lhs, rhs); }

Gt *Gt::create(Expr *lhs, Expr *rhs) { MAKE_GC(Gt, lhs, rhs); }

Le *Le::create(Expr *lhs, Expr *rhs) { MAKE_GC(Le, lhs, rhs); }

Ge *Ge::create(Expr *lhs, Expr *rhs) { MAKE_GC(Ge, lhs, rhs); }

And *And::create(Expr *lhs, Expr *rhs) { MAKE_GC(And, lhs, rhs); }

Or *Or::create(Expr *lhs, Expr *rhs) { MAKE_GC(Or, lhs, rhs); }

Not *Not::create(Expr *value) { MAKE_GC(Not, value); }

Xor *Xor::create(Expr *lhs, Expr *rhs) { MAKE_GC(Xor, lhs, rhs); }

I1 *I1::create() { return g_i1; }

I8 *I8::create() { return g_i8; }

I16 *I16::create() { return g_i16; }

I32 *I32::create() { return g_i32; }

I64 *I64::create() { return g_i64; }

I128 *I128::create() { return g_i128; }

U8 *U8::create() { return g_u8; }

U16 *U16::create() { return g_u16; }

U32 *U32::create() { return g_u32; }

U64 *U64::create() { return g_u64; }

U128 *U128::create() { return g_u128; }

F32 *F32::create() { return g_f32; }

F64 *F64::create() { return g_f64; }

Void *Void::create() { return g_void; }

Ptr *Ptr::create(Type *type) { MAKE_GC(Ptr, type); }

Array *Array::create(Type *type, uint64_t size) { MAKE_GC(Array, type, size); }

Vector *Vector::create(Type *type) { MAKE_GC(Vector, type); }

FType *FType::create(vec<Type *> params, Type *ret, bool variadic, bool pure,
                     bool thread_safe, bool foreign, bool _noexcept) {
  MAKE_GC(FType, params, ret, variadic, pure, thread_safe, foreign, _noexcept);
}

Region *Region::create(str name, vec<Type *> fields, bool packed,
                       bool ordered) {
  MAKE_GC(Region, name, fields, packed, ordered);
}

Union *Union::create(str name, vec<Type *> fields) {
  MAKE_GC(Union, name, fields);
}

Opaque *Opaque::create(str name) { MAKE_GC(Opaque, name); }

IntrinsicType *IntrinsicType::create(QIntrinsicType name) {
  MAKE_GC(IntrinsicType, name);
}

RegionDef *RegionDef::create(str name, vec<pair<str, Value *>> fields,
                             map<str, Segment *> methods) {
  MAKE_GC(RegionDef, name, fields, methods);
}

GroupDef *GroupDef::create(str name, map<str, Value *> fields,
                           map<str, Segment *> methods) {
  MAKE_GC(GroupDef, name, fields, methods);
}

UnionDef *UnionDef::create(str name, map<str, Value *> fields,
                           map<str, Segment *> methods) {
  MAKE_GC(UnionDef, name, fields, methods);
}

Local *Local::create(str name, Type *type, Expr *value) {
  MAKE_GC(Local, name, type, value);
}

Global *Global::create(str name, Type *type, Expr *value, bool _volatile,
                       bool _atomic, bool _extern) {
  MAKE_GC(Global, name, type, value, _volatile, _atomic, _extern);
}

Number *Number::create(str value) { MAKE_GC(Number, value); }

String *String::create(str value) { MAKE_GC(String, value); }

Char *Char::create(str value) { MAKE_GC(Char, value); }

List *List::create(vec<Expr *> values) { MAKE_GC(List, values); }

Assign *Assign::create(Expr *lhs, Expr *rhs) { MAKE_GC(Assign, lhs, rhs); }

PostInc *PostInc::create(Expr *lhs) { MAKE_GC(PostInc, lhs); }

PostDec *PostDec::create(Expr *lhs) { MAKE_GC(PostDec, lhs); }

AddressOf *AddressOf::create(Expr *lhs) { MAKE_GC(AddressOf, lhs); }

Deref *Deref::create(Expr *lhs) { MAKE_GC(Deref, lhs); }

Member *Member::create(Value *lhs, size_t field, Type *field_type) {
  MAKE_GC(Member, lhs, field, field_type);
}

Index *Index::create(Value *lhs, Expr *index, Type *type) {
  MAKE_GC(Index, lhs, index, type);
}