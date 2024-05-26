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
using namespace libquixcc::ir::q;

typedef const Value *QValue;

class QResult
{
    std::vector<QValue> m_values;

public:
    QResult() = default;
    QResult(const QValue &value)
    {
        m_values.push_back(value);
    }
    QResult(const std::vector<QValue> &values)
        : m_values(values) {}
    QResult(const std::initializer_list<QValue> &values)
        : m_values(values) {}

    operator bool() const
    {
        if (m_values.empty())
            return false;

        return m_values[0] != nullptr;
    }

    auto operator*() const -> const std::vector<QValue> & { return m_values; }
    auto operator->() const -> const std::vector<QValue> & { return m_values; }
    bool empty() const { return m_values.empty(); }
    QValue operator[](size_t i) const { return m_values.at(i); }

    auto begin() const { return m_values.begin(); }
    auto end() const { return m_values.end(); }
};

struct QState
{
    std::map<std::string, std::pair<ExportLangType, QResult>> exported;
    std::stack<std::map<std::string, const Type *>> local_idents;
    std::map<std::string, const Type *> global_idents;
    bool inside_segment;
    ExportLangType lang;
    std::stack<const FunctionDefNode *> function;
    std::map<std::string, const DefNode *> typedefs;

    QState()
    {
        inside_segment = false;
        lang = ExportLangType::None;
    }
};

static auto conv(const ParseNode *n, QState &state) -> QResult;
static auto conv(const ExprStmtNode *n, QState &state) -> QResult;
static auto conv(const NopStmtNode *n, QState &state) -> QResult;
static auto conv(const BlockNode *n, QState &state) -> QResult;
static auto conv(const StmtGroupNode *n, QState &state) -> QResult;
static auto conv(const StaticCastExprNode *n, QState &state) -> QResult;
static auto conv(const BitCastExprNode *n, QState &state) -> QResult;
static auto conv(const SignedUpcastExprNode *n, QState &state) -> QResult;
static auto conv(const UnsignedUpcastExprNode *n, QState &state) -> QResult;
static auto conv(const DowncastExprNode *n, QState &state) -> QResult;
static auto conv(const PtrToIntCastExprNode *n, QState &state) -> QResult;
static auto conv(const IntToPtrCastExprNode *n, QState &state) -> QResult;
static auto conv(const UnaryExprNode *n, QState &state) -> QResult;
static auto conv(const BinaryExprNode *n, QState &state) -> QResult;
static auto conv(const CallExprNode *n, QState &state) -> QResult;
static auto conv(const ListExprNode *n, QState &state) -> QResult;
static auto conv(const MemberAccessNode *n, QState &state) -> QResult;
static auto conv(const IndexNode *n, QState &state) -> QResult;
static auto conv(const ConstUnaryExprNode *n, QState &state) -> QResult;
static auto conv(const ConstBinaryExprNode *n, QState &state) -> QResult;
static auto conv(const IdentifierNode *n, QState &state) -> QResult;
static auto conv(const ImmMutTypeNode *n, QState &state) -> QResult;
static auto conv(const U8TypeNode *n, QState &state) -> QResult;
static auto conv(const U16TypeNode *n, QState &state) -> QResult;
static auto conv(const U32TypeNode *n, QState &state) -> QResult;
static auto conv(const U64TypeNode *n, QState &state) -> QResult;
static auto conv(const U128TypeNode *n, QState &state) -> QResult;
static auto conv(const I8TypeNode *n, QState &state) -> QResult;
static auto conv(const I16TypeNode *n, QState &state) -> QResult;
static auto conv(const I32TypeNode *n, QState &state) -> QResult;
static auto conv(const I64TypeNode *n, QState &state) -> QResult;
static auto conv(const I128TypeNode *n, QState &state) -> QResult;
static auto conv(const F32TypeNode *n, QState &state) -> QResult;
static auto conv(const F64TypeNode *n, QState &state) -> QResult;
static auto conv(const BoolTypeNode *n, QState &state) -> QResult;
static auto conv(const VoidTypeNode *n, QState &state) -> QResult;
static auto conv(const PointerTypeNode *n, QState &state) -> QResult;
static auto conv(const OpaqueTypeNode *n, QState &state) -> QResult;
static auto conv(const StringTypeNode *n, QState &state) -> QResult;
static auto conv(const EnumTypeNode *n, QState &state) -> QResult;
static auto conv(const StructTypeNode *n, QState &state) -> QResult;
static auto conv(const GroupTypeNode *n, QState &state) -> QResult;
static auto conv(const RegionTypeNode *n, QState &state) -> QResult;
static auto conv(const UnionTypeNode *n, QState &state) -> QResult;
static auto conv(const ArrayTypeNode *n, QState &state) -> QResult;
static auto conv(const VectorTypeNode *n, QState &state) -> QResult;
static auto conv(const FunctionTypeNode *n, QState &state) -> QResult;
static auto conv(const IntegerNode *n, QState &state) -> QResult;
static auto conv(const FloatLiteralNode *n, QState &state) -> QResult;
static auto conv(const StringNode *n, QState &state) -> QResult;
static auto conv(const CharNode *n, QState &state) -> QResult;
static auto conv(const BoolLiteralNode *n, QState &state) -> QResult;
static auto conv(const NullLiteralNode *n, QState &state) -> QResult;
static auto conv(const TypedefNode *n, QState &state) -> QResult;
static auto conv(const VarDeclNode *n, QState &state) -> QResult;
static auto conv(const LetDeclNode *n, QState &state) -> QResult;
static auto conv(const ConstDeclNode *n, QState &state) -> QResult;
static auto conv(const FunctionDeclNode *n, QState &state) -> QResult;
static auto conv(const StructDefNode *n, QState &state) -> QResult;
static auto conv(const StructFieldNode *n, QState &state) -> QResult;
static auto conv(const RegionDefNode *n, QState &state) -> QResult;
static auto conv(const RegionFieldNode *n, QState &state) -> QResult;
static auto conv(const GroupDefNode *n, QState &state) -> QResult;
static auto conv(const GroupFieldNode *n, QState &state) -> QResult;
static auto conv(const UnionDefNode *n, QState &state) -> QResult;
static auto conv(const UnionFieldNode *n, QState &state) -> QResult;
static auto conv(const EnumDefNode *n, QState &state) -> QResult;
static auto conv(const EnumFieldNode *n, QState &state) -> QResult;
static auto conv(const FunctionDefNode *n, QState &state) -> QResult;
static auto conv(const FunctionParamNode *n, QState &state) -> QResult;
static auto conv(const ExportNode *n, QState &state) -> QResult;
static auto conv(const InlineAsmNode *n, QState &state) -> QResult;
static auto conv(const ReturnStmtNode *n, QState &state) -> QResult;
static auto conv(const RetifStmtNode *n, QState &state) -> QResult;
static auto conv(const RetzStmtNode *n, QState &state) -> QResult;
static auto conv(const RetvStmtNode *n, QState &state) -> QResult;
static auto conv(const IfStmtNode *n, QState &state) -> QResult;
static auto conv(const WhileStmtNode *n, QState &state) -> QResult;
static auto conv(const ForStmtNode *n, QState &state) -> QResult;

static auto conv(const ExprStmtNode *n, QState &state) -> QResult
{
    return conv(n->m_expr.get(), state);
}

static auto conv(const NopStmtNode *n, QState &state) -> QResult
{
    return nullptr;
}

