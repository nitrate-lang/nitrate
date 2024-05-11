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
#include <core/Logger.h>

#include <stack>
#include <any>

using namespace libquixcc;

/*
ExprStmtNode
BlockNode
StmtGroupNode
CastExprNode
StaticCastExprNode
BitCastExprNode
SignedUpcastExprNode
UnsignedUpcastExprNode
DowncastExprNode
PtrToIntCastExprNode
IntToPtrCastExprNode
UnaryExprNode
BinaryExprNode
CallExprNode
ListExprNode
MemberAccessNode
ConstUnaryExprNode
ConstBinaryExprNode
IdentifierNode
MutTypeNode
U8TypeNode
U16TypeNode
U32TypeNode
U64TypeNode
U128TypeNode
I8TypeNode
I16TypeNode
I32TypeNode
I64TypeNode
I128TypeNode
F32TypeNode
F64TypeNode
BoolTypeNode
VoidTypeNode
PointerTypeNode
OpaqueTypeNode
StringTypeNode
EnumTypeNode
StructTypeNode
RegionTypeNode
UnionTypeNode
ArrayTypeNode
FunctionTypeNode
UserTypeNode
IntegerNode
FloatLiteralNode
StringNode
CharNode
BoolLiteralNode
NullLiteralNode
TypedefNode
VarDeclNode
LetDeclNode
FunctionDeclNode
StructDefNode
StructFieldNode
RegionDefNode
RegionFieldNode
GroupDefNode
GroupFieldNode
UnionDefNode
UnionFieldNode
EnumDefNode
EnumFieldNode
FunctionDefNode
FunctionParamNode
SubsystemNode
ExportNode
InlineAsmNode
ReturnStmtNode
RetifStmtNode
RetzStmtNode
RetvStmtNode
IfStmtNode
WhileStmtNode
ForStmtNode
*/

static void push_children(ParseNode *current, std::stack<ParseNode *> &s)
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
        s.push(current->as<CallExprNode>()->m_decl.get());
        for (auto it = current->as<CallExprNode>()->m_named_args.rbegin(); it != current->as<CallExprNode>()->m_named_args.rend(); it++)
            s.push(it->second.get());
        for (auto it = current->as<CallExprNode>()->m_positional_args.rbegin(); it != current->as<CallExprNode>()->m_positional_args.rend(); it++)
            s.push(it->get());
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
    case NodeType::EnumDefNode:
        for (auto it = current->as<EnumDefNode>()->m_fields.rbegin(); it != current->as<EnumDefNode>()->m_fields.rend(); it++)
            s.push(it->get());
        s.push(current->as<EnumDefNode>()->m_type);
        break;
    case NodeType::EnumFieldNode:
        s.push(current->as<EnumFieldNode>()->m_value.get());
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
    case NodeType::ExportNode:
        for (auto it = current->as<ExportNode>()->m_stmts.rbegin(); it != current->as<ExportNode>()->m_stmts.rend(); it++)
            s.push(it->get());
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
        s.push(current->as<IfStmtNode>()->m_cond.get());
        s.push(current->as<IfStmtNode>()->m_then.get());
        if (current->as<IfStmtNode>()->m_else)
            s.push(current->as<IfStmtNode>()->m_else.get());
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

