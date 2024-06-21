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
#include <IR/Q/Function.h>
#include <IR/Q/Ident.h>
#include <IR/Q/Math.h>
#include <IR/Q/Memory.h>
#include <IR/Q/OO.h>
#include <IR/Q/QIR.h>
#include <IR/Q/Type.h>
#include <IR/Q/Variable.h>
#include <IR/delta/Cast.h>
#include <IR/delta/Control.h>
#include <IR/delta/DeltaIR.h>
#include <IR/delta/Ident.h>
#include <IR/delta/Math.h>
#include <IR/delta/Memory.h>
#include <IR/delta/Segment.h>
#include <IR/delta/Type.h>
#include <IR/delta/Variable.h>
#include <core/Logger.h>
#include <mangle/Symbol.h>

#include <any>
#include <optional>
#include <stack>

using namespace libquixcc;
using namespace libquixcc::ir::delta;

struct DState {
  std::map<std::string, const Packet *> regions;
  std::vector<std::pair<const PacketDef *, std::vector<const Global *>>> oop;
  size_t loop_depth = 0;

  DState() {}
};

typedef const Value *DValue;

class DResult {
  std::vector<DValue> m_values;

 public:
  DResult() = default;
  DResult(const DValue &value) { m_values.push_back(value); }
  DResult(const std::vector<DValue> &values) : m_values(values) {}
  DResult(const std::initializer_list<DValue> &values) : m_values(values) {}

  operator bool() const {
    if (m_values.empty()) return false;

    return m_values[0] != nullptr;
  }

  auto operator*() const -> const std::vector<DValue> & { return m_values; }
  auto operator->() const -> const std::vector<DValue> & { return m_values; }
  bool empty() const { return m_values.empty(); }
  DValue operator[](size_t i) const { return m_values.at(i); }

  auto begin() const { return m_values.begin(); }
  auto end() const { return m_values.end(); }
};

