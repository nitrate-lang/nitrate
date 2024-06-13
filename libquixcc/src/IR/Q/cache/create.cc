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

#include <map>
#include <unordered_map>

using namespace libquixcc;
using namespace ir;
using namespace ir::q;

static std::map<std::vector<const Value *>, const RootNode *> root_insts;
static std::map<std::pair<const Value *, const Value *>, const SCast *>
    scast_insts;
static std::map<std::pair<const Value *, const Value *>, const UCast *>
    ucast_insts;
static std::map<const Value *, const PtrICast *> ptricast_insts;
static std::map<std::pair<const Value *, const Value *>, const IPtrCast *>
    iptrcast_insts;
static std::map<std::pair<const Value *, const Value *>, const Bitcast *>
    bitcast_insts;
static std::map<std::tuple<const Value *, const Value *, const Value *>,
                IfElse *>
    ifelse_insts;
static std::map<std::pair<const Value *, const Value *>, const While *>
    while_insts;
static std::map<
    std::tuple<const Value *, const Value *, const Value *, const Value *>,
    const For *>
    for_insts;
static std::map<const Value *, const Loop *> loop_insts;
static Break *break_inst = nullptr;
static Continue *continue_inst = nullptr;
static std::map<const Value *, const Throw *> throw_insts;
static std::map<std::tuple<const Value *,
                           std::vector<std::pair<const Value *, const Value *>>,
                           const Value *>,
                const TryCatchFinally *>
    trycatchfinally_insts;
static std::map<std::pair<const Value *, const Value *>, const Case *>
    case_insts;
static std::map<
    std::tuple<const Value *, const std::set<const Case *>, const Value *>,
    const Switch *>
    switch_insts;
static std::map<std::pair<std::string, const Type *>, const Ident *>
    ident_insts;
static Asm *asm_inst = nullptr;
static std::map<std::vector<const Value *>, const Block *> functionblock_insts;
static std::map<
    std::tuple<std::vector<std::pair<std::string, const Type *>>, const Value *,
               const Value *, bool, bool, bool, bool, bool, bool>,
    const Segment *>
    function_insts;
static std::map<const Value *, const Ret *> ret_insts;
static std::map<std::pair<const Global *, std::vector<const Expr *>>,
                const Call *>
    call_insts;
static std::map<std::pair<const Value *, std::vector<const Expr *>>,
                const CallIndirect *>
    ptrcall_insts;
static std::map<std::pair<const Value *, const Value *>, const Add *> add_insts;
static std::map<std::pair<const Value *, const Value *>, const Sub *> sub_insts;
static std::map<std::pair<const Value *, const Value *>, const Mul *> mul_insts;
static std::map<std::pair<const Value *, const Value *>, const Div *> div_insts;
static std::map<std::pair<const Value *, const Value *>, const Mod *> mod_insts;
static std::map<std::pair<const Value *, const Value *>, const BitAnd *>
    bitand_insts;
static std::map<std::pair<const Value *, const Value *>, const BitOr *>
    bitor_insts;
static std::map<std::pair<const Value *, const Value *>, const BitXor *>
    bitxor_insts;
static std::map<const Value *, const BitNot *> bitnot_insts;
static std::map<std::pair<const Value *, const Value *>, const Shl *> shl_insts;
static std::map<std::pair<const Value *, const Value *>, const Shr *> shr_insts;
static std::map<std::pair<const Value *, const Value *>, const Rotl *>
    rotl_insts;
static std::map<std::pair<const Value *, const Value *>, const Rotr *>
    rotr_insts;
