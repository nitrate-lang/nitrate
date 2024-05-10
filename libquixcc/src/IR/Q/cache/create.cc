////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#include <IR/Q/QIR.h>

#include <IR/Q/Asm.h>
#include <IR/Q/Call.h>
#include <IR/Q/Cast.h>
#include <IR/Q/Control.h>
#include <IR/Q/Function.h>
#include <IR/Q/Ident.h>
#include <IR/Q/Math.h>
#include <IR/Q/OO.h>
#include <IR/Q/Type.h>
#include <IR/Q/Variable.h>

#include <map>

using namespace libquixcc;
using namespace libquixcc::ir;
using namespace libquixcc::ir::q;

static RootNode *root = nullptr;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const SCast *> scast_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const UCast *> ucast_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const PtrICast *> ptricast_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const IPtrCast *> iptrcast_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Bitcast *> bitcast_insts;
static std::map<std::tuple<const Value<Q> *, const Value<Q> *, const Value<Q> *>, IfElse *> ifelse_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const While *> while_insts;
static std::map<std::tuple<const Value<Q> *, const Value<Q> *, const Value<Q> *, const Value<Q> *>, const For *> for_insts;
static std::map<const Value<Q> *, const Loop *> loop_insts;
static Break *break_inst = nullptr;
static Continue *continue_inst = nullptr;
static std::map<const Value<Q> *, const Throw *> throw_insts;
static std::map<std::tuple<const Value<Q> *, const Value<Q> *, const Value<Q> *>, const TryCatchFinally *> trycatchfinally_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Case *> case_insts;
static std::map<std::tuple<const Value<Q> *, const std::set<const Case *>, const Value<Q> *>, const Switch *> switch_insts;
static std::map<std::string, const Ident *> ident_insts;
static Asm *asm_inst = nullptr;
static std::map<std::vector<const Value<Q> *>, const FunctionBlock *> functionblock_insts;
static std::map<std::tuple<std::string, std::vector<const Value<Q> *>, const Value<Q> *, const Value<Q> *, std::set<FConstraint>>, const Function *> function_insts;
static std::map<const Value<Q> *, const Ret *> ret_insts;
static std::map<std::pair<const Function *, std::vector<const Value<Q> *>>, const Call *> call_insts;
static std::map<std::pair<const Value<Q> *, std::vector<const Value<Q> *>>, const CallIndirect *> ptrcall_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Add *> add_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Sub *> sub_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Mul *> mul_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Div *> div_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Mod *> mod_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const BitAnd *> bitand_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const BitOr *> bitor_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const BitXor *> bitxor_insts;
static std::map<const Value<Q> *, const BitNot *> bitnot_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Shl *> shl_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Shr *> shr_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Rotl *> rotl_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Rotr *> rotr_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Eq *> eq_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Ne *> ne_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Lt *> lt_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Gt *> gt_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Le *> le_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Ge *> ge_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const And *> and_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Or *> or_insts;
static std::map<const Value<Q> *, const Not *> not_insts;
static std::map<std::pair<const Value<Q> *, const Value<Q> *>, const Xor *> xor_insts;
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
static std::map<const Value<Q> *, const Ptr *> ptr_insts;
static std::map<std::pair<const Value<Q> *, uint64_t>, const Array *> array_insts;
static std::map<std::pair<std::vector<const Value<Q> *>, const Value<Q> *>, const FType *> ftype_insts;
static std::map<std::pair<std::string, const Value<Q> *>, const Local *> local_insts;
static std::map<std::tuple<std::string, const Value<Q> *, const Value<Q> *, bool, bool>, const Global *> global_insts;
static std::map<std::string, const Number *> number_insts;
static std::map<std::string, const String *> string_insts;

static std::map<q::NodeType, std::mutex> node_mutexes;

#define lock(type) std::lock_guard<std::mutex> lock(node_mutexes[type])

const RootNode *q::RootNode::create(std::vector<const Value<Q> *> children)
{
    lock(NodeType::Root);
    if (root == nullptr)
        root = new RootNode(children);
    return root;
}

const q::SCast *q::SCast::create(const Value<Q> *type, const Value<Q> *value)
{
    lock(NodeType::SCast);
    auto key = std::make_pair(type, value);
    if (!scast_insts.contains(key))
        scast_insts[key] = new SCast(type, value);
    return scast_insts[key];
}

