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

#define QUIXCC_INTERNAL

#include <IR/delta/Cast.h>
#include <IR/delta/Control.h>
#include <IR/delta/Math.h>
#include <IR/delta/Memory.h>
#include <IR/delta/Segment.h>
#include <IR/delta/Type.h>
#include <IR/delta/Variable.h>
#include <core/Logger.h>
#include <generate/CodeGen.h>
#include <llvm/IR/InlineAsm.h>
#include <mangle/Symbol.h>

using namespace libquixcc;
using namespace libquixcc::ir;
using namespace libquixcc::ir::delta;

uint8_t get_numbits(std::string s);

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::I1 *node) {
  return llvm::Type::getInt1Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::I8 *node) {
  return llvm::Type::getInt8Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::I16 *node) {
  return llvm::Type::getInt16Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::I32 *node) {
  return llvm::Type::getInt32Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::I64 *node) {
  return llvm::Type::getInt64Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::I128 *node) {
  return llvm::Type::getInt128Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::U8 *node) {
  return llvm::Type::getInt8Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::U16 *node) {
  return llvm::Type::getInt16Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::U32 *node) {
  return llvm::Type::getInt32Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::U64 *node) {
  return llvm::Type::getInt64Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::U128 *node) {
  return llvm::Type::getInt128Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::F32 *node) {
  return llvm::Type::getFloatTy(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::F64 *node) {
  return llvm::Type::getDoubleTy(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::Void *node) {
  return llvm::Type::getVoidTy(*m_ctx->m_ctx);
}

llvm::PointerType *libquixcc::LLVM14Codegen::gen(const ir::delta::Ptr *node) {
  auto type = gent(node->type);
  if (type->isVoidTy())
    return llvm::PointerType::get(llvm::Type::getInt8Ty(*m_ctx->m_ctx), 0);
  else
    return llvm::PointerType::get(type, 0);
}

llvm::StructType *libquixcc::LLVM14Codegen::gen(const ir::delta::Packet *node) {
  if (m_state.types.contains(node->def->name))
    return static_cast<llvm::StructType *>(m_state.types[node->def->name]);

  std::vector<llvm::Type *> types;
  for (auto &field : node->def->fields) types.push_back(gent(field.second));

  auto st =
      llvm::StructType::create(*m_ctx->m_ctx, types, node->def->name, true);
  m_state.types[node->def->name] = st;

  return st;
}

llvm::ArrayType *libquixcc::LLVM14Codegen::gen(const ir::delta::Array *node) {
  return llvm::ArrayType::get(gent(node->type), node->size);
}

llvm::FunctionType *libquixcc::LLVM14Codegen::gen(
    const ir::delta::FType *node) {
  std::vector<llvm::Type *> types;
  for (auto &param : node->params) types.push_back(gent(param));

  return llvm::FunctionType::get(gent(node->ret), types, node->variadic);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Local *node) {
  auto type = gent(node->type);
  llvm::AllocaInst *alloca =
      m_ctx->m_builder->CreateAlloca(type, nullptr, node->name);
  m_state.locals.top()[node->name] = alloca;

  if (node->value) {
    auto v = gen(node->value);
    m_ctx->m_builder->CreateStore(v, alloca);
  }

  return alloca;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Global *node) {
  bool old_pub = m_state.m_pub;

  if (node->_extern) m_state.m_pub = true;

  llvm::Type *type = gent(node->type);

  if (node->value && node->value->is<Segment>()) {
    m_state.name = node->name;

    auto segment = node->value->as<Segment>();
    llvm::Function *func = gen(segment);
    m_state.name.clear();

    m_state.functions[node->name] = func;

    m_state.m_pub = old_pub;
    return func;
  }

  m_ctx->m_module->getOrInsertGlobal(node->name, type);
  llvm::GlobalVariable *gvar = m_ctx->m_module->getGlobalVariable(node->name);
  m_ctx->m_named_global_vars[node->name] = gvar;

  if (node->_extern)
    gvar->setLinkage(llvm::GlobalValue::ExternalLinkage);
  else
    gvar->setLinkage(llvm::GlobalValue::LinkageTypes::PrivateLinkage);

  if (node->value) {
    gvar->setInitializer(static_cast<llvm::Constant *>(gen(node->value)));
  } else {
    if (type->isIntegerTy())
      gvar->setInitializer(llvm::ConstantInt::get(
          type, llvm::APInt(type->getPrimitiveSizeInBits(), 0, true)));
    else if (type->isFloatTy())
      gvar->setInitializer(llvm::ConstantFP::get(type, 0.0));
    else if (type->isPointerTy())
      gvar->setInitializer(llvm::ConstantPointerNull::get(
          static_cast<llvm::PointerType *>(type)));
    else if (type->isArrayTy() || type->isStructTy())
      gvar->setInitializer(llvm::ConstantAggregateZero::get(type));
    else
      gvar->setInitializer(llvm::Constant::getNullValue(type));
  }

  m_state.globals[node->name] = gvar;
  m_state.m_pub = old_pub;
  return gvar;
}

llvm::Constant *libquixcc::LLVM14Codegen::gen(const ir::delta::Number *node) {
  uint8_t bits = get_numbits(node->value);

  if (node->value.find(".") != std::string::npos) {
    if (bits <= 32)
      return llvm::ConstantFP::get(
          *m_ctx->m_ctx,
          llvm::APFloat(llvm::APFloat::IEEEsingle(), node->value));
    else
      return llvm::ConstantFP::get(
          *m_ctx->m_ctx,
          llvm::APFloat(llvm::APFloat::IEEEdouble(), node->value));
  }

  switch (bits) {
    case 1:
    case 8:
    case 16:
    case 32:
      return llvm::ConstantInt::get(*m_ctx->m_ctx,
                                    llvm::APInt(32, node->value, 10));
    case 64:
      return llvm::ConstantInt::get(*m_ctx->m_ctx,
                                    llvm::APInt(64, node->value, 10));
    case 128:
      return llvm::ConstantInt::get(*m_ctx->m_ctx,
                                    llvm::APInt(128, node->value, 10));
    default:
      throw std::runtime_error("Codegen failed: Number type not supported");
  }
}

llvm::Constant *libquixcc::LLVM14Codegen::gen(const ir::delta::String *node) {
  llvm::Constant *zero = llvm::Constant::getNullValue(
      llvm::IntegerType::getInt32Ty(*m_ctx->m_ctx));
  llvm::Constant *indices[] = {zero, zero};

  auto gvar = m_ctx->m_builder->CreateGlobalString(node->value, "", 0,
                                                   m_ctx->m_module.get());
  gvar->setLinkage(llvm::GlobalValue::PrivateLinkage);

  return llvm::ConstantExpr::getGetElementPtr(gvar->getValueType(), gvar,
                                              indices);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(
    const libquixcc::ir::delta::List *node) {
  std::vector<llvm::Value *> values;
  for (auto &elem : node->values) values.push_back(gen(elem));

  if (values.empty())
    throw std::runtime_error("Codegen failed: Can not use empty list");

  llvm::Value *arr = m_ctx->m_builder->CreateAlloca(
      llvm::ArrayType::get(values[0]->getType(), values.size()));

  for (size_t i = 0; i < values.size(); i++) {
    auto ptr = m_ctx->m_builder->CreateGEP(
        arr->getType()->getPointerElementType(), arr,
        {llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), 0),
         llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), i)});
    m_ctx->m_builder->CreateStore(values[i], ptr);
  }

  if (m_state.m_deref)
    return m_ctx->m_builder->CreateLoad(arr->getType()->getPointerElementType(),
                                        arr);
  else
    return arr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Ident *node) {
  if (!m_state.locals.empty() && !m_state.params.empty()) {
    if (m_state.params.top().contains(node->name))
      return m_state.params.top()[node->name];

    if (m_state.locals.top().contains(node->name)) {
      auto v = m_state.locals.top()[node->name];
      auto t =
          m_state.locals.top()[node->name]->getType()->getPointerElementType();
      if (m_state.m_deref)
        return m_ctx->m_builder->CreateLoad(t, v);
      else
        return v;
    }
  }

  if (m_state.globals.contains(node->name)) {
    auto v = m_state.globals[node->name];
    auto t = m_state.globals[node->name]->getType()->getPointerElementType();
    if (m_state.m_deref)
      return m_ctx->m_builder->CreateLoad(t, v);
    else
      return v;
  } else if (m_state.functions.contains(node->name))
    return m_state.functions[node->name];
  else
    throw std::runtime_error("Codegen failed: Identifier not found: " +
                             node->name);
}

