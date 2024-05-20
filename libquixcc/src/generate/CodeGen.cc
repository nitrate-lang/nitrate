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

#define QUIXCC_INTERNAL

#include <generate/CodeGen.h>
#include <llvm/IR/InlineAsm.h>
#include <mangle/Symbol.h>
#include <core/Logger.h>

#include <IR/delta/Type.h>
#include <IR/delta/Variable.h>
#include <IR/delta/Memory.h>
#include <IR/delta/Cast.h>
#include <IR/delta/Control.h>
#include <IR/delta/Segment.h>
#include <IR/delta/Math.h>

using namespace libquixcc;
using namespace libquixcc::ir;
using namespace libquixcc::ir::delta;

uint8_t get_numbits(std::string s);

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::I1 *node)
{
    return llvm::Type::getInt1Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::I8 *node)
{
    return llvm::Type::getInt8Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::I16 *node)
{
    return llvm::Type::getInt16Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::I32 *node)
{
    return llvm::Type::getInt32Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::I64 *node)
{
    return llvm::Type::getInt64Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::I128 *node)
{
    return llvm::Type::getInt128Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::U8 *node)
{
    return llvm::Type::getInt8Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::U16 *node)
{
    return llvm::Type::getInt16Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::U32 *node)
{
    return llvm::Type::getInt32Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::U64 *node)
{
    return llvm::Type::getInt64Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::U128 *node)
{
    return llvm::Type::getInt128Ty(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::F32 *node)
{
    return llvm::Type::getFloatTy(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::F64 *node)
{
    return llvm::Type::getDoubleTy(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::LLVM14Codegen::gen(const ir::delta::Void *node)
{
    return llvm::Type::getVoidTy(*m_ctx->m_ctx);
}

llvm::PointerType *libquixcc::LLVM14Codegen::gen(const ir::delta::Ptr *node)
{
    auto type = gent(node->type);
    if (type->isVoidTy())
        return llvm::PointerType::get(llvm::Type::getInt8Ty(*m_ctx->m_ctx), 0);
    else
        return llvm::PointerType::get(type, 0);
}

llvm::StructType *libquixcc::LLVM14Codegen::gen(const ir::delta::Packet *node)
{
    if (m_state.types.contains(node->name))
        return static_cast<llvm::StructType *>(m_state.types[node->name]);

    std::vector<llvm::Type *> types;
    for (auto &field : node->fields)
        types.push_back(gent(field.second));

    auto st = llvm::StructType::create(*m_ctx->m_ctx, types, node->name, true);
    m_state.types[node->name] = st;

    return st;
}

llvm::ArrayType *libquixcc::LLVM14Codegen::gen(const ir::delta::Array *node)
{
    return llvm::ArrayType::get(gent(node->type), node->size);
}

llvm::FunctionType *libquixcc::LLVM14Codegen::gen(const ir::delta::FType *node)
{
    std::vector<llvm::Type *> types;
    for (auto &param : node->params)
        types.push_back(gent(param));

    return llvm::FunctionType::get(gent(node->ret), types, node->variadic);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Local *node)
{
    auto type = gent(node->type);
    llvm::AllocaInst *alloca = m_ctx->m_builder->CreateAlloca(type, nullptr, node->name);
    m_state.locals.top()[node->name] = alloca;

    if (node->value)
    {
        auto v = gen(node->value);
        m_ctx->m_builder->CreateStore(v, alloca);
    }

    return alloca;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Global *node)
{
    bool old_pub = m_state.m_pub;

    if (node->_extern)
        m_state.m_pub = true;

    llvm::Type *type = gent(node->type);

    if (node->value && node->value->is<Segment>())
    {
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
        gvar->setLinkage(llvm::GlobalValue::PrivateLinkage);

    if (node->value)
    {
        gvar->setInitializer(static_cast<llvm::Constant *>(gen(node->value)));
    }
    else
    {
        if (type->isIntegerTy())
            gvar->setInitializer(llvm::ConstantInt::get(type, llvm::APInt(type->getPrimitiveSizeInBits(), 0, true)));
        else if (type->isFloatTy())
            gvar->setInitializer(llvm::ConstantFP::get(type, 0.0));
        else if (type->isPointerTy())
            gvar->setInitializer(llvm::ConstantPointerNull::get(static_cast<llvm::PointerType *>(type)));
        else if (type->isArrayTy() || type->isStructTy())
            gvar->setInitializer(llvm::ConstantAggregateZero::get(type));
        else
            gvar->setInitializer(llvm::Constant::getNullValue(type));
    }

    m_state.globals[node->name] = gvar;
    m_state.m_pub = old_pub;
    return gvar;
}

llvm::Constant *libquixcc::LLVM14Codegen::gen(const ir::delta::Number *node)
{
    uint8_t bits = get_numbits(node->value);

    if (node->value.contains("."))
    {
        if (bits <= 32)
            return llvm::ConstantFP::get(*m_ctx->m_ctx, llvm::APFloat(llvm::APFloat::IEEEsingle(), node->value));
        else
            return llvm::ConstantFP::get(*m_ctx->m_ctx, llvm::APFloat(llvm::APFloat::IEEEdouble(), node->value));
    }

    switch (bits)
    {
    case 1:
    case 8:
    case 16:
    case 32:
        return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(32, node->value, 10));
    case 64:
        return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(64, node->value, 10));
    case 128: /* TODO: get_numbits cant handle 128 bits */
        return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(128, node->value, 10));
    default:
        throw std::runtime_error("Codegen failed: Number type not supported");
    }
}

llvm::Constant *libquixcc::LLVM14Codegen::gen(const ir::delta::String *node)
{
    llvm::Constant *zero = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*m_ctx->m_ctx));
    llvm::Constant *indices[] = {zero, zero};

    auto gvar = m_ctx->m_builder->CreateGlobalString(node->value, "", 0, m_ctx->m_module.get());
    gvar->setLinkage(llvm::GlobalValue::PrivateLinkage);

    return llvm::ConstantExpr::getGetElementPtr(gvar->getValueType(), gvar, indices);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const libquixcc::ir::delta::List *node)
{
    std::vector<llvm::Value *> values;
    for (auto &elem : node->values)
        values.push_back(gen(elem));

    if (values.empty())
        throw std::runtime_error("Codegen failed: Can not use empty list");

    llvm::Value *arr = m_ctx->m_builder->CreateAlloca(llvm::ArrayType::get(values[0]->getType(), values.size()));

    for (size_t i = 0; i < values.size(); i++)
    {
        auto ptr = m_ctx->m_builder->CreateGEP(arr->getType()->getPointerElementType(), arr, {llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), 0), llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), i)});
        m_ctx->m_builder->CreateStore(values[i], ptr);
    }

    if (m_state.m_deref)
        return m_ctx->m_builder->CreateLoad(arr->getType()->getPointerElementType(), arr);
    else
        return arr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Ident *node)
{
    if (!m_state.locals.empty() && !m_state.params.empty())
    {
        if (m_state.params.top().contains(node->name))
        {
            auto v = m_state.params.top()[node->name];
            auto t = m_state.params.top()[node->name]->getType();
            // if (m_state.m_deref)
            //     return m_ctx->m_builder->CreateLoad(t, v);
            // else
            return v;
        }

        if (m_state.locals.top().contains(node->name))
        {
            auto v = m_state.locals.top()[node->name];
            auto t = m_state.locals.top()[node->name]->getType()->getPointerElementType();
            if (m_state.m_deref)
                return m_ctx->m_builder->CreateLoad(t, v);
            else
                return v;
        }
    }

    if (m_state.globals.contains(node->name))
    {
        auto v = m_state.globals[node->name];
        auto t = m_state.globals[node->name]->getType()->getPointerElementType();
        if (m_state.m_deref)
            return m_ctx->m_builder->CreateLoad(t, v);
        else
            return v;
    }
    else if (m_state.functions.contains(node->name))
        return m_state.functions[node->name];
    else
        throw std::runtime_error("Codegen failed: Identifier not found: " + node->name);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Assign *node)
{
    bool old = m_state.m_deref;
    m_state.m_deref = false;

    auto ptr = gen(node->var);
    m_state.m_deref = old;

    for (size_t i = 0; i < node->rank; i++)
        ptr = m_ctx->m_builder->CreateLoad(ptr->getType()->getPointerElementType(), ptr);

    auto v = gen(node->value);

    m_ctx->m_builder->CreateStore(v, ptr);

    return v;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const libquixcc::ir::delta::AddressOf *node)
{
    bool old = m_state.m_deref;
    m_state.m_deref = false;
    auto ptr = gen(node->lhs);
    m_state.m_deref = old;

    return ptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const libquixcc::ir::delta::Member *node)
{
    bool old = m_state.m_deref;
    m_state.m_deref = false;
    auto ptr = gen(node->lhs);
    m_state.m_deref = old;

    auto t = ptr->getType();
    while (!t->isStructTy())
        t = t->getPointerElementType();

    while (1)
    {
        if (ptr->getType()->isPointerTy() && ptr->getType()->getPointerElementType()->isStructTy())
            break;

        ptr = m_ctx->m_builder->CreateLoad(ptr->getType()->getPointerElementType(), ptr);
    }

    auto eptr = m_ctx->m_builder->CreateStructGEP(t, ptr, node->field);

    if (m_state.m_deref)
        return m_ctx->m_builder->CreateLoad(eptr->getType()->getPointerElementType(), eptr);
    else
        return eptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Index *node)
{
    bool old = m_state.m_deref;

    if (!node->expr->is<Index>())
        m_state.m_deref = false;
    auto e = gen(node->expr);
    m_state.m_deref = old;

    auto i = gen(node->index);

    if (!i->getType()->isIntegerTy(32))
    {
        if (i->getType()->isIntegerTy(1))
            i = m_ctx->m_builder->CreateZExtOrTrunc(i, llvm::Type::getInt32Ty(*m_ctx->m_ctx));
        else
            i = m_ctx->m_builder->CreateSExtOrTrunc(i, llvm::Type::getInt32Ty(*m_ctx->m_ctx));
    }

    if (e->getType()->getPointerElementType()->isArrayTy())
    {
        auto t = e->getType()->getPointerElementType();

        auto zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), 0);
        llvm::Value *v = m_ctx->m_builder->CreateGEP(t, e, {zero, i});

        if (m_state.m_deref)
            return m_ctx->m_builder->CreateLoad(v->getType()->getPointerElementType(), v);
        else
            return v;
    }
    else if (e->getType()->getPointerElementType()->isPointerTy())
    {
        llvm::Value *v = m_ctx->m_builder->CreateGEP(e->getType()->getPointerElementType(), e, i);

        if (m_state.m_deref)
            return m_ctx->m_builder->CreateLoad(v->getType()->getPointerElementType(), v);
        else
            return v;
    }
    else if (e->getType()->isPointerTy())
    {
        auto t = e->getType()->getPointerElementType();
        llvm::Value *v = m_ctx->m_builder->CreateGEP(t, e, i);

        if (m_state.m_deref)
            return m_ctx->m_builder->CreateLoad(v->getType()->getPointerElementType(), v);
        else
            return v;
    }
    else
    {
        throw std::runtime_error("Codegen failed: Indexing not supported");
    }
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::SCast *node)
{
    auto v = gen(node->value);
    auto t = gent(node->type);

    if (v->getType()->isIntegerTy(1))
        return m_ctx->m_builder->CreateZExtOrTrunc(v, t);

    return m_ctx->m_builder->CreateSExtOrTrunc(v, t);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::UCast *node)
{
    return m_ctx->m_builder->CreateZExtOrTrunc(gen(node->value), gent(node->type));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::PtrICast *node)
{
    return m_ctx->m_builder->CreatePtrToInt(gen(node->value), llvm::Type::getInt64Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::IPtrCast *node)
{
    return m_ctx->m_builder->CreateIntToPtr(gen(node->value), gent(node->type));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Bitcast *node)
{
    return m_ctx->m_builder->CreateBitCast(gen(node->value), gent(node->type));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::IfElse *node)
{
    auto func = m_ctx->m_builder->GetInsertBlock()->getParent();
    bool old2 = m_state.incond;
    m_state.incond = true;

    llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "if.then", func);
    llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "if.else", func);
    llvm::BasicBlock *endBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "if.end", func);

    auto cond = gen(node->cond);
    m_ctx->m_builder->CreateCondBr(cond, thenBB, elseBB);

    bool old1 = m_state.terminate_early;
    m_state.terminate_early = false;
    m_ctx->m_builder->SetInsertPoint(thenBB);
    gen(node->then);
    if (!m_state.terminate_early)
        m_ctx->m_builder->CreateBr(endBB);

    m_state.terminate_early = false;
    m_ctx->m_builder->SetInsertPoint(elseBB);
    gen(node->els);
    if (!m_state.terminate_early)
        m_ctx->m_builder->CreateBr(endBB);

    m_ctx->m_builder->SetInsertPoint(endBB);

    m_state.terminate_early = old1;
    m_state.incond = old2;

    return nullptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::While *node)
{
    auto func = m_ctx->m_builder->GetInsertBlock()->getParent();

    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "while.cond", func);
    llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "while.body", func);
    llvm::BasicBlock *endBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "while.end", func);

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

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Jmp *node)
{
    if (m_state.labels.empty())
        throw std::runtime_error("Codegen failed: Can not jump outside of segment");

    if (!m_state.labels.top().contains(node->target))
        throw std::runtime_error("Codegen failed: Label not found: " + node->target);

    return m_ctx->m_builder->CreateBr(m_state.labels.top()[node->target]);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Label *node)
{
    /// TODO: verify

    if (m_state.labels.empty())
        throw std::runtime_error("Codegen failed: Can not create label outside of segment");

    if (m_state.labels.top().contains(node->name))
        throw std::runtime_error("Codegen failed: Label already exists: " + node->name);

    auto func = m_ctx->m_builder->GetInsertBlock()->getParent();
    auto bb = llvm::BasicBlock::Create(*m_ctx->m_ctx, node->name, func);

    m_state.labels.top()[node->name] = bb;
    m_ctx->m_builder->CreateBr(bb);

    m_ctx->m_builder->SetInsertPoint(bb);

    gen(node->code);

    return bb;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Ret *node)
{
    if (m_state.incond)
        m_state.terminate_early = true;

    if (node->value)
        m_ctx->m_builder->CreateStore(gen(node->value), m_state.locals.top()["__retval"]);

    return m_ctx->m_builder->CreateBr(m_state.labels.top().at("__ret"));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Call *node)
{
    std::vector<llvm::Value *> args;
    for (auto &arg : node->args)
        args.push_back(gen(arg));

    llvm::FunctionType *ft = static_cast<llvm::FunctionType *>(gent(node->ftype));

    llvm::Value *callee = m_ctx->m_module->getOrInsertFunction(node->callee, ft).getCallee();
    if (!callee)
        throw std::runtime_error("Codegen failed: Function not found: " + node->callee);

    return m_ctx->m_builder->CreateCall(ft, callee, args);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::PtrCall *node)
{
    throw std::runtime_error("PtrCall not implemented");
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Halt *node)
{
    throw std::runtime_error("Halt not implemented");
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Block *node)
{
    for (auto child : node->stmts)
        gen(child);

    return nullptr;
}

