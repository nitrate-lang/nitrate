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

static std::string getRandomIdentifier()
{
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static const size_t alphanum_size = sizeof(alphanum) - 1;
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    std::string s;
    for (size_t i = 0; i < 10; ++i)
    {
        s += alphanum[rand() % alphanum_size];
    }
    return "_" + s;
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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::NopStmtNode *node) const
{
    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::UnaryExprNode *node) const
{
    llvm::Value *expr = node->m_expr->codegen(*this);
    if (!expr)
        return nullptr;

    switch (node->m_op)
    {
    case Operator::Minus:
        return llvm::BinaryOperator::CreateNeg(expr, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Plus:
        return expr;
    case Operator::BitNot:
        return llvm::BinaryOperator::CreateNot(expr, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Not:
        return llvm::BinaryOperator::CreateNot(expr, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Increment:
        return llvm::BinaryOperator::CreateAdd(expr, llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 1, false)), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Decrement:
        return llvm::BinaryOperator::CreateSub(expr, llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 1, false)), "", m_ctx->m_builder->GetInsertBlock());
    default:
        return nullptr;
    }
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::BinaryExprNode *node) const
{
    llvm::Value *lhs = node->m_lhs->codegen(*this);
    if (!lhs)
        return nullptr;

    llvm::Value *rhs = node->m_rhs->codegen(*this);
    if (!rhs)
        return nullptr;

    switch (node->m_op)
    {
    case Operator::LessThan:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SLT, lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::GreaterThan:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SGT, lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::LessThanEqual:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SLE, lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::GreaterThanEqual:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SGE, lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Equal:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_EQ, lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::NotEqual:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_NE, lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());

    case Operator::Plus:
        return llvm::BinaryOperator::CreateAdd(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Minus:
        return llvm::BinaryOperator::CreateSub(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Multiply:
        return llvm::BinaryOperator::CreateMul(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());

    case Operator::BitAnd:
        return llvm::BinaryOperator::CreateAnd(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::BitOr:
        return llvm::BinaryOperator::CreateOr(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::BitXor:
        return llvm::BinaryOperator::CreateXor(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::LeftShift:
        return llvm::BinaryOperator::CreateShl(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::RightShift:
        return llvm::BinaryOperator::CreateLShr(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());

    case Operator::And:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_NE, llvm::BinaryOperator::CreateAnd(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock()), llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 0, false)), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Or:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_NE, llvm::BinaryOperator::CreateOr(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock()), llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 0, false)), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Xor:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_NE, llvm::BinaryOperator::CreateXor(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock()), llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 0, false)), "", m_ctx->m_builder->GetInsertBlock());

    default:
        return nullptr;
    }
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::CallExprNode *node) const
{
    if (!m_ctx->m_named_functions.contains(node->m_decl.get()))
    {
        std::string msg = "Unknown function referenced: " + node->m_decl->m_name;
        throw CodegenException(msg);
    }

    auto callee = m_ctx->m_named_functions[node->m_decl.get()];

    auto fn = m_ctx->m_module->getFunction(callee);
    if (!fn)
    {
        std::string msg = "Unknown function referenced: " + callee;
        throw CodegenException(msg);
    }

    if (fn->isVarArg())
    {
        if (fn->arg_size() > node->m_invoke->m_positional_args.size())
        {
            std::string msg = "Incorrect number of arguments passed to function: " + callee;
            throw CodegenException(msg);
        }
    }
    else
    {
        if (fn->arg_size() != node->m_invoke->m_positional_args.size())
        {
            std::string msg = "Incorrect number of arguments passed to function: " + callee;
            throw CodegenException(msg);
        }
    }

    std::vector<llvm::Value *> args;
    for (auto &arg : node->m_invoke->m_positional_args)
    {
        args.push_back(arg->codegen(*this));
        if (!args.back())
            return nullptr;
    }

    return m_ctx->m_builder->CreateCall(fn, args, "calltmp");
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
    auto name = Symbol::join(m_ctx->prefix, node->m_name);

    if (m_ctx->m_named_stack_vars.contains(name))
    {
        // Load the value from the alloca and return it
        auto load = m_ctx->m_builder->CreateLoad(m_ctx->m_named_stack_vars[name].second, static_cast<llvm::Value *>(m_ctx->m_named_stack_vars[name].first), name);

        return load;
    }
    else if (m_ctx->m_named_params.contains(name))
    {
        /// TODO: fix function parameters
        return m_ctx->m_named_params[name].first;
    }
    else if (m_ctx->m_named_global_vars.contains(name))
    {
        return m_ctx->m_builder->CreateLoad(m_ctx->m_named_global_vars[name]->getValueType(), m_ctx->m_named_global_vars[name], name);
    }

    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::U8TypeNode *node) const
{
    return m_ctx->m_builder->getInt8Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::U16TypeNode *node) const
{
    return m_ctx->m_builder->getInt16Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::U32TypeNode *node) const
{
    return m_ctx->m_builder->getInt32Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::U64TypeNode *node) const
{
    return m_ctx->m_builder->getInt64Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::I8TypeNode *node) const
{
    return m_ctx->m_builder->getInt8Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::I16TypeNode *node) const
{
    return m_ctx->m_builder->getInt16Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::I32TypeNode *node) const
{
    return m_ctx->m_builder->getInt32Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::I64TypeNode *node) const
{
    return m_ctx->m_builder->getInt64Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::F32TypeNode *node) const
{
    return m_ctx->m_builder->getFloatTy();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::F64TypeNode *node) const
{
    return m_ctx->m_builder->getDoubleTy();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::BoolTypeNode *node) const
{
    return m_ctx->m_builder->getInt1Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::VoidTypeNode *node) const
{
    return m_ctx->m_builder->getVoidTy();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::PointerTypeNode *node) const
{
    auto type = node->m_type->codegen(*this);
    if (!type)
        return nullptr;

    return llvm::PointerType::get(type, 0);
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::StringTypeNode *node) const
{
    return llvm::Type::getInt8PtrTy(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::EnumTypeNode *node) const
{
    return node->m_member_type->codegen(*this);
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
    auto sz = node->m_size->codegen(*this);
    if (!sz)
        return nullptr;
    if (!sz->getType()->isIntegerTy())
        return nullptr;

    auto type = node->m_type->codegen(*this);
    if (!type)
        return nullptr;

    return llvm::ArrayType::get(type, static_cast<llvm::ConstantInt *>(sz)->getZExtValue());
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::FunctionTypeNode *node) const
{
    std::vector<llvm::Type *> params;
    for (const auto &param : node->m_params)
    {
        params.push_back(param->codegen(*this));
    }

    return llvm::FunctionType::get(node->m_return_type->codegen(*this), params, node->m_variadic);
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
    const auto randId = getRandomIdentifier();
    auto str = llvm::ConstantDataArray::getString(*m_ctx->m_ctx, node->m_val);

    llvm::Constant *zero = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*m_ctx->m_ctx));
    llvm::Constant *indices[] = {zero, zero};

    auto global = m_ctx->m_module->getOrInsertGlobal(randId, str->getType());
    auto gvar = m_ctx->m_module->getGlobalVariable(randId);

    gvar->setLinkage(llvm::GlobalValue::PrivateLinkage);

    gvar->setInitializer(str);

    return llvm::ConstantExpr::getGetElementPtr(str->getType(), global, indices);
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::CharLiteralNode *node) const
{
    return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(8, node->m_val[0]));
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::BoolLiteralNode *node) const
{
    return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, node->m_val));
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::NullLiteralNode *node) const
{
    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::VarDeclNode *node) const
{
    llvm::Type *type;

    if (!(type = node->m_type->codegen(*this)))
        return nullptr;

    m_ctx->m_module->getOrInsertGlobal(Symbol::mangle(node, m_ctx->prefix, m_ctx->m_lang), type);
    llvm::GlobalVariable *gvar = m_ctx->m_module->getGlobalVariable(Symbol::mangle(node, m_ctx->prefix, m_ctx->m_lang));

    if (!gvar)
        return nullptr;

    m_ctx->m_named_global_vars[Symbol::join(m_ctx->prefix, node->m_name)] = gvar;

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
    else
    {
        gvar->setInitializer(llvm::Constant::getNullValue(type));
    }

    return gvar;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::LetDeclNode *node) const
{
    llvm::Type *type;

    if (!(type = node->m_type->codegen(*this)))
        return nullptr;

    m_ctx->m_module->getOrInsertGlobal(Symbol::mangle(node, m_ctx->prefix, m_ctx->m_lang), type);
    llvm::GlobalVariable *gvar = m_ctx->m_module->getGlobalVariable(Symbol::mangle(node, m_ctx->prefix, m_ctx->m_lang));
    if (!gvar)
        return nullptr;

    m_ctx->m_named_global_vars[Symbol::join(m_ctx->prefix, node->m_name)] = gvar;

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
    else
    {
        gvar->setInitializer(llvm::Constant::getNullValue(type));
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
    llvm::FunctionType *ftype = llvm::FunctionType::get(node->m_type->m_return_type->codegen(*this), params, node->m_type->m_variadic);
    llvm::Function *func;

    if (m_ctx->prefix.empty() && node->m_name == "main") // special case for main function
    {
        func = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "main", *m_ctx->m_module);
    }
    else
    {
        std::string name = Symbol::mangle(node, m_ctx->prefix, m_ctx->m_lang);

        if (m_ctx->m_pub)
            func = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, name, *m_ctx->m_module);
        else
            func = llvm::Function::Create(ftype, llvm::Function::PrivateLinkage, name, *m_ctx->m_module);

        m_ctx->m_named_functions[node] = name;
    }

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
    llvm::Type *f;
    std::vector<llvm::Type *> fields;

    for (auto &field : node->m_fields)
    {
        if (!(f = field->m_type->codegen(*this)))
            return nullptr;
        fields.push_back(f);
    }

    llvm::StructType *stype = llvm::StructType::create(*m_ctx->m_ctx, node->m_name);
    stype->setBody(fields);

    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
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

    m_ctx->m_named_params.clear();
    for (auto &arg : func->args())
        m_ctx->m_named_params[arg.getName().str()] = std::make_pair(&arg, arg.getType());

    llvm::BasicBlock *EntryBlock = llvm::BasicBlock::Create(*m_ctx->m_ctx, "entry", func);
    m_ctx->m_builder->SetInsertPoint(EntryBlock);

    if (node->m_decl->m_type->m_return_type->ntype != NodeType::VoidTypeNode)
    {
        auto ret = m_ctx->m_builder->CreateAlloca(node->m_decl->m_type->m_return_type->codegen(*this), nullptr, "ret");
        m_ctx->m_named_stack_vars["ret"] = std::make_pair(ret, node->m_decl->m_type->m_return_type->codegen(*this));
    }

    llvm::BasicBlock *EndBlock = llvm::BasicBlock::Create(*m_ctx->m_ctx, "end", func);
    m_ctx->m_builder->SetInsertPoint(EndBlock);
    m_ctx->m_named_blocks[func->getName().str() + ".end"] = EndBlock;

    if (node->m_decl->m_type->m_return_type->ntype == NodeType::VoidTypeNode)
        m_ctx->m_builder->CreateRetVoid();
    else
        m_ctx->m_builder->CreateRet(m_ctx->m_builder->CreateLoad(m_ctx->m_named_stack_vars["ret"].second, m_ctx->m_named_stack_vars["ret"].first));

    m_ctx->m_builder->SetInsertPoint(EntryBlock);

    for (const auto &stmt : node->m_body->m_stmts)
    {
        switch (stmt->ntype)
        {
        case NodeType::ReturnStmtNode:
            if (!std::static_pointer_cast<ReturnStmtNode>(stmt)->codegen(*this))
                return nullptr;

            break;
        case NodeType::VarDeclNode:
        {
            auto n = std::static_pointer_cast<VarDeclNode>(stmt);

            std::string name = Symbol::join(m_ctx->prefix, n->m_name);
            auto ptr = m_ctx->m_builder->CreateAlloca(n->m_type->codegen(*this), nullptr, name);
            m_ctx->m_named_stack_vars[name] = std::make_pair(ptr, n->m_type->codegen(*this));
            if (n->m_init)
            {
                auto val = n->m_init->codegen(*this);
                if (!val)
                    return nullptr;
                m_ctx->m_builder->CreateStore(val, ptr);
            }
            break;
        }
        case NodeType::LetDeclNode:
        {
            auto n = std::static_pointer_cast<LetDeclNode>(stmt);

            std::string name = Symbol::join(m_ctx->prefix, n->m_name);
            auto ptr = m_ctx->m_builder->CreateAlloca(n->m_type->codegen(*this), nullptr, name);
            m_ctx->m_named_stack_vars[name] = std::make_pair(ptr, n->m_type->codegen(*this));
            if (n->m_init)
            {
                auto val = n->m_init->codegen(*this);
                if (!val)
                    return nullptr;
                m_ctx->m_builder->CreateStore(val, ptr);
            }
            break;
        }
        case NodeType::IfStmtNode:
            if (!std::static_pointer_cast<IfStmtNode>(stmt)->codegen(*this))
                return nullptr;
            break;
        case NodeType::RetifStmtNode:
            if (!std::static_pointer_cast<RetifStmtNode>(stmt)->codegen(*this))
                return nullptr;
            break;
        case NodeType::RetzStmtNode:
            if (!std::static_pointer_cast<RetzStmtNode>(stmt)->codegen(*this))
                return nullptr;
            break;
        case NodeType::RetvStmtNode:
            if (!std::static_pointer_cast<RetvStmtNode>(stmt)->codegen(*this))
                return nullptr;
            break;
        case NodeType::WhileStmtNode:
            if (!std::static_pointer_cast<WhileStmtNode>(stmt)->codegen(*this))
                return nullptr;
            break;
        case NodeType::ForStmtNode:
            if (!std::static_pointer_cast<ForStmtNode>(stmt)->codegen(*this))
                return nullptr;
            break;
        default:
            throw std::runtime_error("Invalid statement type");
        }
    }

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
    ExportLangType lang = m_ctx->m_lang;

    m_ctx->m_pub = true;
    m_ctx->m_lang = node->m_lang_type;

    for (const auto &stmt : node->m_stmts)
    {
        stmt->codegen(*this);
    }

    m_ctx->m_pub = pub;
    m_ctx->m_lang = lang;

    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::ReturnStmtNode *node) const
{
    if (node->m_expr)
    {
        llvm::Value *val = node->m_expr->codegen(*this);
        if (!val)
            return nullptr;

        m_ctx->m_builder->CreateStore(val, m_ctx->m_named_stack_vars.at("ret").first);
    }

    m_ctx->m_skipbr++;

    llvm::Function *func = m_ctx->m_builder->GetInsertBlock()->getParent();
    m_ctx->m_builder->CreateBr(m_ctx->m_named_blocks.at(func->getName().str() + ".end"));

    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::RetifStmtNode *node) const
{
    /*
    if (<cond>)
    {
        return <return_expr>; // may have side effects
    }
    */

    return std::make_shared<IfStmtNode>(node->m_cond, std::make_shared<ReturnStmtNode>(node->m_return), nullptr)->codegen(*this);
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::RetzStmtNode *node) const
{
    /*
    if (!<cond>)
    {
        return <return_expr>; // may have side effects
    }
    */

    auto cond = std::make_shared<UnaryExprNode>(Operator::Not, node->m_cond);
    return std::make_shared<IfStmtNode>(cond, std::make_shared<ReturnStmtNode>(node->m_return), nullptr)->codegen(*this);
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::RetvStmtNode *node) const
{
    /*
    if (<cond>)
    {
        return;
    }
    */

    return std::make_shared<IfStmtNode>(node->m_cond, std::make_shared<ReturnStmtNode>(nullptr), nullptr)->codegen(*this);
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::IfStmtNode *node) const
{
    llvm::Value *cond = node->m_cond->codegen(*this);
    if (!cond)
        return nullptr;

    llvm::Function *func = m_ctx->m_builder->GetInsertBlock()->getParent();

    if (!node->m_else)
    {
        // single if statement, no else
        llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "", func);
        llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "");

        m_ctx->m_builder->CreateCondBr(cond, ThenBB, MergeBB);

        m_ctx->m_builder->SetInsertPoint(ThenBB);
        if (!node->m_then->codegen(*this))
            return nullptr;

        if (m_ctx->m_skipbr > 0)
            m_ctx->m_skipbr--;
        else
            m_ctx->m_builder->CreateBr(MergeBB);

        func->getBasicBlockList().push_back(MergeBB);
        m_ctx->m_builder->SetInsertPoint(MergeBB);
    }
    else
    {
        // if-else statement
        llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "", func);
        llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "", func);
        llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "");

        m_ctx->m_builder->CreateCondBr(cond, ThenBB, ElseBB);

        m_ctx->m_builder->SetInsertPoint(ThenBB);
        if (!node->m_then->codegen(*this))
            return nullptr;

        if (m_ctx->m_skipbr > 0)
            m_ctx->m_skipbr--;
        else
            m_ctx->m_builder->CreateBr(MergeBB);

        m_ctx->m_builder->SetInsertPoint(ElseBB);
        if (!node->m_else->codegen(*this))
            return nullptr;

        if (m_ctx->m_skipbr > 0)
            m_ctx->m_skipbr--;
        else
            m_ctx->m_builder->CreateBr(MergeBB);

        func->getBasicBlockList().push_back(MergeBB);
        m_ctx->m_builder->SetInsertPoint(MergeBB);
    }

    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::WhileStmtNode *node) const
{
    llvm::Function *func = m_ctx->m_builder->GetInsertBlock()->getParent();

    llvm::BasicBlock *CondBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "", func);
    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "", func);
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "");

    m_ctx->m_builder->CreateBr(CondBB);

    m_ctx->m_builder->SetInsertPoint(CondBB);
    llvm::Value *cond = node->m_cond->codegen(*this);
    if (!cond)
        return nullptr;

    m_ctx->m_builder->CreateCondBr(cond, LoopBB, MergeBB);

    m_ctx->m_builder->SetInsertPoint(LoopBB);
    if (!node->m_stmt->codegen(*this))
        return nullptr;

    if (m_ctx->m_skipbr > 0)
        m_ctx->m_skipbr--;
    else
        m_ctx->m_builder->CreateBr(CondBB);

    func->getBasicBlockList().push_back(MergeBB);
    m_ctx->m_builder->SetInsertPoint(MergeBB);

    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::ForStmtNode *node) const
{
    /// TODO: verify correctness
    llvm::Function *func = m_ctx->m_builder->GetInsertBlock()->getParent();

    llvm::BasicBlock *CondBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "", func);
    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "", func);
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*m_ctx->m_ctx, "");

    if (node->m_init)
    {
        if (!node->m_init->codegen(*this))
            return nullptr;
    }

    m_ctx->m_builder->CreateBr(CondBB);

    m_ctx->m_builder->SetInsertPoint(CondBB);
    llvm::Value *cond = node->m_cond->codegen(*this);
    if (!cond)
        return nullptr;

    m_ctx->m_builder->CreateCondBr(cond, LoopBB, MergeBB);

    m_ctx->m_builder->SetInsertPoint(LoopBB);
    if (!node->m_stmt->codegen(*this))
        return nullptr;

    if (node->m_step)
    {
        if (!node->m_step->codegen(*this))
            return nullptr;
    }

    if (m_ctx->m_skipbr > 0)
        m_ctx->m_skipbr--;
    else
        m_ctx->m_builder->CreateBr(CondBB);

    func->getBasicBlockList().push_back(MergeBB);
    m_ctx->m_builder->SetInsertPoint(MergeBB);

    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}