static auto conv(const ir::q::Value *n, DState &state) -> DResult;
static auto conv(const ir::q::I1 *n, DState &state) -> DResult;
static auto conv(const ir::q::RootNode *n, DState &state) -> DResult;
static auto conv(const ir::q::I8 *n, DState &state) -> DResult;
static auto conv(const ir::q::I16 *n, DState &state) -> DResult;
static auto conv(const ir::q::I32 *n, DState &state) -> DResult;
static auto conv(const ir::q::I64 *n, DState &state) -> DResult;
static auto conv(const ir::q::I128 *n, DState &state) -> DResult;
static auto conv(const ir::q::U8 *n, DState &state) -> DResult;
static auto conv(const ir::q::U16 *n, DState &state) -> DResult;
static auto conv(const ir::q::U32 *n, DState &state) -> DResult;
static auto conv(const ir::q::U64 *n, DState &state) -> DResult;
static auto conv(const ir::q::U128 *n, DState &state) -> DResult;
static auto conv(const ir::q::F32 *n, DState &state) -> DResult;
static auto conv(const ir::q::F64 *n, DState &state) -> DResult;
static auto conv(const ir::q::Void *n, DState &state) -> DResult;
static auto conv(const ir::q::Ptr *n, DState &state) -> DResult;
static auto conv(const ir::q::Array *n, DState &state) -> DResult;
static auto conv(const ir::q::Vector *n, DState &state) -> DResult;
static auto conv(const ir::q::FType *n, DState &state) -> DResult;
static auto conv(const ir::q::Region *n, DState &state) -> DResult;
static auto conv(const ir::q::Union *n, DState &state) -> DResult;
static auto conv(const ir::q::Opaque *n, DState &state) -> DResult;
static auto conv(const ir::q::Block *n, DState &state) -> DResult;
static auto conv(const ir::q::Segment *n, DState &state) -> DResult;
static auto conv(const ir::q::Asm *n, DState &state) -> DResult;
static auto conv(const ir::q::RegionDef *n, DState &state) -> DResult;
static auto conv(const ir::q::GroupDef *n, DState &state) -> DResult;
static auto conv(const ir::q::UnionDef *n, DState &state) -> DResult;
static auto conv(const ir::q::SCast *n, DState &state) -> DResult;
static auto conv(const ir::q::UCast *n, DState &state) -> DResult;
static auto conv(const ir::q::PtrICast *n, DState &state) -> DResult;
static auto conv(const ir::q::IPtrCast *n, DState &state) -> DResult;
static auto conv(const ir::q::Bitcast *n, DState &state) -> DResult;
static auto conv(const ir::q::Call *n, DState &state) -> DResult;
static auto conv(const ir::q::CallIndirect *n, DState &state) -> DResult;
static auto conv(const ir::q::IfElse *n, DState &state) -> DResult;
static auto conv(const ir::q::While *n, DState &state) -> DResult;
static auto conv(const ir::q::For *n, DState &state) -> DResult;
static auto conv(const ir::q::Loop *n, DState &state) -> DResult;
static auto conv(const ir::q::Break *n, DState &state) -> DResult;
static auto conv(const ir::q::Continue *n, DState &state) -> DResult;
static auto conv(const ir::q::Ret *n, DState &state) -> DResult;
static auto conv(const ir::q::Throw *n, DState &state) -> DResult;
static auto conv(const ir::q::TryCatchFinally *n, DState &state) -> DResult;
static auto conv(const ir::q::Case *n, DState &state) -> DResult;
static auto conv(const ir::q::Switch *n, DState &state) -> DResult;
static auto conv(const ir::q::Ident *n, DState &state) -> DResult;
static auto conv(const ir::q::Add *n, DState &state) -> DResult;
static auto conv(const ir::q::Sub *n, DState &state) -> DResult;
static auto conv(const ir::q::Mul *n, DState &state) -> DResult;
static auto conv(const ir::q::Div *n, DState &state) -> DResult;
static auto conv(const ir::q::Mod *n, DState &state) -> DResult;
static auto conv(const ir::q::BitAnd *n, DState &state) -> DResult;
static auto conv(const ir::q::BitOr *n, DState &state) -> DResult;
static auto conv(const ir::q::BitXor *n, DState &state) -> DResult;
static auto conv(const ir::q::BitNot *n, DState &state) -> DResult;
static auto conv(const ir::q::Shl *n, DState &state) -> DResult;
static auto conv(const ir::q::Shr *n, DState &state) -> DResult;
static auto conv(const ir::q::Rotl *n, DState &state) -> DResult;
static auto conv(const ir::q::Rotr *n, DState &state) -> DResult;
static auto conv(const ir::q::Eq *n, DState &state) -> DResult;
static auto conv(const ir::q::Ne *n, DState &state) -> DResult;
static auto conv(const ir::q::Lt *n, DState &state) -> DResult;
static auto conv(const ir::q::Gt *n, DState &state) -> DResult;
static auto conv(const ir::q::Le *n, DState &state) -> DResult;
static auto conv(const ir::q::Ge *n, DState &state) -> DResult;
static auto conv(const ir::q::And *n, DState &state) -> DResult;
static auto conv(const ir::q::Or *n, DState &state) -> DResult;
static auto conv(const ir::q::Not *n, DState &state) -> DResult;
static auto conv(const ir::q::Xor *n, DState &state) -> DResult;
static auto conv(const ir::q::Local *n, DState &state) -> DResult;
static auto conv(const ir::q::Global *n, DState &state) -> DResult;
static auto conv(const ir::q::Number *n, DState &state) -> DResult;
static auto conv(const ir::q::String *n, DState &state) -> DResult;
static auto conv(const ir::q::Char *n, DState &state) -> DResult;
static auto conv(const ir::q::List *n, DState &state) -> DResult;
static auto conv(const ir::q::Assign *n, DState &state) -> DResult;
static auto conv(const ir::q::PostInc *n, DState &state) -> DResult;
static auto conv(const ir::q::PostDec *n, DState &state) -> DResult;
static auto conv(const ir::q::AddressOf *n, DState &state) -> DResult;
static auto conv(const ir::q::Deref *n, DState &state) -> DResult;
static auto conv(const ir::q::Member *n, DState &state) -> DResult;
static auto conv(const ir::q::Index *n, DState &state) -> DResult;

static auto conv(const ir::q::RootNode *n, DState &state) -> DResult {
  std::vector<DValue> content;
  for (auto node : n->children) {
    auto result = conv(node, state);
    if (!result) continue;

    for (auto &value : *result) content.push_back(value);
  }

  for (auto it = state.oop.begin(); it != state.oop.end();) {
    auto packet = it->first;
    auto methods = it->second;

    for (auto method : methods) content.push_back(method);

    content.insert(content.begin(), packet);

    it = state.oop.erase(it);
  }

  std::vector<const Global *> forward_decls;

  for (auto it = content.begin(); it != content.end();) {
    if (!(*it)->is<Global>()) {
      ++it;
      continue;
    }

    auto global = (*it)->as<Global>();

    if (!global->value || !global->value->is<Segment>()) {
      ++it;
      continue;
    }

    auto seg = global->value->as<Segment>();

    if (!seg->block) {
      ++it;
      continue;
    }

    forward_decls.push_back(Global::create(
        global->name, seg->infer(),
        Segment::create(seg->ret, seg->variadic, seg->params, nullptr),
        global->_volatile, global->_atomic, global->_extern));

    ++it;
  }

  std::vector<DValue> values;
  for (auto decl : forward_decls) values.push_back(decl);
  for (auto value : content) values.push_back(value);

  return RootNode::create(values);
}