static auto conv(const BlockNode *n, QState &state) -> QResult
{
    std::vector<QValue> sub;
    for (auto &stmt : n->m_stmts)
    {
        auto res = conv(stmt.get(), state);
        if (!res)
            continue;

        for (auto val : *res)
        {
            if (!val)
                continue;

            sub.push_back(val);
        }
    }
    return Block::create(sub);
}

static auto conv(const StmtGroupNode *n, QState &state) -> QResult
{
    std::vector<QValue> sub;
    for (auto &stmt : n->m_stmts)
    {
        auto res = conv(stmt.get(), state);
        if (!res)
            continue;
        for (auto &val : *res)
        {
            if (!val)
                continue;
            sub.push_back(val);
        }
    }
    return sub;
}

static auto conv(const StaticCastExprNode *n, QState &state) -> QResult
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

    auto expr = conv(n->m_expr.get(), state)[0]->as<Expr>();
    auto to = conv(n->m_type, state)[0]->as<Type>();

    auto from = expr->infer();
    if (to->is(from))
        return expr;

    if (to->is_ptr() && from->is_integer())
        return ir::q::IPtrCast::create(to, expr);
    if (to->is_integer() && from->is_ptr())
        return ir::q::PtrICast::create(expr);
    if (from->is_signed() || to->is_signed())
        return ir::q::SCast::create(to, expr);
    if (from->is_unsigned() && to->is_unsigned())
        return ir::q::UCast::create(to, expr);

    LOG(FATAL) << "error converting from static_cast to primitive casts" << std::endl;
    return nullptr;
}

static auto conv(const BitCastExprNode *n, QState &state) -> QResult
{
    return Bitcast::create(conv(n->m_type, state)[0]->as<Type>(), conv(n->m_expr.get(), state)[0]->as<Expr>());
}

static auto conv(const SignedUpcastExprNode *n, QState &state) -> QResult
{
    return SCast::create(conv(n->m_type, state)[0]->as<Type>(), conv(n->m_expr.get(), state)[0]->as<Expr>());
}

static auto conv(const UnsignedUpcastExprNode *n, QState &state) -> QResult
{
    return UCast::create(conv(n->m_type, state)[0]->as<Type>(), conv(n->m_expr.get(), state)[0]->as<Expr>());
}

static auto conv(const DowncastExprNode *n, QState &state) -> QResult
{
    return UCast::create(conv(n->m_type, state)[0]->as<Type>(), conv(n->m_expr.get(), state)[0]->as<Expr>());
}

static auto conv(const PtrToIntCastExprNode *n, QState &state) -> QResult
{
    return PtrICast::create(conv(n->m_expr.get(), state)[0]->as<Expr>());
}

static auto conv(const IntToPtrCastExprNode *n, QState &state) -> QResult
{
    return IPtrCast::create(conv(n->m_type, state)[0]->as<Type>(), conv(n->m_expr.get(), state)[0]->as<Expr>());
}

static auto conv(const UnaryExprNode *n, QState &state) -> QResult
{
    auto e = conv(n->m_expr.get(), state)[0]->as<Expr>();

    switch (n->m_op)
    {
    case Operator::Plus:
        return e;
    case Operator::Minus:
        return Sub::create(UCast::create(e->infer(), Number::create("0")), e);
    case Operator::LogicalNot:
        return Not::create(e);
    case Operator::BitwiseNot:
        return BitNot::create(e);
    case Operator::Increment:
        return Assign::create(e, Add::create(e, UCast::create(e->infer(), Number::create("1"))));
    case Operator::Decrement:
        return Assign::create(e, Sub::create(e, UCast::create(e->infer(), Number::create("1"))));
    case Operator::BitwiseAnd:
        return AddressOf::create(e);
    case Operator::Multiply:
        return Deref::create(e);
    default:
        throw std::runtime_error("UnaryExprNode not implemented");
    }
}

static const Expr *promote(const Type *lht, const libquixcc::ir::q::Expr *rhs)
{
    auto rht = rhs->infer();

    if (lht->is(rht))
        return rhs;

    if (lht->is_void() || rht->is_void())
        throw std::runtime_error("cannot promote void type");

    if (lht->is_integer() && rht->is_ptr())
        return ir::q::PtrICast::create(rhs);

    if (lht->is_ptr() && rht->is_integer())
        return ir::q::IPtrCast::create(lht, rhs);

    if (lht->is_float() || rht->is_float())
        return ir::q::SCast::create(lht, rhs);

    if (lht->is<I1>() && !rht->is<I1>())
        return ir::q::Ne::create(rhs, ir::q::Number::create("0"));

    if (lht->is_signed() || rht->is_signed())
        return ir::q::SCast::create(lht, rhs);

    if (lht->is_unsigned() || rht->is_unsigned())
        return ir::q::UCast::create(lht, rhs);

    if (lht->is_ptr() && rht->is_ptr())
        return ir::q::Bitcast::create(lht, rhs);

    if (lht->is_ptr() && rht->is<Array>())
    {
        auto l = lht->as<Ptr>();
        auto r = rht->as<Array>();

        if (!l->type->is(r->type))
            throw std::runtime_error("cannot promote type to pointer");

        return ir::q::AddressOf::create(ir::q::Index::create(rhs, ir::q::Number::create("0"), l->type));
    }

    throw std::runtime_error("cannot promote types");
}

static const Expr *promote(const libquixcc::ir::q::Expr *lhs, const libquixcc::ir::q::Expr *rhs)
{
    return promote(lhs->infer(), rhs);
}

static void bipromote(const libquixcc::ir::q::Expr **lhs, const libquixcc::ir::q::Expr **rhs)
{
    auto lht = (*lhs)->infer();
    auto rht = (*rhs)->infer();

    if (!lht || !rht)
        throw std::runtime_error("failed promote types");

    if (lht->is_ptr())
        *lhs = ir::q::PtrICast::create(*lhs);

    if (rht->is_ptr())
        *rhs = ir::q::PtrICast::create(*rhs);

    lht = (*lhs)->infer();
    rht = (*rhs)->infer();

    if (lht->is(rht))
        return;

    if (lht->is_void() || rht->is_void())
        throw std::runtime_error("cannot promote void type");

    if ((*lhs)->is<Ident>())
    {
        *rhs = promote(*lhs, *rhs);
        return;
    }
    else if ((*rhs)->is<Ident>())
    {
        *lhs = promote(*rhs, *lhs);
        return;
    }

    if (lht->is_float() || rht->is_float())
    {
        /* We know they are not equal*/
        if (lht->is<F32>())
            *lhs = ir::q::SCast::create(ir::q::F64::create(), *lhs);
        else
            *rhs = ir::q::SCast::create(ir::q::F64::create(), *rhs);

        return;
    }

    if (lht->is_signed() || rht->is_signed())
    {
        if (lht->size() < rht->size())
            *lhs = ir::q::SCast::create(rht, *lhs);
        else
            *rhs = ir::q::SCast::create(lht, *rhs);
    }
    else
    {
        if (lht->size() < rht->size())
            *lhs = ir::q::UCast::create(rht, *lhs);
        else
            *rhs = ir::q::UCast::create(lht, *rhs);
    }

    return;
}