static std::map<std::pair<const Value *, const Value *>, const Eq *> eq_insts;
static std::map<std::pair<const Value *, const Value *>, const Ne *> ne_insts;
static std::map<std::pair<const Value *, const Value *>, const Lt *> lt_insts;
static std::map<std::pair<const Value *, const Value *>, const Gt *> gt_insts;
static std::map<std::pair<const Value *, const Value *>, const Le *> le_insts;
static std::map<std::pair<const Value *, const Value *>, const Ge *> ge_insts;
static std::map<std::pair<const Value *, const Value *>, const And *> and_insts;
static std::map<std::pair<const Value *, const Value *>, const Or *> or_insts;
static std::map<const Value *, const Not *> not_insts;
static std::map<std::pair<const Value *, const Value *>, const Xor *> xor_insts;
static I1 *i1_inst = nullptr;
static I8 *i8_inst = nullptr;
static I16 *i16_inst = nullptr;
static I32 *i32_inst = nullptr;
static I64 *i64_inst = nullptr;
static I128 *i128_inst = nullptr;
static U8 *u8_inst = nullptr;
static U16 *u16_inst = nullptr;
static U32 *u32_inst = nullptr;
static U64 *u64_inst = nullptr;
static U128 *u128_inst = nullptr;
static F32 *f32_inst = nullptr;
static F64 *f64_inst = nullptr;
static Void *void_inst = nullptr;
static std::map<const Value *, const Ptr *> ptr_insts;
static std::map<std::pair<const Value *, uint64_t>, const Array *> array_insts;
static std::map<const Value *, const Vector *> vector_insts;
static std::map<std::tuple<std::vector<const Type *>, const Value *, bool, bool,
                           bool, bool, bool>,
                const FType *>
    ftype_insts;
static std::map<std::pair<std::string, std::vector<const Type *>>,
                const Region *>
    region_insts;
static std::map<std::pair<std::string, std::vector<const Type *>>,
                const Group *>
    group_insts;
static std::map<std::pair<std::string, std::vector<const Type *>>,
                const Union *>
    union_insts;
static std::map<std::string, const Opaque *> opaque_insts;
static std::map<
    std::tuple<std::string, std::vector<std::pair<std::string, const Value *>>,
               std::map<std::string, const Segment *>>,
    const RegionDef *>
    regiondef_insts;
static std::map<std::tuple<std::string, std::map<std::string, const Value *>,
                           std::map<std::string, const Segment *>>,
                const GroupDef *>
    groupdef_insts;
static std::map<std::tuple<std::string, std::map<std::string, const Value *>,
                           std::map<std::string, const Segment *>>,
                const UnionDef *>
    uniondef_insts;
static std::map<std::tuple<std::string, const Value *, const Value *>,
                const Local *>
    local_insts;
static std::map<
    std::tuple<std::string, const Value *, const Value *, bool, bool, bool>,
    const Global *>
    global_insts;
static std::map<std::string, const Number *> number_insts;
static std::map<std::string, const String *> string_insts;
static std::map<std::string, const Char *> char_insts;
static std::map<std::vector<const Expr *>, const List *> list_insts;
static std::map<std::pair<const Value *, const Value *>, const Assign *>
    assign_insts;
static std::map<const Expr *, const AddressOf *> addressof_insts;
static std::map<const Expr *, const Deref *> deref_insts;
static std::map<std::tuple<const Value *, size_t, const Type *>, const Member *>
    member_insts;
static std::map<std::tuple<const Value *, const Value *, const Type *>,
                const Index *>
    index_insts;

static std::array<std::mutex, (int)q::NodeType::EnumMax> node_mutexes;

#define lock(type) std::lock_guard<std::mutex> lock(node_mutexes[(int)type])

const RootNode *q::RootNode::create(std::vector<const Value *> children) {
  lock(NodeType::Root);

  if (!root_insts.contains(children))
    root_insts[children] = new RootNode(children);

  return root_insts[children];
}

const q::SCast *q::SCast::create(const Type *type, const Expr *value) {
  lock(NodeType::SCast);
  auto key = std::make_pair(type, value);
  if (!scast_insts.contains(key)) scast_insts[key] = new SCast(type, value);
  return scast_insts[key];
}

const q::UCast *q::UCast::create(const Type *type, const Expr *value) {
  lock(NodeType::UCast);
  auto key = std::make_pair(type, value);
  if (!ucast_insts.contains(key)) ucast_insts[key] = new UCast(type, value);
  return ucast_insts[key];
}

const q::PtrICast *q::PtrICast::create(const Expr *value) {
  lock(NodeType::PtrICast);
  if (!ptricast_insts.contains(value))
    ptricast_insts[value] = new PtrICast(value);
  return ptricast_insts[value];
}