static auto conv(const ir::q::I1 *n, DState &state) -> DResult {
  return I1::create();
}

static auto conv(const ir::q::I8 *n, DState &state) -> DResult {
  return I8::create();
}

static auto conv(const ir::q::I16 *n, DState &state) -> DResult {
  return I16::create();
}

static auto conv(const ir::q::I32 *n, DState &state) -> DResult {
  return I32::create();
}

static auto conv(const ir::q::I64 *n, DState &state) -> DResult {
  return I64::create();
}

static auto conv(const ir::q::I128 *n, DState &state) -> DResult {
  return I128::create();
}

static auto conv(const ir::q::U8 *n, DState &state) -> DResult {
  return U8::create();
}

static auto conv(const ir::q::U16 *n, DState &state) -> DResult {
  return U16::create();
}

static auto conv(const ir::q::U32 *n, DState &state) -> DResult {
  return U32::create();
}

static auto conv(const ir::q::U64 *n, DState &state) -> DResult {
  return U64::create();
}

static auto conv(const ir::q::U128 *n, DState &state) -> DResult {
  return U128::create();
}

static auto conv(const ir::q::F32 *n, DState &state) -> DResult {
  return F32::create();
}

static auto conv(const ir::q::F64 *n, DState &state) -> DResult {
  return F64::create();
}

static auto conv(const ir::q::Void *n, DState &state) -> DResult {
  return Void::create();
}

static auto conv(const ir::q::Ptr *n, DState &state) -> DResult {
  return Ptr::create(conv(n->type, state)[0]->as<Type>());
}

static auto conv(const ir::q::Array *n, DState &state) -> DResult {
  return Array::create(conv(n->type, state)[0]->as<Type>(), n->size);
}

static auto conv(const ir::q::Vector *n, DState &state) -> DResult {
  /// TODO: Implement Vector
  throw std::runtime_error("DeltaIR translation: Vector not implemented");
}

static auto conv(const ir::q::FType *n, DState &state) -> DResult {
  auto ret = conv(n->ret, state)[0]->as<Type>();
  std::vector<const Type *> params;
  for (auto param : n->params)
    params.push_back(conv(param, state)[0]->as<Type>());

  return FType::create(params, ret, n->m_variadic);
}

static auto conv(const ir::q::Region *n, DState &state) -> DResult {
  if (state.regions.contains(n->name)) {
    return state.regions.at(n->name);
  }

  std::vector<std::pair<std::string, const Type *>> fields;
  for (auto field : n->fields)
    fields.push_back({"", conv(field, state)[0]->as<Type>()});

  auto packet = PacketDef::create(fields, n->name);
  return state.regions[n->name] = Packet::create(packet);
}

static auto conv(const ir::q::Union *n, DState &state) -> DResult {
  if (!state.regions.contains(n->name))
    throw std::runtime_error("DeltaIR translation: Union not found");

  return state.regions.at(n->name);
}

static auto conv(const ir::q::Opaque *n, DState &state) -> DResult {
  return Void::create();
}

static auto conv(const ir::q::Block *n, DState &state) -> DResult {
  std::vector<DValue> values;
  for (auto node : n->stmts) {
    auto result = conv(node, state);
    if (!result) continue;

    for (auto value : *result) values.push_back(value);
  }

  return Block::create(values);
}

static auto conv(const ir::q::Segment *n, DState &state) -> DResult {
  auto ret = conv(n->return_type, state)[0]->as<Type>();
  std::vector<std::pair<std::string, const Type *>> params;

  for (auto param : n->params)
    params.push_back({param.first, conv(param.second, state)[0]->as<Type>()});

  const Block *block = nullptr;
  if (n->block) block = conv(n->block, state)[0]->as<Block>();

  return Segment::create(ret, n->infer()->as<ir::q::FType>()->m_variadic,
                         params, block);
}

static auto conv(const ir::q::Asm *n, DState &state) -> DResult {
  std::vector<std::pair<std::string, const Value *>> outputs;
  std::vector<std::pair<std::string, const Value *>> inputs;

  for (auto output : n->outputs)
    outputs.push_back({output.first, conv(output.second, state)[0]});

  for (auto input : n->inputs)
    inputs.push_back({input.first, conv(input.second, state)[0]});

  return Asm::create(n->asm_str, outputs, inputs, n->clobbers);
}

