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

#include <parse/Parser.h>
#include <LibMacro.h>
#include <error/Message.h>

using namespace libquixcc;

static std::map<std::string, TypeNode *> primitive_types = {
    {"u8", U8TypeNode::create()},
    {"u16", U16TypeNode::create()},
    {"u32", U32TypeNode::create()},
    {"u64", U64TypeNode::create()},
    {"i8", I8TypeNode::create()},
    {"i16", I16TypeNode::create()},
    {"i32", I32TypeNode::create()},
    {"i64", I64TypeNode::create()},
    {"f32", F32TypeNode::create()},
    {"f64", F64TypeNode::create()},
    {"bool", BoolTypeNode::create()},
    {"string", StringTypeNode::create()},
    {"void", VoidTypeNode::create()}};

bool libquixcc::parse_type(quixcc_job_t &job, std::shared_ptr<libquixcc::Scanner> scanner, TypeNode **node)
{
    Token tok = scanner->next();
    if (tok.type() == TokenType::Keyword)
    {
        std::shared_ptr<StmtNode> fn;

        switch (std::get<Keyword>(tok.val()))
        {
        case Keyword::Void:
            *node = VoidTypeNode::create();
            return true;
        case Keyword::Fn:
            if (!parse_function(job, scanner, fn))
            {
                PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_FUNCTION]);
                return false;
            }

            if (fn->ntype != NodeType::FunctionDeclNode)
            {
                PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_FUNCTION]);
                return false;
            }

            *node = std::static_pointer_cast<FunctionDeclNode>(fn)->m_type;
            scanner->push(Token(TokenType::Punctor, Punctor::Semicolon));
            return true;
        default:
            return false;
        }
    }
    else if (tok.type() == TokenType::Identifier)
    {
        if (primitive_types.contains(std::get<std::string>(tok.val())))
        {
            *node = primitive_types[std::get<std::string>(tok.val())];
            return true;
        }
        else
        {
            *node = UserTypeNode::create(std::get<std::string>(tok.val()));
            return true;
        }
    }
    else if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::OpenBracket)
    {
        // Array type
        // syntax [type; size]
        TypeNode *type;
        if (!parse_type(job, scanner, &type))
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_TYPE]);
            return false;
        }

        tok = scanner->next();
        if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::Semicolon)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_SEMICOLON]);
            return false;
        }

        std::shared_ptr<ConstExprNode> size;
        if (!parse_const_expr(job, scanner, Token(TokenType::Punctor, Punctor::CloseBracket), size))
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_CONST_EXPR]);
            return false;
        }

        tok = scanner->next();
        if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::CloseBracket)
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_CLOSE_BRACKET]);
            return false;
        }

        *node = ArrayTypeNode::create(type, size);
        return true;
    }
    else if (tok.type() == TokenType::Operator && std::get<Operator>(tok.val()) == Operator::Multiply)
    {
        // Pointer type
        TypeNode *type;
        if (!parse_type(job, scanner, &type))
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_TYPE]);
            return false;
        }

        *node = PointerTypeNode::create(type);
        return true;
    }
    else if (tok.type() == TokenType::Operator && std::get<Operator>(tok.val()) == Operator::Modulo)
    {
        // '%' means mutability
        TypeNode *type;
        if (!parse_type(job, scanner, &type))
        {
            PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_TYPE]);
            return false;
        }

        type->m_mut = true;
        *node = type;
        return true;
    }
    else
    {
        PARMSG(tok, libquixcc::Err::ERROR, feedback[TYPE_EXPECTED_IDENTIFIER_OR_BRACKET]);
        return false;
    }
}
