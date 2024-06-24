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

#include <IR/delta/Cast.h>
#include <IR/delta/Control.h>
#include <IR/delta/DeltaIR.h>
#include <IR/delta/Ident.h>
#include <IR/delta/Math.h>
#include <IR/delta/Memory.h>
#include <IR/delta/Segment.h>
#include <IR/delta/Type.h>
#include <IR/delta/Variable.h>

#include <map>

using namespace libquixcc;
using namespace libquixcc::ir;
using namespace libquixcc::ir::delta;

template <typename T>
using vec = std::vector<T>;

template <typename K, typename V>
using map = std::map<K, V>;

template <typename X, typename Y>
using pair = std::pair<X, Y>;

using str = std::string;

class ThreadGarbageCollecter {
  vec<Value *> m_ptrs;

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

const RootNode *delta::RootNode::create(vec<const Value *> children) {
  MAKE_GC(RootNode, children);
}

const delta::SCast *delta::SCast::create(const Type *type, const Expr *value) {
  MAKE_GC(SCast, type, value);
}

const delta::UCast *delta::UCast::create(const Type *type, const Expr *value) {
  MAKE_GC(UCast, type, value);
}

const delta::PtrICast *delta::PtrICast::create(const Expr *value) {
  MAKE_GC(PtrICast, value);
}

const delta::IPtrCast *delta::IPtrCast::create(const Type *type,
                                               const Expr *value) {
  MAKE_GC(IPtrCast, type, value);
}

const delta::Bitcast *delta::Bitcast::create(const Type *type,
                                             const Expr *value) {
  MAKE_GC(Bitcast, type, value);
}

const delta::IfElse *delta::IfElse::create(const Expr *cond, const Value *then,
                                           const Value *els) {
  MAKE_GC(IfElse, cond, then, els);
}

const delta::While *delta::While::create(const Expr *cond, const Value *body) {
  MAKE_GC(While, cond, body);
}

const delta::Jmp *delta::Jmp::create(str target) { MAKE_GC(Jmp, target); }

const delta::Label *delta::Label::create(str name, const Value *code) {
  MAKE_GC(Label, name, code);
}

const delta::Ret *delta::Ret::create(const Expr *value) { MAKE_GC(Ret, value); }

const delta::Call *delta::Call::create(str callee, vec<const Expr *> args,
                                       const FType *ftype) {
  MAKE_GC(Call, callee, args, ftype);
}

const delta::PtrCall *delta::PtrCall::create(const Expr *callee,
                                             vec<const Expr *> args) {
  MAKE_GC(PtrCall, callee, args);
}

const delta::Halt *delta::Halt::create() { MAKE_GC(Halt); }

const delta::Break *delta::Break::create() { MAKE_GC(Break); }

const delta::Continue *delta::Continue::create() { MAKE_GC(Continue); }

const delta::Case *delta::Case::create(const delta::Number *value,
                                       const delta::Value *code) {
  MAKE_GC(Case, value, code);
}

const delta::Switch *delta::Switch::create(const delta::Expr *cond,
                                           vec<const delta::Case *> cases,
                                           const delta::Value *def) {
  MAKE_GC(Switch, cond, cases, def);
}

const delta::Add *delta::Add::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Add, lhs, rhs);
}

const delta::Sub *delta::Sub::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Sub, lhs, rhs);
}

const delta::Mul *delta::Mul::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Mul, lhs, rhs);
}

const delta::Div *delta::Div::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Div, lhs, rhs);
}

const delta::Mod *delta::Mod::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Mod, lhs, rhs);
}

const delta::BitAnd *delta::BitAnd::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(BitAnd, lhs, rhs);
}

const delta::BitOr *delta::BitOr::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(BitOr, lhs, rhs);
}

const delta::BitXor *delta::BitXor::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(BitXor, lhs, rhs);
}

const delta::BitNot *delta::BitNot::create(const Expr *value) {
  MAKE_GC(BitNot, value);
}

const delta::Shl *delta::Shl::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Shl, lhs, rhs);
}

const delta::Shr *delta::Shr::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Shr, lhs, rhs);
}

const delta::Rotl *delta::Rotl::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Rotl, lhs, rhs);
}

const delta::Rotr *delta::Rotr::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Rotr, lhs, rhs);
}

const delta::Eq *delta::Eq::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Eq, lhs, rhs);
}

const delta::Ne *delta::Ne::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Ne, lhs, rhs);
}

const delta::Lt *delta::Lt::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Lt, lhs, rhs);
}

const delta::Gt *delta::Gt::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Gt, lhs, rhs);
}