static auto conv(const BinaryExprNode *n, QState &state) -> QResult
{
    auto lhs = conv(n->m_lhs.get(), state)[0]->as<Expr>();
    auto rhs = conv(n->m_rhs.get(), state)[0]->as<Expr>();

    switch (n->m_op)
    {
    case Operator::Plus:
        return bipromote(&lhs, &rhs), Add::create(lhs, rhs);
    case Operator::Minus:
        return bipromote(&lhs, &rhs), Sub::create(lhs, rhs);
    case Operator::Multiply:
        return bipromote(&lhs, &rhs), Mul::create(lhs, rhs);
    case Operator::Divide:
        return bipromote(&lhs, &rhs), Div::create(lhs, rhs);
    case Operator::Modulo:
        return bipromote(&lhs, &rhs), Mod::create(lhs, rhs);
    case Operator::BitwiseAnd:
        return bipromote(&lhs, &rhs), BitAnd::create(lhs, rhs);
    case Operator::BitwiseOr:
        return bipromote(&lhs, &rhs), BitOr::create(lhs, rhs);
    case Operator::BitwiseXor:
        return bipromote(&lhs, &rhs), BitXor::create(lhs, rhs);
    case Operator::LeftShift:
        return bipromote(&lhs, &rhs), Shl::create(lhs, rhs);
    case Operator::RightShift:
        return bipromote(&lhs, &rhs), Shr::create(lhs, rhs);
    case Operator::LogicalAnd:
        return bipromote(&lhs, &rhs), And::create(lhs, rhs);
    case Operator::LogicalOr:
        return bipromote(&lhs, &rhs), Or::create(lhs, rhs);
    case Operator::LogicalXor:
        return bipromote(&lhs, &rhs), Xor::create(lhs, rhs);
    case Operator::LessThan:
        return bipromote(&lhs, &rhs), Lt::create(lhs, rhs);
    case Operator::GreaterThan:
        return bipromote(&lhs, &rhs), Gt::create(lhs, rhs);
    case Operator::LessThanEqual:
        return bipromote(&lhs, &rhs), Le::create(lhs, rhs);
    case Operator::GreaterThanEqual:
        return bipromote(&lhs, &rhs), Ge::create(lhs, rhs);
    case Operator::Equal:
        return bipromote(&lhs, &rhs), Eq::create(lhs, rhs);
    case Operator::NotEqual:
        return bipromote(&lhs, &rhs), Ne::create(lhs, rhs);
    case Operator::Assign:
        return Assign::create(lhs, promote(lhs, rhs));
    case Operator::PlusAssign:
        return Assign::create(lhs, Add::create(lhs, promote(lhs, rhs)));
    case Operator::MinusAssign:
        return Assign::create(lhs, Sub::create(lhs, promote(lhs, rhs)));
    case Operator::MultiplyAssign:
        return Assign::create(lhs, Mul::create(lhs, promote(lhs, rhs)));
    case Operator::DivideAssign:
        return Assign::create(lhs, Div::create(lhs, promote(lhs, rhs)));
    case Operator::ModuloAssign:
        return Assign::create(lhs, Mod::create(lhs, promote(lhs, rhs)));
    case Operator::BitwiseOrAssign:
        return Assign::create(lhs, BitOr::create(lhs, promote(lhs, rhs)));
    case Operator::BitwiseAndAssign:
        return Assign::create(lhs, BitAnd::create(lhs, promote(lhs, rhs)));
    case Operator::BitwiseXorAssign:
        return Assign::create(lhs, BitXor::create(lhs, promote(lhs, rhs)));
    case Operator::XorAssign:
        return Assign::create(lhs, Xor::create(lhs, promote(lhs, rhs)));
    case Operator::OrAssign:
        return Assign::create(lhs, Or::create(lhs, promote(lhs, rhs)));
    case Operator::AndAssign:
        return Assign::create(lhs, And::create(lhs, promote(lhs, rhs)));
    case Operator::LeftShiftAssign:
        return Assign::create(lhs, Shl::create(lhs, promote(lhs, rhs)));
    case Operator::RightShiftAssign:
        return Assign::create(lhs, Shr::create(lhs, promote(lhs, rhs)));

    default:
        throw std::runtime_error("BinaryExprNode not implemented");
    }
}

static auto conv(const CallExprNode *n, QState &state) -> QResult
{
    const Global *callee = nullptr;

    if (state.exported.contains(n->m_decl->m_name))
        callee = state.exported[n->m_decl->m_name].second[0]->as<Global>();
    else
        callee = conv(n->m_decl.get(), state)[0]->as<Global>();

    std::vector<const Expr *> args;
    const Segment *seg = callee->value->as<Segment>();

    size_t i = 0;
    for (auto &arg : n->m_positional_args)
    {
        auto v = conv(arg.get(), state)[0]->as<Expr>();

        if (seg->params.size() <= i || !seg->params.at(i).second)
            args.push_back(v);
        else
            args.push_back(promote(seg->params[i].second, v));

        i++;
    }

    while (i < callee->value->as<Segment>()->params.size())
    {
        auto v = conv(n->m_decl->m_params.at(i)->m_value.get(), state)[0]->as<Expr>();
        if (seg->params[i].second)
            args.push_back(promote(seg->params[i].second, v));
        else
            args.push_back(v);
        i++;
    }

    return Call::create(callee, args);
}

static auto conv(const ListExprNode *n, QState &state) -> QResult
{
    std::vector<const Expr *> values;
    for (auto &elem : n->m_elements)
    {
        auto v = conv(elem.get(), state)[0]->as<Expr>();
        values.push_back(v);
    }

    return List::create(values);
}

static auto conv(const MemberAccessNode *n, QState &state) -> QResult
{
    auto e = conv(n->m_expr.get(), state)[0]->as<Expr>();
    auto t = e->infer()->as<Type>();

    while (t->is_ptr())
    {
        t = t->as<Ptr>()->type;
    }

    switch ((ir::q::NodeType)t->ntype)
    {
    case ir::q::NodeType::Region:
    {
        auto x = t->as<Region>();

        if (!state.typedefs.contains(x->name))
            throw std::runtime_error("QIR translation: MemberAccessNode not found");

        if (state.typedefs[x->name]->is<RegionDefNode>())
        {
            auto def = state.typedefs[x->name]->as<RegionDefNode>();
            for (size_t i = 0; i < def->m_fields.size(); i++)
            {
                if (def->m_fields[i]->m_name == n->m_field)
                {
                    auto t = conv(def->m_fields[i]->m_type, state)[0]->as<Type>();
                    return Member::create(e, i, t);
                }
            }
        }
        else
        {
            auto def = state.typedefs[x->name]->as<StructDefNode>();
            for (size_t i = 0; i < def->m_fields.size(); i++)
            {
                if (def->m_fields[i]->m_name == n->m_field)
                {
                    auto t = conv(def->m_fields[i]->m_type, state)[0]->as<Type>();
                    return Member::create(e, i, t);
                }
            }
        }

        throw std::runtime_error("QIR translation: MemberAccessNode not found");
    }
    case ir::q::NodeType::Group:
    {
        auto x = t->as<Group>();

        if (!state.typedefs.contains(x->name))
            throw std::runtime_error("QIR translation: MemberAccessNode not found");

        auto def = state.typedefs[x->name]->as<GroupDefNode>();
        for (size_t i = 0; i < def->m_fields.size(); i++)
        {
            if (def->m_fields[i]->m_name == n->m_field)
            {
                auto t = conv(def->m_fields[i]->m_type, state)[0]->as<Type>();
                return Member::create(e, i, t);
            }
        }

        throw std::runtime_error("QIR translation: MemberAccessNode not found");
    }
    case ir::q::NodeType::Union:
    {
        auto x = t->as<Union>();

        if (!state.typedefs.contains(x->name))
            throw std::runtime_error("QIR translation: MemberAccessNode not found");

        auto def = state.typedefs[x->name]->as<UnionDefNode>();
        for (size_t i = 0; i < def->m_fields.size(); i++)
        {
            if (def->m_fields[i]->m_name == n->m_field)
            {
                auto t = conv(def->m_fields[i]->m_type, state)[0]->as<Type>();
                return Member::create(e, i, t);
            }
        }

        throw std::runtime_error("QIR translation: MemberAccessNode not found");
    }
    default:
        throw std::runtime_error("MemberAccessNode not implemented");
    }
}