const q::IPtrCast *q::IPtrCast::create(const Type *type, const Expr *value) {
  lock(NodeType::IPtrCast);
  auto key = std::make_pair(type, value);
  if (!iptrcast_insts.contains(key))
    iptrcast_insts[key] = new IPtrCast(type, value);
  return iptrcast_insts[key];
}

const q::Bitcast *q::Bitcast::create(const Type *type, const Expr *value) {
  lock(NodeType::Bitcast);
  auto key = std::make_pair(type, value);
  if (!bitcast_insts.contains(key))
    bitcast_insts[key] = new Bitcast(type, value);
  return bitcast_insts[key];
}

const q::IfElse *q::IfElse::create(const Expr *cond, const Value *then,
                                   const Value *els) {
  lock(NodeType::IfElse);
  auto key = std::make_tuple(cond, then, els);
  if (!ifelse_insts.contains(key))
    ifelse_insts[key] = new IfElse(cond, then, els);
  return ifelse_insts[key];
}

const q::While *q::While::create(const Expr *cond, const Value *body) {
  lock(NodeType::While);
  auto key = std::make_pair(cond, body);
  if (!while_insts.contains(key)) while_insts[key] = new While(cond, body);
  return while_insts[key];
}

const ir::q::For *ir::q::For::create(const Expr *init, const Expr *cond,
                                     const Expr *step, const Value *body) {
  lock(NodeType::For);
  auto key = std::make_tuple(init, cond, step, body);
  if (!for_insts.contains(key))
    for_insts[key] = new For(init, cond, step, body);
  return for_insts[key];
}

const ir::q::Loop *ir::q::Loop::create(const Value *body) {
  lock(NodeType::Loop);
  if (!loop_insts.contains(body)) loop_insts[body] = new Loop(body);
  return loop_insts[body];
}

const ir::q::Break *ir::q::Break::create() {
  lock(NodeType::Break);
  if (break_inst == nullptr) break_inst = new Break();
  return break_inst;
}

const ir::q::Continue *ir::q::Continue::create() {
  lock(NodeType::Continue);
  if (continue_inst == nullptr) continue_inst = new Continue();
  return continue_inst;
}

const q::Ret *q::Ret::create(const Expr *value) {
  lock(NodeType::Ret);
  if (!ret_insts.contains(value)) ret_insts[value] = new Ret(value);
  return ret_insts[value];
}

const ir::q::Throw *ir::q::Throw::create(const Expr *value) {
  lock(NodeType::Throw);
  if (!throw_insts.contains(value)) throw_insts[value] = new Throw(value);
  return throw_insts[value];
}

const ir::q::TryCatchFinally *ir::q::TryCatchFinally::create(
    const Value *tryblock,
    std::vector<std::pair<const Value *, const Value *>> catchblocks,
    const Value *finallyblock) {
  lock(NodeType::TryCatchFinally);
  auto key = std::make_tuple(tryblock, catchblocks, finallyblock);
  if (!trycatchfinally_insts.contains(key))
    trycatchfinally_insts[key] =
        new TryCatchFinally(tryblock, catchblocks, finallyblock);
  return trycatchfinally_insts[key];
}

const ir::q::Case *ir::q::Case::create(const Expr *value, const Value *body) {
  lock(NodeType::Case);
  auto key = std::make_pair(value, body);
  if (!case_insts.contains(key)) case_insts[key] = new Case(value, body);
  return case_insts[key];
}

const ir::q::Switch *ir::q::Switch::create(
    const Expr *value, const std::set<const ir::q::Case *> &cases,
    const Value *defaultcase) {
  lock(NodeType::Switch);
  auto key = std::make_tuple(value, cases, defaultcase);
  if (!switch_insts.contains(key))
    switch_insts[key] = new Switch(value, cases, defaultcase);
  return switch_insts[key];
}

const ir::q::Call *ir::q::Call::create(const ir::q::Global *func,
                                       std::vector<const Expr *> args) {
  lock(NodeType::Call);
  auto key = std::make_pair(func, args);
  if (!call_insts.contains(key)) call_insts[key] = new Call(func, args);
  return call_insts[key];
}

