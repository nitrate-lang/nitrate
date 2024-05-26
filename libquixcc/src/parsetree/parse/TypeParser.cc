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

#include <parsetree/Parser.h>
#include <LibMacro.h>
#include <core/Logger.h>

using namespace libquixcc;

static std::map<std::string, TypeNode *> primitive_types = {
    {"u8", U8TypeNode::create()},
    {"u16", U16TypeNode::create()},
    {"u32", U32TypeNode::create()},
    {"u64", U64TypeNode::create()},
    {"u128", U128TypeNode::create()},
    {"i8", I8TypeNode::create()},
    {"i16", I16TypeNode::create()},
    {"i32", I32TypeNode::create()},
    {"i64", I64TypeNode::create()},
    {"i128", I128TypeNode::create()},
    {"f32", F32TypeNode::create()},
    {"f64", F64TypeNode::create()},
    {"i1", BoolTypeNode::create()},
    {"string", StringTypeNode::create()},
    {"void", VoidTypeNode::create()}};

bool libquixcc::parse_type(quixcc_job_t &job, libquixcc::Scanner *scanner, TypeNode **node)
{
    Token tok = scanner->next();
    TypeNode *type = nullptr, *inner = nullptr;

    if (tok.type == TT::Keyword)
    {
        std::shared_ptr<StmtNode> fn;

        switch (tok.as<Keyword>())
        {
        case Keyword::Void:
            inner = VoidTypeNode::create();
            goto suffix;
        case Keyword::Fn:
            if (!parse_function(job, scanner, fn))
            {
                LOG(ERROR) << feedback[TYPE_EXPECTED_FUNCTION] << tok << std::endl;
                goto error;
            }

            if (!fn->is<FunctionDeclNode>())
            {
                LOG(ERROR) << feedback[TYPE_EXPECTED_FUNCTION] << tok << std::endl;
                goto error;
            }

            inner = std::static_pointer_cast<FunctionDeclNode>(fn)->m_type;
            scanner->push(Token(TT::Punctor, Punctor::Semicolon));
            goto suffix;
        case Keyword::Opaque:
        {
            tok = scanner->next();
            if (!tok.is<Punctor>(Punctor::OpenParen))
            {
                LOG(ERROR) << feedback[TYPE_OPAQUE_EXPECTED_PAREN] << tok << std::endl;
                goto error;
            }
            tok = scanner->next();
            if (tok.type != TT::Identifier)
            {
                LOG(ERROR) << feedback[TYPE_OPAQUE_EXPECTED_IDENTIFIER] << tok << std::endl;
                goto error;
            }
            std::string name = tok.as<std::string>();
            tok = scanner->next();
            if (!tok.is<Punctor>(Punctor::CloseParen))
            {
                LOG(ERROR) << feedback[TYPE_OPAQUE_EXPECTED_CLOSE_PAREN] << tok << std::endl;
                goto error;
            }

            inner = OpaqueTypeNode::create(name);
            goto suffix;
        }
        default:
            goto error;
        }
    }
    else if (tok.type == TT::Identifier)
    {
        if (primitive_types.contains(tok.as<std::string>()))
        {
            inner = primitive_types[tok.as<std::string>()];
            goto suffix;
        }
        else
        {
            inner = UserTypeNode::create(tok.as<std::string>());
            goto suffix;
        }
    }
    else if (tok.is<Punctor>(Punctor::OpenBracket))
    {
        // Array type or Vector type
        // syntax [type; size]
        if (!parse_type(job, scanner, &type))
        {
            LOG(ERROR) << feedback[TYPE_EXPECTED_TYPE] << tok << std::endl;
            goto error;
        }

        tok = scanner->next();
        if (tok.is<Punctor>(Punctor::CloseBracket))
        {
            inner = VectorTypeNode::create(type);
            goto suffix;
        }

        if (!tok.is<Punctor>(Punctor::Semicolon))
        {
            LOG(ERROR) << feedback[TYPE_EXPECTED_SEMICOLON] << tok << std::endl;
            goto error;
        }

        std::shared_ptr<ConstExprNode> size;
        if (!parse_const_expr(job, scanner, Token(TT::Punctor, Punctor::CloseBracket), size))
        {
            LOG(ERROR) << feedback[TYPE_EXPECTED_CONST_EXPR] << tok << std::endl;
            goto error;
        }

        tok = scanner->next();
        if (!tok.is<Punctor>(Punctor::CloseBracket))
        {
            LOG(ERROR) << feedback[TYPE_EXPECTED_CLOSE_BRACKET] << tok << std::endl;
            goto error;
        }

        inner = ArrayTypeNode::create(type, size);
        goto suffix;
    }
    else if (tok.is<Operator>(Operator::Multiply))
    {
        // Pointer type
        if (!parse_type(job, scanner, &type))
        {
            LOG(ERROR) << feedback[TYPE_EXPECTED_TYPE] << tok << std::endl;
            goto error;
        }

        inner = PointerTypeNode::create(type);
        goto suffix;
    }
    else if (tok.is<Operator>(Operator::LogicalNot))
    {
        // ! means mutability
        if (!parse_type(job, scanner, &type))
        {
            LOG(ERROR) << feedback[TYPE_EXPECTED_TYPE] << tok << std::endl;
            goto error;
        }

        inner = MutTypeNode::create(type);
        goto suffix;
    }
    else
    {
        LOG(ERROR) << feedback[EXPECTED_TYPE] << tok << std::endl;
        goto error;
    }

suffix:
    while (true)
    {
        tok = scanner->peek();
        if (tok.is<Operator>(Operator::Question))
        {
            // ? means Result type
            scanner->next();
            inner = ResultTypeNode::create(inner);
        }
        else if (tok.is<Operator>(Operator::BitwiseXor))
        {
            // ^ means Generator type
            scanner->next();
            inner = GeneratorTypeNode::create(inner);
        }
        else
        {
            break;
        }
    }

    *node = inner;

    return true;

error:
    return false;
}
