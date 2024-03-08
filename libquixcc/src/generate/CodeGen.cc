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
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#define QUIXCC_INTERNAL

#include <generate/CodeGen.h>
#include <parse/nodes/AllNodes.h>
#include <mangle/Symbol.h>

static uint8_t get_numbits(const std::string &s)
{
    if (s.find('.') != std::string::npos)
    {
        float f0 = std::stof(s);
        double f1 = std::stod(s);
        const double delta = 0.0000001;

        return std::abs(f0 - f1) < delta ? 64 : 32;
    }

    uint64_t val = std::stoull(s);
    uint8_t bits = 0;
    while (val)
    {
        val >>= 1;
        bits++;
    }

    if (bits > 32)
        return 64;
    else if (bits > 16)
        return 32;
    else if (bits > 8)
        return 16;
    return 8;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::BlockNode *node) const
{
    for (auto &stmt : node->m_stmts)
    {
        llvm::Value *val = stmt->codegen(*this);
        if (!val)
            return nullptr;
    }
    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::ConstUnaryExprNode *node) const
{
    llvm::Constant *expr = node->m_expr->codegen(*this);
    if (!expr)
        return nullptr;

    switch (node->m_op)
    {
    case Operator::Minus:
        return llvm::ConstantExpr::getNeg(expr);
    case Operator::Plus:
        return expr;
    case Operator::BitNot:
        return llvm::ConstantExpr::getNot(expr);
    case Operator::Not:
        return llvm::ConstantExpr::getICmp(llvm::CmpInst::Predicate::ICMP_EQ, expr, llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 0, false)));
    case Operator::Increment:
        return llvm::ConstantExpr::getAdd(expr, llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 1, false)));
    case Operator::Decrement:
        return llvm::ConstantExpr::getSub(expr, llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 1, false)));
    default:
        return nullptr;
    }
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::ConstBinaryExprNode *node) const
{
    llvm::Constant *lhs = node->m_lhs->codegen(*this);
    if (!lhs)
        return nullptr;

    llvm::Constant *rhs = node->m_rhs->codegen(*this);
    if (!rhs)
        return nullptr;

    switch (node->m_op)
    {
    case Operator::LessThan:
        return llvm::ConstantExpr::getICmp(llvm::CmpInst::Predicate::ICMP_SLT, lhs, rhs);
    case Operator::GreaterThan:
        return llvm::ConstantExpr::getICmp(llvm::CmpInst::Predicate::ICMP_SGT, lhs, rhs);
    case Operator::LessThanEqual:
        return llvm::ConstantExpr::getICmp(llvm::CmpInst::Predicate::ICMP_SLE, lhs, rhs);
    case Operator::GreaterThanEqual:
        return llvm::ConstantExpr::getICmp(llvm::CmpInst::Predicate::ICMP_SGE, lhs, rhs);
    case Operator::Equal:
        return llvm::ConstantExpr::getICmp(llvm::CmpInst::Predicate::ICMP_EQ, lhs, rhs);
    case Operator::NotEqual:
        return llvm::ConstantExpr::getICmp(llvm::CmpInst::Predicate::ICMP_NE, lhs, rhs);

    case Operator::Plus:
        return llvm::ConstantExpr::getAdd(lhs, rhs);
    case Operator::Minus:
        return llvm::ConstantExpr::getSub(lhs, rhs);
    case Operator::Multiply:
        return llvm::ConstantExpr::getMul(lhs, rhs);

    case Operator::BitAnd:
        return llvm::ConstantExpr::getAnd(lhs, rhs);
    case Operator::BitOr:
        return llvm::ConstantExpr::getOr(lhs, rhs);
    case Operator::BitXor:
        return llvm::ConstantExpr::getXor(lhs, rhs);
    case Operator::LeftShift:
        return llvm::ConstantExpr::getShl(lhs, rhs);
    case Operator::RightShift:
        return llvm::ConstantExpr::getLShr(lhs, rhs);

    case Operator::And:
        return llvm::ConstantExpr::getICmp(llvm::CmpInst::Predicate::ICMP_NE, llvm::ConstantExpr::getAnd(lhs, rhs), llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 0, false)));
    case Operator::Or:
        return llvm::ConstantExpr::getICmp(llvm::CmpInst::Predicate::ICMP_NE, llvm::ConstantExpr::getOr(lhs, rhs), llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 0, false)));
    case Operator::Xor:
        return llvm::ConstantExpr::getICmp(llvm::CmpInst::Predicate::ICMP_NE, llvm::ConstantExpr::getXor(lhs, rhs), llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 0, false)));

    default:
        return nullptr;
    }
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::IdentifierNode *node) const
{
    return m_ctx->m_named_values.at(node->m_name);
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::U8TypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::U16TypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::U32TypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::U64TypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::I8TypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::I16TypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::I32TypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::I64TypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::F32TypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::F64TypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::BoolTypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::CharTypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::VoidTypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::StringTypeNode *node) const
{
    return llvm::Type::getInt8PtrTy(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::StructTypeNode *node) const
{
    std::vector<llvm::Type *> fields;

    for (auto &field : node->m_fields)
        fields.push_back(field->codegen(*this));

    return llvm::StructType::create(*m_ctx->m_ctx, fields);
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::UnionTypeNode *node) const
{
    std::vector<llvm::Type *> fields;

    fields.push_back(llvm::Type::getInt32Ty(*m_ctx->m_ctx)); // selector

    size_t largest = 0;
    for (auto &field : node->m_fields)
    {
        auto type = field->codegen(*this);
        if (type->getPrimitiveSizeInBits() > largest)
            largest = type->getPrimitiveSizeInBits();
    }

    fields.push_back(llvm::ArrayType::get(llvm::Type::getInt8Ty(*m_ctx->m_ctx), largest / 8));

    return llvm::StructType::create(*m_ctx->m_ctx, fields);
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::ArrayTypeNode *node) const
{
    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::UserTypeNode *node) const
{
    return nullptr;
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::IntegerLiteralNode *node) const
{
    return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(get_numbits(node->m_val), node->m_val, 10));
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::FloatLiteralNode *node) const
{
    if (get_numbits(node->m_val) > 32)
    {
        return llvm::ConstantFP::get(*m_ctx->m_ctx, llvm::APFloat(llvm::APFloat::IEEEdouble(), node->m_val));
    }

    return llvm::ConstantFP::get(*m_ctx->m_ctx, llvm::APFloat(llvm::APFloat::IEEEsingle(), node->m_val));
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::StringLiteralNode *node) const
{
    auto str = llvm::ConstantDataArray::getString(*m_ctx->m_ctx, node->m_val);

    llvm::Constant *zero = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*m_ctx->m_ctx));
    llvm::Constant *indices[] = {zero, zero};

    return llvm::ConstantExpr::getGetElementPtr(str->getType(), str, indices, true);
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::CharLiteralNode *node) const
{
    return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(8, node->m_val[0]));
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::BoolLiteralNode *node) const
{
    return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, node->m_val));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::VarDeclNode *node) const
{
    llvm::Type *type;

    if (!(type = node->m_type->codegen(*this)))
        return nullptr;

    m_ctx->m_module->getOrInsertGlobal(Symbol::mangle(node, m_ctx->prefix), type);
    llvm::GlobalVariable *gvar = m_ctx->m_module->getGlobalVariable(Symbol::mangle(node, m_ctx->prefix));

    if (m_ctx->m_pub)
        gvar->setLinkage(llvm::GlobalValue::ExternalLinkage);
    else
        gvar->setLinkage(llvm::GlobalValue::PrivateLinkage);

    if (node->m_init)
    {
        llvm::Constant *init;
        if (!(init = static_cast<llvm::Constant *>(node->m_init->codegen(*this))))
            return nullptr;

        gvar->setInitializer(init);
    }

    return gvar;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::LetDeclNode *node) const
{
    llvm::Type *type;

    if (!(type = node->m_type->codegen(*this)))
        return nullptr;

    m_ctx->m_module->getOrInsertGlobal(Symbol::mangle(node, m_ctx->prefix), type);
    llvm::GlobalVariable *gvar = m_ctx->m_module->getGlobalVariable(Symbol::mangle(node, m_ctx->prefix));

    if (m_ctx->m_pub)
        gvar->setLinkage(llvm::GlobalValue::ExternalLinkage);
    else
        gvar->setLinkage(llvm::GlobalValue::PrivateLinkage);

    if (node->m_init)
    {
        llvm::Constant *init;
        if (!(init = static_cast<llvm::Constant *>(node->m_init->codegen(*this))))
            return nullptr;

        gvar->setInitializer(init);
    }

    return gvar;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::ConstDeclNode *node) const
{
    llvm::Type *type;

    if (!(type = node->m_type->codegen(*this)))
        return nullptr;

    m_ctx->m_module->getOrInsertGlobal(Symbol::mangle(node, m_ctx->prefix), type);
    llvm::GlobalVariable *gvar = m_ctx->m_module->getGlobalVariable(Symbol::mangle(node, m_ctx->prefix));

    if (m_ctx->m_pub)
        gvar->setLinkage(llvm::GlobalValue::ExternalLinkage);
    else
        gvar->setLinkage(llvm::GlobalValue::PrivateLinkage);

    if (node->m_init)
    {
        llvm::Constant *init;
        if (!(init = static_cast<llvm::Constant *>(node->m_init->codegen(*this))))
            return nullptr;

        gvar->setInitializer(init);
    }

    return gvar;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::StructDeclNode *node) const
{
    return nullptr;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::UnionDeclNode *node) const
{
    return nullptr;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::EnumDeclNode *node) const
{
    return nullptr;
}

