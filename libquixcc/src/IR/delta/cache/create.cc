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
using namespace delta;

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
static std::map<std::string, const Jmp *> jmp_insts;
static std::map<std::pair<std::string, const Value *>, const Label *>
    label_insts;
static std::map<const Value *, const Ret *> ret_insts;
static std::map<
    std::tuple<std::string, std::vector<const Expr *>, const FType *>,
    const Call *>
    call_insts;
static std::map<std::pair<const Value *, std::vector<const Expr *>>,
                const PtrCall *>
    ptrcall_insts;
static Halt *halt_inst = nullptr;
static Break *break_inst = nullptr;
static Continue *continue_inst = nullptr;
static std::map<std::pair<const Value *, const Value *>, const Case *>
    case_insts;
static std::map<
    std::tuple<const Value *, std::vector<const Case *>, const Value *>,
    const Switch *>
    switch_insts;
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
static std::map<const Expr *, const BitNot *> bitnot_insts;
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
static std::map<std::tuple<const Value *, const Value *, uint64_t>,
                const Assign *>
    assign_insts;
static std::map<const Expr *, const AddressOf *> addressof_insts;
static std::map<const Expr *, const Deref *> deref_insts;
static std::map<std::tuple<const Value *, size_t, const Type *>, const Member *>
    member_insts;
static std::map<std::tuple<const Value *, const Value *, const Type *>,
                const Index *>
    index_insts;
static std::map<std::vector<const Value *>, const Block *> block_insts;
static std::map<
    std::tuple<const Type *, bool,
               const std::vector<std::pair<std::string, const Type *>>,
               const Block *>,
    const Segment *>
    segment_insts;
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
static std::map<const Type *, const Ptr *> ptr_insts;
static std::map<const PacketDef *, const Packet *> packet_insts;
static std::map<
    std::pair<std::vector<std::pair<std::string, const Type *>>, std::string>,
    const PacketDef *>
    packetdef_insts;
static std::map<std::pair<const Type *, uint64_t>, const Array *> array_insts;
static std::map<std::tuple<std::vector<const Type *>, const Type *, bool>,
                const FType *>
    ftype_insts;
static std::map<std::tuple<std::string, const Value *, const Value *>,
                const Local *>
    local_insts;
static std::map<
    std::tuple<std::string, const Value *, const Value *, bool, bool, bool>,
    const Global *>
    global_insts;
static std::map<std::string, const Number *> number_insts;
static std::map<std::string, const String *> string_insts;
static std::map<std::vector<const Expr *>, const List *> list_insts;
static std::map<std::pair<std::string, const Type *>, const Ident *>
    ident_insts;

static std::array<std::mutex, (int)delta::NodeType::EnumMax> node_mutexes;

#define lock(type) std::lock_guard<std::mutex> lock(node_mutexes[(int)type])

const RootNode *delta::RootNode::create(std::vector<const Value *> children) {
  lock(NodeType::Root);
  if (!root_insts.contains(children))
    root_insts[children] = new RootNode(children);
  return root_insts[children];
}

const delta::SCast *delta::SCast::create(const Type *type, const Expr *value) {
  lock(NodeType::SCast);
  auto key = std::make_pair(type, value);
  if (!scast_insts.contains(key)) scast_insts[key] = new SCast(type, value);
  return scast_insts[key];
}

const delta::UCast *delta::UCast::create(const Type *type, const Expr *value) {
  lock(NodeType::UCast);
  auto key = std::make_pair(type, value);
  if (!ucast_insts.contains(key)) ucast_insts[key] = new UCast(type, value);
  return ucast_insts[key];
}

const delta::PtrICast *delta::PtrICast::create(const Expr *value) {
  lock(NodeType::PtrICast);
  if (!ptricast_insts.contains(value))
    ptricast_insts[value] = new PtrICast(value);
  return ptricast_insts[value];
}

const delta::IPtrCast *delta::IPtrCast::create(const Type *type,
                                               const Expr *value) {
  lock(NodeType::IPtrCast);
  auto key = std::make_pair(type, value);
  if (!iptrcast_insts.contains(key))
    iptrcast_insts[key] = new IPtrCast(type, value);
  return iptrcast_insts[key];
}