const q::UCast *q::UCast::create(const Value<Q> *type, const Value<Q> *value)
{
    lock(NodeType::UCast);
    auto key = std::make_pair(type, value);
    if (!ucast_insts.contains(key))
        ucast_insts[key] = new UCast(type, value);
    return ucast_insts[key];
}

const q::PtrICast *q::PtrICast::create(const Value<Q> *type, const Value<Q> *value)
{
    lock(NodeType::PtrICast);
    auto key = std::make_pair(type, value);
    if (!ptricast_insts.contains(key))
        ptricast_insts[key] = new PtrICast(type, value);
    return ptricast_insts[key];
}

const q::IPtrCast *q::IPtrCast::create(const Value<Q> *type, const Value<Q> *value)
{
    lock(NodeType::IPtrCast);
    auto key = std::make_pair(type, value);
    if (!iptrcast_insts.contains(key))
        iptrcast_insts[key] = new IPtrCast(type, value);
    return iptrcast_insts[key];
}

const q::Bitcast *q::Bitcast::create(const Value<Q> *type, const Value<Q> *value)
{
    lock(NodeType::Bitcast);
    auto key = std::make_pair(type, value);
    if (!bitcast_insts.contains(key))
        bitcast_insts[key] = new Bitcast(type, value);
    return bitcast_insts[key];
}

const q::IfElse *q::IfElse::create(const Value<Q> *cond, const Value<Q> *then, const Value<Q> *els)
{
    lock(NodeType::IfElse);
    auto key = std::make_tuple(cond, then, els);
    if (!ifelse_insts.contains(key))
        ifelse_insts[key] = new IfElse(cond, then, els);
    return ifelse_insts[key];
}

const q::While *q::While::create(const Value<Q> *cond, const Value<Q> *body)
{
    lock(NodeType::While);
    auto key = std::make_pair(cond, body);
    if (!while_insts.contains(key))
        while_insts[key] = new While(cond, body);
    return while_insts[key];
}

const libquixcc::ir::q::For *libquixcc::ir::q::For::create(const libquixcc::ir::Value<Q> *init, const libquixcc::ir::Value<Q> *cond, const libquixcc::ir::Value<Q> *step, const libquixcc::ir::Value<Q> *body)
{
    lock(NodeType::For);
    auto key = std::make_tuple(init, cond, step, body);
    if (!for_insts.contains(key))
        for_insts[key] = new For(init, cond, step, body);
    return for_insts[key];
}

const libquixcc::ir::q::Loop *libquixcc::ir::q::Loop::create(const libquixcc::ir::Value<libquixcc::ir::Q> *body)
{
    lock(NodeType::Loop);
    if (!loop_insts.contains(body))
        loop_insts[body] = new Loop(body);
    return loop_insts[body];
}

const libquixcc::ir::q::Break *libquixcc::ir::q::Break::create()
{
    lock(NodeType::Break);
    if (break_inst == nullptr)
        break_inst = new Break();
    return break_inst;
}

const libquixcc::ir::q::Continue *libquixcc::ir::q::Continue::create()
{
    lock(NodeType::Continue);
    if (continue_inst == nullptr)
        continue_inst = new Continue();
    return continue_inst;
}

const q::Ret *q::Ret::create(const Value<Q> *value)
{
    lock(NodeType::Ret);
    if (!ret_insts.contains(value))
        ret_insts[value] = new Ret(value);
    return ret_insts[value];
}

const libquixcc::ir::q::Throw *libquixcc::ir::q::Throw::create(const libquixcc::ir::Value<libquixcc::ir::Q> *value)
{
    lock(NodeType::Throw);
    if (!throw_insts.contains(value))
        throw_insts[value] = new Throw(value);
    return throw_insts[value];
}

const libquixcc::ir::q::TryCatchFinally *libquixcc::ir::q::TryCatchFinally::create(const libquixcc::ir::Value<Q> *tryblock, const libquixcc::ir::Value<Q> *catchblock, const libquixcc::ir::Value<Q> *finallyblock)
{
    lock(NodeType::TryCatchFinally);
    auto key = std::make_tuple(tryblock, catchblock, finallyblock);
    if (!trycatchfinally_insts.contains(key))
        trycatchfinally_insts[key] = new TryCatchFinally(tryblock, catchblock, finallyblock);
    return trycatchfinally_insts[key];
}

