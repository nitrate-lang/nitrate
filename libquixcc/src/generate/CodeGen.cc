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

/*
    TODO: make this code less shitty
*/

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
        double delta = 0.0000001;

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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::BlockNode *node)
{
    for (auto &stmt : node->m_stmts)
    {
        llvm::Value *val = stmt->codegen(*this);
        if (!val)
            return nullptr;
    }
    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::StmtGroupNode *node)
{
    for (auto &stmt : node->m_stmts)
    {
        llvm::Value *val = stmt->codegen(*this);
        if (!val)
            return nullptr;
    }
    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::ExprStmtNode *node)
{
    return node->m_expr->codegen(*this);
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::UnaryExprNode *node)
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
        llvm::Value *address = nullptr;
        if (node->m_expr->ntype == NodeType::IdentifierNode)
        {
            bool state = m_state.implicit_load;
            m_state.implicit_load = false;
            address = node->m_expr->codegen(*this);
            m_state.implicit_load = state;
        }
        else if (node->m_expr->ntype == NodeType::MemberAccessNode)
        {
            bool state = m_state.implicit_load;
            m_state.implicit_load = false;
            address = node->m_expr->codegen(*this);
            m_state.implicit_load = state;
        }
        else
        {
            return nullptr;
        }

        if (!address)
            return nullptr;

        return address;
    }
    case Operator::Multiply:
    {
        return m_ctx->m_builder->CreateLoad(expr->getType(), m_ctx->m_builder->CreateZExtOrBitCast(expr, llvm::PointerType::get(expr->getType(), 0)));
    }
    default:
        return nullptr;
    }
}

std::optional<std::pair<llvm::Value *, llvm::Type *>> libquixcc::CodegenVisitor::create_struct_gep(llvm::Value *struct_ptr, const std::string &field_name)
{
    if (!struct_ptr->getType()->isPointerTy())
        return std::nullopt;

    auto struct_type = struct_ptr->getType()->getPointerElementType();
    if (!struct_type->isStructTy())
        return std::nullopt;

    std::string name = struct_type->getStructName().str();

    // Remove the suffic '.num' from the struct name
    if (name.find_last_of('.') != std::string::npos)
        name = name.substr(0, name.find_last_of('.'));

    auto key = std::make_pair(NodeType::StructDefNode, name);
    if (!m_ctx->m_named_construsts.contains(key))
    {
        throw CodegenException("Struct not found: " + name);
        return std::nullopt;
    }
    auto struct_def = m_ctx->m_named_construsts[key];

    auto struct_node = std::static_pointer_cast<StructDefNode>(struct_def);

    size_t index = std::find_if(struct_node->m_fields.begin(), struct_node->m_fields.end(), [&](const auto &field)
                                { return field->m_name == field_name; }) -
                   struct_node->m_fields.begin();

    if (index == struct_node->m_fields.size())
        return std::nullopt;

    return std::make_pair(m_ctx->m_builder->CreateStructGEP(struct_type, struct_ptr, index), struct_node->m_fields[index]->m_type->codegen(*this));
}