const q::CallIndirect *q::CallIndirect::create(const Segment *callee,
                                               std::vector<const Expr *> args) {
  lock(NodeType::CallIndirect);
  auto key = std::make_pair(callee, args);
  if (!ptrcall_insts.contains(key))
    ptrcall_insts[key] = new CallIndirect(callee, args);
  return ptrcall_insts[key];
}

const ir::q::Ident *ir::q::Ident::create(std::string name, const Type *type) {
  lock(NodeType::Ident);
  auto key = std::make_pair(name, type);
  if (!ident_insts.contains(key)) ident_insts[key] = new Ident(name, type);

  return ident_insts[key];
}

const ir::q::Asm *ir::q::Asm::create() {
  lock(NodeType::Asm);
  if (asm_inst == nullptr) asm_inst = new Asm();
  return asm_inst;
}

const ir::q::Block *ir::q::Block::create(std::vector<const Value *> stmts) {
  lock(NodeType::Block);
  auto key = stmts;
  if (!functionblock_insts.contains(key))
    functionblock_insts[key] = new Block(stmts);
  return functionblock_insts[key];
}

const libquixcc::ir::q::Segment *libquixcc::ir::q::Segment::create(
    std::vector<std::pair<std::string, const Type *>> params,
    const libquixcc::ir::q::Type *return_type,
    const libquixcc::ir::q::Block *block, bool is_variadic, bool is_pure,
    bool is_thread_safe, bool is_no_throw, bool is_no_return, bool is_foriegn) {
  lock(NodeType::Segment);
  auto key =
      std::make_tuple(params, return_type, block, is_variadic, is_pure,
                      is_thread_safe, is_no_throw, is_no_return, is_foriegn);
  if (!function_insts.contains(key))
    function_insts[key] =
        new Segment(params, return_type, block, is_variadic, is_pure,
                    is_thread_safe, is_no_throw, is_no_return, is_foriegn);
  return function_insts[key];
}

const q::Add *q::Add::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Add);
  auto key = std::make_pair(lhs, rhs);
  if (!add_insts.contains(key)) add_insts[key] = new Add(lhs, rhs);
  return add_insts[key];
}

const q::Sub *q::Sub::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Sub);
  auto key = std::make_pair(lhs, rhs);
  if (!sub_insts.contains(key)) sub_insts[key] = new Sub(lhs, rhs);
  return sub_insts[key];
}

const q::Mul *q::Mul::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Mul);
  auto key = std::make_pair(lhs, rhs);
  if (!mul_insts.contains(key)) mul_insts[key] = new Mul(lhs, rhs);
  return mul_insts[key];
}

const q::Div *q::Div::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Div);
  auto key = std::make_pair(lhs, rhs);
  if (!div_insts.contains(key)) div_insts[key] = new Div(lhs, rhs);
  return div_insts[key];
}

const q::Mod *q::Mod::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Mod);
  auto key = std::make_pair(lhs, rhs);
  if (!mod_insts.contains(key)) mod_insts[key] = new Mod(lhs, rhs);
  return mod_insts[key];
}

const q::BitAnd *q::BitAnd::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::BitAnd);
  auto key = std::make_pair(lhs, rhs);
  if (!bitand_insts.contains(key)) bitand_insts[key] = new BitAnd(lhs, rhs);
  return bitand_insts[key];
}

const q::BitOr *q::BitOr::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::BitOr);
  auto key = std::make_pair(lhs, rhs);
  if (!bitor_insts.contains(key)) bitor_insts[key] = new BitOr(lhs, rhs);
  return bitor_insts[key];
}

const q::BitXor *q::BitXor::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::BitXor);
  auto key = std::make_pair(lhs, rhs);
  if (!bitxor_insts.contains(key)) bitxor_insts[key] = new BitXor(lhs, rhs);
  return bitxor_insts[key];
}

const q::BitNot *q::BitNot::create(const Expr *value) {
  lock(NodeType::BitNot);
  if (!bitnot_insts.contains(value)) bitnot_insts[value] = new BitNot(value);
  return bitnot_insts[value];
}

const q::Shl *q::Shl::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Shl);
  auto key = std::make_pair(lhs, rhs);
  if (!shl_insts.contains(key)) shl_insts[key] = new Shl(lhs, rhs);
  return shl_insts[key];
}