llvm::Value *libquixcc::LLVM14Codegen::special_load(
    const ir::delta::Expr *node) {
  if (!node->is<Index>()) return gen(node);

  size_t rank = 0;
  const Value *n = node->as<Index>();

  while (n->is<Index>()) {
    n = n->as<Index>()->expr;
    rank++;
  }

  size_t old = m_state.index_rank;
  m_state.index_rank = rank;
  llvm::Value *v = gen(node);
  m_state.index_rank = old;

  return v;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Assign *node) {
  bool old = m_state.m_deref;

  m_state.m_deref = false;
  auto ptr = special_load(node->var);

  // for (size_t i = 0; i < node->rank; i++)
  // ptr = m_ctx->m_builder->CreateLoad(ptr->getType()->getPointerElementType(),
  // ptr);

  m_state.m_deref = true;
  auto v = gen(node->value);
  m_state.m_deref = old;

  m_ctx->m_builder->CreateStore(v, ptr);

  return v;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(
    const libquixcc::ir::delta::PostInc *node) {
  bool old = m_state.m_deref;
  m_state.m_deref = false;
  auto ptr = special_load(node->var);

  auto v = m_ctx->m_builder->CreateLoad(ptr->getType()->getPointerElementType(),
                                        ptr);
  auto inc = m_ctx->m_builder->CreateAdd(
      v, llvm::ConstantInt::get(v->getType(), 1, true));
  m_ctx->m_builder->CreateStore(inc, ptr);

  m_state.m_deref = old;
  return v;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(
    const libquixcc::ir::delta::PostDec *node) {
  bool old = m_state.m_deref;
  m_state.m_deref = false;
  auto ptr = special_load(node->var);

  auto v = m_ctx->m_builder->CreateLoad(ptr->getType()->getPointerElementType(),
                                        ptr);
  auto dec = m_ctx->m_builder->CreateSub(
      v, llvm::ConstantInt::get(v->getType(), 1, true));
  m_ctx->m_builder->CreateStore(dec, ptr);

  m_state.m_deref = old;
  return v;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(
    const libquixcc::ir::delta::AddressOf *node) {
  bool old = m_state.m_deref;
  m_state.m_deref = false;
  auto ptr = gen(node->lhs);
  m_state.m_deref = old;

  return ptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(
    const libquixcc::ir::delta::Deref *node) {
  auto ptr = gen(node->lhs);
  return m_ctx->m_builder->CreateLoad(ptr->getType()->getPointerElementType(),
                                      ptr);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(
    const libquixcc::ir::delta::Member *node) {
  bool old = m_state.m_deref;
  m_state.m_deref = false;
  auto ptr = gen(node->lhs);
  m_state.m_deref = old;

  auto t = ptr->getType();
  while (!t->isStructTy()) t = t->getPointerElementType();

  while (1) {
    if (ptr->getType()->isPointerTy() &&
        ptr->getType()->getPointerElementType()->isStructTy())
      break;

    ptr = m_ctx->m_builder->CreateLoad(ptr->getType()->getPointerElementType(),
                                       ptr);
  }

  auto eptr = m_ctx->m_builder->CreateStructGEP(t, ptr, node->field);

  if (m_state.m_deref)
    return m_ctx->m_builder->CreateLoad(
        eptr->getType()->getPointerElementType(), eptr);
  else
    return eptr;
}

llvm::Value *libquixcc::LLVM14Codegen::bounds_wrap(llvm::Value *i,
                                                   llvm::Value *n) {
  ///========================================================================
  /// BEGIN: EXPERIMENTAL SECURITY FEATURE

  auto wrapped = m_ctx->m_builder->CreateURem(i, n);

  /// This is a security feature that prevents the user from accessing
  /// memory outside of the array bounds. This is a very simple check
  /// that only works for statically allocated arrays. It is not a
  /// replacement for proper bounds checking.

  /// Given a bounded array of size N and an unsigned Index I, we compute
  /// the effective index as (I % N). This guarantees that `out-of-bounds`
  /// accesses are `contained` within the array. This protection may
  /// make certain types of buffer overflow attacks infeasible.
  /// Meaning that an attacker's damage is limited to corrupting elements
  /// (likely of the same type) within the array. This also makes the behavior
  /// well-defined, no need for manaully modulo-indexing into any arrays.
  ///========================================================================

  return wrapped;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Index *node) {
  llvm::Value *v = nullptr;
  auto zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), 0);

  bool old = m_state.m_deref;
  m_state.m_deref = true;
  auto i = gen(node->index);

  m_state.m_deref = false;
  auto e = gen(node->expr);
  m_state.m_deref = old;

  bool is_inarr = m_state.index_rank > 1;
  m_state.index_rank--;

  if (e->getType()->isPointerTy() &&
      e->getType()->getPointerElementType()->isArrayTy()) {
    llvm::ConstantInt *mod_n = nullptr;
    uint64_t n = e->getType()->getPointerElementType()->getArrayNumElements();

    auto it = i->getType();
    if (it->isIntegerTy(128)) {
      mod_n = llvm::ConstantInt::get(llvm::Type::getInt128Ty(*m_ctx->m_ctx), n);
    } else if (it->isIntegerTy(64))
      mod_n = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*m_ctx->m_ctx), n);
    else if (it->isIntegerTy(32))
      mod_n = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), n);
    else if (it->isIntegerTy(16))
      mod_n = llvm::ConstantInt::get(llvm::Type::getInt16Ty(*m_ctx->m_ctx), n);
    else if (it->isIntegerTy(8))
      mod_n = llvm::ConstantInt::get(llvm::Type::getInt8Ty(*m_ctx->m_ctx), n);
    else if (it->isIntegerTy(1))
      mod_n = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*m_ctx->m_ctx), n);
    else
      throw std::runtime_error("Codegen failed: Indexing TYPE not supported");

    auto red = bounds_wrap(i, mod_n);
    v = m_ctx->m_builder->CreateGEP(e->getType()->getPointerElementType(), e,
                                    {zero, red});

    if (!m_state.m_deref) {
      return v;
    }
    auto tv = v->getType()->getPointerElementType();
    return m_ctx->m_builder->CreateLoad(tv, v);
  } else if (e->getType()->isPointerTy()) {
    auto ev = e->getType()->getPointerElementType();
    v = m_ctx->m_builder->CreateGEP(ev, e, i);
    if (!m_state.m_deref && !is_inarr) {
      return v;
    }
    auto tv = v->getType()->getPointerElementType();
    return m_ctx->m_builder->CreateLoad(tv, v);
  } else {
    throw std::runtime_error("Codegen failed: Indexing TYPE not supported");
  }
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::SCast *node) {
  auto v = gen(node->value);
  auto t = gent(node->type);

  if (v->getType()->isIntegerTy(1))
    return m_ctx->m_builder->CreateZExtOrTrunc(v, t);

  return m_ctx->m_builder->CreateSExtOrTrunc(v, t);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::UCast *node) {
  return m_ctx->m_builder->CreateZExtOrTrunc(gen(node->value),
                                             gent(node->type));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::PtrICast *node) {
  return m_ctx->m_builder->CreatePtrToInt(
      gen(node->value), llvm::Type::getInt64Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::IPtrCast *node) {
  return m_ctx->m_builder->CreateIntToPtr(gen(node->value), gent(node->type));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Bitcast *node) {
  return m_ctx->m_builder->CreateBitCast(gen(node->value), gent(node->type));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::IfElse *node) {
  auto func = m_ctx->m_builder->GetInsertBlock()->getParent();
  bool old2 = m_state.incond;
  m_state.incond = true;

  llvm::BasicBlock *thenBB =
      llvm::BasicBlock::Create(*m_ctx->m_ctx, "if.then", func);
  llvm::BasicBlock *elseBB =
      llvm::BasicBlock::Create(*m_ctx->m_ctx, "if.else", func);
  llvm::BasicBlock *endBB =
      llvm::BasicBlock::Create(*m_ctx->m_ctx, "if.end", func);

  auto cond = gen(node->cond);
  m_ctx->m_builder->CreateCondBr(cond, thenBB, elseBB);

  bool old1 = m_state.terminate_early;
  m_state.terminate_early = false;
  m_ctx->m_builder->SetInsertPoint(thenBB);
  gen(node->then);
  if (!m_state.terminate_early) m_ctx->m_builder->CreateBr(endBB);

  m_state.terminate_early = false;
  m_ctx->m_builder->SetInsertPoint(elseBB);
  gen(node->els);
  if (!m_state.terminate_early) m_ctx->m_builder->CreateBr(endBB);

  m_ctx->m_builder->SetInsertPoint(endBB);

  m_state.terminate_early = old1;
  m_state.incond = old2;

  return nullptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::While *node) {
  auto func = m_ctx->m_builder->GetInsertBlock()->getParent();

  llvm::BasicBlock *condBB =
      llvm::BasicBlock::Create(*m_ctx->m_ctx, "while.cond", func);
  llvm::BasicBlock *bodyBB =
      llvm::BasicBlock::Create(*m_ctx->m_ctx, "while.body", func);
  llvm::BasicBlock *endBB =
      llvm::BasicBlock::Create(*m_ctx->m_ctx, "while.end", func);

  m_ctx->m_builder->CreateBr(condBB);
  m_ctx->m_builder->SetInsertPoint(condBB);

  auto cond = gen(node->cond);
  m_ctx->m_builder->CreateCondBr(cond, bodyBB, endBB);

  m_ctx->m_builder->SetInsertPoint(bodyBB);
  gen(node->body);

  m_ctx->m_builder->CreateBr(condBB);

  m_ctx->m_builder->SetInsertPoint(endBB);

  return nullptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Jmp *node) {
  if (m_state.labels.empty())
    throw std::runtime_error("Codegen failed: Can not jump outside of segment");

  if (!m_state.labels.top().contains(node->target))
    throw std::runtime_error("Codegen failed: Label not found: " +
                             node->target);

  return m_ctx->m_builder->CreateBr(m_state.labels.top()[node->target]);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Label *node) {
  if (m_state.labels.empty())
    throw std::runtime_error(
        "Codegen failed: Can not create label outside of segment");

  if (m_state.labels.top().contains(node->name))
    throw std::runtime_error("Codegen failed: Label already exists: " +
                             node->name);

  auto func = m_ctx->m_builder->GetInsertBlock()->getParent();
  auto bb = llvm::BasicBlock::Create(*m_ctx->m_ctx, node->name, func);

  m_state.labels.top()[node->name] = bb;
  m_ctx->m_builder->CreateBr(bb);

  m_ctx->m_builder->SetInsertPoint(bb);

  gen(node->code);

  return bb;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Ret *node) {
  if (m_state.incond) m_state.terminate_early = true;

  if (node->value)
    m_ctx->m_builder->CreateStore(gen(node->value),
                                  m_state.locals.top()["__retval"]);

  return m_ctx->m_builder->CreateBr(m_state.labels.top().at("__ret"));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Call *node) {
  std::vector<llvm::Value *> args;
  for (auto &arg : node->args) args.push_back(gen(arg));

  llvm::FunctionType *ft = static_cast<llvm::FunctionType *>(gent(node->ftype));

  llvm::Value *callee =
      m_ctx->m_module->getOrInsertFunction(node->callee, ft).getCallee();
  if (!callee)
    throw std::runtime_error("Codegen failed: Function not found: " +
                             node->callee);

  return m_ctx->m_builder->CreateCall(ft, callee, args);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::PtrCall *node) {
  throw std::runtime_error("PtrCall not implemented");
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Halt *node) {
  throw std::runtime_error("Halt not implemented");
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const libquixcc::ir::delta::Break *node)
{
  throw std::runtime_error("Break not implemented");
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const libquixcc::ir::delta::Continue *node)
{
  throw std::runtime_error("Continue not implemented");
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const libquixcc::ir::delta::Switch *node)
{
  throw std::runtime_error("Switch not implemented");
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const libquixcc::ir::delta::Case *node)
{
  throw std::runtime_error("Case not implemented");
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Block *node) {
  for (auto child : node->stmts) gen(child);

  return nullptr;
}

llvm::Function *libquixcc::LLVM14Codegen::gen(const ir::delta::Segment *node) {
  std::vector<llvm::Type *> types;
  for (auto &param : node->params) types.push_back(gent(param.second));

  llvm::FunctionType *ftype =
      llvm::FunctionType::get(gent(node->ret), types, node->variadic);
  llvm::Function *func;

  if (!m_ctx->m_module->getFunction(m_state.name)) {
    if (m_state.m_pub)
      func = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage,
                                    m_state.name, m_ctx->m_module.get());
    else
      func = llvm::Function::Create(ftype, llvm::Function::PrivateLinkage,
                                    m_state.name, m_ctx->m_module.get());
  } else {
    func = m_ctx->m_module->getFunction(m_state.name);

    if (m_state.m_pub)
      func->setLinkage(llvm::Function::ExternalLinkage);
    else
      func->setLinkage(llvm::Function::PrivateLinkage);
  }

  if (node->block) {
    /* Finite-Stateful gencode */
    m_state.labels.push({});
    m_state.locals.push({});
    m_state.params.push({});

    llvm::BasicBlock *bb =
        llvm::BasicBlock::Create(*m_ctx->m_ctx, "entry", func);
    m_ctx->m_builder->SetInsertPoint(bb);

    for (auto &arg : func->args()) {
      auto &param = node->params[arg.getArgNo()];
      arg.setName(param.first);
      m_state.params.top()[param.first] = &arg;
    }

    if (!node->ret->is<Void>()) {
      auto retval =
          m_ctx->m_builder->CreateAlloca(gent(node->ret), nullptr, "__retval");
      m_state.locals.top()["__retval"] = retval;
    }

    llvm::BasicBlock *retBB =
        llvm::BasicBlock::Create(*m_ctx->m_ctx, "__ret", func);
    m_ctx->m_builder->SetInsertPoint(retBB);
    if (!node->ret->is<Void>()) {
      auto retval = m_ctx->m_builder->CreateLoad(
          gent(node->ret), m_state.locals.top()["__retval"]);
      m_ctx->m_builder->CreateRet(retval);
    } else {
      m_ctx->m_builder->CreateRetVoid();
    }

    m_state.labels.top()["__ret"] = retBB;

    m_ctx->m_builder->SetInsertPoint(bb);

    gen(node->block);

    m_state.params.pop();
    m_state.locals.pop();
    m_state.labels.pop();
  }

  return func;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Asm *node) {
  std::string constraints;
  bool sideeffects = false;
  bool alignstack = false;

  /// TODO: build out parameters in LLVM inline asm format

  llvm::InlineAsm *asm_ = llvm::InlineAsm::get(
      llvm::FunctionType::get(llvm::Type::getVoidTy(*m_ctx->m_ctx), false),
      node->asm_str, constraints, sideeffects, alignstack);

  return m_ctx->m_builder->CreateCall(asm_, {});
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Add *node) {
  return m_ctx->m_builder->CreateAdd(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Sub *node) {
  return m_ctx->m_builder->CreateSub(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Mul *node) {
  return m_ctx->m_builder->CreateMul(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Div *node) {
  return m_ctx->m_builder->CreateSDiv(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Mod *node) {
  return m_ctx->m_builder->CreateSRem(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::BitAnd *node) {
  return m_ctx->m_builder->CreateAnd(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::BitOr *node) {
  return m_ctx->m_builder->CreateOr(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::BitXor *node) {
  return m_ctx->m_builder->CreateXor(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::BitNot *node) {
  return m_ctx->m_builder->CreateNot(gen(node->operand));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Shl *node) {
  return m_ctx->m_builder->CreateShl(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Shr *node) {
  auto l = gen(node->lhs);
  auto r = gen(node->rhs);

  if (node->lhs->infer()->is_signed())  // signed
    return m_ctx->m_builder->CreateAShr(l, r);
  else  // unsigned
    return m_ctx->m_builder->CreateLShr(l, r);
}

#define ROTL(x, n, w) (((x) << (n)) | ((x) >> ((w) - (n))))
#define ROTR(x, n, w) (((x) >> (n)) | ((x) << ((w) - (n))))

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Rotl *node) {
  /// TODO: verify this formula

  auto lhs = gen(node->lhs);
  auto rhs = gen(node->rhs);

  auto bits = lhs->getType()->getIntegerBitWidth();

  auto ls = m_ctx->m_builder->CreateShl(lhs, rhs);
  auto sub = m_ctx->m_builder->CreateSub(
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), bits), rhs);
  auto rs = m_ctx->m_builder->CreateLShr(lhs, sub);

  return m_ctx->m_builder->CreateOr(ls, rs);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Rotr *node) {
  /// TODO: verify this formula

  auto lhs = gen(node->lhs);
  auto rhs = gen(node->rhs);

  auto bits = lhs->getType()->getIntegerBitWidth();

  auto rs = m_ctx->m_builder->CreateLShr(lhs, rhs);
  auto sub = m_ctx->m_builder->CreateSub(
      llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), bits), rhs);
  auto ls = m_ctx->m_builder->CreateShl(lhs, sub);

  return m_ctx->m_builder->CreateOr(ls, rs);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Eq *node) {
  return m_ctx->m_builder->CreateICmpEQ(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Ne *node) {
  return m_ctx->m_builder->CreateICmpNE(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Lt *node) {
  return m_ctx->m_builder->CreateICmpSLT(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Gt *node) {
  return m_ctx->m_builder->CreateICmpSGT(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Le *node) {
  return m_ctx->m_builder->CreateICmpSLE(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Ge *node) {
  return m_ctx->m_builder->CreateICmpSGE(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::And *node) {
  return m_ctx->m_builder->CreateLogicalAnd(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Or *node) {
  return m_ctx->m_builder->CreateLogicalOr(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Not *node) {
  return m_ctx->m_builder->CreateNot(gen(node->operand));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Xor *node) {
  return m_ctx->m_builder->CreateXor(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::RootNode *node) {
  for (auto child : node->children) gen(child);

  return nullptr;
}

llvm::Type *libquixcc::LLVM14Codegen::gent(
    const libquixcc::ir::delta::Type *n) {
  switch ((delta::NodeType)n->ntype) {
    case delta::NodeType::I1:
      return gen(n->as<I1>());
    case delta::NodeType::I8:
      return gen(n->as<I8>());
    case delta::NodeType::I16:
      return gen(n->as<I16>());
    case delta::NodeType::I32:
      return gen(n->as<I32>());
    case delta::NodeType::I64:
      return gen(n->as<I64>());
    case delta::NodeType::I128:
      return gen(n->as<I128>());
    case delta::NodeType::U8:
      return gen(n->as<U8>());
    case delta::NodeType::U16:
      return gen(n->as<U16>());
    case delta::NodeType::U32:
      return gen(n->as<U32>());
    case delta::NodeType::U64:
      return gen(n->as<U64>());
    case delta::NodeType::U128:
      return gen(n->as<U128>());
    case delta::NodeType::F32:
      return gen(n->as<F32>());
    case delta::NodeType::F64:
      return gen(n->as<F64>());
    case delta::NodeType::Void:
      return gen(n->as<Void>());
    case delta::NodeType::Ptr:
      return gen(n->as<Ptr>());
    case delta::NodeType::Packet:
      return gen(n->as<Packet>());
    case delta::NodeType::Array:
      return gen(n->as<Array>());
    case delta::NodeType::FType:
      return gen(n->as<FType>());
    default:
      throw std::runtime_error(
          "Codegen failed: codegen not implemented for type: " +
          std::to_string(n->ntype));
  }
}

llvm::Value *libquixcc::LLVM14Codegen::gen(
    const libquixcc::ir::delta::Value *n) {
  switch ((delta::NodeType)n->ntype) {
    case delta::NodeType::Local:
      return gen(n->as<Local>());
    case delta::NodeType::Global:
      return gen(n->as<Global>());
    case delta::NodeType::Number:
      return gen(n->as<Number>());
    case delta::NodeType::String:
      return gen(n->as<String>());
    case delta::NodeType::List:
      return gen(n->as<List>());
    case delta::NodeType::Ident:
      return gen(n->as<Ident>());
    case delta::NodeType::Assign:
      return gen(n->as<Assign>());
    case delta::NodeType::PostInc:
      return gen(n->as<PostInc>());
    case delta::NodeType::PostDec:
      return gen(n->as<PostDec>());
    case delta::NodeType::AddressOf:
      return gen(n->as<AddressOf>());
    case delta::NodeType::Deref:
      return gen(n->as<Deref>());
    case delta::NodeType::Member:
      return gen(n->as<Member>());
    case delta::NodeType::Index:
      return gen(n->as<Index>());
    case delta::NodeType::SCast:
      return gen(n->as<SCast>());
    case delta::NodeType::UCast:
      return gen(n->as<UCast>());
    case delta::NodeType::PtrICast:
      return gen(n->as<PtrICast>());
    case delta::NodeType::IPtrCast:
      return gen(n->as<IPtrCast>());
    case delta::NodeType::Bitcast:
      return gen(n->as<Bitcast>());
    case delta::NodeType::IfElse:
      return gen(n->as<IfElse>());
    case delta::NodeType::While:
      return gen(n->as<While>());
    case delta::NodeType::Jmp:
      return gen(n->as<Jmp>());
    case delta::NodeType::Label:
      return gen(n->as<Label>());
    case delta::NodeType::Ret:
      return gen(n->as<Ret>());
    case delta::NodeType::Call:
      return gen(n->as<Call>());
    case delta::NodeType::PtrCall:
      return gen(n->as<PtrCall>());
    case delta::NodeType::Halt:
      return gen(n->as<Halt>());
    case delta::NodeType::Break:
      return gen(n->as<Break>());
    case delta::NodeType::Continue:
      return gen(n->as<Continue>());
    case delta::NodeType::Switch:
      return gen(n->as<Switch>());
    case delta::NodeType::Case:
      return gen(n->as<Case>());
    case delta::NodeType::Block:
      return gen(n->as<Block>());
    case delta::NodeType::Segment:
      return gen(n->as<Segment>());
    case delta::NodeType::Asm:
      return gen(n->as<Asm>());
    case delta::NodeType::Add:
      return gen(n->as<Add>());
    case delta::NodeType::Sub:
      return gen(n->as<Sub>());
    case delta::NodeType::Mul:
      return gen(n->as<Mul>());
    case delta::NodeType::Div:
      return gen(n->as<Div>());
    case delta::NodeType::Mod:
      return gen(n->as<Mod>());
    case delta::NodeType::BitAnd:
      return gen(n->as<BitAnd>());
    case delta::NodeType::BitOr:
      return gen(n->as<BitOr>());
    case delta::NodeType::BitXor:
      return gen(n->as<BitXor>());
    case delta::NodeType::BitNot:
      return gen(n->as<BitNot>());
    case delta::NodeType::Shl:
      return gen(n->as<Shl>());
    case delta::NodeType::Shr:
      return gen(n->as<Shr>());
    case delta::NodeType::Rotl:
      return gen(n->as<Rotl>());
    case delta::NodeType::Rotr:
      return gen(n->as<Rotr>());
    case delta::NodeType::Eq:
      return gen(n->as<Eq>());
    case delta::NodeType::Ne:
      return gen(n->as<Ne>());
    case delta::NodeType::Lt:
      return gen(n->as<Lt>());
    case delta::NodeType::Gt:
      return gen(n->as<Gt>());
    case delta::NodeType::Le:
      return gen(n->as<Le>());
    case delta::NodeType::Ge:
      return gen(n->as<Ge>());
    case delta::NodeType::And:
      return gen(n->as<And>());
    case delta::NodeType::Or:
      return gen(n->as<Or>());
    case delta::NodeType::Not:
      return gen(n->as<Not>());
    case delta::NodeType::Xor:
      return gen(n->as<Xor>());
    case delta::NodeType::Root:
      return gen(n->as<RootNode>());
    case delta::NodeType::Packet:
    case delta::NodeType::PacketDef:
    case delta::NodeType::FType:
      return nullptr;

    default:
      throw std::runtime_error(
          "Codegen failed: codegen not implemented for value: " +
          std::to_string(n->ntype));
  }
}

bool libquixcc::LLVM14Codegen::codegen(
    const std::unique_ptr<libquixcc::ir::delta::IRDelta> &ir,
    libquixcc::LLVMContext &ctx) {
  LLVM14Codegen codegen(ctx);

  return codegen.gen(ir->root()), true; /* Errors -> exceptions */
}