const delta::Bitcast *delta::Bitcast::create(const Type *type,
                                             const Expr *value) {
  lock(NodeType::Bitcast);
  auto key = std::make_pair(type, value);
  if (!bitcast_insts.contains(key))
    bitcast_insts[key] = new Bitcast(type, value);
  return bitcast_insts[key];
}

const delta::IfElse *delta::IfElse::create(const Expr *cond, const Value *then,
                                           const Value *els) {
  lock(NodeType::IfElse);
  auto key = std::make_tuple(cond, then, els);
  if (!ifelse_insts.contains(key))
    ifelse_insts[key] = new IfElse(cond, then, els);
  return ifelse_insts[key];
}

const delta::While *delta::While::create(const Expr *cond, const Value *body) {
  lock(NodeType::While);
  auto key = std::make_pair(cond, body);
  if (!while_insts.contains(key)) while_insts[key] = new While(cond, body);
  return while_insts[key];
}

const delta::Jmp *delta::Jmp::create(std::string target) {
  lock(NodeType::Jmp);
  if (!jmp_insts.contains(target)) jmp_insts[target] = new Jmp(target);
  return jmp_insts[target];
}

const delta::Label *delta::Label::create(std::string name, const Value *code) {
  lock(NodeType::Label);
  auto key = std::make_pair(name, code);
  if (!label_insts.contains(key)) label_insts[key] = new Label(name, code);
  return label_insts[key];
}

const delta::Ret *delta::Ret::create(const Expr *value) {
  lock(NodeType::Ret);
  if (!ret_insts.contains(value)) ret_insts[value] = new Ret(value);
  return ret_insts[value];
}

const delta::Call *delta::Call::create(std::string callee,
                                       std::vector<const Expr *> args,
                                       const FType *ftype) {
  lock(NodeType::Call);
  auto key = std::make_tuple(callee, args, ftype);
  if (!call_insts.contains(key))
    call_insts[key] = new Call(callee, args, ftype);
  return call_insts[key];
}

const delta::PtrCall *delta::PtrCall::create(const Value *callee,
                                             std::vector<const Expr *> args) {
  lock(NodeType::PtrCall);
  auto key = std::make_pair(callee, args);
  if (!ptrcall_insts.contains(key))
    ptrcall_insts[key] = new PtrCall(callee, args);
  return ptrcall_insts[key];
}

const delta::Halt *delta::Halt::create() {
  lock(NodeType::Halt);
  if (halt_inst == nullptr) halt_inst = new Halt();
  return halt_inst;
}

const delta::Break *delta::Break::create() {
  lock(NodeType::Break);
  if (break_inst == nullptr) break_inst = new Break();
  return break_inst;
}

const delta::Continue *delta::Continue::create() {
  lock(NodeType::Continue);
  if (continue_inst == nullptr) continue_inst = new Continue();
  return continue_inst;
}

const delta::Case *delta::Case::create(const delta::Expr *value,
                                       const delta::Value *code) {
  lock(NodeType::Case);
  auto key = std::make_pair(value, code);
  if (!case_insts.contains(key)) case_insts[key] = new Case(value, code);
  return case_insts[key];
}
const delta::Switch *delta::Switch::create(
    const delta::Expr *cond, std::vector<const delta::Case *> cases,
    const delta::Value *def) {
  lock(NodeType::Switch);
  auto key = std::make_tuple(cond, cases, def);
  if (!switch_insts.contains(key))
    switch_insts[key] = new Switch(cond, cases, def);
  return switch_insts[key];
}

const delta::Add *delta::Add::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Add);
  auto key = std::make_pair(lhs, rhs);
  if (!add_insts.contains(key)) add_insts[key] = new Add(lhs, rhs);
  return add_insts[key];
}

const delta::Sub *delta::Sub::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Sub);
  auto key = std::make_pair(lhs, rhs);
  if (!sub_insts.contains(key)) sub_insts[key] = new Sub(lhs, rhs);
  return sub_insts[key];
}

const delta::Mul *delta::Mul::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Mul);
  auto key = std::make_pair(lhs, rhs);
  if (!mul_insts.contains(key)) mul_insts[key] = new Mul(lhs, rhs);
  return mul_insts[key];
}

