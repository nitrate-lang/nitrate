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
#include <IR/Q/Memory.h>

#include <core/Logger.h>

#include <mangle/Symbol.h>

#include <stack>
#include <any>
#include <optional>

using namespace libquixcc;

struct State
{
    ReductionState red;
};

static void push_children(ParseNode *current, std::stack<ParseNode *> &s, State &state)
{
    switch (current->ntype)
    {
    case NodeType::ExprStmtNode:
        s.push(current->as<ExprStmtNode>()->m_expr.get());
        break;
    case NodeType::BlockNode:
        for (auto it = current->as<BlockNode>()->m_stmts.rbegin(); it != current->as<BlockNode>()->m_stmts.rend(); it++)
            s.push(it->get());
        break;
    case NodeType::StmtGroupNode:
        for (auto it = current->as<StmtGroupNode>()->m_stmts.rbegin(); it != current->as<StmtGroupNode>()->m_stmts.rend(); it++)
            s.push(it->get());
        break;
    case NodeType::CastExprNode:
        s.push(current->as<CastExprNode>()->m_expr.get());
        s.push(current->as<CastExprNode>()->m_type);
        break;
    case NodeType::StaticCastExprNode:
        s.push(current->as<StaticCastExprNode>()->m_expr.get());
        s.push(current->as<StaticCastExprNode>()->m_type);
        break;
    case NodeType::BitCastExprNode:
        s.push(current->as<BitCastExprNode>()->m_expr.get());
        s.push(current->as<BitCastExprNode>()->m_type);
        break;
    case NodeType::SignedUpcastExprNode:
        s.push(current->as<SignedUpcastExprNode>()->m_expr.get());
        s.push(current->as<SignedUpcastExprNode>()->m_type);
        break;
    case NodeType::UnsignedUpcastExprNode:
        s.push(current->as<UnsignedUpcastExprNode>()->m_expr.get());
        s.push(current->as<UnsignedUpcastExprNode>()->m_type);
        break;
    case NodeType::DowncastExprNode:
        s.push(current->as<DowncastExprNode>()->m_expr.get());
        s.push(current->as<DowncastExprNode>()->m_type);
        break;
    case NodeType::PtrToIntCastExprNode:
        s.push(current->as<PtrToIntCastExprNode>()->m_expr.get());
        s.push(current->as<PtrToIntCastExprNode>()->m_type);
        break;
    case NodeType::IntToPtrCastExprNode:
        s.push(current->as<IntToPtrCastExprNode>()->m_expr.get());
        s.push(current->as<IntToPtrCastExprNode>()->m_type);
        break;
    case NodeType::UnaryExprNode:
        s.push(current->as<UnaryExprNode>()->m_expr.get());
        break;
    case NodeType::BinaryExprNode:
        s.push(current->as<BinaryExprNode>()->m_lhs.get());
        s.push(current->as<BinaryExprNode>()->m_rhs.get());
        break;
    case NodeType::CallExprNode:
        // for (auto it = current->as<CallExprNode>()->m_named_args.rbegin(); it != current->as<CallExprNode>()->m_named_args.rend(); it++)
        //     s.push(it->second.get());
        for (auto it = current->as<CallExprNode>()->m_positional_args.rbegin(); it != current->as<CallExprNode>()->m_positional_args.rend(); it++)
            s.push(it->get());
        s.push(current->as<CallExprNode>()->m_decl.get());
        break;
    case NodeType::ListExprNode:
        for (auto it = current->as<ListExprNode>()->m_elements.rbegin(); it != current->as<ListExprNode>()->m_elements.rend(); it++)
            s.push(it->get());
        break;
    case NodeType::MemberAccessNode:
        s.push(current->as<MemberAccessNode>()->m_expr.get());
        break;
    case NodeType::ConstUnaryExprNode:
        s.push(current->as<ConstUnaryExprNode>()->m_expr.get());
        break;
    case NodeType::ConstBinaryExprNode:
        s.push(current->as<ConstBinaryExprNode>()->m_lhs.get());
        s.push(current->as<ConstBinaryExprNode>()->m_rhs.get());
        break;
    case NodeType::MutTypeNode:
        s.push(current->as<MutTypeNode>()->m_type);
        break;
    case NodeType::PointerTypeNode:
        s.push(current->as<PointerTypeNode>()->m_type);
        break;
    case NodeType::StructTypeNode:
        for (auto it = current->as<StructTypeNode>()->m_fields.rbegin(); it != current->as<StructTypeNode>()->m_fields.rend(); it++)
            s.push(*it);
        break;
    case NodeType::RegionTypeNode:
        for (auto it = current->as<RegionTypeNode>()->m_fields.rbegin(); it != current->as<RegionTypeNode>()->m_fields.rend(); it++)
            s.push(*it);
        break;
    case NodeType::UnionTypeNode:
        for (auto it = current->as<UnionTypeNode>()->m_fields.rbegin(); it != current->as<UnionTypeNode>()->m_fields.rend(); it++)
            s.push(*it);
        break;
    case NodeType::ArrayTypeNode:
        s.push(current->as<ArrayTypeNode>()->m_size.get());
        s.push(current->as<ArrayTypeNode>()->m_type);
        break;
    case NodeType::FunctionTypeNode:
        for (auto it = current->as<FunctionTypeNode>()->m_params.rbegin(); it != current->as<FunctionTypeNode>()->m_params.rend(); it++)
            s.push(it->second);
        s.push(current->as<FunctionTypeNode>()->m_return_type);
        break;
    case NodeType::VarDeclNode:
        s.push(current->as<VarDeclNode>()->m_type);
        if (current->as<VarDeclNode>()->m_init)
            s.push(current->as<VarDeclNode>()->m_init.get());
        break;
    case NodeType::LetDeclNode:
        s.push(current->as<LetDeclNode>()->m_type);
        if (current->as<LetDeclNode>()->m_init)
            s.push(current->as<LetDeclNode>()->m_init.get());
        break;
    case NodeType::FunctionDeclNode:
        for (auto it = current->as<FunctionDeclNode>()->m_params.rbegin(); it != current->as<FunctionDeclNode>()->m_params.rend(); it++)
            s.push(it->get());
        s.push(current->as<FunctionDeclNode>()->m_type->m_return_type);
        break;
    case NodeType::StructDefNode:
        for (auto it = current->as<StructDefNode>()->m_fields.rbegin(); it != current->as<StructDefNode>()->m_fields.rend(); it++)
            s.push(it->get());
        for (auto it = current->as<StructDefNode>()->m_methods.rbegin(); it != current->as<StructDefNode>()->m_methods.rend(); it++)
            s.push(it->get());
        for (auto it = current->as<StructDefNode>()->m_static_methods.rbegin(); it != current->as<StructDefNode>()->m_static_methods.rend(); it++)
            s.push(it->get());
        break;
    case NodeType::StructFieldNode:
        s.push(current->as<StructFieldNode>()->m_type);
        if (current->as<StructFieldNode>()->m_value)
            s.push(current->as<StructFieldNode>()->m_value.get());
        break;
    case NodeType::RegionDefNode:
        for (auto it = current->as<RegionDefNode>()->m_fields.rbegin(); it != current->as<RegionDefNode>()->m_fields.rend(); it++)
            s.push(it->get());
        break;
    case NodeType::RegionFieldNode:
        s.push(current->as<RegionFieldNode>()->m_type);
        if (current->as<RegionFieldNode>()->m_value)
            s.push(current->as<RegionFieldNode>()->m_value.get());
        break;
    case NodeType::GroupFieldNode:
        s.push(current->as<GroupFieldNode>()->m_type);
        if (current->as<GroupFieldNode>()->m_value)
            s.push(current->as<GroupFieldNode>()->m_value.get());
        break;
    case NodeType::UnionDefNode:
        for (auto it = current->as<UnionDefNode>()->m_fields.rbegin(); it != current->as<UnionDefNode>()->m_fields.rend(); it++)
            s.push(it->get());
        break;
    case NodeType::UnionFieldNode:
        s.push(current->as<UnionFieldNode>()->m_type);
        if (current->as<UnionFieldNode>()->m_value)
            s.push(current->as<UnionFieldNode>()->m_value.get());
        break;
    case NodeType::FunctionDefNode:
        s.push(current->as<FunctionDefNode>()->m_decl.get());
        s.push(current->as<FunctionDefNode>()->m_body.get());
        break;
    case NodeType::FunctionParamNode:
        s.push(current->as<FunctionParamNode>()->m_type);
        if (current->as<FunctionParamNode>()->m_value)
            s.push(current->as<FunctionParamNode>()->m_value.get());
        break;
    case NodeType::SubsystemNode:
        s.push(current->as<SubsystemNode>()->m_block.get());
        break;
    case NodeType::InlineAsmNode:
        for (auto it = current->as<InlineAsmNode>()->m_inputs.rbegin(); it != current->as<InlineAsmNode>()->m_inputs.rend(); it++)
            s.push(it->second.get());
        for (auto it = current->as<InlineAsmNode>()->m_outputs.rbegin(); it != current->as<InlineAsmNode>()->m_outputs.rend(); it++)
            s.push(it->second.get());
        break;
    case NodeType::ReturnStmtNode:
        if (current->as<ReturnStmtNode>()->m_expr)
            s.push(current->as<ReturnStmtNode>()->m_expr.get());
        break;
    case NodeType::RetifStmtNode:
        s.push(current->as<RetifStmtNode>()->m_cond.get());
        s.push(current->as<RetifStmtNode>()->m_return.get());
        break;
    case NodeType::RetzStmtNode:
        s.push(current->as<RetzStmtNode>()->m_cond.get());
        s.push(current->as<RetzStmtNode>()->m_return.get());
        break;
    case NodeType::RetvStmtNode:
        s.push(current->as<RetvStmtNode>()->m_cond.get());
        break;
    case NodeType::IfStmtNode:
        if (current->as<IfStmtNode>()->m_else)
            s.push(current->as<IfStmtNode>()->m_else.get());
        s.push(current->as<IfStmtNode>()->m_then.get());
        s.push(current->as<IfStmtNode>()->m_cond.get());
        break;
    case NodeType::WhileStmtNode:
        s.push(current->as<WhileStmtNode>()->m_cond.get());
        s.push(current->as<WhileStmtNode>()->m_stmt.get());
        break;
    case NodeType::ForStmtNode:
        if (current->as<ForStmtNode>()->m_init)
            s.push(current->as<ForStmtNode>()->m_init.get());
        if (current->as<ForStmtNode>()->m_cond)
            s.push(current->as<ForStmtNode>()->m_cond.get());
        if (current->as<ForStmtNode>()->m_step)
            s.push(current->as<ForStmtNode>()->m_step.get());
        if (current->as<ForStmtNode>()->m_stmt)
            s.push(current->as<ForStmtNode>()->m_stmt.get());
        break;

    default:
        break;
    }
}