const q::Shr *q::Shr::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Shr);
  auto key = std::make_pair(lhs, rhs);
  if (!shr_insts.contains(key)) shr_insts[key] = new Shr(lhs, rhs);
  return shr_insts[key];
}

const q::Rotl *q::Rotl::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Rotl);
  auto key = std::make_pair(lhs, rhs);
  if (!rotl_insts.contains(key)) rotl_insts[key] = new Rotl(lhs, rhs);
  return rotl_insts[key];
}

const q::Rotr *q::Rotr::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Rotr);
  auto key = std::make_pair(lhs, rhs);
  if (!rotr_insts.contains(key)) rotr_insts[key] = new Rotr(lhs, rhs);
  return rotr_insts[key];
}

const q::Eq *q::Eq::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Eq);
  auto key = std::make_pair(lhs, rhs);
  if (!eq_insts.contains(key)) eq_insts[key] = new Eq(lhs, rhs);
  return eq_insts[key];
}

const q::Ne *q::Ne::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Ne);
  auto key = std::make_pair(lhs, rhs);
  if (!ne_insts.contains(key)) ne_insts[key] = new Ne(lhs, rhs);
  return ne_insts[key];
}

const q::Lt *q::Lt::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Lt);
  auto key = std::make_pair(lhs, rhs);
  if (!lt_insts.contains(key)) lt_insts[key] = new Lt(lhs, rhs);
  return lt_insts[key];
}

const q::Gt *q::Gt::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Gt);
  auto key = std::make_pair(lhs, rhs);
  if (!gt_insts.contains(key)) gt_insts[key] = new Gt(lhs, rhs);
  return gt_insts[key];
}

const q::Le *q::Le::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Le);
  auto key = std::make_pair(lhs, rhs);
  if (!le_insts.contains(key)) le_insts[key] = new Le(lhs, rhs);
  return le_insts[key];
}

const q::Ge *q::Ge::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Ge);
  auto key = std::make_pair(lhs, rhs);
  if (!ge_insts.contains(key)) ge_insts[key] = new Ge(lhs, rhs);
  return ge_insts[key];
}

const q::And *q::And::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::And);
  auto key = std::make_pair(lhs, rhs);
  if (!and_insts.contains(key)) and_insts[key] = new And(lhs, rhs);
  return and_insts[key];
}

const q::Or *q::Or::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Or);
  auto key = std::make_pair(lhs, rhs);
  if (!or_insts.contains(key)) or_insts[key] = new Or(lhs, rhs);
  return or_insts[key];
}

const q::Not *q::Not::create(const Expr *value) {
  lock(NodeType::Not);
  if (!not_insts.contains(value)) not_insts[value] = new Not(value);
  return not_insts[value];
}

const q::Xor *q::Xor::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Xor);
  auto key = std::make_pair(lhs, rhs);
  if (!xor_insts.contains(key)) xor_insts[key] = new Xor(lhs, rhs);
  return xor_insts[key];
}

const q::I1 *q::I1::create() {
  lock(NodeType::I1);
  if (i1_inst == nullptr) i1_inst = new I1();
  return i1_inst;
}

const q::I8 *q::I8::create() {
  lock(NodeType::I8);
  if (i8_inst == nullptr) i8_inst = new I8();
  return i8_inst;
}

const q::I16 *q::I16::create() {
  lock(NodeType::I16);
  if (i16_inst == nullptr) i16_inst = new I16();
  return i16_inst;
}

const q::I32 *q::I32::create() {
  lock(NodeType::I32);
  if (i32_inst == nullptr) i32_inst = new I32();
  return i32_inst;
}

const q::I64 *q::I64::create() {
  lock(NodeType::I64);
  if (i64_inst == nullptr) i64_inst = new I64();
  return i64_inst;
}

const q::I128 *q::I128::create() {
  lock(NodeType::I128);
  if (i128_inst == nullptr) i128_inst = new I128();
  return i128_inst;
}

const q::U8 *q::U8::create() {
  lock(NodeType::U8);
  if (u8_inst == nullptr) u8_inst = new U8();
  return u8_inst;
}

const q::U16 *q::U16::create() {
  lock(NodeType::U16);
  if (u16_inst == nullptr) u16_inst = new U16();
  return u16_inst;
}