static void translate_ast(std::shared_ptr<libquixcc::BlockNode> ast, const libquixcc::ir::q::RootNode **root)
{
    /*
     * 1. Iterate over the general polymorphic AST `bottom-up`
     * 2. Translate each node into 0 or more QIR nodes
     */

    using namespace libquixcc::ir;

    ReductionState state;
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
        push_children(current, s1);
        s2.push(current);
    }

    while (!s2.empty())
    {
        auto current = s2.top();
        s2.pop();

        LOG(libquixcc::INFO) << NodeTypeNames[current->ntype] << std::endl;

        switch (current->ntype)
        {
        case NodeType::BlockNode:
        {
            std::vector<const libquixcc::ir::Value<libquixcc::ir::Q> *> stmts;
            for (auto &stmt : current->as<BlockNode>()->m_stmts)
            {
                stmts.push_back(s3.top());
                s3.pop();
            }
            s3.push(q::FunctionBlock::create(stmts));
            break;
        }
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
            s3.push(q::Ptr::create(s3.top()));
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
            s3.push(q::Array::create(s3.top(), std::atoll(s3.top()->as<ConstExprNode>()->reduce<IntegerNode>(state)->m_val.c_str())));
            s3.pop();
            s3.pop();
            break;
        case NodeType::FunctionTypeNode:
        {
            std::vector<const libquixcc::ir::Value<libquixcc::ir::Q> *> params;
            for (auto &param : current->as<FunctionTypeNode>()->m_params)
            {
                auto type = s3.top();
                s3.pop();
                params.push_back(type);
            }
            auto return_type = s3.top();
            s3.pop();
            s3.push(q::Function::create("", params, return_type, nullptr, {}));
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
            auto type = s3.top();
            s3.pop();
            s3.push(q::Local::create(current->as<LetDeclNode>()->m_name, type));
            break;
        }
        case NodeType::FunctionDeclNode:
        {
            auto type = s3.top();
            s3.pop();
            std::vector<const libquixcc::ir::Value<libquixcc::ir::Q> *> params;
            for (auto &param : current->as<FunctionDeclNode>()->m_params)
            {
                auto type = s3.top();
                s3.pop();
                params.push_back(type);
            }
            std::set<q::FConstraint> constraints;
            s3.push(q::Function::create(current->as<FunctionDeclNode>()->m_name, params, type, nullptr, constraints));
            break;
        }
        case NodeType::StructDefNode:
        {
            std::vector<std::pair<std::string, const libquixcc::ir::Value<libquixcc::ir::Q> *>> fields;
            std::set<const libquixcc::ir::q::Function *> methods;
            for (auto &field : current->as<StructDefNode>()->m_fields)
            {
                auto type = s3.top();
                s3.pop();
                fields.push_back({field->m_name, type});
            }
            for (auto &method : current->as<StructDefNode>()->m_methods)
            {
                auto type = s3.top();
                s3.pop();
                methods.insert(type->as<libquixcc::ir::q::Function>());
            }
            for (auto &static_method : current->as<StructDefNode>()->m_static_methods)
            {
                auto type = s3.top();
                s3.pop();
                methods.insert(type->as<libquixcc::ir::q::Function>());
            }

            s3.push(q::RegionDef::create(current->as<StructDefNode>()->m_name, fields, methods));
            break;
        }
        case NodeType::RegionDefNode:
        {
            std::vector<std::pair<std::string, const libquixcc::ir::Value<libquixcc::ir::Q> *>> fields;
            std::set<const libquixcc::ir::q::Function *> methods;
            for (auto &field : current->as<RegionDefNode>()->m_fields)
            {
                auto type = s3.top();
                s3.pop();
                fields.push_back({field->m_name, type});
            }

            s3.push(q::RegionDef::create(current->as<StructDefNode>()->m_name, fields, methods));
            break;
        }
        case NodeType::UnionDefNode:
        {
            std::map<std::string, const libquixcc::ir::Value<libquixcc::ir::Q> *> fields;
            std::set<const libquixcc::ir::q::Function *> methods;
            for (auto &field : current->as<UnionDefNode>()->m_fields)
            {
                auto type = s3.top();
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
            auto body = s3.top()->as<libquixcc::ir::q::FunctionBlock>();
            s3.pop();
            auto decl = s3.top()->as<libquixcc::ir::q::Function>();
            s3.pop();

            s3.push(q::Function::create(decl->name, decl->params, decl->return_type, body, decl->constraints));
            break;
        }
        case NodeType::FunctionParamNode:
        {
            s3.pop();
            if (current->as<FunctionParamNode>()->m_value)
                s3.pop();

            /// TODO: Implement FunctionParamNode
            break;
        }
        case NodeType::ExportNode:
            throw std::runtime_error("ExportNode not implemented");
        case NodeType::InlineAsmNode:
            throw std::runtime_error("InlineAsmNode not implemented");
        case NodeType::ReturnStmtNode:
        {
            if (current->as<ReturnStmtNode>()->m_expr)
            {
                auto expr = s3.top();
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
            auto cond = s3.top();
            s3.pop();
            auto ret = s3.top();
            s3.pop();
            s3.push(q::IfElse::create(cond, q::Ret::create(ret), nullptr));
            break;
        }
        case NodeType::RetzStmtNode:
        {
            auto cond = s3.top();
            s3.pop();
            auto ret = s3.top();
            s3.pop();
            s3.push(q::IfElse::create(q::Not::create(cond), q::Ret::create(ret), nullptr));
            break;
        }
        case NodeType::RetvStmtNode:
        {
            auto cond = s3.top();
            s3.pop();
            s3.push(q::IfElse::create(cond, q::Ret::create(nullptr), nullptr));
            break;
        }
        case NodeType::IfStmtNode:
        {
            auto else_block = s3.top();
            s3.pop();
            auto then_block = s3.top();
            s3.pop();
            auto cond = s3.top();
            s3.pop();
            s3.push(q::IfElse::create(cond, then_block, else_block));
            break;
        }
        case NodeType::WhileStmtNode:
        {
            auto stmt = s3.top();
            s3.pop();
            auto cond = s3.top();
            s3.pop();
            s3.push(q::While::create(cond, stmt));
            break;
        }
        case NodeType::ForStmtNode:
        {
            auto stmt = s3.top();
            s3.pop();
            auto step = s3.top();
            s3.pop();
            auto cond = s3.top();
            s3.pop();
            auto init = s3.top();
            s3.pop();
            s3.push(q::For::create(init, cond, step, stmt));
            break;
        }
        default:
            break;
        }
    }

    std::vector<const Value<Q> *> children;
    while (!s3.empty())
    {
        children.push_back(s3.top());
        s3.pop();
    }

    *root = q::RootNode::create(children);
}

bool libquixcc::ir::q::QModule::from_ast(std::shared_ptr<libquixcc::BlockNode> ast)
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