static auto conv(const IndexNode *n, QState &state) -> QResult
{
    auto e = conv(n->m_expr.get(), state)[0]->as<Expr>();
    auto i = conv(n->m_index.get(), state)[0]->as<Expr>();
    auto t = e->infer()->as<Type>();

    if (t->is_ptr())
        return Index::create(e, i, t->as<Ptr>()->type);

    return Index::create(e, i, t->as<Array>()->type);
}

static auto conv(const ConstUnaryExprNode *n, QState &state) -> QResult
{
    auto e = conv(n->m_expr.get(), state)[0]->as<Expr>();

    switch (n->m_op)
    {
    case Operator::Plus:
        return e;
    case Operator::Minus:
        return Sub::create(Number::create("0"), e);
    case Operator::LogicalNot:
        return Not::create(e);
    case Operator::BitwiseNot:
        return BitNot::create(e);
    default:
        throw std::runtime_error("ConstUnaryExprNode not implemented");
    }
}

static auto conv(const ConstBinaryExprNode *n, QState &state) -> QResult
{
    auto lhs = conv(n->m_lhs.get(), state)[0]->as<Expr>();
    auto rhs = conv(n->m_rhs.get(), state)[0]->as<Expr>();

    switch (n->m_op)
    {
    case Operator::Plus:
        return bipromote(&lhs, &rhs), Add::create(lhs, rhs);
    case Operator::Minus:
        return bipromote(&lhs, &rhs), Sub::create(lhs, rhs);
    case Operator::Multiply:
        return bipromote(&lhs, &rhs), Mul::create(lhs, rhs);
    case Operator::Divide:
        return bipromote(&lhs, &rhs), Div::create(lhs, rhs);
    case Operator::Modulo:
        return bipromote(&lhs, &rhs), Mod::create(lhs, rhs);
    case Operator::BitwiseAnd:
        return bipromote(&lhs, &rhs), BitAnd::create(lhs, rhs);
    case Operator::BitwiseOr:
        return bipromote(&lhs, &rhs), BitOr::create(lhs, rhs);
    case Operator::BitwiseXor:
        return bipromote(&lhs, &rhs), BitXor::create(lhs, rhs);
    case Operator::LeftShift:
        return bipromote(&lhs, &rhs), Shl::create(lhs, rhs);
    case Operator::RightShift:
        return bipromote(&lhs, &rhs), Shr::create(lhs, rhs);
    case Operator::LogicalAnd:
        return bipromote(&lhs, &rhs), And::create(lhs, rhs);
    case Operator::LogicalOr:
        return bipromote(&lhs, &rhs), Or::create(lhs, rhs);
    case Operator::LogicalXor:
        return Xor::create(lhs, rhs);
    case Operator::LessThan:
        return bipromote(&lhs, &rhs), Lt::create(lhs, rhs);
    case Operator::GreaterThan:
        return bipromote(&lhs, &rhs), Gt::create(lhs, rhs);
    case Operator::LessThanEqual:
        return bipromote(&lhs, &rhs), Le::create(lhs, rhs);
    case Operator::GreaterThanEqual:
        return bipromote(&lhs, &rhs), Ge::create(lhs, rhs);
    case Operator::Equal:
        return bipromote(&lhs, &rhs), Eq::create(lhs, rhs);
    case Operator::NotEqual:
        return bipromote(&lhs, &rhs), Ne::create(lhs, rhs);

    default:
        throw std::runtime_error("ConstBinaryExprNode not implemented");
    }
}

static auto conv(const IdentifierNode *n, QState &state) -> QResult
{
    if (state.inside_segment)
    {
        if (state.local_idents.top().contains(n->m_name))
            return Ident::create(n->m_name, state.local_idents.top()[n->m_name]);
    }

    if (!state.global_idents.contains(n->m_name))
        throw std::runtime_error("QIR translation: IdentifierNode not found: " + n->m_name);

    return Ident::create(n->m_name, state.global_idents[n->m_name]);
}

static auto conv(const ImmMutTypeNode *n, QState &state) -> QResult
{
    return conv(n->m_type, state)[0]->as<Type>();
}

static auto conv(const U8TypeNode *n, QState &state) -> QResult
{
    return U8::create();
}

static auto conv(const U16TypeNode *n, QState &state) -> QResult
{
    return U16::create();
}

static auto conv(const U32TypeNode *n, QState &state) -> QResult
{
    return U32::create();
}

static auto conv(const U64TypeNode *n, QState &state) -> QResult
{
    return U64::create();
}

static auto conv(const U128TypeNode *n, QState &state) -> QResult
{
    return U128::create();
}

static auto conv(const I8TypeNode *n, QState &state) -> QResult
{
    return I8::create();
}

static auto conv(const I16TypeNode *n, QState &state) -> QResult
{
    return I16::create();
}

static auto conv(const I32TypeNode *n, QState &state) -> QResult
{
    return I32::create();
}

static auto conv(const I64TypeNode *n, QState &state) -> QResult
{
    return I64::create();
}

static auto conv(const I128TypeNode *n, QState &state) -> QResult
{
    return I128::create();
}

static auto conv(const F32TypeNode *n, QState &state) -> QResult
{
    return F32::create();
}

static auto conv(const F64TypeNode *n, QState &state) -> QResult
{
    return F64::create();
}

static auto conv(const BoolTypeNode *n, QState &state) -> QResult
{
    return I1::create();
}

static auto conv(const VoidTypeNode *n, QState &state) -> QResult
{
    return Void::create();
}

static auto conv(const PointerTypeNode *n, QState &state) -> QResult
{
    return Ptr::create(conv(n->m_type, state)[0]->as<Type>());
}

static auto conv(const OpaqueTypeNode *n, QState &state) -> QResult
{
    return Opaque::create(n->m_name);
}

static auto conv(const StringTypeNode *n, QState &state) -> QResult
{
    return Ptr::create(U8::create());
}

static auto conv(const EnumTypeNode *n, QState &state) -> QResult
{
    return conv(n->m_member_type, state)[0]->as<Type>();
}

static auto conv(const StructTypeNode *n, QState &state) -> QResult
{
    std::vector<const Type *> fields;
    for (auto &field : n->m_fields)
        fields.push_back(conv(field, state)[0]->as<Type>());

    return Region::create(n->m_name, fields);
}

static auto conv(const libquixcc::GroupTypeNode *n, QState &state) -> QResult
{
    std::vector<const Type *> fields;
    for (auto &field : n->m_fields)
        fields.push_back(conv(field, state)[0]->as<Type>());

    return Group::create(n->m_name, fields);
}

static auto conv(const RegionTypeNode *n, QState &state) -> QResult
{
    std::vector<const Type *> fields;
    for (auto &field : n->m_fields)
        fields.push_back(conv(field, state)[0]->as<Type>());

    return Region::create(n->m_name, fields);
}

static auto conv(const UnionTypeNode *n, QState &state) -> QResult
{
    std::vector<const Type *> fields;
    for (auto &field : n->m_fields)
        fields.push_back(conv(field, state)[0]->as<Type>());

    return Union::create(n->m_name, fields);
}