const delta::Div *delta::Div::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Div);
  auto key = std::make_pair(lhs, rhs);
  if (!div_insts.contains(key)) div_insts[key] = new Div(lhs, rhs);
  return div_insts[key];
}

const delta::Mod *delta::Mod::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Mod);
  auto key = std::make_pair(lhs, rhs);
  if (!mod_insts.contains(key)) mod_insts[key] = new Mod(lhs, rhs);
  return mod_insts[key];
}

const delta::BitAnd *delta::BitAnd::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::BitAnd);
  auto key = std::make_pair(lhs, rhs);
  if (!bitand_insts.contains(key)) bitand_insts[key] = new BitAnd(lhs, rhs);
  return bitand_insts[key];
}

const delta::BitOr *delta::BitOr::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::BitOr);
  auto key = std::make_pair(lhs, rhs);
  if (!bitor_insts.contains(key)) bitor_insts[key] = new BitOr(lhs, rhs);
  return bitor_insts[key];
}

const delta::BitXor *delta::BitXor::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::BitXor);
  auto key = std::make_pair(lhs, rhs);
  if (!bitxor_insts.contains(key)) bitxor_insts[key] = new BitXor(lhs, rhs);
  return bitxor_insts[key];
}

const delta::BitNot *delta::BitNot::create(const Expr *value) {
  lock(NodeType::BitNot);
  if (!bitnot_insts.contains(value)) bitnot_insts[value] = new BitNot(value);
  return bitnot_insts[value];
}

const delta::Shl *delta::Shl::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Shl);
  auto key = std::make_pair(lhs, rhs);
  if (!shl_insts.contains(key)) shl_insts[key] = new Shl(lhs, rhs);
  return shl_insts[key];
}

const delta::Shr *delta::Shr::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Shr);
  auto key = std::make_pair(lhs, rhs);
  if (!shr_insts.contains(key)) shr_insts[key] = new Shr(lhs, rhs);
  return shr_insts[key];
}

const delta::Rotl *delta::Rotl::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Rotl);
  auto key = std::make_pair(lhs, rhs);
  if (!rotl_insts.contains(key)) rotl_insts[key] = new Rotl(lhs, rhs);
  return rotl_insts[key];
}

const delta::Rotr *delta::Rotr::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Rotr);
  auto key = std::make_pair(lhs, rhs);
  if (!rotr_insts.contains(key)) rotr_insts[key] = new Rotr(lhs, rhs);
  return rotr_insts[key];
}

const delta::Eq *delta::Eq::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Eq);
  auto key = std::make_pair(lhs, rhs);
  if (!eq_insts.contains(key)) eq_insts[key] = new Eq(lhs, rhs);
  return eq_insts[key];
}

const delta::Ne *delta::Ne::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Ne);
  auto key = std::make_pair(lhs, rhs);
  if (!ne_insts.contains(key)) ne_insts[key] = new Ne(lhs, rhs);
  return ne_insts[key];
}

const delta::Lt *delta::Lt::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Lt);
  auto key = std::make_pair(lhs, rhs);
  if (!lt_insts.contains(key)) lt_insts[key] = new Lt(lhs, rhs);
  return lt_insts[key];
}

const delta::Gt *delta::Gt::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Gt);
  auto key = std::make_pair(lhs, rhs);
  if (!gt_insts.contains(key)) gt_insts[key] = new Gt(lhs, rhs);
  return gt_insts[key];
}

const delta::Le *delta::Le::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Le);
  auto key = std::make_pair(lhs, rhs);
  if (!le_insts.contains(key)) le_insts[key] = new Le(lhs, rhs);
  return le_insts[key];
}

const delta::Ge *delta::Ge::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Ge);
  auto key = std::make_pair(lhs, rhs);
  if (!ge_insts.contains(key)) ge_insts[key] = new Ge(lhs, rhs);
  return ge_insts[key];
}

const delta::And *delta::And::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::And);
  auto key = std::make_pair(lhs, rhs);
  if (!and_insts.contains(key)) and_insts[key] = new And(lhs, rhs);
  return and_insts[key];
}