std::optional<std::tuple<llvm::Value *, llvm::Type *, libquixcc::CodegenVisitor::AORLocality>> libquixcc::CodegenVisitor::address_of_identifier(const std::string &name)
{
    if (m_ctx->m_named_stack_vars.contains(name))
    {
        return std::make_tuple(static_cast<llvm::Value *>(m_ctx->m_named_stack_vars[name].first), m_ctx->m_named_stack_vars[name].second, AORLocality::Local);
    }
    else if (m_ctx->m_named_params.contains(name))
    {
        return std::make_tuple(m_ctx->m_named_params[name].first, m_ctx->m_named_params[name].first->getType(), AORLocality::Parameter);
    }
    else if (m_ctx->m_named_global_vars.contains(name))
    {
        return std::make_tuple(m_ctx->m_named_global_vars[name], m_ctx->m_named_global_vars[name]->getValueType(), AORLocality::Global);
    }

    for (auto &pair : m_ctx->m_named_functions)
    {
        if (pair.second->m_name == name)
            return std::make_tuple(pair.first, pair.first->getType(), AORLocality::Function);
    }

    return std::nullopt;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::BinaryExprNode *node)
{
    static std::set<Operator> normal_ops = {
        Operator::LessThan,
        Operator::GreaterThan,
        Operator::LessThanEqual,
        Operator::GreaterThanEqual,
        Operator::Equal,
        Operator::NotEqual,
        Operator::Plus,
        Operator::Minus,
        Operator::Multiply,
        Operator::BitwiseAnd,
        Operator::BitwiseOr,
        Operator::BitwiseXor,
        Operator::LeftShift,
        Operator::RightShift,
        Operator::And,
        Operator::Or,
        Operator::Xor};

    if (normal_ops.contains(node->m_op))
    {
        auto lhs = node->m_lhs->codegen(*this);
        if (!lhs)
            return nullptr;
        auto rhs = node->m_rhs->codegen(*this);
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
        case Operator::BitwiseAnd:
            return llvm::BinaryOperator::CreateAnd(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
        case Operator::BitwiseOr:
            return llvm::BinaryOperator::CreateOr(lhs, rhs, "", m_ctx->m_builder->GetInsertBlock());
        case Operator::BitwiseXor:
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

    if (node->m_op == Operator::Assign)
    {
        llvm::Value *address = nullptr;

        if (node->m_lhs->ntype == NodeType::IdentifierNode)
        {
            bool state = m_state.implicit_load;
            m_state.implicit_load = false;
            address = node->m_lhs->codegen(*this);
            m_state.implicit_load = state;
        }
        else if (node->m_lhs->ntype == NodeType::UnaryExprNode &&
                 static_cast<UnaryExprNode *>(node->m_lhs.get())->m_op == Operator::Multiply)
        {
            auto lhs_expr = static_cast<UnaryExprNode *>(node->m_lhs.get())->m_expr;

            auto lhs = lhs_expr->codegen(*this);
            if (!lhs)
                return nullptr;

            auto rhs = node->m_rhs->codegen(*this);
            if (!rhs)
                return nullptr;

            return m_ctx->m_builder->CreateStore(rhs, lhs);
        }
        else if (node->m_lhs->ntype == NodeType::MemberAccessNode)
        {
            bool state = m_state.implicit_load;
            m_state.implicit_load = false;
            address = node->m_lhs->codegen(*this);
            m_state.implicit_load = state;
        }
        else
        {
            address = node->m_lhs->codegen(*this);
        }

        if (!address)
            return nullptr;

        auto rhs = node->m_rhs->codegen(*this);
        if (!rhs)
            return nullptr;

        return m_ctx->m_builder->CreateStore(rhs, address);
    }

    return nullptr;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::CallExprNode *node)
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
        auto v = arg->codegen(*this);
        if (!v)
            return nullptr;

        args.push_back(v);
    }

    return m_ctx->m_builder->CreateCall(fn, args);
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::ListExprNode *node)
{
    std::vector<llvm::Value *> values;
    std::vector<llvm::Type *> types;

    for (auto &elem : node->m_elements)
    {
        auto val = elem->codegen(*this);
        if (!val)
            return nullptr;

        values.push_back(val);
        types.push_back(val->getType());
    }

    auto struct_type = llvm::StructType::get(*m_ctx->m_ctx, types);

    auto alloca = m_ctx->m_builder->CreateAlloca(struct_type);

    for (size_t i = 0; i < values.size(); i++)
    {
        auto gep = m_ctx->m_builder->CreateStructGEP(struct_type, alloca, i);
        m_ctx->m_builder->CreateStore(values[i], gep);
    }

    auto val = m_ctx->m_builder->CreateLoad(struct_type, alloca);

    return val;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::MemberAccessNode *node)
{
    llvm::Value *struct_ptr = nullptr;

    bool state = m_state.implicit_load;
    m_state.implicit_load = false;
    struct_ptr = node->m_expr->codegen(*this);
    m_state.implicit_load = state;

    auto result = create_struct_gep(struct_ptr, node->m_field);
    if (!result)
        return nullptr;

    if (m_state.implicit_load)
        return m_ctx->m_builder->CreateLoad(std::get<1>(*result), std::get<0>(*result));

    return std::get<0>(*result);
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::ConstUnaryExprNode *node)
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

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::ConstBinaryExprNode *node)
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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::IdentifierNode *node)
{
    auto name = Symbol::join(m_ctx->prefix, node->m_name);

    auto ret = address_of_identifier(name);
    if (!ret)
        return nullptr;

    auto ptr = std::get<0>(*ret);
    auto type = std::get<1>(*ret);
    auto locality = std::get<2>(*ret);

    if (!m_state.implicit_load)
        return ptr;

    switch (locality)
    {
    case AORLocality::Local:
        return m_ctx->m_builder->CreateLoad(type, ptr);
    case AORLocality::Parameter:
        return ptr;
    case AORLocality::Global:
        return m_ctx->m_builder->CreateLoad(type, ptr);
    case AORLocality::Function:
        return ptr;
    default:
        return nullptr;
    }

    return nullptr;
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::U8TypeNode *node)
{
    return m_ctx->m_builder->getInt8Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::U16TypeNode *node)
{
    return m_ctx->m_builder->getInt16Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::U32TypeNode *node)
{
    return m_ctx->m_builder->getInt32Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::U64TypeNode *node)
{
    return m_ctx->m_builder->getInt64Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::I8TypeNode *node)
{
    return m_ctx->m_builder->getInt8Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::I16TypeNode *node)
{
    return m_ctx->m_builder->getInt16Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::I32TypeNode *node)
{
    return m_ctx->m_builder->getInt32Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::I64TypeNode *node)
{
    return m_ctx->m_builder->getInt64Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::F32TypeNode *node)
{
    return m_ctx->m_builder->getFloatTy();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::F64TypeNode *node)
{
    return m_ctx->m_builder->getDoubleTy();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::BoolTypeNode *node)
{
    return m_ctx->m_builder->getInt1Ty();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::VoidTypeNode *node)
{
    return m_ctx->m_builder->getVoidTy();
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::PointerTypeNode *node)
{
    if (node->m_type->ntype == NodeType::OpaqueTypeNode)
        return llvm::Type::getInt8PtrTy(*m_ctx->m_ctx);

    auto type = node->m_type->codegen(*this);
    if (!type)
        return nullptr;

    return llvm::PointerType::get(type, 0);
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::StringTypeNode *node)
{
    return llvm::Type::getInt8PtrTy(*m_ctx->m_ctx);
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::EnumTypeNode *node)
{
    return node->m_member_type->codegen(*this);
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::StructTypeNode *node)
{
    if (m_ctx->m_named_structs.contains(node->m_name))
        return m_ctx->m_named_structs[node->m_name];

    std::vector<llvm::Type *> fields;

    for (auto &field : node->m_fields)
        fields.push_back(field->codegen(*this));

    m_ctx->m_named_structs[node->m_name] = llvm::StructType::create(*m_ctx->m_ctx, fields, node->m_name);

    return m_ctx->m_named_structs[node->m_name];
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::RegionTypeNode *node)
{
    std::vector<llvm::Type *> fields;

    for (auto &field : node->m_fields)
        fields.push_back(field->codegen(*this));

    return llvm::StructType::get(*m_ctx->m_ctx, fields);
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::UnionTypeNode *node)
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

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::ArrayTypeNode *node)
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

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::FunctionTypeNode *node)
{
    std::vector<llvm::Type *> params;
    for (const auto &param : node->m_params)
    {
        params.push_back(param->codegen(*this));
    }

    return llvm::FunctionType::get(node->m_return_type->codegen(*this), params, node->m_variadic);
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::IntegerLiteralNode *node)
{
    return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(get_numbits(node->m_val), node->m_val, 10));
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::FloatLiteralNode *node)
{
    if (node->m_val_type == F32TypeNode::create())
        return llvm::ConstantFP::get(*m_ctx->m_ctx, llvm::APFloat(llvm::APFloat::IEEEsingle(), node->m_val));
    else if (node->m_val_type == F64TypeNode::create())
        return llvm::ConstantFP::get(*m_ctx->m_ctx, llvm::APFloat(llvm::APFloat::IEEEdouble(), node->m_val));
    else
        return nullptr;
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::StringLiteralNode *node)
{
    llvm::Constant *zero = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*m_ctx->m_ctx));
    llvm::Constant *indices[] = {zero, zero};

    auto gvar = m_ctx->m_builder->CreateGlobalString(node->m_val);

    gvar->setLinkage(llvm::GlobalValue::PrivateLinkage);

    return llvm::ConstantExpr::getGetElementPtr(gvar->getValueType(), gvar, indices);
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::CharLiteralNode *node)
{
    return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(8, node->m_val[0]));
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::BoolLiteralNode *node)
{
    return llvm::ConstantInt::get(*m_ctx->m_ctx, llvm::APInt(1, node->m_val));
}