static auto conv(const ArrayTypeNode *n, QState &state) -> QResult
{
    auto size = std::atoll(conv(n->m_size.get(), state)[0]->as<Number>()->value.c_str());

    return Array::create(conv(n->m_type, state)[0]->as<Type>(), size);
}

static auto conv(const VectorTypeNode *n, QState &state) -> QResult
{
    return Vector::create(conv(n->m_type, state)[0]->as<Type>());
}

static auto conv(const FunctionTypeNode *n, QState &state) -> QResult
{
    std::vector<const Type *> params;
    for (auto &param : n->m_params)
        params.push_back(conv(param.second, state)[0]->as<Type>());

    return FType::create(params, conv(n->m_return_type, state)[0]->as<Type>(), n->m_variadic, n->m_pure, n->m_thread_safe, n->m_foreign, n->m_nothrow);
}

static auto conv(const IntegerNode *n, QState &state) -> QResult
{
    return Number::create(n->m_val);
}

static auto conv(const FloatLiteralNode *n, QState &state) -> QResult
{
    return Number::create(n->m_val);
}

static auto conv(const StringNode *n, QState &state) -> QResult
{
    return String::create(n->m_val);
}

static auto conv(const CharNode *n, QState &state) -> QResult
{
    return Char::create(n->m_val);
}

static auto conv(const BoolLiteralNode *n, QState &state) -> QResult
{
    return UCast::create(I1::create(), Number::create(n->m_val ? "1" : "0"));
}

static auto conv(const NullLiteralNode *n, QState &state) -> QResult
{
    return Number::create("0");
}

static auto conv(const TypedefNode *n, QState &state) -> QResult
{
    return nullptr;
}

static auto conv(const VarDeclNode *n, QState &state) -> QResult
{
    /// TODO: Implement VarDeclNode
    throw std::runtime_error("QIR translation: VarDeclNode not implemented");
}

static auto create_defaults(const libquixcc::ir::q::Value *var, libquixcc::TypeNode *type, QState &state) -> QResult
{
    switch (type->ntype)
    {
    case libquixcc::NodeType::StructTypeNode:
    {
        auto s = type->as<libquixcc::StructTypeNode>();
        if (!state.typedefs.contains(s->m_name))
            throw std::runtime_error("QIR translation: structdef not found");

        std::vector<QValue> res;
        for (size_t i = 0; i < state.typedefs[s->m_name]->as<StructDefNode>()->m_fields.size(); i++)
        {
            auto field = state.typedefs[s->m_name]->as<StructDefNode>()->m_fields[i];
            if (!field->m_value)
                continue;

            auto v = conv(field->m_value.get(), state)[0]->as<Expr>();
            auto t = conv(field->m_type, state)[0]->as<Type>();

            res.push_back(Assign::create(Member::create(var, i, t), promote(t, v)));
        }

        return res;
    }
    case libquixcc::NodeType::RegionTypeNode:
    {
        auto r = type->as<libquixcc::RegionTypeNode>();
        if (!state.typedefs.contains(r->m_name))
            throw std::runtime_error("QIR translation: regiondef not found");

        std::vector<QValue> res;
        for (size_t i = 0; i < state.typedefs[r->m_name]->as<RegionDefNode>()->m_fields.size(); i++)
        {
            auto field = state.typedefs[r->m_name]->as<RegionDefNode>()->m_fields[i];
            if (!field->m_value)
                continue;

            auto v = conv(field->m_value.get(), state)[0]->as<Expr>();
            auto t = conv(field->m_type, state)[0]->as<Type>();

            res.push_back(Assign::create(Member::create(var, i, t), promote(t, v)));
        }

        return res;
    }
    case libquixcc::NodeType::UnionTypeNode:
    {
        /// TODO: think about this one

        auto u = type->as<libquixcc::UnionTypeNode>();
        if (!state.typedefs.contains(u->m_name))
            throw std::runtime_error("QIR translation: uniondef not found");

        std::vector<QValue> res;
        for (size_t i = 0; i < state.typedefs[u->m_name]->as<UnionDefNode>()->m_fields.size(); i++)
        {
            auto field = state.typedefs[u->m_name]->as<UnionDefNode>()->m_fields[i];
            if (!field->m_value)
                continue;

            auto v = conv(field->m_value.get(), state)[0]->as<Expr>();
            auto t = conv(field->m_type, state)[0]->as<Type>();

            res.push_back(Assign::create(Member::create(var, i, t), promote(t, v)));
        }

        return res;
    }
    default:
        throw std::runtime_error("QIR translation: create_defaults not implemented");
    }
}

static bool is_composite(const TypeNode *n)
{
    switch (n->ntype)
    {
    case libquixcc::NodeType::StructTypeNode:
    case libquixcc::NodeType::GroupTypeNode:
    case libquixcc::NodeType::RegionTypeNode:
    case libquixcc::NodeType::UnionTypeNode:
        return true;
    default:
        return false;
    }
}

static auto conv(const LetDeclNode *n, QState &state) -> QResult
{
    if (state.inside_segment)
    {
        const Expr *init = nullptr;

        if (n->m_init)
            init = conv(n->m_init.get(), state)[0]->as<Expr>();

        const Type *type = nullptr;
        if (n->m_type)
            type = conv(n->m_type, state)[0]->as<Type>();
        else if (init)
            type = init->infer();
        else
            throw std::runtime_error("QIR translation: LetDeclNode is malformed");

        auto l = Local::create(n->m_name, type, init);
        state.local_idents.top()[n->m_name] = l->type;
        std::vector<QValue> res = {l};
        auto ident = Ident::create(n->m_name, l->type);

        if (l->type == nullptr && n->m_type && is_composite(n->m_type))
        {
            auto defaults = create_defaults(ident, n->m_type, state);
            for (auto &d : defaults)
                res.push_back(d);
        }

        return res;
    }

    const Expr *expr = nullptr;
    if (n->m_init)
        expr = conv(n->m_init.get(), state)[0]->as<Expr>();

    auto tmp = Global::create(n->m_name, conv(n->m_type, state)[0]->as<Type>(), expr, false, false, false);

    std::string mangled = Symbol::mangle(tmp, "", state.lang == ExportLangType::None ? ExportLangType::Default : state.lang);

    auto g = Global::create(mangled, conv(n->m_type, state)[0]->as<Type>(), expr, false, false, state.lang != ExportLangType::None);

    state.global_idents[n->m_name] = g->type;
    state.global_idents[mangled] = g->type;
    state.exported[n->m_name] = {state.lang, g};
    return g;
}

static auto conv(const ConstDeclNode *n, QState &state) -> QResult
{
    if (state.inside_segment)
    {
        auto init = conv(n->m_init.get(), state)[0]->as<Expr>();

        const Type *type = nullptr;
        if (n->m_type)
            type = conv(n->m_type, state)[0]->as<Type>();
        else
            type = init->infer();

        auto l = Local::create(n->m_name, type, init);
        state.local_idents.top()[n->m_name] = l->type;

        return l;
    }

    const Expr *expr = nullptr;
    if (n->m_init)
        expr = conv(n->m_init.get(), state)[0]->as<Expr>();

    const Type *type = nullptr;
    if (n->m_type)
        type = conv(n->m_type, state)[0]->as<Type>();
    else
        type = expr->infer();

    auto tmp = Global::create(n->m_name, type, expr, false, false, state.lang != ExportLangType::None);
    std::string mangled = Symbol::mangle(tmp, "", state.lang == ExportLangType::None ? ExportLangType::Default : state.lang);
    auto g = Global::create(mangled, type, expr, false, false, state.lang != ExportLangType::None);

    state.global_idents[n->m_name] = g->type;
    state.global_idents[mangled] = g->type;
    state.exported[n->m_name] = {state.lang, g};
    return g;
}