llvm::Function *libquixcc::CodegenVisitor::visit(const libquixcc::FunctionDeclNode *node) const
{
    std::vector<llvm::Type *> params;
    for (const auto &param : node->m_params)
    {
        params.push_back(param->m_type->codegen(*this));
    }
    llvm::FunctionType *ftype = llvm::FunctionType::get(node->m_return_type->codegen(*this), params, node->m_variadic);
    llvm::Function *func;

    if (m_ctx->m_pub)
        func = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, node->m_name, *m_ctx->m_module);
    else
        func = llvm::Function::Create(ftype, llvm::Function::PrivateLinkage, node->m_name, *m_ctx->m_module);

    size_t i = 0;
    for (auto &arg : func->args())
    {
        arg.setName(node->m_params[i]->m_name);
        i++;
    }
    return func;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::StructDefNode *node) const
{
    return nullptr;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::StructFieldNode *node) const
{
    return nullptr;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::UnionDefNode *node) const
{
    return nullptr;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::UnionFieldNode *node) const
{
    return nullptr;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::EnumDefNode *node) const
{
    return nullptr;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::EnumFieldNode *node) const
{
    return nullptr;
}

llvm::Function *libquixcc::CodegenVisitor::visit(const libquixcc::FunctionDefNode *node) const
{
    llvm::Function *func = node->m_decl->codegen(*this);

    llvm::BasicBlock *EntryBlock = llvm::BasicBlock::Create(*m_ctx->m_ctx, "entry", func);
    m_ctx->m_builder->SetInsertPoint(EntryBlock);

    // Create the loop block
    llvm::BasicBlock *LoopBlock = llvm::BasicBlock::Create(*m_ctx->m_ctx, "loop", func);

    // Branch to the loop block
    m_ctx->m_builder->CreateBr(LoopBlock);

    // Set insertion point to the loop block
    m_ctx->m_builder->SetInsertPoint(LoopBlock);

    // Here you would insert code for the loop body

    // Create an unconditional branch back to the loop block
    m_ctx->m_builder->CreateBr(LoopBlock);

    // If the function returns void, insert a return void instruction at the end of the entry block
    if (func->getReturnType()->isVoidTy())
        m_ctx->m_builder->SetInsertPoint(EntryBlock);
    // m_ctx->m_builder->CreateRetVoid();

    return func;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::FunctionParamNode *node) const
{
    return nullptr;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::SubsystemNode *node) const
{
    size_t len = m_ctx->prefix.size();
    if (len == 0)
        m_ctx->prefix = node->m_name;
    else
        m_ctx->prefix += "::" + node->m_name;

    auto block = node->m_block->codegen(*this);
    m_ctx->prefix = m_ctx->prefix.substr(0, len);
    return block;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::ExportNode *node) const
{
    bool pub = m_ctx->m_pub;

    m_ctx->m_pub = true;

    auto stmt = node->m_stmt->codegen(*this);

    if (!pub)
        m_ctx->m_pub = false;

    return stmt;
}
