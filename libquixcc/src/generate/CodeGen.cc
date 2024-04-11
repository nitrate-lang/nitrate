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
///     * Copyright (C) 2020-2024 Wesley C. Jones                                ///
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
#include <parse/nodes/AllNodes.h>
#include <mangle/Symbol.h>
#include <error/Logger.h>

#include <llvm/IR/InlineAsm.h>

uint8_t get_numbits(std::string s)
{
    if (s.starts_with("-"))
        s = s.substr(1);

    if (s.find('.') != std::string::npos)
    {
        float f0;
        try
        {
            f0 = std::stof(s);
        }
        catch (const std::out_of_range &e)
        {
            return 64;
        }
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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::StmtGroupNode *node) const
{
    for (auto &stmt : node->m_stmts)
    {
        llvm::Value *val = stmt->codegen(*this);
        if (!val)
            return nullptr;
    }
    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::ExprStmtNode *node) const
{
    return node->m_expr->codegen(*this);
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
    case Operator::BitwiseNot:
        return llvm::BinaryOperator::CreateNot(expr, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Not:
        return llvm::BinaryOperator::CreateNot(expr, "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Increment:
        return llvm::BinaryOperator::CreateAdd(expr, llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 1, false)), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Decrement:
        return llvm::BinaryOperator::CreateSub(expr, llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 1, false)), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::BitwiseAnd:
    {
        if (node->m_expr->ntype != NodeType::IdentifierNode)
            return nullptr;

        auto name = Symbol::join(m_ctx->prefix, static_cast<IdentifierNode *>(node->m_expr.get())->m_name);
        if (m_ctx->m_named_stack_vars.contains(name))
            return m_ctx->m_named_stack_vars[name].first;
        else if (m_ctx->m_named_global_vars.contains(name))
            return m_ctx->m_named_global_vars[name];
        else if (m_ctx->m_named_params.contains(name))
            return m_ctx->m_named_params[name].first;

        for (auto &pair : m_ctx->m_named_functions)
        {
            if (pair.second->m_name == name)
                return pair.first;
        }

        return nullptr;
    }
    case Operator::Multiply:
    {
        if (node->m_expr->ntype == NodeType::IdentifierNode)
        {
            auto name = Symbol::join(m_ctx->prefix, static_cast<IdentifierNode *>(node->m_expr.get())->m_name);
            if (m_ctx->m_named_stack_vars.contains(name))
            {
                auto target_val_ptr = m_ctx->m_builder->CreateLoad(m_ctx->m_named_stack_vars[name].second, m_ctx->m_named_stack_vars[name].first);
                auto target_val = m_ctx->m_builder->CreateLoad(target_val_ptr->getType(), target_val_ptr);
                return target_val;
            }

            return nullptr;
        }
        else
        {
            return m_ctx->m_builder->CreateLoad(expr->getType(), m_ctx->m_builder->CreateZExtOrBitCast(expr, llvm::PointerType::get(expr->getType(), 0)));
        }
    }
    default:
        return nullptr;
    }
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::BinaryExprNode *node) const
{
    switch (node->m_op)
    {
    case Operator::LessThan:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SLT, node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::GreaterThan:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SGT, node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::LessThanEqual:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SLE, node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::GreaterThanEqual:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_SGE, node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Equal:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_EQ, node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::NotEqual:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_NE, node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());

    case Operator::Plus:
        return llvm::BinaryOperator::CreateAdd(node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Minus:
        return llvm::BinaryOperator::CreateSub(node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Multiply:
        return llvm::BinaryOperator::CreateMul(node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());

    case Operator::BitwiseAnd:
        return llvm::BinaryOperator::CreateAnd(node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::BitwiseOr:
        return llvm::BinaryOperator::CreateOr(node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::BitwiseXor:
        return llvm::BinaryOperator::CreateXor(node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::LeftShift:
        return llvm::BinaryOperator::CreateShl(node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::RightShift:
        return llvm::BinaryOperator::CreateLShr(node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock());

    case Operator::And:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_NE, llvm::BinaryOperator::CreateAnd(node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock()), llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 0, false)), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Or:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_NE, llvm::BinaryOperator::CreateOr(node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock()), llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 0, false)), "", m_ctx->m_builder->GetInsertBlock());
    case Operator::Xor:
        return llvm::CmpInst::Create(llvm::Instruction::ICmp, llvm::CmpInst::ICMP_NE, llvm::BinaryOperator::CreateXor(node->m_lhs->codegen(*this), node->m_rhs->codegen(*this), "", m_ctx->m_builder->GetInsertBlock()), llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, 0, false)), "", m_ctx->m_builder->GetInsertBlock());

    case Operator::Assign:
    {
        if (node->m_lhs->ntype == NodeType::IdentifierNode)
        {
            auto name = Symbol::join(m_ctx->prefix, static_cast<IdentifierNode *>(node->m_lhs.get())->m_name);
            auto rhs = node->m_rhs->codegen(*this);
            if (!rhs)
                return nullptr;

            if (m_ctx->m_named_stack_vars.contains(name))
            {
                m_ctx->m_builder->CreateStore(rhs, m_ctx->m_named_stack_vars[name].first);
                return rhs;
            }
            else if (m_ctx->m_named_global_vars.contains(name))
            {
                m_ctx->m_builder->CreateStore(rhs, m_ctx->m_named_global_vars[name]);
                return rhs;
            }
            else if (m_ctx->m_named_params.contains(name))
            {
                m_ctx->m_builder->CreateStore(rhs, m_ctx->m_named_params[name].first);
                return rhs;
            }

            return nullptr;
        }
        else if (node->m_lhs->ntype == NodeType::UnaryExprNode)
        {
            auto unary = static_cast<UnaryExprNode *>(node->m_lhs.get());
            if (unary->m_op == Operator::Multiply)
            {
                auto name = Symbol::join(m_ctx->prefix, static_cast<IdentifierNode *>(unary->m_expr.get())->m_name);
                if (!m_ctx->m_named_stack_vars.contains(name))
                {
                    return nullptr;
                }
                auto target_val = m_ctx->m_builder->CreateLoad(m_ctx->m_named_stack_vars[name].second, m_ctx->m_named_stack_vars[name].first);
                auto rhs = node->m_rhs->codegen(*this);
                if (!rhs)
                    return nullptr;

                m_ctx->m_builder->CreateStore(rhs, target_val);
                return rhs;
            }

            auto lhs = unary->codegen(*this);
            if (!lhs)
                return nullptr;
            auto rhs = node->m_rhs->codegen(*this);
            m_ctx->m_builder->CreateStore(rhs, lhs);
            return rhs;
        }
        return nullptr;
    }
    default:
        return nullptr;
    }
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::CallExprNode *node) const
{
    std::string name;
    llvm::Function *fn = nullptr;

    bool found = std::any_of(m_ctx->m_named_functions.begin(), m_ctx->m_named_functions.end(), [&](const auto &pair)
                             {
        if (pair.second->m_name == node->m_name)
        {
            name = pair.second->m_name;
            fn = pair.first;
            return true;
        }
        return false; });

    if (!found)
    {
        std::string msg = "Unknown function referenced: " + name;
        throw CodegenException(msg);
    }

    if (fn->isVarArg())
    {
        if (fn->arg_size() > node->m_positional_args.size())
        {
            std::string msg = "Incorrect number of arguments passed to function: " + name;
            throw CodegenException(msg);
        }
    }
    else
    {
        if (fn->arg_size() != node->m_positional_args.size())
        {
            std::string msg = "Incorrect number of arguments passed to function: " + name;
            throw CodegenException(msg);
        }
    }

    std::vector<llvm::Value *> args;
    for (auto &arg : node->m_positional_args)
    {
        args.push_back(arg->codegen(*this));
        if (!args.back())
            return nullptr;
    }

    return m_ctx->m_builder->CreateCall(fn, args);
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
    case Operator::BitwiseNot:
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

    case Operator::BitwiseAnd:
        return llvm::ConstantExpr::getAnd(lhs, rhs);
    case Operator::BitwiseOr:
        return llvm::ConstantExpr::getOr(lhs, rhs);
    case Operator::BitwiseXor:
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

    for (auto &pair : m_ctx->m_named_functions)
    {
        if (pair.second->m_name == name)
            return pair.first;
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
    if (node->m_type->ntype == NodeType::OpaqueTypeNode)
        return llvm::Type::getInt8PtrTy(*m_ctx->m_ctx);

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

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::RegionTypeNode *node) const
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
    llvm::Constant *zero = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*m_ctx->m_ctx));
    llvm::Constant *indices[] = {zero, zero};

    auto gvar = m_ctx->m_builder->CreateGlobalString(node->m_val);

    gvar->setLinkage(llvm::GlobalValue::PrivateLinkage);

    return llvm::ConstantExpr::getGetElementPtr(gvar->getValueType(), gvar, indices);
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

        m_ctx->m_named_functions.push_back(std::make_pair(func, node));
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

    llvm::StructType::create(fields, node->m_name, true);

    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::RegionDefNode *node) const
{
    llvm::Type *f;
    std::vector<llvm::Type *> fields;

    for (auto &field : node->m_fields)
    {
        if (!(f = field->m_type->codegen(*this)))
            return nullptr;
        fields.push_back(f);
    }

    llvm::StructType::create(fields, node->m_name, true);

    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::UnionDefNode *node) const
{
    std::vector<llvm::Type *> fields;

    fields.push_back(llvm::Type::getInt32Ty(*m_ctx->m_ctx)); // selector

    size_t largest = 0;
    for (auto &field : node->get_type()->m_fields)
    {
        auto type = field->codegen(*this);
        if (type->getPrimitiveSizeInBits() > largest)
            largest = type->getPrimitiveSizeInBits();
    }

    fields.push_back(llvm::ArrayType::get(llvm::Type::getInt8Ty(*m_ctx->m_ctx), largest / 8));

    llvm::StructType::create(fields, node->m_name, true);

    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
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
        default:
            if (!stmt->codegen(*this))
                return nullptr;
            break;
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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::InlineAsmNode *node) const
{
    /*
        What were doing here:
         1. Gencode the input expressions and compute their types
         1. Generate the output expressions and compute their types
         1. Generate LLVM IR function using this signature
         1. Translate ASM constraints to LLVM IR constraints
         1. Create ASM call
         1. If any outputs, store the values in the output expressions
    */

    /// Make inputs/codegen inputs
    std::vector<llvm::Type *> params;
    std::vector<llvm::Value *> args;
    for (const auto &param : node->m_inputs)
    {
        auto val = param.second->codegen(*this);
        if (!val)
            return nullptr;

        params.push_back(val->getType());
        args.push_back(val);
    }

    /// Make outputs/codegen outputs
    std::vector<llvm::Value *> outputs;
    llvm::Type *ret = nullptr;
    if (node->m_outputs.empty())
    {
        ret = llvm::Type::getVoidTy(*m_ctx->m_ctx);
    }
    else if (node->m_outputs.size() == 1)
    {
        auto val = node->m_outputs.begin()->second->codegen(*this);
        if (!val)
            return nullptr;

        // It is a pointer, get element type
        if (val->getType()->isPointerTy())
            ret = val->getType()->getPointerElementType();
        else
            ret = val->getType();

        outputs.push_back(val);
    }
    else
    {
        std::vector<llvm::Type *> types;
        for (const auto &param : node->m_outputs)
        {
            auto val = param.second->codegen(*this);
            if (!val)
                return nullptr;
            types.push_back(val->getType());
            outputs.push_back(val);
        }
        ret = llvm::StructType::get(*m_ctx->m_ctx, types);
    }

    llvm::FunctionType *ftype = llvm::FunctionType::get(ret, params, false);

    std::string constraints = "";
    for (const auto &param : node->m_outputs)
    {
        constraints += param.first + ",";
    }
    for (const auto &param : node->m_inputs)
    {
        constraints += param.first + ",";
    }
    for (const auto &param : node->m_clobbers)
    {
        constraints += "~{" + param + "},";
    }

    constraints += "~{dirflag},~{fpsr},~{flags}";

    llvm::InlineAsm *asmcode = llvm::InlineAsm::get(ftype, node->m_asm, constraints, true, false, llvm::InlineAsm::AD_ATT);

    auto call = m_ctx->m_builder->CreateCall(asmcode, args);

    if (outputs.empty())
    {
        return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
    }
    else
    {
        for (size_t i = 0; i < outputs.size(); i++)
        {
            m_ctx->m_builder->CreateStore(call, outputs[i]);
        }

        return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
    }
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