static auto conv(const FunctionDeclNode *n, QState &state) -> QResult
{
    std::vector<std::pair<std::string, const Type *>> params;
    for (auto &p : n->m_params)
    {
        auto res = conv(p.get(), state);
        auto t = res[0]->as<Type>();

        if (t->is<Array>())
            params.push_back({p->m_name, Ptr::create(t->as<Array>()->type)});
        else
            params.push_back({p->m_name, t});
    }

    std::set<FConstraint> constraints;
    if (n->m_type->m_pure)
        constraints.insert(FConstraint::Pure);
    if (n->m_type->m_thread_safe)
        constraints.insert(FConstraint::ThreadSafe);
    if (n->m_type->m_foreign)
        constraints.insert(FConstraint::C_ABI);
    if (n->m_type->m_nothrow)
        constraints.insert(FConstraint::NoThrow);
    if (n->m_type->m_variadic)
        constraints.insert(FConstraint::Variadic);

    auto seg = Segment::create(params, conv(n->m_type->m_return_type, state)[0]->as<Type>(), nullptr, constraints);

    const Global *g = nullptr;
    std::string mangled;

    g = Global::create(n->m_name, seg->infer(),
                       seg, false, false, true);

    if (n->m_name == "main")
    {
        g = Global::create(mangled = Symbol::mangle(g, "", ExportLangType::C),
                           seg->infer(),
                           seg, false, false, true);
    }
    else
    {
        if (state.lang == ExportLangType::None)
        {
            g = Global::create(mangled = Symbol::mangle(g, "", ExportLangType::Default),
                               seg->infer(), seg, false, false, false);
        }
        else
        {
            g = Global::create(mangled = Symbol::mangle(g, "", state.lang),
                               seg->infer(), seg, false, false, true);
        }
    }

    state.global_idents[n->m_name] = g->type;
    state.global_idents[mangled] = g->type;
    state.exported[n->m_name] = {state.lang, g};
    return g;
}

static auto conv(const StructDefNode *n, QState &state) -> QResult
{
    std::vector<std::pair<std::string, QValue>> fields;
    std::map<std::string, const Segment *> methods;

    state.typedefs[n->m_name] = n;
    for (auto &field : n->m_fields)
    {
        auto res = conv(field.get(), state);
        fields.push_back({field->m_name, res[0]});
    }

    for (auto &method : n->m_methods)
    {
        auto res = conv(method.get(), state)[0]->as<Global>();
        methods[res->name] = res->value->as<Segment>();
    }

    std::vector<QValue> result;
    result.push_back(RegionDef::create(n->m_name, fields, methods));

    for (auto static_method : n->m_static_methods)
    {
        std::shared_ptr<StmtNode> clone;
        if (static_method->is<FunctionDeclNode>())
        {
            clone = std::make_shared<FunctionDeclNode>(*static_method->as<FunctionDeclNode>());
            auto x = std::static_pointer_cast<FunctionDeclNode>(clone);
            x->m_name = n->m_name + "::" + x->m_name;
        }
        else if (static_method->is<FunctionDefNode>())
        {
            clone = std::make_shared<FunctionDefNode>(*static_method->as<FunctionDefNode>());
            auto x = std::static_pointer_cast<FunctionDefNode>(clone);
            x->m_decl->m_name = n->m_name + "::" + x->m_decl->m_name;
        }

        auto res = conv(clone.get(), state);
        result.insert(result.end(), res.begin(), res.end());
    }

    return result;
}

static auto conv(const StructFieldNode *n, QState &state) -> QResult
{
    return conv(n->m_type, state)[0]->as<Type>();
}

static auto conv(const RegionDefNode *n, QState &state) -> QResult
{
    std::vector<std::pair<std::string, QValue>> fields;
    std::map<std::string, const Segment *> methods;

    state.typedefs[n->m_name] = n;
    for (auto &field : n->m_fields)
    {
        auto res = conv(field.get(), state);
        fields.push_back({field->m_name, res[0]});
    }

    for (auto &method : n->m_methods)
    {
        auto res = conv(method.get(), state)[0]->as<Global>();
        methods[res->name] = res->value->as<Segment>();
    }

    std::vector<QValue> result;
    result.push_back(RegionDef::create(n->m_name, fields, methods));

    for (auto &static_method : n->m_static_methods)
    {
        std::shared_ptr<StmtNode> clone;
        if (static_method->is<FunctionDeclNode>())
        {
            clone = std::make_shared<FunctionDeclNode>(*static_method->as<FunctionDeclNode>());
            auto x = std::static_pointer_cast<FunctionDeclNode>(clone);
            x->m_name = n->m_name + "::" + x->m_name;
        }
        else if (static_method->is<FunctionDefNode>())
        {
            clone = std::make_shared<FunctionDefNode>(*static_method->as<FunctionDefNode>());
            auto x = std::static_pointer_cast<FunctionDefNode>(clone);
            x->m_decl->m_name = n->m_name + "::" + x->m_decl->m_name;
        }

        auto res = conv(clone.get(), state);
        result.insert(result.end(), res.begin(), res.end());
    }

    return result;
}

static auto conv(const RegionFieldNode *n, QState &state) -> QResult
{
    return conv(n->m_type, state)[0]->as<Type>();
}

static auto conv(const GroupDefNode *n, QState &state) -> QResult
{
    std::map<std::string, QValue> fields;
    std::map<std::string, const Segment *> methods;

    state.typedefs[n->m_name] = n;
    for (auto &field : n->m_fields)
    {
        auto res = conv(field.get(), state);
        fields[field->m_name] = res[0];
    }

    for (auto &method : n->m_methods)
    {
        auto res = conv(method.get(), state)[0]->as<Global>();
        methods[res->name] = res->value->as<Segment>();
    }

    std::vector<QValue> result;
    result.push_back(GroupDef::create(n->m_name, fields, methods));

    for (auto &static_method : n->m_static_methods)
    {
        std::shared_ptr<StmtNode> clone;
        if (static_method->is<FunctionDeclNode>())
        {
            clone = std::make_shared<FunctionDeclNode>(*static_method->as<FunctionDeclNode>());
            auto x = std::static_pointer_cast<FunctionDeclNode>(clone);
            x->m_name = n->m_name + "::" + x->m_name;
        }
        else if (static_method->is<FunctionDefNode>())
        {
            clone = std::make_shared<FunctionDefNode>(*static_method->as<FunctionDefNode>());
            auto x = std::static_pointer_cast<FunctionDefNode>(clone);
            x->m_decl->m_name = n->m_name + "::" + x->m_decl->m_name;
        }

        auto res = conv(clone.get(), state);
        result.insert(result.end(), res.begin(), res.end());
    }

    return result;
}

static auto conv(const GroupFieldNode *n, QState &state) -> QResult
{
    return conv(n->m_type, state)[0]->as<Type>();
}

static auto conv(const UnionDefNode *n, QState &state) -> QResult
{
    /// TODO: Implement UnionDefNode
    std::map<std::string, QValue> fields;
    for (auto &field : n->m_fields)
    {
        auto res = conv(field.get(), state);
        fields.insert({field->m_name, res[0]});
    }

    state.typedefs[n->m_name] = n;
    return UnionDef::create(n->m_name, fields, {});
}