const q::U32 *q::U32::create() {
  lock(NodeType::U32);
  if (u32_inst == nullptr) u32_inst = new U32();
  return u32_inst;
}

const q::U64 *q::U64::create() {
  lock(NodeType::U64);
  if (u64_inst == nullptr) u64_inst = new U64();
  return u64_inst;
}

const q::U128 *q::U128::create() {
  lock(NodeType::U128);
  if (u128_inst == nullptr) u128_inst = new U128();
  return u128_inst;
}

const q::F32 *q::F32::create() {
  lock(NodeType::F32);
  if (f32_inst == nullptr) f32_inst = new F32();
  return f32_inst;
}

const q::F64 *q::F64::create() {
  lock(NodeType::F64);
  if (f64_inst == nullptr) f64_inst = new F64();
  return f64_inst;
}

const q::Void *q::Void::create() {
  lock(NodeType::Void);
  if (void_inst == nullptr) void_inst = new Void();
  return void_inst;
}

const q::Ptr *q::Ptr::create(const Type *type) {
  lock(NodeType::Ptr);
  if (!ptr_insts.contains(type)) ptr_insts[type] = new Ptr(type);
  return ptr_insts[type];
}

const q::Array *q::Array::create(const Type *type, uint64_t size) {
  lock(NodeType::Array);
  auto key = std::make_pair(type, size);
  if (!array_insts.contains(key)) array_insts[key] = new Array(type, size);
  return array_insts[key];
}

const libquixcc::ir::q::Vector *libquixcc::ir::q::Vector::create(
    const Type *type) {
  lock(NodeType::Vector);
  if (!vector_insts.contains(type)) vector_insts[type] = new Vector(type);
  return vector_insts[type];
}

const libquixcc::ir::q::FType *libquixcc::ir::q::FType::create(
    std::vector<const Type *> params, const Type *ret, bool variadic, bool pure,
    bool thread_safe, bool foreign, bool _noexcept) {
  lock(NodeType::FType);
  auto key = std::make_tuple(params, ret, variadic, pure, thread_safe, foreign,
                             _noexcept);
  if (!ftype_insts.contains(key))
    ftype_insts[key] =
        new FType(params, ret, variadic, pure, thread_safe, foreign, _noexcept);
  return ftype_insts[key];
}

const libquixcc::ir::q::Region *libquixcc::ir::q::Region::create(
    std::string name, std::vector<const Type *> fields) {
  lock(NodeType::Region);
  auto key = std::make_pair(name, fields);
  if (!region_insts.contains(key)) region_insts[key] = new Region(name, fields);
  return region_insts[key];
}

const libquixcc::ir::q::Group *libquixcc::ir::q::Group::create(
    std::string name, std::vector<const Type *> fields) {
  lock(NodeType::Group);
  auto key = std::make_pair(name, fields);
  if (!group_insts.contains(key)) group_insts[key] = new Group(name, fields);
  return group_insts[key];
}

const libquixcc::ir::q::Union *libquixcc::ir::q::Union::create(
    std::string name, std::vector<const Type *> fields) {
  lock(NodeType::Union);
  auto key = std::make_pair(name, fields);
  if (!union_insts.contains(key)) union_insts[key] = new Union(name, fields);
  return union_insts[key];
}

const libquixcc::ir::q::Opaque *libquixcc::ir::q::Opaque::create(
    std::string name) {
  lock(NodeType::Opaque);
  if (!opaque_insts.contains(name)) opaque_insts[name] = new Opaque(name);
  return opaque_insts[name];
}

const libquixcc::ir::q::RegionDef *libquixcc::ir::q::RegionDef::create(
    std::string name, std::vector<std::pair<std::string, const Value *>> fields,
    std::map<std::string, const Segment *> methods) {
  lock(NodeType::RegionDef);
  auto key = std::make_tuple(name, fields, methods);
  if (!regiondef_insts.contains(key))
    regiondef_insts[key] = new RegionDef(name, fields, methods);
  return regiondef_insts[key];
}