llvm::Constant *libquixcc::CodegenVisitor::visit(const libquixcc::NullLiteralNode *node)
{
    return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*m_ctx->m_ctx));
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::LetDeclNode *node)
{
    if (m_state.inside_function)
    {
        std::string name = Symbol::join(m_ctx->prefix, node->m_name);
        auto ptr = m_ctx->m_builder->CreateAlloca(node->m_type->codegen(*this), nullptr, name);
        m_ctx->m_named_stack_vars[name] = std::make_pair(ptr, node->m_type->codegen(*this));
        if (node->m_init)
        {
            auto val = node->m_init->codegen(*this);
            if (!val)
                return nullptr;
            m_ctx->m_builder->CreateStore(val, ptr);
        }

        return ptr;
    }
    else
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
}

llvm::Function *libquixcc::CodegenVisitor::visit(const libquixcc::FunctionDeclNode *node)
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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::StructDefNode *node)
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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::RegionDefNode *node)
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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::UnionDefNode *node)
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

llvm::Function *libquixcc::CodegenVisitor::visit(const libquixcc::FunctionDefNode *node)
{
    bool state = m_state.inside_function;
    m_state.inside_function = true;

    llvm::Function *func = static_cast<llvm::Function *>(node->m_decl->codegen(*this));

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

    if (!node->m_body->codegen(*this))
        return nullptr;

    m_state.inside_function = state;

    return func;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::FunctionParamNode *node)
{
    return nullptr;
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::SubsystemNode *node)
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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::ExportNode *node)
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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::InlineAsmNode *node)
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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::ReturnStmtNode *node)
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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::IfStmtNode *node)
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

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::WhileStmtNode *node)
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