llvm::Function *libquixcc::LLVM14Codegen::gen(const ir::delta::Segment *node)
{
    std::vector<llvm::Type *> types;
    for (auto &param : node->params)
        types.push_back(gent(param.second));

    llvm::FunctionType *ftype = llvm::FunctionType::get(gent(node->ret), types, node->variadic);
    llvm::Function *func;

    if (!m_ctx->m_module->getFunction(m_state.name))
    {
        if (m_state.m_pub)
            func = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, m_state.name, m_ctx->m_module.get());
        else
            func = llvm::Function::Create(ftype, llvm::Function::InternalLinkage, m_state.name, m_ctx->m_module.get());
    }
    else
    {
        func = m_ctx->m_module->getFunction(m_state.name);
    }

    if (node->block)
    {
        m_state.labels.push({});
        m_state.locals.push({});
        m_state.params.push({});

        llvm::BasicBlock *bb = llvm::BasicBlock::Create(*m_ctx->m_ctx, "entry", func);
        m_ctx->m_builder->SetInsertPoint(bb);

        for (auto &arg : func->args())
        {
            auto &param = node->params[arg.getArgNo()];

            if (param.second->is<Ptr>())
            {
                arg.setName(param.first);
                m_state.params.top()[param.first] = &arg;
            }
            else
            {
                arg.setName("__" + param.first);
                auto alloca = m_ctx->m_builder->CreateAlloca(arg.getType(), nullptr, param.first);
                m_ctx->m_builder->CreateStore(&arg, alloca);
                m_state.locals.top()[param.first] = alloca;
            }
        }

        if (!node->ret->is<Void>())
        {
            auto retval = m_ctx->m_builder->CreateAlloca(gent(node->ret), nullptr, "__retval");
            m_state.locals.top()["__retval"] = retval;
        }

        llvm::BasicBlock *retBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "__ret", func);
        m_ctx->m_builder->SetInsertPoint(retBB);
        if (!node->ret->is<Void>())
        {
            auto retval = m_ctx->m_builder->CreateLoad(gent(node->ret), m_state.locals.top()["__retval"]);
            m_ctx->m_builder->CreateRet(retval);
        }
        else
        {
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

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Add *node)
{
    return m_ctx->m_builder->CreateAdd(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Sub *node)
{
    return m_ctx->m_builder->CreateSub(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Mul *node)
{
    return m_ctx->m_builder->CreateMul(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Div *node)
{
    return m_ctx->m_builder->CreateSDiv(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Mod *node)
{
    return m_ctx->m_builder->CreateSRem(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::BitAnd *node)
{
    return m_ctx->m_builder->CreateAnd(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::BitOr *node)
{
    return m_ctx->m_builder->CreateOr(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::BitXor *node)
{
    return m_ctx->m_builder->CreateXor(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::BitNot *node)
{
    return m_ctx->m_builder->CreateNeg(gen(node->operand));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Shl *node)
{
    return m_ctx->m_builder->CreateShl(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Shr *node)
{
    return m_ctx->m_builder->CreateAShr(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Rotl *node)
{
    /// TODO: verify this formula

    auto lhs = gen(node->lhs);
    auto rhs = gen(node->rhs);

    auto bits = lhs->getType()->getIntegerBitWidth();
    auto n = m_ctx->m_builder->CreateURem(rhs, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), bits));
    auto w = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), bits);

    auto shl = m_ctx->m_builder->CreateShl(lhs, n);
    auto shr = m_ctx->m_builder->CreateLShr(lhs, m_ctx->m_builder->CreateSub(w, n));

    return m_ctx->m_builder->CreateOr(shl, shr);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Rotr *node)
{
    /// TODO: verify this formula

    auto lhs = gen(node->lhs);
    auto rhs = gen(node->rhs);

    auto bits = lhs->getType()->getIntegerBitWidth();
    auto n = m_ctx->m_builder->CreateURem(rhs, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), bits));
    auto w = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*m_ctx->m_ctx), bits);

    auto shr = m_ctx->m_builder->CreateLShr(lhs, n);
    auto shl = m_ctx->m_builder->CreateShl(lhs, m_ctx->m_builder->CreateSub(w, n));

    return m_ctx->m_builder->CreateOr(shl, shr);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Eq *node)
{
    return m_ctx->m_builder->CreateICmpEQ(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Ne *node)
{
    return m_ctx->m_builder->CreateICmpNE(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Lt *node)
{
    return m_ctx->m_builder->CreateICmpSLT(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Gt *node)
{
    return m_ctx->m_builder->CreateICmpSGT(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Le *node)
{
    return m_ctx->m_builder->CreateICmpSLE(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Ge *node)
{
    return m_ctx->m_builder->CreateICmpSGE(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::And *node)
{
    return m_ctx->m_builder->CreateLogicalAnd(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Or *node)
{
    return m_ctx->m_builder->CreateLogicalOr(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Not *node)
{
    return m_ctx->m_builder->CreateNot(gen(node->operand));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Xor *node)
{
    return m_ctx->m_builder->CreateXor(gen(node->lhs), gen(node->rhs));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::RootNode *node)
{
    for (auto child : node->children)
        gen(child);

    return nullptr;
}

#define match(type)    \
    if (n->is<type>()) \
    return gen(n->as<type>())

#define ignore(type)   \
    if (n->is<type>()) \
    return nullptr

llvm::Type *libquixcc::LLVM14Codegen::gent(const libquixcc::ir::delta::Type *n)
{
    match(I1);
    match(I8);
    match(I16);
    match(I32);
    match(I64);
    match(I128);
    match(U8);
    match(U16);
    match(U32);
    match(U64);
    match(U128);
    match(F32);
    match(F64);
    match(Void);
    match(Ptr);
    match(Packet);
    match(Array);
    match(FType);

    throw std::runtime_error("Codegen failed: codegen not implemented for type: " + std::to_string(n->ntype));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const libquixcc::ir::delta::Value *n)
{
    match(Local);
    match(Global);
    match(Number);
    match(String);
    match(List);
    match(Ident);
    match(Assign);
    match(AddressOf);
    match(Member);
    match(Index);
    match(SCast);
    match(UCast);
    match(PtrICast);
    match(IPtrCast);
    match(Bitcast);
    match(IfElse);
    match(While);
    match(Jmp);
    match(Label);
    match(Ret);
    match(Call);
    match(PtrCall);
    match(Halt);
    match(Block);
    match(Segment);
    match(Add);
    match(Sub);
    match(Mul);
    match(Div);
    match(Mod);
    match(BitAnd);
    match(BitOr);
    match(BitXor);
    match(BitNot);
    match(Shl);
    match(Shr);
    match(Rotl);
    match(Rotr);
    match(Eq);
    match(Ne);
    match(Lt);
    match(Gt);
    match(Le);
    match(Ge);
    match(And);
    match(Or);
    match(Not);
    match(Xor);
    match(RootNode);

    ignore(Packet);

    throw std::runtime_error("Codegen failed: codegen not implemented for value: " + std::to_string(n->ntype));
}

bool libquixcc::LLVM14Codegen::codegen(const std::unique_ptr<libquixcc::ir::delta::IRDelta> &ir, libquixcc::LLVMContext &ctx)
{
    LLVM14Codegen codegen(ctx);

    return codegen.gen(ir->root()), true; /* Errors -> exceptions */
}