const delta::Or *delta::Or::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Or);
  auto key = std::make_pair(lhs, rhs);
  if (!or_insts.contains(key)) or_insts[key] = new Or(lhs, rhs);
  return or_insts[key];
}

const delta::Not *delta::Not::create(const Value *value) {
  lock(NodeType::Not);
  if (!not_insts.contains(value)) not_insts[value] = new Not(value);
  return not_insts[value];
}

const delta::Xor *delta::Xor::create(const Expr *lhs, const Expr *rhs) {
  lock(NodeType::Xor);
  auto key = std::make_pair(lhs, rhs);
  if (!xor_insts.contains(key)) xor_insts[key] = new Xor(lhs, rhs);
  return xor_insts[key];
}

const delta::Assign *delta::Assign::create(const Expr *var, const Expr *value,
                                           uint64_t rank) {
  lock(NodeType::Assign);
  auto key = std::make_tuple(var, value, rank);
  if (!assign_insts.contains(key))
    assign_insts[key] = new Assign(var, value, rank);
  return assign_insts[key];
}

const delta::AddressOf *delta::AddressOf::create(const delta::Expr *lhs) {
  lock(NodeType::AddressOf);
  if (!addressof_insts.contains(lhs)) addressof_insts[lhs] = new AddressOf(lhs);
  return addressof_insts[lhs];
}

const delta::Deref *delta::Deref::create(const delta::Expr *lhs) {
  lock(NodeType::Deref);
  if (!deref_insts.contains(lhs)) deref_insts[lhs] = new Deref(lhs);
  return deref_insts[lhs];
}

const delta::Member *delta::Member::create(const delta::Expr *lhs, size_t field,
                                           const delta::Type *field_type) {
  lock(NodeType::Member);
  auto key = std::make_tuple(lhs, field, field_type);
  if (!member_insts.contains(key))
    member_insts[key] = new Member(lhs, field, field_type);
  return member_insts[key];
}

const delta::Index *delta::Index::create(const Expr *var, const Expr *index,
                                         const Type *type) {
  lock(NodeType::Index);
  auto key = std::make_tuple(var, index, type);
  if (!index_insts.contains(key))
    index_insts[key] = new Index(var, index, type);
  return index_insts[key];
}

const delta::Block *delta::Block::create(std::vector<const Value *> stmts) {
  lock(NodeType::Block);
  if (!block_insts.contains(stmts)) block_insts[stmts] = new Block(stmts);
  return block_insts[stmts];
}

const delta::Segment *delta::Segment::create(
    const Type *ret, bool variadic,
    const std::vector<std::pair<std::string, const Type *>> &params,
    const Block *stmts) {
  lock(NodeType::Segment);
  auto key = std::make_tuple(ret, variadic, params, stmts);
  if (!segment_insts.contains(key))
    segment_insts[key] = new Segment(ret, variadic, params, stmts);
  return segment_insts[key];
}

const delta::I1 *delta::I1::create() {
  lock(NodeType::I1);
  if (i1_inst == nullptr) i1_inst = new I1();
  return i1_inst;
}

const delta::I8 *delta::I8::create() {
  lock(NodeType::I8);
  if (i8_inst == nullptr) i8_inst = new I8();
  return i8_inst;
}

const delta::I16 *delta::I16::create() {
  lock(NodeType::I16);
  if (i16_inst == nullptr) i16_inst = new I16();
  return i16_inst;
}

const delta::I32 *delta::I32::create() {
  lock(NodeType::I32);
  if (i32_inst == nullptr) i32_inst = new I32();
  return i32_inst;
}

const delta::I64 *delta::I64::create() {
  lock(NodeType::I64);
  if (i64_inst == nullptr) i64_inst = new I64();
  return i64_inst;
}

const delta::I128 *delta::I128::create() {
  lock(NodeType::I128);
  if (i128_inst == nullptr) i128_inst = new I128();
  return i128_inst;
}

const delta::U8 *delta::U8::create() {
  lock(NodeType::U8);
  if (u8_inst == nullptr) u8_inst = new U8();
  return u8_inst;
}

