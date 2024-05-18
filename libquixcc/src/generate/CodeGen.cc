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
    return llvm::PointerType::get(gent(node->type), 0);
}

llvm::StructType *libquixcc::LLVM14Codegen::gen(const ir::delta::Packet *node)
{
    std::vector<llvm::Type *> types;
    for (auto &field : node->fields)
        types.push_back(gent(field.second));

    return llvm::StructType::create(*m_ctx->m_ctx, types, node->name, true);
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

    return llvm::FunctionType::get(gent(node->ret), types, false);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Local *node)
{
    return nullptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Global *node)
{
    llvm::Type *type = gent(node->type);

    if (node->value && node->value->is<Segment>())
    {
        m_state.name = node->name;
        auto segment = node->value->as<Segment>();
        llvm::Function *func = gen(segment);
        m_state.name.clear();
        return func;
    }

    m_ctx->m_module->getOrInsertGlobal(node->name, type);
    llvm::GlobalVariable *gvar = m_ctx->m_module->getGlobalVariable(node->name);
    m_ctx->m_named_global_vars[node->name] = gvar;

    if (m_ctx->m_pub)
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
    case 8:
        return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(8, node->value, 10));
    case 16:
        return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(16, node->value, 10));
    case 32:
        return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(32, node->value, 10));
    case 64:
        return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(64, node->value, 10));
    case 128: /* TODO: get_numbits cant handle 128 bits */
        return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(128, node->value, 10));
    default:
        throw std::runtime_error("Codegen failed");
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

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Assign *node)
{
    return nullptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Load *node)
{
    return nullptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Index *node)
{
    return nullptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::SCast *node)
{
    return m_ctx->m_builder->CreateSExtOrTrunc(gen(node->value), gent(node->type));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::UCast *node)
{
    return m_ctx->m_builder->CreateZExtOrTrunc(gen(node->value), gent(node->type));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::PtrICast *node)
{
    return m_ctx->m_builder->CreateIntToPtr(gen(node->value), gent(node->type));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::IPtrCast *node)
{
    return m_ctx->m_builder->CreatePtrToInt(gen(node->value), gent(node->type));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Bitcast *node)
{
    return m_ctx->m_builder->CreateBitCast(gen(node->value), gent(node->type));
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::IfElse *node)
{
    return nullptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::While *node)
{
    return nullptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Jmp *node)
{
    return nullptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Label *node)
{
    return nullptr;
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Ret *node)
{
    if (node->value)
        return m_ctx->m_builder->CreateRet(gen(node->value));
    else
        return m_ctx->m_builder->CreateRetVoid();
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Call *node)
{
    std::vector<llvm::Value *> args;
    for (auto &arg : node->args)
        args.push_back(gen(arg));

    auto callee = m_ctx->m_module->getFunction(node->callee);
    if (!callee)
        throw std::runtime_error("Codegen failed");

    return m_ctx->m_builder->CreateCall(callee, args);
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::PtrCall *node)
{
    throw std::runtime_error("PtrCall not implemented");
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Halt *node)
{
    throw std::runtime_error("Halt not implemented");
}

llvm::Function *libquixcc::LLVM14Codegen::gen(const ir::delta::Segment *node)
{
    std::vector<llvm::Type *> types;
    for (auto &param : node->params)
        types.push_back(gent(param.second));

    llvm::FunctionType *ftype = llvm::FunctionType::get(gent(node->ret), types, false);
    llvm::Function *func = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, m_state.name, m_ctx->m_module.get());

    llvm::BasicBlock *bb = llvm::BasicBlock::Create(*m_ctx->m_ctx, "entry", func);
    m_ctx->m_builder->SetInsertPoint(bb);

    for (auto it = func->arg_begin(); it != func->arg_end(); ++it)
    {
        auto &param = node->params[it->getArgNo()];
        it->setName(param.first);
    }

    gen(node->block);

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
    throw std::runtime_error("Rotl not implemented");
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const ir::delta::Rotr *node)
{
    throw std::runtime_error("Rotr not implemented");
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
    {
        if (!gen(child))
            throw std::runtime_error("Codegen failed");
    }

    return nullptr;
}

#define match(type)    \
    if (n->is<type>()) \
    return gen(n->as<type>())

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

    throw std::runtime_error("Codegen failed");
}

llvm::Value *libquixcc::LLVM14Codegen::gen(const libquixcc::ir::delta::Value *n)
{
    match(Local);
    match(Global);
    match(Number);
    match(String);
    match(Assign);
    match(Load);
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

    throw std::runtime_error("Codegen failed");
}

bool libquixcc::LLVM14Codegen::codegen(const std::unique_ptr<libquixcc::ir::delta::IRDelta> &ir, libquixcc::LLVMContext &ctx)
{
    LLVM14Codegen codegen(ctx);

    return codegen.gen(ir->root()), true; /* Errors -> exceptions */
}