static auto conv(const ir::q::RegionDef *n, DState &state) -> DResult {
  std::vector<std::pair<std::string, const Type *>> fields;
  for (auto field : n->fields)
    fields.push_back({field.first, conv(field.second, state)[0]->as<Type>()});

  auto packet = PacketDef::create(fields, n->name);
  state.regions[n->name] = Packet::create(packet);

  std::vector<const Global *> methods;
  for (auto method : n->methods) {
    auto seg = conv(method.second, state)[0]->as<Segment>();
    auto g =
        Global::create(method.first, seg->infer(), seg, false, false, false);
    methods.push_back(g);
  }

  state.oop.push_back({packet, methods});
  return nullptr;
}

static auto conv(const ir::q::GroupDef *n, DState &state) -> DResult {
  std::vector<std::pair<std::string, const Type *>> fields;
  for (auto field : n->fields)
    fields.push_back({field.first, conv(field.second, state)[0]->as<Type>()});

  auto packet = PacketDef::create(fields, n->name);
  state.regions[n->name] = Packet::create(packet);

  std::vector<const Global *> methods;
  for (auto method : n->methods) {
    auto seg = conv(method.second, state)[0]->as<Segment>();
    auto g =
        Global::create(method.first, seg->infer(), seg, false, false, false);
    methods.push_back(g);
  }

  state.oop.push_back({packet, methods});
  return nullptr;
}

static auto conv(const ir::q::UnionDef *n, DState &state) -> DResult {
  /// TODO: Implement UnionDef
  throw std::runtime_error("DeltaIR translation: UnionDef not implemented");
}

static auto conv(const ir::q::SCast *n, DState &state) -> DResult {
  return SCast::create(conv(n->type, state)[0]->as<Type>(),
                       conv(n->value, state)[0]->as<Expr>());
}

static auto conv(const ir::q::UCast *n, DState &state) -> DResult {
  return UCast::create(conv(n->type, state)[0]->as<Type>(),
                       conv(n->value, state)[0]->as<Expr>());
}

static auto conv(const ir::q::PtrICast *n, DState &state) -> DResult {
  return PtrICast::create(conv(n->value, state)[0]->as<Expr>());
}