const delta::U16 *delta::U16::create() {
  lock(NodeType::U16);
  if (u16_inst == nullptr) u16_inst = new U16();
  return u16_inst;
}

const delta::U32 *delta::U32::create() {
  lock(NodeType::U32);
  if (u32_inst == nullptr) u32_inst = new U32();
  return u32_inst;
}

const delta::U64 *delta::U64::create() {
  lock(NodeType::U64);
  if (u64_inst == nullptr) u64_inst = new U64();
  return u64_inst;
}

const delta::U128 *delta::U128::create() {
  lock(NodeType::U128);
  if (u128_inst == nullptr) u128_inst = new U128();
  return u128_inst;
}

const delta::F32 *delta::F32::create() {
  lock(NodeType::F32);
  if (f32_inst == nullptr) f32_inst = new F32();
  return f32_inst;
}

const delta::F64 *delta::F64::create() {
  lock(NodeType::F64);
  if (f64_inst == nullptr) f64_inst = new F64();
  return f64_inst;
}

const delta::Void *delta::Void::create() {
  lock(NodeType::Void);
  if (void_inst == nullptr) void_inst = new Void();
  return void_inst;
}

const delta::Ptr *delta::Ptr::create(const Type *type) {
  lock(NodeType::Ptr);
  if (!ptr_insts.contains(type)) ptr_insts[type] = new Ptr(type);
  return ptr_insts[type];
}

const delta::Packet *delta::Packet::create(const delta::PacketDef *def) {
  lock(NodeType::Packet);
  if (!packet_insts.contains(def)) packet_insts[def] = new Packet(def);
  return packet_insts[def];
}

const delta::PacketDef *delta::PacketDef::create(
    std::vector<std::pair<std::string, const delta::Type *>> fields,
    std::string name) {
  lock(NodeType::PacketDef);
  auto key = std::make_pair(fields, name);
  if (!packetdef_insts.contains(key))
    packetdef_insts[key] = new PacketDef(fields, name);
  return packetdef_insts[key];
}

const delta::Array *delta::Array::create(const Type *type, uint64_t size) {
  lock(NodeType::Array);
  auto key = std::make_pair(type, size);
  if (!array_insts.contains(key)) array_insts[key] = new Array(type, size);
  return array_insts[key];
}

const delta::FType *delta::FType::create(std::vector<const Type *> params,
                                         const Type *ret, bool variadic) {
  lock(NodeType::FType);
  auto key = std::make_tuple(params, ret, variadic);
  if (!ftype_insts.contains(key))
    ftype_insts[key] = new FType(params, ret, variadic);
  return ftype_insts[key];
}

const delta::Local *delta::Local::create(std::string name, const Type *type,
                                         const Expr *value) {
  lock(NodeType::Local);
  auto key = std::make_tuple(name, type, value);
  if (!local_insts.contains(key))
    local_insts[key] = new Local(name, type, value);
  return local_insts[key];
}

const delta::Global *delta::Global::create(std::string name, const Type *type,
                                           const Expr *value, bool _volatile,
                                           bool _atomic, bool _extern) {
  lock(NodeType::Global);
  auto key = std::make_tuple(name, type, value, _volatile, _atomic, _extern);
  if (!global_insts.contains(key))
    global_insts[key] =
        new Global(name, type, value, _volatile, _atomic, _extern);
  return global_insts[key];
}

const delta::Number *delta::Number::create(std::string value) {
  lock(NodeType::Number);
  if (!number_insts.contains(value)) number_insts[value] = new Number(value);
  return number_insts[value];
}

const delta::String *delta::String::create(std::string value) {
  lock(NodeType::String);
  if (!string_insts.contains(value)) string_insts[value] = new String(value);
  return string_insts[value];
}

const delta::List *delta::List::create(
    std::vector<const delta::Expr *> values) {
  lock(NodeType::List);
  if (!list_insts.contains(values)) list_insts[values] = new List(values);
  return list_insts[values];
}

const delta::Ident *delta::Ident::create(std::string name, const Type *type) {
  lock(NodeType::Ident);
  auto key = std::make_pair(name, type);
  if (!ident_insts.contains(key)) ident_insts[key] = new Ident(name, type);
  return ident_insts[key];
}