///=============================================================================

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::ExprNode *node)
{
    switch (node->ntype)
    {
    case NodeType::UnaryExprNode:
        return visit(static_cast<const UnaryExprNode *>(node));
    case NodeType::BinaryExprNode:
        return visit(static_cast<const BinaryExprNode *>(node));
    case NodeType::CallExprNode:
        return visit(static_cast<const CallExprNode *>(node));
    case NodeType::ListExprNode:
        return visit(static_cast<const ListExprNode *>(node));
    case NodeType::MemberAccessNode:
        return visit(static_cast<const MemberAccessNode *>(node));
    case NodeType::ConstUnaryExprNode:
        return visit(static_cast<const ConstUnaryExprNode *>(node));
    case NodeType::ConstBinaryExprNode:
        return visit(static_cast<const ConstBinaryExprNode *>(node));
    case NodeType::IdentifierNode:
        return visit(static_cast<const IdentifierNode *>(node));
    case NodeType::IntegerLiteralNode:
        return visit(static_cast<const IntegerLiteralNode *>(node));
    case NodeType::FloatLiteralNode:
        return visit(static_cast<const FloatLiteralNode *>(node));
    case NodeType::StringLiteralNode:
        return visit(static_cast<const StringLiteralNode *>(node));
    case NodeType::CharLiteralNode:
        return visit(static_cast<const CharLiteralNode *>(node));
    case NodeType::BoolLiteralNode:
        return visit(static_cast<const BoolLiteralNode *>(node));
    case NodeType::NullLiteralNode:
        return visit(static_cast<const NullLiteralNode *>(node));
    default:
        throw CodegenException("NodeType: " + std::to_string(static_cast<int>(node->ntype)) + " is not codegenable");
    }
}