const libquixcc::ir::q::GroupDef *libquixcc::ir::q::GroupDef::create(
    std::string name, std::map<std::string, const Value *> fields,
    std::map<std::string, const Segment *> methods) {
  lock(NodeType::GroupDef);
  auto key = std::make_tuple(name, fields, methods);
  if (!groupdef_insts.contains(key))
    groupdef_insts[key] = new GroupDef(name, fields, methods);
  return groupdef_insts[key];
}

const libquixcc::ir::q::UnionDef *libquixcc::ir::q::UnionDef::create(
    std::string name, std::map<std::string, const Value *> fields,
    std::map<std::string, const Segment *> methods) {
  lock(NodeType::UnionDef);
  auto key = std::make_tuple(name, fields, methods);
  if (!uniondef_insts.contains(key))
    uniondef_insts[key] = new UnionDef(name, fields, methods);
  return uniondef_insts[key];
}

const q::Local *q::Local::create(std::string name, const Type *type,
                                 const Expr *value) {
  lock(NodeType::Local);
  auto key = std::make_tuple(name, type, value);
  if (!local_insts.contains(key))
    local_insts[key] = new Local(name, type, value);
  return local_insts[key];
}

const q::Global *q::Global::create(std::string name, const Type *type,
                                   const Expr *value, bool _volatile,
                                   bool _atomic, bool _extern) {
  lock(NodeType::Global);
  auto key = std::make_tuple(name, type, value, _volatile, _atomic, _extern);
  if (!global_insts.contains(key))
    global_insts[key] =
        new Global(name, type, value, _volatile, _atomic, _extern);

  return global_insts[key];
}

const q::Number *q::Number::create(std::string value) {
  lock(NodeType::Number);
  if (!number_insts.contains(value)) number_insts[value] = new Number(value);
  return number_insts[value];
}

const q::String *q::String::create(std::string value) {
  lock(NodeType::String);
  if (!string_insts.contains(value)) string_insts[value] = new String(value);
  return string_insts[value];
}

const libquixcc::ir::q::Char *libquixcc::ir::q::Char::create(
    std::string value) {
  lock(NodeType::Char);
  if (!char_insts.contains(value)) char_insts[value] = new Char(value);
  return char_insts[value];
}

const libquixcc::ir::q::List *libquixcc::ir::q::List::create(
    std::vector<const libquixcc::ir::q::Expr *> values) {
  lock(NodeType::List);
  if (!list_insts.contains(values)) list_insts[values] = new List(values);
  return list_insts[values];
}

const libquixcc::ir::q::Assign *libquixcc::ir::q::Assign::create(
    const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Assign);
  auto key = std::make_pair(lhs, rhs);
  if (!assign_insts.contains(key)) assign_insts[key] = new Assign(lhs, rhs);
  return assign_insts[key];
}

const libquixcc::ir::q::AddressOf *libquixcc::ir::q::AddressOf::create(
    const libquixcc::ir::q::Expr *lhs) {
  lock(NodeType::AddressOf);
  if (!addressof_insts.contains(lhs)) addressof_insts[lhs] = new AddressOf(lhs);
  return addressof_insts[lhs];
}

const libquixcc::ir::q::Deref *libquixcc::ir::q::Deref::create(
    const libquixcc::ir::q::Expr *lhs) {
  lock(NodeType::Deref);
  if (!deref_insts.contains(lhs)) deref_insts[lhs] = new Deref(lhs);
  return deref_insts[lhs];
}

const libquixcc::ir::q::Member *libquixcc::ir::q::Member::create(
    const libquixcc::ir::q::Value *lhs, size_t field, const Type *field_type) {
  lock(NodeType::Member);
  auto key = std::make_tuple(lhs, field, field_type);
  if (!member_insts.contains(key))
    member_insts[key] = new Member(lhs, field, field_type);
  return member_insts[key];
}

const libquixcc::ir::q::Index *libquixcc::ir::q::Index::create(
    const libquixcc::ir::q::Value *lhs, const libquixcc::ir::q::Expr *index,
    const libquixcc::ir::q::Type *type) {
  lock(NodeType::Index);
  auto key = std::make_tuple(lhs, index, type);
  if (!index_insts.contains(key))
    index_insts[key] = new Index(lhs, index, type);
  return index_insts[key];
}