const delta::Le *delta::Le::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Le, lhs, rhs);
}

const delta::Ge *delta::Ge::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Ge, lhs, rhs);
}

const delta::And *delta::And::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(And, lhs, rhs);
}

const delta::Or *delta::Or::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Or, lhs, rhs);
}

const delta::Not *delta::Not::create(const Value *value) {
  MAKE_GC(Not, value);
}

const delta::Xor *delta::Xor::create(const Expr *lhs, const Expr *rhs) {
  MAKE_GC(Xor, lhs, rhs);
}

const delta::Assign *delta::Assign::create(const Expr *var, const Expr *value,
                                           uint64_t rank) {
  MAKE_GC(Assign, var, value, rank);
}

const delta::PostInc *delta::PostInc::create(const delta::Expr *var) {
  MAKE_GC(PostInc, var);
}

const delta::PostDec *delta::PostDec::create(const delta::Expr *var) {
  MAKE_GC(PostDec, var);
}

const delta::AddressOf *delta::AddressOf::create(const delta::Expr *lhs) {
  MAKE_GC(AddressOf, lhs);
}

const delta::Deref *delta::Deref::create(const delta::Expr *lhs) {
  MAKE_GC(Deref, lhs);
}

const delta::Member *delta::Member::create(const delta::Expr *lhs, size_t field,
                                           const delta::Type *field_type) {
  MAKE_GC(Member, lhs, field, field_type);
}

const delta::Index *delta::Index::create(const Expr *var, const Expr *index,
                                         const Type *type) {
  MAKE_GC(Index, var, index, type);
}

const delta::Block *delta::Block::create(vec<const Value *> stmts) {
  MAKE_GC(Block, stmts);
}

const delta::Segment *delta::Segment::create(
    const Type *ret, bool variadic,
    const vec<std::pair<str, const Type *>> &params, const Block *stmts) {
  MAKE_GC(Segment, ret, variadic, params, stmts);
}

const delta::I1 *delta::I1::create() { return g_i1; }

const delta::I8 *delta::I8::create() { return g_i8; }

const delta::I16 *delta::I16::create() { return g_i16; }

const delta::I32 *delta::I32::create() { return g_i32; }

const delta::I64 *delta::I64::create() { return g_i64; }

const delta::I128 *delta::I128::create() { return g_i128; }

const delta::U8 *delta::U8::create() { return g_u8; }

const delta::U16 *delta::U16::create() { return g_u16; }

const delta::U32 *delta::U32::create() { return g_u32; }

const delta::U64 *delta::U64::create() { return g_u64; }

const delta::U128 *delta::U128::create() { return g_u128; }

const delta::F32 *delta::F32::create() { return g_f32; }

const delta::F64 *delta::F64::create() { return g_f64; }

const delta::Void *delta::Void::create() { return g_void; }

const delta::Ptr *delta::Ptr::create(const Type *type) { MAKE_GC(Ptr, type); }

const delta::Packet *delta::Packet::create(const delta::PacketDef *def) {
  MAKE_GC(Packet, def);
}

const delta::PacketDef *delta::PacketDef::create(
    vec<std::pair<str, const delta::Type *>> fields, str name) {
  MAKE_GC(PacketDef, fields, name);
}

const delta::Array *delta::Array::create(const Type *type, uint64_t size) {
  MAKE_GC(Array, type, size);
}

const delta::FType *delta::FType::create(vec<const Type *> params,
                                         const Type *ret, bool variadic) {
  MAKE_GC(FType, params, ret, variadic);
}

const delta::Asm *delta::Asm::create(
    const str &asm_str, const vec<std::pair<str, const Value *>> &inputs,
    const vec<std::pair<str, const Value *>> &outputs,
    const vec<str> &clobbers) {
  MAKE_GC(Asm, asm_str, inputs, outputs, clobbers);
}

const delta::Local *delta::Local::create(str name, const Type *type,
                                         const Expr *value) {
  MAKE_GC(Local, name, type, value);
}

const delta::Global *delta::Global::create(str name, const Type *type,
                                           const Expr *value, bool _volatile,
                                           bool _atomic, bool _extern) {
  MAKE_GC(Global, name, type, value, _volatile, _atomic, _extern);
}

const delta::Number *delta::Number::create(str value) {
  MAKE_GC(Number, value);
}

const delta::String *delta::String::create(str value) {
  MAKE_GC(String, value);
}

const delta::List *delta::List::create(vec<const delta::Expr *> values) {
  MAKE_GC(List, values);
}

const delta::Ident *delta::Ident::create(str name, const Type *type) {
  MAKE_GC(Ident, name, type);
}