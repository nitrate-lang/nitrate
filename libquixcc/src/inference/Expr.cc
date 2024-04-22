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

#include <error/Logger.h>
#include <parse/nodes/ExprNode.h>
#include <parse/nodes/FunctionNode.h>
#include <parse/nodes/StructNode.h>

libquixcc::TypeNode *libquixcc::UnaryExprNode::infer(libquixcc::TypeInferenceState &state) const
{
    switch (m_op)
    {
    case Operator::Minus:
    case Operator::Plus:
    case Operator::BitwiseNot:
    case Operator::Increment:
    case Operator::Decrement:
        return m_expr->infer(state);
    case Operator::LogicalNot:
        return BoolTypeNode::create();
    case Operator::BitwiseAnd:
        return PointerTypeNode::create(m_expr->infer(state));
    case Operator::Multiply:
    {
        auto type = m_expr->infer(state);
        if (type->ntype != NodeType::PointerTypeNode)
        {
            LOG(ERROR) << "Can not dereference a non-pointer type" << std::endl;
            return nullptr;
        }

        return static_cast<PointerTypeNode *>(type)->m_type;
    }
    default:
        LOG(ERROR) << "Invalid unary operator" << std::endl;
        return nullptr;
    }

    return nullptr;
}

/*

I'm just going to copy C++20 for now.

Here's a comprehensive table of C++20 type promotions (modified for Quix):

| Type A      | Type B      | Result Type |
|-------------|-------------|-------------|
| bool        | any         | i32         |
| i8          | any integer | i32         |
| i8          | any integer | i32         |
| u8          | any integer | i32         |
| i16         | any integer | i32         |
| u16         | any integer | i32         |
| i32         | any integer | i32         |
| u32         | any integer | u32         |
| i64         | any integer | i64         |
| u64         | any integer | u64         |
| f32         | any integer | f32         |
| f32         | f64         | f64         |
| f64         | any integer | f64         |
| %ptr to any | %ptr to void| %ptr to void|
| %ptr to any | ptr to void | ptr to void |
| %ptr to any | %ptr to any | %ptr to void|
| ptr to any  | %ptr to any | ptr to void |
| *           | *           | undefined   |

*/

libquixcc::TypeNode *libquixcc::BinaryExprNode::infer(libquixcc::TypeInferenceState &state) const
{
    auto lhs = m_lhs->infer(state);
    auto rhs = m_rhs->infer(state);


    /* Logical operations return bool */
    // switch (m_op)
    // {
    //     // case 
    // }



    /// TODO: Implement this function
    return nullptr;
}

libquixcc::TypeNode *libquixcc::CallExprNode::infer(libquixcc::TypeInferenceState &state) const
{
    return m_decl->m_type->m_return_type;
}

libquixcc::TypeNode *libquixcc::ListExprNode::infer(libquixcc::TypeInferenceState &state) const
{
    std::vector<TypeNode *> types;
    for (auto &expr : m_elements)
    {
        types.push_back(expr->infer(state));
    }

    return StructTypeNode::create(types);
}

libquixcc::TypeNode *libquixcc::MemberAccessNode::infer(libquixcc::TypeInferenceState &state) const
{
    /// TODO: Implement this function
    return nullptr;
}