static auto conv(const UnionFieldNode *n, QState &state) -> QResult
{
    return conv(n->m_type, state)[0]->as<Type>();
}

static auto conv(const EnumDefNode *n, QState &state) -> QResult
{
    return nullptr;
}

static auto conv(const EnumFieldNode *n, QState &state) -> QResult
{
    return nullptr;
}

static auto conv(const FunctionDefNode *n, QState &state) -> QResult
{
    state.local_idents.push({});
    state.function.push(n);

    bool old = state.inside_segment;
    state.inside_segment = true;

    auto glob = conv(n->m_decl.get(), state)[0]->as<Global>();
    auto dseg = glob->value->as<Segment>();

    auto body = conv(n->m_body.get(), state)[0]->as<Block>();
    state.inside_segment = old;

    state.function.pop();
    state.local_idents.pop();

    auto f = Segment::create(dseg->params, dseg->return_type, body, dseg->constraints);

    return Global::create(glob->name, glob->type, f, glob->_volatile, glob->_atomic, glob->_extern);
}

static auto conv(const FunctionParamNode *n, QState &state) -> QResult
{
    auto t = conv(n->m_type, state)[0]->as<Type>();
    if (state.inside_segment)
        state.local_idents.top()[n->m_name] = t;
    return t;
}

static auto conv(const ExportNode *n, QState &state) -> QResult
{
    std::vector<QValue> sub;
    ExportLangType old = state.lang;
    state.lang = n->m_lang_type;

    for (auto &stmt : n->m_stmts)
    {
        auto res = conv(stmt.get(), state);
        if (!res)
            continue;
        sub.insert(sub.end(), res.begin(), res.end());
    }

    state.lang = old;
    return sub;
}

static auto conv(const InlineAsmNode *n, QState &state) -> QResult
{
    /// TODO: Implement InlineAsmNode
    throw std::runtime_error("QIR translation: InlineAsmNode not implemented");
}

static auto conv(const ReturnStmtNode *n, QState &state) -> QResult
{
    if (!n->m_expr)
        return Ret::create(nullptr);

    auto e = conv(n->m_expr.get(), state)[0]->as<Expr>();
    auto t = conv(state.function.top()->m_decl->m_type->m_return_type, state)[0]->as<Type>();
    return Ret::create(promote(t, e));
}

static auto conv(const RetifStmtNode *n, QState &state) -> QResult
{
    auto cond = conv(n->m_cond.get(), state)[0]->as<Expr>();
    auto ret = conv(n->m_return.get(), state)[0]->as<Expr>();
    return IfElse::create(cond, Ret::create(ret), nullptr);
}

static auto conv(const RetzStmtNode *n, QState &state) -> QResult
{
    auto cond = conv(n->m_cond.get(), state)[0]->as<Expr>();
    auto ret = conv(n->m_return.get(), state)[0]->as<Expr>();
    return IfElse::create(Not::create(cond), Ret::create(ret), nullptr);
}

static auto conv(const RetvStmtNode *n, QState &state) -> QResult
{
    auto cond = conv(n->m_cond.get(), state)[0]->as<Expr>();
    return IfElse::create(cond, Ret::create(nullptr), nullptr);
}

static auto conv(const IfStmtNode *n, QState &state) -> QResult
{
    auto cond = conv(n->m_cond.get(), state)[0]->as<Expr>();
    auto then_block = conv(n->m_then.get(), state)[0]->as<Block>();
    QResult else_block;
    if (n->m_else)
        else_block = conv(n->m_else.get(), state);
    else
        else_block = Block::create({});

    return IfElse::create(cond, then_block, else_block[0]->as<Block>());
}

static auto conv(const WhileStmtNode *n, QState &state) -> QResult
{
    auto cond = conv(n->m_cond.get(), state)[0]->as<Expr>();
    auto stmt = conv(n->m_stmt.get(), state)[0]->as<Block>();
    return While::create(cond, stmt);
}

static auto conv(const ForStmtNode *n, QState &state) -> QResult
{
    auto init = conv(n->m_init.get(), state)[0]->as<Expr>();
    auto cond = conv(n->m_cond.get(), state)[0]->as<Expr>();
    auto step = conv(n->m_step.get(), state)[0]->as<Expr>();
    auto stmt = conv(n->m_stmt.get(), state)[0]->as<Block>();
    return For::create(init, cond, step, stmt);
}