const libquixcc::ir::q::Case *libquixcc::ir::q::Case::create(const libquixcc::ir::Value<libquixcc::ir::Q> *value, const libquixcc::ir::Value<libquixcc::ir::Q> *body)
{
    lock(NodeType::Case);
    auto key = std::make_pair(value, body);
    if (!case_insts.contains(key))
        case_insts[key] = new Case(value, body);
    return case_insts[key];
}

const libquixcc::ir::q::Switch *libquixcc::ir::q::Switch::create(const libquixcc::ir::Value<libquixcc::ir::Q> *value, const std::set<const libquixcc::ir::q::Case *> &cases, const libquixcc::ir::Value<libquixcc::ir::Q> *defaultcase)
{
    lock(NodeType::Switch);
    auto key = std::make_tuple(value, cases, defaultcase);
    if (!switch_insts.contains(key))
        switch_insts[key] = new Switch(value, cases, defaultcase);
    return switch_insts[key];
}

const libquixcc::ir::q::Call *libquixcc::ir::q::Call::create(const libquixcc::ir::q::Function *func, std::vector<const libquixcc::ir::Value<libquixcc::ir::Q> *> args)
{
    lock(NodeType::Call);
    auto key = std::make_pair(func, args);
    if (!call_insts.contains(key))
        call_insts[key] = new Call(func, args);
    return call_insts[key];
}

const q::CallIndirect *q::CallIndirect::create(const Value<Q> *callee, std::vector<const Value<Q> *> args)
{
    lock(NodeType::CallIndirect);
    auto key = std::make_pair(callee, args);
    if (!ptrcall_insts.contains(key))
        ptrcall_insts[key] = new CallIndirect(callee, args);
    return ptrcall_insts[key];
}

const libquixcc::ir::q::Ident *libquixcc::ir::q::Ident::create(std::string name)
{
    lock(NodeType::Ident);
    if (!ident_insts.contains(name))
        ident_insts[name] = new Ident(name);
    return ident_insts[name];
}

const libquixcc::ir::q::Asm *libquixcc::ir::q::Asm::create()
{
    lock(NodeType::Asm);
    if (asm_inst == nullptr)
        asm_inst = new Asm();
    return asm_inst;
}

const libquixcc::ir::q::FunctionBlock *libquixcc::ir::q::FunctionBlock::create(std::vector<const libquixcc::ir::Value<libquixcc::ir::Q> *> stmts)
{
    lock(NodeType::FunctionBlock);
    auto key = stmts;
    if (!functionblock_insts.contains(key))
        functionblock_insts[key] = new FunctionBlock(stmts);
    return functionblock_insts[key];
}

const libquixcc::ir::q::Function *libquixcc::ir::q::Function::create(std::string name, std::vector<const Value<Q> *> params, const libquixcc::ir::Value<Q> *return_type, const libquixcc::ir::Value<Q> *block, std::set<libquixcc::ir::q::FConstraint> constraints)
{
    lock(NodeType::Function);
    auto key = std::make_tuple(name, params, return_type, block, constraints);
    if (!function_insts.contains(key))
        function_insts[key] = new Function(name, params, return_type, block, constraints);
    return function_insts[key];
}

const q::Add *q::Add::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Add);
    auto key = std::make_pair(lhs, rhs);
    if (!add_insts.contains(key))
        add_insts[key] = new Add(lhs, rhs);
    return add_insts[key];
}

const q::Sub *q::Sub::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Sub);
    auto key = std::make_pair(lhs, rhs);
    if (!sub_insts.contains(key))
        sub_insts[key] = new Sub(lhs, rhs);
    return sub_insts[key];
}

const q::Mul *q::Mul::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Mul);
    auto key = std::make_pair(lhs, rhs);
    if (!mul_insts.contains(key))
        mul_insts[key] = new Mul(lhs, rhs);
    return mul_insts[key];
}

const q::Div *q::Div::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Div);
    auto key = std::make_pair(lhs, rhs);
    if (!div_insts.contains(key))
        div_insts[key] = new Div(lhs, rhs);
    return div_insts[key];
}