static auto conv(const ir::q::IPtrCast *n, DState &state) -> DResult {
  return IPtrCast::create(conv(n->type, state)[0]->as<Type>(),
                          conv(n->value, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Bitcast *n, DState &state) -> DResult {
  return Bitcast::create(conv(n->type, state)[0]->as<Type>(),
                         conv(n->value, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Call *n, DState &state) -> DResult {
  std::vector<const Expr *> args;
  for (auto arg : n->args) args.push_back(conv(arg, state)[0]->as<Expr>());

  auto ftype = conv(n->func->type, state)[0]->as<FType>();

  return Call::create(n->func->name, args, ftype);
}

static auto conv(const ir::q::CallIndirect *n, DState &state) -> DResult {
  /// TODO: Implement CallIndirect
  throw std::runtime_error("DeltaIR translation: CallIndirect not implemented");
}

static auto conv(const ir::q::IfElse *n, DState &state) -> DResult {
  auto cond = conv(n->cond, state)[0]->as<Expr>();
  auto if_block = conv(n->then, state)[0]->as<Block>();
  auto else_block = conv(n->els, state)[0]->as<Block>();

  return IfElse::create(cond, if_block, else_block);
}

static auto conv(const ir::q::While *n, DState &state) -> DResult {
  auto cond = conv(n->cond, state)[0]->as<Expr>();
  auto body = conv(n->body, state)[0];

  return While::create(cond, body);
}

static auto conv(const ir::q::For *n, DState &state) -> DResult {
  const Expr *init = nullptr;
  const Expr *cond = nullptr;
  const Expr *step = nullptr;
  const Value *body = nullptr;

  if (n->init) init = conv(n->init, state)[0]->as<Expr>();
  if (n->cond) cond = conv(n->cond, state)[0]->as<Expr>();
  if (n->step) step = conv(n->step, state)[0]->as<Expr>();
  if (n->body) body = conv(n->body, state)[0];

  if (!init) init = Number::create("1");
  if (!cond) cond = Number::create("1");
  if (!step) step = Number::create("1");

  std::string label = "__step" + std::to_string(state.loop_depth++);

  auto block = Block::create({body, Label::create(label, step)});
  auto loop = While::create(cond, block);

  auto x = Block::create({init, loop});

  return x;
}

static auto conv(const ir::q::Loop *n, DState &state) -> DResult {
  /// TODO: Implement Loop
  throw std::runtime_error("DeltaIR translation: Loop not implemented");
}

static auto conv(const ir::q::Break *n, DState &state) -> DResult {
  return Break::create();
}

static auto conv(const ir::q::Continue *n, DState &state) -> DResult {
  return Continue::create();
}

static auto conv(const ir::q::Ret *n, DState &state) -> DResult {
  if (!n->value) return Ret::create(nullptr);

  return Ret::create(conv(n->value, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Throw *n, DState &state) -> DResult {
  /// TODO: Implement Throw
  throw std::runtime_error("DeltaIR translation: Throw not implemented");
}

static auto conv(const ir::q::TryCatchFinally *n, DState &state) -> DResult {
  /// TODO: Implement TryCatchFinally
  throw std::runtime_error(
      "DeltaIR translation: TryCatchFinally not implemented");
}

static auto conv(const ir::q::Case *n, DState &state) -> DResult {
  auto value = conv(n->value, state)[0]->as<Expr>();
  auto block = conv(n->body, state)[0]->as<Block>();

  return Case::create(value, block);
}

static auto conv(const ir::q::Switch *n, DState &state) -> DResult {
  /// TODO: Implement Switch

  auto value = conv(n->value, state)[0]->as<Expr>();

  auto def =
      n->defaultcase ? conv(n->defaultcase, state)[0]->as<Block>() : nullptr;

  if (value->infer()->is_numeric()) {
    std::vector<const Case *> cases;
    for (auto c : n->cases) cases.push_back(conv(c, state)[0]->as<Case>());
    return Switch::create(value, cases, def);
  } else {
    throw std::runtime_error(
        "DeltaIR translation: Switch not implemented for non-numeric types");
  }
}

static auto conv(const ir::q::Ident *n, DState &state) -> DResult {
  return Ident::create(n->name, conv(n->type, state)[0]->as<Type>());
}

static auto conv(const ir::q::Add *n, DState &state) -> DResult {
  return Add::create(conv(n->lhs, state)[0]->as<Expr>(),
                     conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Sub *n, DState &state) -> DResult {
  return Sub::create(conv(n->lhs, state)[0]->as<Expr>(),
                     conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Mul *n, DState &state) -> DResult {
  return Mul::create(conv(n->lhs, state)[0]->as<Expr>(),
                     conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Div *n, DState &state) -> DResult {
  return Div::create(conv(n->lhs, state)[0]->as<Expr>(),
                     conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Mod *n, DState &state) -> DResult {
  return Mod::create(conv(n->lhs, state)[0]->as<Expr>(),
                     conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::BitAnd *n, DState &state) -> DResult {
  return BitAnd::create(conv(n->lhs, state)[0]->as<Expr>(),
                        conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::BitOr *n, DState &state) -> DResult {
  return BitOr::create(conv(n->lhs, state)[0]->as<Expr>(),
                       conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::BitXor *n, DState &state) -> DResult {
  return BitXor::create(conv(n->lhs, state)[0]->as<Expr>(),
                        conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::BitNot *n, DState &state) -> DResult {
  return BitNot::create(conv(n->operand, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Shl *n, DState &state) -> DResult {
  return Shl::create(conv(n->lhs, state)[0]->as<Expr>(),
                     conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Shr *n, DState &state) -> DResult {
  return Shr::create(conv(n->lhs, state)[0]->as<Expr>(),
                     conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Rotl *n, DState &state) -> DResult {
  return Rotl::create(conv(n->lhs, state)[0]->as<Expr>(),
                      conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Rotr *n, DState &state) -> DResult {
  return Rotr::create(conv(n->lhs, state)[0]->as<Expr>(),
                      conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Eq *n, DState &state) -> DResult {
  return Eq::create(conv(n->lhs, state)[0]->as<Expr>(),
                    conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Ne *n, DState &state) -> DResult {
  return Ne::create(conv(n->lhs, state)[0]->as<Expr>(),
                    conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Lt *n, DState &state) -> DResult {
  return Lt::create(conv(n->lhs, state)[0]->as<Expr>(),
                    conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Gt *n, DState &state) -> DResult {
  return Gt::create(conv(n->lhs, state)[0]->as<Expr>(),
                    conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Le *n, DState &state) -> DResult {
  return Le::create(conv(n->lhs, state)[0]->as<Expr>(),
                    conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Ge *n, DState &state) -> DResult {
  return Ge::create(conv(n->lhs, state)[0]->as<Expr>(),
                    conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::And *n, DState &state) -> DResult {
  return And::create(conv(n->lhs, state)[0]->as<Expr>(),
                     conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Or *n, DState &state) -> DResult {
  return Or::create(conv(n->lhs, state)[0]->as<Expr>(),
                    conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Not *n, DState &state) -> DResult {
  return Not::create(conv(n->operand, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Xor *n, DState &state) -> DResult {
  return Xor::create(conv(n->lhs, state)[0]->as<Expr>(),
                     conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Local *n, DState &state) -> DResult {
  const Expr *v = nullptr;
  if (n->value) v = conv(n->value, state)[0]->as<Expr>();

  return Local::create(n->name, conv(n->type, state)[0]->as<Type>(), v);
}

static auto conv(const ir::q::Global *n, DState &state) -> DResult {
  auto type = conv(n->type, state)[0]->as<Type>();

  const Expr *init = nullptr;

  if (n->value) init = conv(n->value, state)[0]->as<Expr>();

  return Global::create(n->name, type, init, n->_volatile, n->_atomic,
                        n->_extern);
}

static auto conv(const ir::q::Number *n, DState &state) -> DResult {
  return Number::create(n->value);
}

static auto conv(const ir::q::String *n, DState &state) -> DResult {
  return String::create(n->value);
}

static auto conv(const ir::q::Char *n, DState &state) -> DResult {
  /// TODO: Implement Char
  throw std::runtime_error("DeltaIR translation: Char not implemented");
}

static auto conv(const ir::q::List *n, DState &state) -> DResult {
  std::vector<const Expr *> values;
  for (auto value : n->values)
    values.push_back(conv(value, state)[0]->as<Expr>());

  return List::create(values);
}

static auto conv(const ir::q::Assign *n, DState &state) -> DResult {
  return Assign::create(conv(n->lhs, state)[0]->as<Expr>(),
                        conv(n->rhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::PostInc *n, DState &state) -> DResult {
  return PostInc::create(conv(n->lhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::PostDec *n, DState &state) -> DResult {
  return PostDec::create(conv(n->lhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::AddressOf *n, DState &state) -> DResult {
  return AddressOf::create(conv(n->lhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Deref *n, DState &state) -> DResult {
  return Deref::create(conv(n->lhs, state)[0]->as<Expr>());
}

static auto conv(const ir::q::Member *n, DState &state) -> DResult {
  auto e = conv(n->lhs, state)[0]->as<Expr>();
  auto t = conv(n->field_type, state)[0]->as<Type>();

  return Member::create(e, n->field, t);
}

auto conv(const libquixcc::ir::q::Index *n, DState &state) -> DResult {
  auto e = conv(n->lhs, state)[0]->as<Expr>();
  auto i = conv(n->index, state)[0]->as<Expr>();
  auto t = conv(n->type, state)[0]->as<Type>();

  return Index::create(e, i, t);
}

static auto conv(const ir::q::Value *n, DState &state) -> DResult {
  DResult r;

  switch ((libquixcc::ir::q::NodeType)n->ntype) {
    case libquixcc::ir::q::NodeType::Root:
      r = conv(n->as<ir::q::RootNode>(), state);
      break;

    case libquixcc::ir::q::NodeType::I1:
      r = conv(n->as<ir::q::I1>(), state);
      break;

    case libquixcc::ir::q::NodeType::I8:
      r = conv(n->as<ir::q::I8>(), state);
      break;

    case libquixcc::ir::q::NodeType::I16:
      r = conv(n->as<ir::q::I16>(), state);
      break;

    case libquixcc::ir::q::NodeType::I32:
      r = conv(n->as<ir::q::I32>(), state);
      break;

    case libquixcc::ir::q::NodeType::I64:
      r = conv(n->as<ir::q::I64>(), state);
      break;

    case libquixcc::ir::q::NodeType::I128:
      r = conv(n->as<ir::q::I128>(), state);
      break;

    case libquixcc::ir::q::NodeType::U8:
      r = conv(n->as<ir::q::U8>(), state);
      break;

    case libquixcc::ir::q::NodeType::U16:
      r = conv(n->as<ir::q::U16>(), state);
      break;

    case libquixcc::ir::q::NodeType::U32:
      r = conv(n->as<ir::q::U32>(), state);
      break;

    case libquixcc::ir::q::NodeType::U64:
      r = conv(n->as<ir::q::U64>(), state);
      break;

    case libquixcc::ir::q::NodeType::U128:
      r = conv(n->as<ir::q::U128>(), state);
      break;

    case libquixcc::ir::q::NodeType::F32:
      r = conv(n->as<ir::q::F32>(), state);
      break;

    case libquixcc::ir::q::NodeType::F64:
      r = conv(n->as<ir::q::F64>(), state);
      break;

    case libquixcc::ir::q::NodeType::Void:
      r = conv(n->as<ir::q::Void>(), state);
      break;

    case libquixcc::ir::q::NodeType::Ptr:
      r = conv(n->as<ir::q::Ptr>(), state);
      break;

    case libquixcc::ir::q::NodeType::Array:
      r = conv(n->as<ir::q::Array>(), state);
      break;

    case libquixcc::ir::q::NodeType::Vector:
      r = conv(n->as<ir::q::Vector>(), state);
      break;

    case libquixcc::ir::q::NodeType::FType:
      r = conv(n->as<ir::q::FType>(), state);
      break;

    case libquixcc::ir::q::NodeType::Region:
      r = conv(n->as<ir::q::Region>(), state);
      break;

    case libquixcc::ir::q::NodeType::Union:
      r = conv(n->as<ir::q::Union>(), state);
      break;

    case libquixcc::ir::q::NodeType::Opaque:
      r = conv(n->as<ir::q::Opaque>(), state);
      break;

    case libquixcc::ir::q::NodeType::Block:
      r = conv(n->as<ir::q::Block>(), state);
      break;

    case libquixcc::ir::q::NodeType::Segment:
      r = conv(n->as<ir::q::Segment>(), state);
      break;

    case libquixcc::ir::q::NodeType::Asm:
      r = conv(n->as<ir::q::Asm>(), state);
      break;

    case libquixcc::ir::q::NodeType::RegionDef:
      r = conv(n->as<ir::q::RegionDef>(), state);
      break;

    case libquixcc::ir::q::NodeType::GroupDef:
      r = conv(n->as<ir::q::GroupDef>(), state);
      break;

    case libquixcc::ir::q::NodeType::UnionDef:
      r = conv(n->as<ir::q::UnionDef>(), state);
      break;

    case libquixcc::ir::q::NodeType::SCast:
      r = conv(n->as<ir::q::SCast>(), state);
      break;

    case libquixcc::ir::q::NodeType::UCast:
      r = conv(n->as<ir::q::UCast>(), state);
      break;

    case libquixcc::ir::q::NodeType::PtrICast:
      r = conv(n->as<ir::q::PtrICast>(), state);
      break;

    case libquixcc::ir::q::NodeType::IPtrCast:
      r = conv(n->as<ir::q::IPtrCast>(), state);
      break;

    case libquixcc::ir::q::NodeType::Bitcast:
      r = conv(n->as<ir::q::Bitcast>(), state);
      break;

    case libquixcc::ir::q::NodeType::Call:
      r = conv(n->as<ir::q::Call>(), state);
      break;

    case libquixcc::ir::q::NodeType::CallIndirect:
      r = conv(n->as<ir::q::CallIndirect>(), state);
      break;

    case libquixcc::ir::q::NodeType::IfElse:
      r = conv(n->as<ir::q::IfElse>(), state);
      break;

    case libquixcc::ir::q::NodeType::While:
      r = conv(n->as<ir::q::While>(), state);
      break;

    case libquixcc::ir::q::NodeType::For:
      r = conv(n->as<ir::q::For>(), state);
      break;

    case libquixcc::ir::q::NodeType::Loop:
      r = conv(n->as<ir::q::Loop>(), state);
      break;

    case libquixcc::ir::q::NodeType::Break:
      r = conv(n->as<ir::q::Break>(), state);
      break;

    case libquixcc::ir::q::NodeType::Continue:
      r = conv(n->as<ir::q::Continue>(), state);
      break;

    case libquixcc::ir::q::NodeType::Ret:
      r = conv(n->as<ir::q::Ret>(), state);
      break;

    case libquixcc::ir::q::NodeType::Throw:
      r = conv(n->as<ir::q::Throw>(), state);
      break;

    case libquixcc::ir::q::NodeType::TryCatchFinally:
      r = conv(n->as<ir::q::TryCatchFinally>(), state);
      break;

    case libquixcc::ir::q::NodeType::Case:
      r = conv(n->as<ir::q::Case>(), state);
      break;

    case libquixcc::ir::q::NodeType::Switch:
      r = conv(n->as<ir::q::Switch>(), state);
      break;

    case libquixcc::ir::q::NodeType::Ident:
      r = conv(n->as<ir::q::Ident>(), state);
      break;

    case libquixcc::ir::q::NodeType::Add:
      r = conv(n->as<ir::q::Add>(), state);
      break;

    case libquixcc::ir::q::NodeType::Sub:
      r = conv(n->as<ir::q::Sub>(), state);
      break;

    case libquixcc::ir::q::NodeType::Mul:
      r = conv(n->as<ir::q::Mul>(), state);
      break;

    case libquixcc::ir::q::NodeType::Div:
      r = conv(n->as<ir::q::Div>(), state);
      break;

    case libquixcc::ir::q::NodeType::Mod:
      r = conv(n->as<ir::q::Mod>(), state);
      break;

    case libquixcc::ir::q::NodeType::BitAnd:
      r = conv(n->as<ir::q::BitAnd>(), state);
      break;

    case libquixcc::ir::q::NodeType::BitOr:
      r = conv(n->as<ir::q::BitOr>(), state);
      break;

    case libquixcc::ir::q::NodeType::BitXor:
      r = conv(n->as<ir::q::BitXor>(), state);
      break;

    case libquixcc::ir::q::NodeType::BitNot:
      r = conv(n->as<ir::q::BitNot>(), state);
      break;

    case libquixcc::ir::q::NodeType::Shl:
      r = conv(n->as<ir::q::Shl>(), state);
      break;

    case libquixcc::ir::q::NodeType::Shr:
      r = conv(n->as<ir::q::Shr>(), state);
      break;

    case libquixcc::ir::q::NodeType::Rotl:
      r = conv(n->as<ir::q::Rotl>(), state);
      break;

    case libquixcc::ir::q::NodeType::Rotr:
      r = conv(n->as<ir::q::Rotr>(), state);
      break;

    case libquixcc::ir::q::NodeType::Eq:
      r = conv(n->as<ir::q::Eq>(), state);
      break;

    case libquixcc::ir::q::NodeType::Ne:
      r = conv(n->as<ir::q::Ne>(), state);
      break;

    case libquixcc::ir::q::NodeType::Lt:
      r = conv(n->as<ir::q::Lt>(), state);
      break;

    case libquixcc::ir::q::NodeType::Gt:
      r = conv(n->as<ir::q::Gt>(), state);
      break;

    case libquixcc::ir::q::NodeType::Le:
      r = conv(n->as<ir::q::Le>(), state);
      break;

    case libquixcc::ir::q::NodeType::Ge:
      r = conv(n->as<ir::q::Ge>(), state);
      break;

    case libquixcc::ir::q::NodeType::And:
      r = conv(n->as<ir::q::And>(), state);
      break;

    case libquixcc::ir::q::NodeType::Or:
      r = conv(n->as<ir::q::Or>(), state);
      break;

    case libquixcc::ir::q::NodeType::Not:
      r = conv(n->as<ir::q::Not>(), state);
      break;

    case libquixcc::ir::q::NodeType::Xor:
      r = conv(n->as<ir::q::Xor>(), state);
      break;

    case libquixcc::ir::q::NodeType::Local:
      r = conv(n->as<ir::q::Local>(), state);
      break;

    case libquixcc::ir::q::NodeType::Global:
      r = conv(n->as<ir::q::Global>(), state);
      break;

    case libquixcc::ir::q::NodeType::Number:
      r = conv(n->as<ir::q::Number>(), state);
      break;

    case libquixcc::ir::q::NodeType::String:
      r = conv(n->as<ir::q::String>(), state);
      break;

    case libquixcc::ir::q::NodeType::Char:
      r = conv(n->as<ir::q::Char>(), state);
      break;

    case libquixcc::ir::q::NodeType::List:
      r = conv(n->as<ir::q::List>(), state);
      break;

    case libquixcc::ir::q::NodeType::Assign:
      r = conv(n->as<ir::q::Assign>(), state);
      break;

    case libquixcc::ir::q::NodeType::PostInc:
      r = conv(n->as<ir::q::PostInc>(), state);
      break;

    case libquixcc::ir::q::NodeType::PostDec:
      r = conv(n->as<ir::q::PostDec>(), state);
      break;

    case libquixcc::ir::q::NodeType::AddressOf:
      r = conv(n->as<ir::q::AddressOf>(), state);
      break;

    case libquixcc::ir::q::NodeType::Deref:
      r = conv(n->as<ir::q::Deref>(), state);
      break;

    case libquixcc::ir::q::NodeType::Member:
      r = conv(n->as<ir::q::Member>(), state);
      break;

    case libquixcc::ir::q::NodeType::Index:
      r = conv(n->as<ir::q::Index>(), state);
      break;

    default:
      throw std::runtime_error("DIR translation: Unknown node type");
  }
  return r;
}

bool libquixcc::ir::delta::IRDelta::from_qir(
    quixcc_job_t *job, const std::unique_ptr<libquixcc::ir::q::QModule> &ir) {
  LOG(DEBUG) << "Translating QUIX intermediate representation to DeltaIR"
             << std::endl;

  DState state;

  m_root = conv(ir->root(), state)[0]->as<RootNode>();

  if (!verify()) {
    LOG(FATAL) << "Failed to qualify QUIX intermediate representation"
               << std::endl;
    return false;
  }

  if (job->m_debug) {
    LOG(DEBUG) << log::raw << to_string() << std::endl;
  }

  LOG(DEBUG)
      << "Successfully translated QUIX intermediate representation to DeltaIR"
      << std::endl;

  return true;
}