static auto conv(const ParseNode *n, QState &state) -> QResult
{
    QResult r;

    switch (n->ntype)
    {
    case libquixcc::NodeType::ExprStmtNode:
        r = conv(n->as<ExprStmtNode>(), state);
        break;

    case libquixcc::NodeType::NopStmtNode:
        r = conv(n->as<NopStmtNode>(), state);
        break;

    case libquixcc::NodeType::BlockNode:
        r = conv(n->as<BlockNode>(), state);
        break;

    case libquixcc::NodeType::StmtGroupNode:
        r = conv(n->as<StmtGroupNode>(), state);
        break;

    case libquixcc::NodeType::ConstUnaryExprNode:
        r = conv(n->as<ConstUnaryExprNode>(), state);
        break;

    case libquixcc::NodeType::ConstBinaryExprNode:
        r = conv(n->as<ConstBinaryExprNode>(), state);
        break;

    case libquixcc::NodeType::StaticCastExprNode:
        r = conv(n->as<StaticCastExprNode>(), state);
        break;

    case libquixcc::NodeType::BitCastExprNode:
        r = conv(n->as<BitCastExprNode>(), state);
        break;

    case libquixcc::NodeType::SignedUpcastExprNode:
        r = conv(n->as<SignedUpcastExprNode>(), state);
        break;

    case libquixcc::NodeType::UnsignedUpcastExprNode:
        r = conv(n->as<UnsignedUpcastExprNode>(), state);
        break;

    case libquixcc::NodeType::DowncastExprNode:
        r = conv(n->as<DowncastExprNode>(), state);
        break;

    case libquixcc::NodeType::PtrToIntCastExprNode:
        r = conv(n->as<PtrToIntCastExprNode>(), state);
        break;

    case libquixcc::NodeType::IntToPtrCastExprNode:
        r = conv(n->as<IntToPtrCastExprNode>(), state);
        break;

    case libquixcc::NodeType::UnaryExprNode:
        r = conv(n->as<UnaryExprNode>(), state);
        break;

    case libquixcc::NodeType::BinaryExprNode:
        r = conv(n->as<BinaryExprNode>(), state);
        break;

    case libquixcc::NodeType::CallExprNode:
        r = conv(n->as<CallExprNode>(), state);
        break;

    case libquixcc::NodeType::ListExprNode:
        r = conv(n->as<ListExprNode>(), state);
        break;

    case libquixcc::NodeType::MemberAccessNode:
        r = conv(n->as<MemberAccessNode>(), state);
        break;

    case libquixcc::NodeType::IndexNode:
        r = conv(n->as<IndexNode>(), state);
        break;

    case libquixcc::NodeType::IdentifierNode:
        r = conv(n->as<IdentifierNode>(), state);
        break;

    case libquixcc::NodeType::ImmMutTypeNode:
        r = conv(n->as<ImmMutTypeNode>(), state);
        break;

    case libquixcc::NodeType::U8TypeNode:
        r = conv(n->as<U8TypeNode>(), state);
        break;

    case libquixcc::NodeType::U16TypeNode:
        r = conv(n->as<U16TypeNode>(), state);
        break;

    case libquixcc::NodeType::U32TypeNode:
        r = conv(n->as<U32TypeNode>(), state);
        break;

    case libquixcc::NodeType::U64TypeNode:
        r = conv(n->as<U64TypeNode>(), state);
        break;

    case libquixcc::NodeType::U128TypeNode:
        r = conv(n->as<U128TypeNode>(), state);
        break;

    case libquixcc::NodeType::I8TypeNode:
        r = conv(n->as<I8TypeNode>(), state);
        break;

    case libquixcc::NodeType::I16TypeNode:
        r = conv(n->as<I16TypeNode>(), state);
        break;

    case libquixcc::NodeType::I32TypeNode:
        r = conv(n->as<I32TypeNode>(), state);
        break;

    case libquixcc::NodeType::I64TypeNode:
        r = conv(n->as<I64TypeNode>(), state);
        break;

    case libquixcc::NodeType::I128TypeNode:
        r = conv(n->as<I128TypeNode>(), state);
        break;

    case libquixcc::NodeType::F32TypeNode:
        r = conv(n->as<F32TypeNode>(), state);
        break;

    case libquixcc::NodeType::F64TypeNode:
        r = conv(n->as<F64TypeNode>(), state);
        break;

    case libquixcc::NodeType::BoolTypeNode:
        r = conv(n->as<BoolTypeNode>(), state);
        break;

    case libquixcc::NodeType::VoidTypeNode:
        r = conv(n->as<VoidTypeNode>(), state);
        break;

    case libquixcc::NodeType::PointerTypeNode:
        r = conv(n->as<PointerTypeNode>(), state);
        break;

    case libquixcc::NodeType::OpaqueTypeNode:
        r = conv(n->as<OpaqueTypeNode>(), state);
        break;

    case libquixcc::NodeType::StringTypeNode:
        r = conv(n->as<StringTypeNode>(), state);
        break;

    case libquixcc::NodeType::EnumTypeNode:
        r = conv(n->as<EnumTypeNode>(), state);
        break;

    case libquixcc::NodeType::StructTypeNode:
        r = conv(n->as<StructTypeNode>(), state);
        break;

    case libquixcc::NodeType::GroupTypeNode:
        r = conv(n->as<GroupTypeNode>(), state);
        break;

    case libquixcc::NodeType::RegionTypeNode:
        r = conv(n->as<RegionTypeNode>(), state);
        break;

    case libquixcc::NodeType::UnionTypeNode:
        r = conv(n->as<UnionTypeNode>(), state);
        break;

    case libquixcc::NodeType::ArrayTypeNode:
        r = conv(n->as<ArrayTypeNode>(), state);
        break;

    case libquixcc::NodeType::VectorTypeNode:
        r = conv(n->as<VectorTypeNode>(), state);
        break;

    case libquixcc::NodeType::FunctionTypeNode:
        r = conv(n->as<FunctionTypeNode>(), state);
        break;

    case libquixcc::NodeType::IntegerNode:
        r = conv(n->as<IntegerNode>(), state);
        break;

    case libquixcc::NodeType::FloatLiteralNode:
        r = conv(n->as<FloatLiteralNode>(), state);
        break;

    case libquixcc::NodeType::StringNode:
        r = conv(n->as<StringNode>(), state);
        break;

    case libquixcc::NodeType::CharNode:
        r = conv(n->as<CharNode>(), state);
        break;

    case libquixcc::NodeType::BoolLiteralNode:
        r = conv(n->as<BoolLiteralNode>(), state);
        break;

    case libquixcc::NodeType::NullLiteralNode:
        r = conv(n->as<NullLiteralNode>(), state);
        break;

    case libquixcc::NodeType::TypedefNode:
        r = conv(n->as<TypedefNode>(), state);
        break;

    case libquixcc::NodeType::VarDeclNode:
        r = conv(n->as<VarDeclNode>(), state);
        break;

    case libquixcc::NodeType::LetDeclNode:
        r = conv(n->as<LetDeclNode>(), state);
        break;

    case libquixcc::NodeType::ConstDeclNode:
        r = conv(n->as<ConstDeclNode>(), state);
        break;

    case libquixcc::NodeType::FunctionDeclNode:
        r = conv(n->as<FunctionDeclNode>(), state);
        break;

    case libquixcc::NodeType::StructDefNode:
        r = conv(n->as<StructDefNode>(), state);
        break;

    case libquixcc::NodeType::StructFieldNode:
        r = conv(n->as<StructFieldNode>(), state);
        break;

    case libquixcc::NodeType::RegionDefNode:
        r = conv(n->as<RegionDefNode>(), state);
        break;

    case libquixcc::NodeType::RegionFieldNode:
        r = conv(n->as<RegionFieldNode>(), state);
        break;

    case libquixcc::NodeType::GroupDefNode:
        r = conv(n->as<GroupDefNode>(), state);
        break;

    case libquixcc::NodeType::GroupFieldNode:
        r = conv(n->as<GroupFieldNode>(), state);
        break;

    case libquixcc::NodeType::UnionDefNode:
        r = conv(n->as<UnionDefNode>(), state);
        break;

    case libquixcc::NodeType::UnionFieldNode:
        r = conv(n->as<UnionFieldNode>(), state);
        break;

    case libquixcc::NodeType::EnumDefNode:
        r = conv(n->as<EnumDefNode>(), state);
        break;

    case libquixcc::NodeType::EnumFieldNode:
        r = conv(n->as<EnumFieldNode>(), state);
        break;

    case libquixcc::NodeType::FunctionDefNode:
        r = conv(n->as<FunctionDefNode>(), state);
        break;

    case libquixcc::NodeType::FunctionParamNode:
        r = conv(n->as<FunctionParamNode>(), state);
        break;

    case libquixcc::NodeType::ExportNode:
        r = conv(n->as<ExportNode>(), state);
        break;

    case libquixcc::NodeType::InlineAsmNode:
        r = conv(n->as<InlineAsmNode>(), state);
        break;

    case libquixcc::NodeType::ReturnStmtNode:
        r = conv(n->as<ReturnStmtNode>(), state);
        break;

    case libquixcc::NodeType::RetifStmtNode:
        r = conv(n->as<RetifStmtNode>(), state);
        break;

    case libquixcc::NodeType::RetzStmtNode:
        r = conv(n->as<RetzStmtNode>(), state);
        break;

    case libquixcc::NodeType::RetvStmtNode:
        r = conv(n->as<RetvStmtNode>(), state);
        break;

    case libquixcc::NodeType::IfStmtNode:
        r = conv(n->as<IfStmtNode>(), state);
        break;

    case libquixcc::NodeType::WhileStmtNode:
        r = conv(n->as<WhileStmtNode>(), state);
        break;

    case libquixcc::NodeType::ForStmtNode:
        r = conv(n->as<ForStmtNode>(), state);
        break;
    default:
        throw std::runtime_error("QIR translation: Unknown node type");
    }

    return r;
}

bool ir::q::QModule::from_ptree(std::shared_ptr<BlockNode> ast)
{
    LOG(DEBUG) << "Converting Ptree to QUIX intermediate representation" << std::endl;

    QState state;

    auto r = conv(ast.get(), state);

    m_root = RootNode::create(r[0]->as<Block>()->stmts);

    if (!verify())
    {
        LOG(FATAL) << "Failed to qualify QUIX intermediate representation" << std::endl;
        return false;
    }

    LOG(DEBUG) << log::raw << this->to_string() << std::endl;

    LOG(DEBUG) << "Successfully converted Ptree to QUIX intermediate representation" << std::endl;

    return true;
}