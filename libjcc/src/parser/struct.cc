#define JCC_INTERNAL

#include <parse/parser.h>
#include <macro.h>
#include <error/message.h>

using namespace libjcc;

static bool parse_struct_field(jcc_job_t &job, std::shared_ptr<libjcc::Scanner> scanner, std::shared_ptr<StructFieldNode> &node)
{
    Token tok = scanner->next();
    if (tok.type() != TokenType::Identifier)
    {
        PARMSG(tok, libjcc::Err::ERROR, feedback[STRUCT_FIELD_MISSING_IDENTIFIER]);
        return false;
    }

    auto name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::Colon)
    {
        PARMSG(tok, libjcc::Err::ERROR, feedback[STRUCT_FIELD_MISSING_COLON]);
        return false;
    }

    std::shared_ptr<TypeNode> type;
    if (!parse_type(job, scanner, type))
    {
        PARMSG(tok, libjcc::Err::ERROR, feedback[STRUCT_FIELD_TYPE_ERR], name.c_str());
        return false;
    }

    std::shared_ptr<ConstExprNode> value;

    tok = scanner->next();
    if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::Semicolon)
    {
        node = std::make_shared<StructFieldNode>(name, type);
        return true;
    }
    else if (tok.type() == TokenType::Operator && std::get<Operator>(tok.val()) == Operator::Assign)
    {
        if (!parse_const_expr(job, scanner, Token(TokenType::Punctor, Punctor::Semicolon), value))
        {
            PARMSG(tok, libjcc::Err::ERROR, feedback[STRUCT_FIELD_INIT_ERR], name.c_str());
            return false;
        }
    }
    else
    {
        PARMSG(tok, libjcc::Err::ERROR, feedback[STRUCT_FIELD_MISSING_PUNCTOR], name.c_str());
        return false;
    }

    tok = scanner->next();
    if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::Semicolon)
    {
        PARMSG(tok, libjcc::Err::ERROR, feedback[STRUCT_FIELD_MISSING_PUNCTOR], name.c_str());
        return false;
    }

    node = std::make_shared<StructFieldNode>(name, type, value);

    return true;
}

bool libjcc::parse_struct(jcc_job_t &job, std::shared_ptr<libjcc::Scanner> scanner, std::shared_ptr<libjcc::StmtNode> &node)
{
    Token tok = scanner->next();
    if (tok.type() != TokenType::Identifier)
    {
        PARMSG(tok, libjcc::Err::ERROR, feedback[STRUCT_DECL_MISSING_IDENTIFIER]);
        return false;
    }

    std::string name = std::get<std::string>(tok.val());

    tok = scanner->next();
    if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::Semicolon)
    {
        node = std::make_shared<StructDeclNode>();
        std::static_pointer_cast<StructDeclNode>(node)->m_name = name;
        return true;
    }
    else if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::OpenBrace)
    {
        PARMSG(tok, libjcc::Err::ERROR, feedback[STRUCT_DEF_EXPECTED_OPEN_BRACE]);
        return false;
    }

    std::vector<std::shared_ptr<StructFieldNode>> fields;

    while (true)
    {
        tok = scanner->peek();
        if (tok.type() == TokenType::Punctor && std::get<Punctor>(tok.val()) == Punctor::CloseBrace)
        {
            scanner->next();
            break;
        }

        std::shared_ptr<StructFieldNode> field;
        if (!parse_struct_field(job, scanner, field))
            return false;
        fields.push_back(field);
    }

    tok = scanner->next();
    if (tok.type() != TokenType::Punctor || std::get<Punctor>(tok.val()) != Punctor::Semicolon)
    {
        PARMSG(tok, libjcc::Err::ERROR, feedback[STRUCT_DEF_EXPECTED_SEMICOLON]);
        return false;
    }

    auto sdef = std::make_shared<StructDefNode>();
    sdef->m_name = name;
    sdef->m_fields = fields;
    node = sdef;
    return true;
}