const q::Mod *q::Mod::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Mod);
    auto key = std::make_pair(lhs, rhs);
    if (!mod_insts.contains(key))
        mod_insts[key] = new Mod(lhs, rhs);
    return mod_insts[key];
}

const q::BitAnd *q::BitAnd::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::BitAnd);
    auto key = std::make_pair(lhs, rhs);
    if (!bitand_insts.contains(key))
        bitand_insts[key] = new BitAnd(lhs, rhs);
    return bitand_insts[key];
}

const q::BitOr *q::BitOr::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::BitOr);
    auto key = std::make_pair(lhs, rhs);
    if (!bitor_insts.contains(key))
        bitor_insts[key] = new BitOr(lhs, rhs);
    return bitor_insts[key];
}

const q::BitXor *q::BitXor::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::BitXor);
    auto key = std::make_pair(lhs, rhs);
    if (!bitxor_insts.contains(key))
        bitxor_insts[key] = new BitXor(lhs, rhs);
    return bitxor_insts[key];
}

const q::BitNot *q::BitNot::create(const Value<Q> *value)
{
    lock(NodeType::BitNot);
    if (!bitnot_insts.contains(value))
        bitnot_insts[value] = new BitNot(value);
    return bitnot_insts[value];
}

const q::Shl *q::Shl::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Shl);
    auto key = std::make_pair(lhs, rhs);
    if (!shl_insts.contains(key))
        shl_insts[key] = new Shl(lhs, rhs);
    return shl_insts[key];
}

const q::Shr *q::Shr::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Shr);
    auto key = std::make_pair(lhs, rhs);
    if (!shr_insts.contains(key))
        shr_insts[key] = new Shr(lhs, rhs);
    return shr_insts[key];
}

const q::Rotl *q::Rotl::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Rotl);
    auto key = std::make_pair(lhs, rhs);
    if (!rotl_insts.contains(key))
        rotl_insts[key] = new Rotl(lhs, rhs);
    return rotl_insts[key];
}

const q::Rotr *q::Rotr::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Rotr);
    auto key = std::make_pair(lhs, rhs);
    if (!rotr_insts.contains(key))
        rotr_insts[key] = new Rotr(lhs, rhs);
    return rotr_insts[key];
}

const q::Eq *q::Eq::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Eq);
    auto key = std::make_pair(lhs, rhs);
    if (!eq_insts.contains(key))
        eq_insts[key] = new Eq(lhs, rhs);
    return eq_insts[key];
}

const q::Ne *q::Ne::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Ne);
    auto key = std::make_pair(lhs, rhs);
    if (!ne_insts.contains(key))
        ne_insts[key] = new Ne(lhs, rhs);
    return ne_insts[key];
}

const q::Lt *q::Lt::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Lt);
    auto key = std::make_pair(lhs, rhs);
    if (!lt_insts.contains(key))
        lt_insts[key] = new Lt(lhs, rhs);
    return lt_insts[key];
}

const q::Gt *q::Gt::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Gt);
    auto key = std::make_pair(lhs, rhs);
    if (!gt_insts.contains(key))
        gt_insts[key] = new Gt(lhs, rhs);
    return gt_insts[key];
}

const q::Le *q::Le::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Le);
    auto key = std::make_pair(lhs, rhs);
    if (!le_insts.contains(key))
        le_insts[key] = new Le(lhs, rhs);
    return le_insts[key];
}

const q::Ge *q::Ge::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Ge);
    auto key = std::make_pair(lhs, rhs);
    if (!ge_insts.contains(key))
        ge_insts[key] = new Ge(lhs, rhs);
    return ge_insts[key];
}

const q::And *q::And::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::And);
    auto key = std::make_pair(lhs, rhs);
    if (!and_insts.contains(key))
        and_insts[key] = new And(lhs, rhs);
    return and_insts[key];
}

const q::Or *q::Or::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Or);
    auto key = std::make_pair(lhs, rhs);
    if (!or_insts.contains(key))
        or_insts[key] = new Or(lhs, rhs);
    return or_insts[key];
}

const q::Not *q::Not::create(const Value<Q> *value)
{
    lock(NodeType::Not);
    if (!not_insts.contains(value))
        not_insts[value] = new Not(value);
    return not_insts[value];
}