llvm::Value *libquixcc::CodegenVisitor::visit(const libquixcc::StmtNode *node)
{
    switch (node->ntype)
    {
    case NodeType::BlockNode:
        return visit(static_cast<const BlockNode *>(node));
    case NodeType::StmtGroupNode:
        return visit(static_cast<const StmtGroupNode *>(node));
    case NodeType::ExprStmtNode:
        return visit(static_cast<const ExprStmtNode *>(node));
    case NodeType::LetDeclNode:
        return visit(static_cast<const LetDeclNode *>(node));
    case NodeType::FunctionDeclNode:
        return visit(static_cast<const FunctionDeclNode *>(node));
    case NodeType::StructDefNode:
        return visit(static_cast<const StructDefNode *>(node));
    case NodeType::RegionDefNode:
        return visit(static_cast<const RegionDefNode *>(node));
    case NodeType::UnionDefNode:
        return visit(static_cast<const UnionDefNode *>(node));
    case NodeType::FunctionDefNode:
        return visit(static_cast<const FunctionDefNode *>(node));
    case NodeType::FunctionParamNode:
        return visit(static_cast<const FunctionParamNode *>(node));
    case NodeType::SubsystemNode:
        return visit(static_cast<const SubsystemNode *>(node));
    case NodeType::ExportNode:
        return visit(static_cast<const ExportNode *>(node));
    case NodeType::InlineAsmNode:
        return visit(static_cast<const InlineAsmNode *>(node));
    case NodeType::ReturnStmtNode:
        return visit(static_cast<const ReturnStmtNode *>(node));
    case NodeType::IfStmtNode:
        return visit(static_cast<const IfStmtNode *>(node));
    case NodeType::WhileStmtNode:
        return visit(static_cast<const WhileStmtNode *>(node));
    default:
        throw CodegenException("NodeType: " + std::to_string(static_cast<int>(node->ntype)) + " is not codegenable");
    }
}

llvm::Type *libquixcc::CodegenVisitor::visit(const libquixcc::TypeNode *node)
{
    switch (node->ntype)
    {
    case NodeType::U8TypeNode:
        return visit(static_cast<const U8TypeNode *>(node));
    case NodeType::U16TypeNode:
        return visit(static_cast<const U16TypeNode *>(node));
    case NodeType::U32TypeNode:
        return visit(static_cast<const U32TypeNode *>(node));
    case NodeType::U64TypeNode:
        return visit(static_cast<const U64TypeNode *>(node));
    case NodeType::I8TypeNode:
        return visit(static_cast<const I8TypeNode *>(node));
    case NodeType::I16TypeNode:
        return visit(static_cast<const I16TypeNode *>(node));
    case NodeType::I32TypeNode:
        return visit(static_cast<const I32TypeNode *>(node));
    case NodeType::I64TypeNode:
        return visit(static_cast<const I64TypeNode *>(node));
    case NodeType::F32TypeNode:
        return visit(static_cast<const F32TypeNode *>(node));
    case NodeType::F64TypeNode:
        return visit(static_cast<const F64TypeNode *>(node));
    case NodeType::BoolTypeNode:
        return visit(static_cast<const BoolTypeNode *>(node));
    case NodeType::VoidTypeNode:
        return visit(static_cast<const VoidTypeNode *>(node));
    case NodeType::PointerTypeNode:
        return visit(static_cast<const PointerTypeNode *>(node));
    case NodeType::StringTypeNode:
        return visit(static_cast<const StringTypeNode *>(node));
    case NodeType::EnumTypeNode:
        return visit(static_cast<const EnumTypeNode *>(node));
    case NodeType::StructTypeNode:
        return visit(static_cast<const StructTypeNode *>(node));
    case NodeType::RegionTypeNode:
        return visit(static_cast<const RegionTypeNode *>(node));
    case NodeType::UnionTypeNode:
        return visit(static_cast<const UnionTypeNode *>(node));
    case NodeType::ArrayTypeNode:
        return visit(static_cast<const ArrayTypeNode *>(node));
    case NodeType::FunctionTypeNode:
        return visit(static_cast<const FunctionTypeNode *>(node));
    default:
        throw CodegenException("NodeType: " + std::to_string(static_cast<int>(node->ntype)) + " is not codegenable");
    }
}