std::optional<const ir::Value<ir::Q> *> staticcast_reduce(const ir::q::Expr *expr, const ir::q::Type *to)
{
    /*
    | Type A    | Type B    | Cast Type     |
    |-----------|-----------|---------------|
    | signed    | unsigned  | signed        |
    | signed    | signed    | signed        |
    | unsigned  | signed    | signed        |
    | unsigned  | unsigned  | unsigned      |
    | pointer   | integer   | ptr-to-int    |
    | integer   | pointer   | int-to-ptr    |
    */

    auto from = expr->infer();
    auto from_size = from->bitcount();
    auto to_size = to->as<ir::q::Type>()->bitcount();

    bool upcasting = from_size < to_size;

    if (to->is_ptr() && from->is_integer())
        return ir::q::IPtrCast::create(to, expr);
    if (to->is_integer() && from->is_ptr())
        return ir::q::PtrICast::create(to, expr);
    if (from->is_signed() || to->is_signed())
        return ir::q::SCast::create(to, expr);
    if (from->is_unsigned() && to->is_unsigned())
        return ir::q::UCast::create(to, expr);

    LOG(FATAL) << "error converting from static_cast to primitive casts" << std::endl;
    return nullptr;
}

class Adapter {

};

static void transform(std::stack<ParseNode *> &s2, std::stack<const ir::Value<ir::Q> *> &s3, State &state)
{
    using namespace libquixcc::ir;

    while (!s2.empty())
    {
        auto current = s2.top();
        s2.pop();

        switch (current->ntype)
        {
        case NodeType::BlockNode:
        {
            std::vector<const ir::Value<ir::Q> *> stmts;
            for (auto &stmt : current->as<BlockNode>()->m_stmts)
            {
                stmts.push_back(s3.top());
                s3.pop();
            }
            s3.push(q::Block::create(stmts));
            break;
        }
        case NodeType::CastExprNode:
            throw std::runtime_error("CastExprNode not implemented");
        case NodeType::StaticCastExprNode:
        {
            auto t = s3.top()->as<q::Type>();
            s3.pop();
            auto e = s3.top()->as<q::Expr>();
            s3.pop();

            auto c = staticcast_reduce(e, t);
            if (!c)
                throw std::runtime_error("Failed to reduce static_cast expression");

            s3.push(*c);
            break;
        }
        case NodeType::BitCastExprNode:
        {
            auto expr = s3.top()->as<q::Expr>();
            s3.pop();
            auto type = s3.top()->as<q::Type>();
            s3.pop();
            s3.push(q::Bitcast::create(type, expr));
            break;
        }
        case NodeType::SignedUpcastExprNode:
        {
            auto expr = s3.top()->as<q::Expr>();
            s3.pop();
            auto type = s3.top()->as<q::Type>();
            s3.pop();
            s3.push(q::SCast::create(type, expr));
            break;
        }
        case NodeType::UnsignedUpcastExprNode:
        {
            auto expr = s3.top()->as<q::Expr>();
            s3.pop();
            auto type = s3.top()->as<q::Type>();
            s3.pop();
            s3.push(q::UCast::create(type, expr));
            break;
        }
        case NodeType::DowncastExprNode:
        {
            auto expr = s3.top()->as<q::Expr>();
            s3.pop();
            auto type = s3.top()->as<q::Type>();
            s3.pop();
            s3.push(q::UCast::create(type, expr));
            break;
        }
        case NodeType::PtrToIntCastExprNode:
        {
            auto expr = s3.top()->as<q::Expr>();
            s3.pop();
            auto type = s3.top()->as<q::Type>();
            s3.pop();
            s3.push(q::PtrICast::create(type, expr));
            break;
        }
        case NodeType::IntToPtrCastExprNode:
        {
            auto expr = s3.top()->as<q::Expr>();
            s3.pop();
            auto type = s3.top()->as<q::Type>();
            s3.pop();
            s3.push(q::IPtrCast::create(type, expr));
            break;
        }
        case NodeType::UnaryExprNode:
        {
            auto ex = current->as<UnaryExprNode>();
            auto e = s3.top()->as<q::Expr>();
            s3.pop();

            switch (ex->m_op)
            {
            case Operator::Plus:
                s3.push(e);
                break;
            case Operator::Minus:
                s3.push(q::Sub::create(q::Number::create("0"), e));
                break;
            case Operator::LogicalNot:
                s3.push(q::Not::create(e));
                break;
            case Operator::BitwiseNot:
                s3.push(q::BitNot::create(e));
                break;
            case Operator::Increment:
                s3.push(q::Add::create(e, q::Number::create("1")));
                break;
            case Operator::Decrement:
                s3.push(q::Sub::create(e, q::Number::create("1")));
                break;
            default:
                throw std::runtime_error("UnaryExprNode not implemented");
            }
            break;
        }
        case NodeType::BinaryExprNode:
        {
            auto ex = current->as<BinaryExprNode>();
            auto rhs = s3.top()->as<q::Expr>();
            s3.pop();
            auto lhs = s3.top()->as<q::Expr>();
            s3.pop();

            switch (ex->m_op)
            {
            case Operator::Plus:
                s3.push(q::Add::create(lhs, rhs));
                break;
            case Operator::Minus:
                s3.push(q::Sub::create(lhs, rhs));
                break;
            case Operator::Multiply:
                s3.push(q::Mul::create(lhs, rhs));
                break;
            case Operator::Divide:
                s3.push(q::Div::create(lhs, rhs));
                break;
            case Operator::Modulo:
                s3.push(q::Mod::create(lhs, rhs));
                break;
            case Operator::BitwiseAnd:
                s3.push(q::BitAnd::create(lhs, rhs));
                break;
            case Operator::BitwiseOr:
                s3.push(q::BitOr::create(lhs, rhs));
                break;
            case Operator::BitwiseXor:
                s3.push(q::BitXor::create(lhs, rhs));
                break;
            case Operator::LeftShift:
                s3.push(q::Shl::create(lhs, rhs));
                break;
            case Operator::RightShift:
                s3.push(q::Shr::create(lhs, rhs));
                break;
            case Operator::Assign:
                s3.push(q::Assign::create(lhs, rhs));
                break;
            case Operator::LogicalAnd:
                s3.push(q::And::create(lhs, rhs));
                break;
            case Operator::LogicalOr:
                s3.push(q::Or::create(lhs, rhs));
                break;
            case Operator::LogicalXor:
                s3.push(q::Xor::create(lhs, rhs));
                break;
            case Operator::LessThan:
                s3.push(q::Lt::create(lhs, rhs));
                break;
            case Operator::GreaterThan:
                s3.push(q::Gt::create(lhs, rhs));
                break;
            case Operator::LessThanEqual:
                s3.push(q::Le::create(lhs, rhs));
                break;
            case Operator::GreaterThanEqual:
                s3.push(q::Ge::create(lhs, rhs));
                break;
            case Operator::Equal:
                s3.push(q::Eq::create(lhs, rhs));
                break;
            case Operator::NotEqual:
                s3.push(q::Ne::create(lhs, rhs));
                break;
            default:
                throw std::runtime_error("BinaryExprNode not implemented");
            }
            break;
        }
        case NodeType::CallExprNode:
        {
            /// TODO: default arguments, named arguments
            auto decl = s3.top()->as<ir::q::Global>();
            s3.pop();
            std::vector<const ir::Value<ir::Q> *> args;
            for (auto &arg : current->as<CallExprNode>()->m_positional_args)
            {
                auto expr = s3.top()->as<q::Expr>();
                s3.pop();
                args.push_back(expr);
            }
            s3.push(q::Call::create(decl, args));
            break;
        }
        case NodeType::ListExprNode:
            throw std::runtime_error("ListExprNode not implemented");
        case NodeType::MemberAccessNode:
            throw std::runtime_error("MemberAccessNode not implemented");
        case NodeType::ConstUnaryExprNode:
            throw std::runtime_error("ConstUnaryExprNode not implemented");
        case NodeType::ConstBinaryExprNode:
            throw std::runtime_error("ConstBinaryExprNode not implemented");
        case NodeType::IdentifierNode:
            s3.push(q::Ident::create(current->as<IdentifierNode>()->m_name));
            break;
        case NodeType::U8TypeNode:
            s3.push(q::U8::create());
            break;
        case NodeType::U16TypeNode:
            s3.push(q::U16::create());
            break;
        case NodeType::U32TypeNode:
            s3.push(q::U32::create());
            break;
        case NodeType::U64TypeNode:
            s3.push(q::U64::create());
            break;
        case NodeType::U128TypeNode:
            s3.push(q::U128::create());
            break;
        case NodeType::I8TypeNode:
            s3.push(q::I8::create());
            break;
        case NodeType::I16TypeNode:
            s3.push(q::I16::create());
            break;
        case NodeType::I32TypeNode:
            s3.push(q::I32::create());
            break;
        case NodeType::I64TypeNode:
            s3.push(q::I64::create());
            break;
        case NodeType::I128TypeNode:
            s3.push(q::I128::create());
            break;
        case NodeType::F32TypeNode:
            s3.push(q::F32::create());
            break;
        case NodeType::F64TypeNode:
            s3.push(q::F64::create());
            break;
        case NodeType::BoolTypeNode:
            s3.push(q::I1::create());
            break;
        case NodeType::VoidTypeNode:
            s3.push(q::Void::create());
            break;
        case NodeType::PointerTypeNode:
            s3.push(q::Ptr::create(s3.top()->as<q::Type>()));
            s3.pop();
            break;
        case NodeType::OpaqueTypeNode:
            s3.push(q::Opaque::create(current->as<OpaqueTypeNode>()->m_name));
            break;
        case NodeType::StringTypeNode:
            s3.push(q::Ptr::create(q::U8::create()));
            break;
        case NodeType::StructTypeNode:
            for (auto &field : current->as<StructTypeNode>()->m_fields)
                s3.pop();
            s3.push(q::Region::create(current->as<StructTypeNode>()->m_name));
            break;
        case NodeType::RegionTypeNode:
            for (auto &field : current->as<StructTypeNode>()->m_fields)
                s3.pop();
            s3.push(q::Region::create(current->as<RegionTypeNode>()->m_name));
            break;
        case NodeType::UnionTypeNode:
            for (auto &field : current->as<StructTypeNode>()->m_fields)
                s3.pop();
            s3.push(q::Union::create(current->as<UnionTypeNode>()->m_name));
            break;
        case NodeType::ArrayTypeNode:
        {
            auto type = s3.top()->as<q::Type>();
            s3.pop();
            auto size = std::atoll(current->as<ArrayTypeNode>()->m_size->reduce<IntegerNode>(state.red)->m_val.c_str());
            s3.pop();
            s3.push(q::Array::create(type, size));
            break;
        }
        case NodeType::FunctionTypeNode:
        {
            std::vector<const ir::Value<ir::Q> *> params;
            auto f = current->as<FunctionTypeNode>();
            for (auto &param : current->as<FunctionTypeNode>()->m_params)
            {
                auto type = s3.top()->as<q::Type>();
                s3.pop();
                params.push_back(type);
            }
            auto return_type = s3.top();
            s3.pop();
            std::set<q::FConstraint> constraints;
            if (f->m_pure)
                constraints.insert(q::FConstraint::Pure);
            if (f->m_thread_safe)
                constraints.insert(q::FConstraint::ThreadSafe);
            if (f->m_foreign)
                constraints.insert(q::FConstraint::C_ABI);
            if (f->m_nothrow)
                constraints.insert(q::FConstraint::NoThrow);
            if (f->m_variadic)
                constraints.insert(q::FConstraint::Variadic);
            s3.push(q::Segment::create(params, return_type, nullptr, constraints));
            break;
        }
        case NodeType::IntegerNode:
            s3.push(q::Number::create(current->as<IntegerNode>()->m_val));
            break;
        case NodeType::FloatLiteralNode:
            s3.push(q::Number::create(current->as<FloatLiteralNode>()->m_val));
            break;
        case NodeType::StringNode:
            s3.push(q::String::create(current->as<StringNode>()->m_val));
            break;
        case NodeType::CharNode:
            s3.push(q::Number::create(current->as<CharNode>()->m_val));
            break;
        case NodeType::BoolLiteralNode:
            s3.push(q::Number::create(current->as<BoolLiteralNode>()->m_val ? "1" : "0"));
            break;
        case NodeType::NullLiteralNode:
            s3.push(q::Number::create("0"));
            break;
        case NodeType::VarDeclNode:
            throw std::runtime_error("VarDeclNode not implemented");
        case NodeType::LetDeclNode:
        {
            /// TODO: set default value
            // const q::Expr *expr = nullptr;
            if (current->as<LetDeclNode>()->m_init)
            {
                // expr = s3.top()->as<q::Expr>();
                s3.pop();
            }
            auto type = s3.top()->as<q::Type>();
            s3.pop();
            auto name = current->as<LetDeclNode>()->m_name;

            // if (expr)

            s3.push(q::Local::create(name, type));
            // s3.push(q::Assign::create(q::Ident::create(name), q::Number::create("0")));

            break;
        }
        case NodeType::FunctionDeclNode:
        {
            auto type = s3.top()->as<q::Type>();
            s3.pop();
            auto f = current->as<FunctionDeclNode>()->m_type;
            std::vector<const ir::Value<ir::Q> *> params;
            for (auto &param : current->as<FunctionDeclNode>()->m_params)
            {
                auto type = s3.top()->as<q::Type>();
                s3.pop();
                params.push_back(type);
            }
            std::set<q::FConstraint> constraints;
            if (f->m_pure)
                constraints.insert(q::FConstraint::Pure);
            if (f->m_thread_safe)
                constraints.insert(q::FConstraint::ThreadSafe);
            if (f->m_foreign)
                constraints.insert(q::FConstraint::C_ABI);
            if (f->m_nothrow)
                constraints.insert(q::FConstraint::NoThrow);
            if (f->m_variadic)
                constraints.insert(q::FConstraint::Variadic);

            std::vector<const q::Type *> ptypes;
            for (auto &param : params)
                ptypes.push_back(param->as<q::Type>());
            auto ftype = q::FType::create(ptypes, type, f->m_variadic, f->m_pure, f->m_thread_safe, f->m_foreign, f->m_nothrow);
            auto fndecl = q::Segment::create(params, type, nullptr, constraints);
            const libquixcc::ir::q::Global *globfb;

            if (current->_m_export_lang == ExportLangType::None)
            {
                globfb = q::Global::create(Symbol::mangle(current->as<DeclNode>(), "", ExportLangType::Default), ftype, fndecl, false, false, false);
            }
            else
            {
                globfb = q::Global::create(Symbol::mangle(current->as<DeclNode>(), "", current->_m_export_lang), ftype, fndecl, false, false, true);
            }

            s3.push(globfb);
            break;
        }
        case NodeType::StructDefNode:
        {
            std::vector<std::pair<std::string, const ir::Value<ir::Q> *>> fields;
            std::set<const ir::q::Segment *> methods;
            for (auto &field : current->as<StructDefNode>()->m_fields)
            {
                auto type = s3.top()->as<q::Type>();
                s3.pop();
                fields.push_back({field->m_name, type});
            }
            for (auto &method : current->as<StructDefNode>()->m_methods)
            {
                auto type = s3.top()->as<q::Type>();
                s3.pop();
                methods.insert(type->as<ir::q::Segment>());
            }
            for (auto &static_method : current->as<StructDefNode>()->m_static_methods)
            {
                auto type = s3.top()->as<q::Type>();
                s3.pop();
                methods.insert(type->as<ir::q::Segment>());
            }

            s3.push(q::RegionDef::create(current->as<StructDefNode>()->m_name, fields, methods));
            break;
        }
        case NodeType::RegionDefNode:
        {
            std::vector<std::pair<std::string, const ir::Value<ir::Q> *>> fields;
            std::set<const ir::q::Segment *> methods;
            for (auto &field : current->as<RegionDefNode>()->m_fields)
            {
                auto type = s3.top()->as<q::Type>();
                s3.pop();
                fields.push_back({field->m_name, type});
            }

            s3.push(q::RegionDef::create(current->as<StructDefNode>()->m_name, fields, methods));
            break;
        }
        case NodeType::UnionDefNode:
        {
            std::map<std::string, const ir::Value<ir::Q> *> fields;
            std::set<const ir::q::Segment *> methods;
            for (auto &field : current->as<UnionDefNode>()->m_fields)
            {
                auto type = s3.top()->as<q::Type>();
                s3.pop();
                fields.insert({field->m_name, type});
            }

            s3.push(q::UnionDef::create(current->as<UnionDefNode>()->m_name, fields, methods));
            break;
        }
        case NodeType::GroupDefNode:
            throw std::runtime_error("GroupDefNode not implemented");
        case NodeType::FunctionDefNode:
        {
            auto body = s3.top()->as<ir::q::Block>();
            s3.pop();
            auto glob = s3.top()->as<ir::q::Global>();
            s3.pop();
            auto seg = glob->value->as<ir::q::Segment>();

            auto f = q::Segment::create(seg->params, seg->return_type, body, seg->constraints);
            s3.push(q::Global::create(glob->name, glob->type, f, glob->_volatile, glob->_atomic));
            break;
        }
        case NodeType::FunctionParamNode:
        {
            if (current->as<FunctionParamNode>()->m_value)
                s3.pop();
            // auto p = s3.top();
            // s3.pop();

            /// TODO: Implement FunctionParamNode
            break;
        }
        case NodeType::InlineAsmNode:
            throw std::runtime_error("InlineAsmNode not implemented");
        case NodeType::ReturnStmtNode:
        {
            if (current->as<ReturnStmtNode>()->m_expr)
            {
                auto expr = s3.top()->as<q::Expr>();
                s3.pop();
                s3.push(q::Ret::create(expr));
            }
            else
            {
                s3.push(q::Ret::create(nullptr));
            }
            break;
        }
        case NodeType::RetifStmtNode:
        {
            auto cond = s3.top()->as<q::Expr>();
            s3.pop();
            auto ret = s3.top()->as<q::Expr>();
            s3.pop();
            s3.push(q::IfElse::create(cond, q::Ret::create(ret), nullptr));
            break;
        }
        case NodeType::RetzStmtNode:
        {
            auto cond = s3.top()->as<q::Expr>();
            s3.pop();
            auto ret = s3.top()->as<q::Expr>();
            s3.pop();
            s3.push(q::IfElse::create(q::Not::create(cond), q::Ret::create(ret), nullptr));
            break;
        }
        case NodeType::RetvStmtNode:
        {
            auto cond = s3.top()->as<q::Expr>();
            s3.pop();
            s3.push(q::IfElse::create(cond, q::Ret::create(nullptr), nullptr));
            break;
        }
        case NodeType::IfStmtNode:
        {
            auto cond = s3.top()->as<q::Expr>();
            s3.pop();
            auto then_block = s3.top();
            s3.pop();

            const Value<Q> *else_block = nullptr;
            if (current->as<IfStmtNode>()->m_else)
            {
                else_block = s3.top();
                s3.pop();
            }
            else
            {
                else_block = q::Block::create({});
            }

            s3.push(q::IfElse::create(cond, then_block, else_block));
            break;
        }
        case NodeType::WhileStmtNode:
        {
            auto stmt = s3.top();
            s3.pop();
            auto cond = s3.top()->as<q::Expr>();
            s3.pop();
            s3.push(q::While::create(cond, stmt));
            break;
        }
        case NodeType::ForStmtNode:
        {
            auto stmt = s3.top();
            s3.pop();
            auto step = s3.top()->as<q::Expr>();
            s3.pop();
            auto cond = s3.top()->as<q::Expr>();
            s3.pop();
            auto init = s3.top()->as<q::Expr>();
            s3.pop();
            s3.push(q::For::create(init, cond, step, stmt));
            break;
        }
        default:
            break;
        }
    }
}

static void translate_ast(std::shared_ptr<BlockNode> ast, const ir::q::RootNode **root)
{
    /*
     * 1. Iterate over the general polymorphic AST `bottom-up`
     * 2. Translate each node into 0 or more QIR nodes
     */

    using namespace ir;

    State state;
    std::stack<ParseNode *> s1;
    std::stack<ParseNode *> s2;
    std::stack<const Value<Q> *> s3;
    bool is_public = false;

    for (auto it = ast->m_stmts.rbegin(); it != ast->m_stmts.rend(); it++)
        s1.push(it->get());

    while (!s1.empty())
    {
        auto current = s1.top();
        s1.pop();
        push_children(current, s1, state);
        s2.push(current);
    }

    transform(s2, s3, state);

    std::vector<const Value<Q> *> children;
    while (!s3.empty())
    {
        children.push_back(s3.top());
        s3.pop();
    }

    *root = q::RootNode::create(children);
}

bool ir::q::QModule::from_ast(std::shared_ptr<BlockNode> ast)
{
    translate_ast(ast, &m_root);

    if (!verify())
    {
        LOG(FATAL) << "Failed to qualify QUIX intermediate representation" << std::endl;
        return false;
    }

    print(std::cout);

    return true;
}