const q::Xor *q::Xor::create(const Value<Q> *lhs, const Value<Q> *rhs)
{
    lock(NodeType::Xor);
    auto key = std::make_pair(lhs, rhs);
    if (!xor_insts.contains(key))
        xor_insts[key] = new Xor(lhs, rhs);
    return xor_insts[key];
}

const q::I1 *q::I1::create()
{
    lock(NodeType::I1);
    if (i1_inst == nullptr)
        i1_inst = new I1();
    return i1_inst;
}

const q::I8 *q::I8::create()
{
    lock(NodeType::I8);
    if (i8_inst == nullptr)
        i8_inst = new I8();
    return i8_inst;
}

const q::I16 *q::I16::create()
{
    lock(NodeType::I16);
    if (i16_inst == nullptr)
        i16_inst = new I16();
    return i16_inst;
}

const q::I32 *q::I32::create()
{
    lock(NodeType::I32);
    if (i32_inst == nullptr)
        i32_inst = new I32();
    return i32_inst;
}

const q::I64 *q::I64::create()
{
    lock(NodeType::I64);
    if (i64_inst == nullptr)
        i64_inst = new I64();
    return i64_inst;
}

const q::I128 *q::I128::create()
{
    lock(NodeType::I128);
    if (i128_inst == nullptr)
        i128_inst = new I128();
    return i128_inst;
}

const q::U8 *q::U8::create()
{
    lock(NodeType::U8);
    if (u8_inst == nullptr)
        u8_inst = new U8();
    return u8_inst;
}

const q::U16 *q::U16::create()
{
    lock(NodeType::U16);
    if (u16_inst == nullptr)
        u16_inst = new U16();
    return u16_inst;
}

const q::U32 *q::U32::create()
{
    lock(NodeType::U32);
    if (u32_inst == nullptr)
        u32_inst = new U32();
    return u32_inst;
}

const q::U64 *q::U64::create()
{
    lock(NodeType::U64);
    if (u64_inst == nullptr)
        u64_inst = new U64();
    return u64_inst;
}

const q::U128 *q::U128::create()
{
    lock(NodeType::U128);
    if (u128_inst == nullptr)
        u128_inst = new U128();
    return u128_inst;
}

const q::F32 *q::F32::create()
{
    lock(NodeType::F32);
    if (f32_inst == nullptr)
        f32_inst = new F32();
    return f32_inst;
}

const q::F64 *q::F64::create()
{
    lock(NodeType::F64);
    if (f64_inst == nullptr)
        f64_inst = new F64();
    return f64_inst;
}

const q::Void *q::Void::create()
{
    lock(NodeType::Void);
    if (void_inst == nullptr)
        void_inst = new Void();
    return void_inst;
}

const q::Ptr *q::Ptr::create(Value<Q> *type)
{
    lock(NodeType::Ptr);
    if (!ptr_insts.contains(type))
        ptr_insts[type] = new Ptr(type);
    return ptr_insts[type];
}

const q::Array *q::Array::create(Value<Q> *type, uint64_t size)
{
    lock(NodeType::Array);
    auto key = std::make_pair(type, size);
    if (!array_insts.contains(key))
        array_insts[key] = new Array(type, size);
    return array_insts[key];
}

const q::Local *q::Local::create(std::string name, const Value<Q> *type)
{
    lock(NodeType::Local);
    auto key = std::make_pair(name, type);
    if (!local_insts.contains(key))
        local_insts[key] = new Local(name, type);
    return local_insts[key];
}

const q::Global *q::Global::create(std::string name, const Value<Q> *type, const Value<Q> *value, bool _volatile, bool _atomic)
{
    lock(NodeType::Global);
    auto key = std::make_tuple(name, type, value, _volatile, _atomic);
    if (!global_insts.contains(key))
        global_insts[key] = new Global(name, type, value, _volatile, _atomic);
    return global_insts[key];
}

const q::Number *q::Number::create(std::string value)
{
    lock(NodeType::Number);
    if (!number_insts.contains(value))
        number_insts[value] = new Number(value);
    return number_insts[value];
}

const q::String *q::String::create(std::string value)
{
    lock(NodeType::String);
    if (!string_insts.contains(value))
        string_insts[value] = new String(value);
    return string_insts[value];
}
