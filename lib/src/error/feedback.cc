#include <error/messages.h>

using namespace libj;

std::map<libj::Msg, const char *> libj::feedback = {
    {PARSER_EXPECTED_KEYWORD, "Parser failed because a keyword was expected, but the token %s was found"},
    {LET_DECL_MISSING_IDENTIFIER, "Expected identifier in variable declaration. To fix the issue, name your variable. Syntax: let name: type [= expr];"},
    {LET_DECL_MISSING_COLON, "Expected colon separator in variable declaration. To fix the issue, insert a colon between the variable name and the typename. Syntax: let name: type [= expr];"},
    {LET_DECL_TYPE_ERR, "An error occurred while parsing the type of variable '%s'. Syntax: let name: type [= expr];"},
    {LET_DECL_INIT_ERR, "Declaration of variable '%s' requires an initializer, but an error occurred while parsing the initializer. Syntax: let name: type [= expr];"},
    {LET_DECL_MISSING_PUNCTOR, "Declaration of variable '%s' requires an initializer OR a semicolon punctuator, but neither was found. Make sure to terminate all statements with a semicolon. Syntax: let name: type [= expr];"},
    {VAR_DECL_MISSING_IDENTIFIER, "Expected identifier in variable declaration. To fix the issue, name your variable. Syntax: var name: type [= expr];"},
    {VAR_DECL_MISSING_COLON, "Expected colon separator in variable declaration. To fix the issue, insert a colon between the variable name and the typename. Syntax: var name: type [= expr];"},
    {VAR_DECL_TYPE_ERR, "An error occurred while parsing the type of variable '%s'. Syntax: var name: type [= expr];"},
    {VAR_DECL_INIT_ERR, "Declaration of variable '%s' requires an initializer, but an error occurred while parsing the initializer. Syntax: var name: type [= expr];"},
    {VAR_DECL_MISSING_PUNCTOR, "Declaration of variable '%s' requires an initializer OR a semicolon punctuator, but neither was found. Make sure to terminate all statements with a semicolon. Syntax: var name: type [= expr];"},
    {CONST_DECL_MISSING_IDENTIFIER, "Expected identifier in variable declaration. To fix the issue, name your variable. Syntax: const name: type [= expr];"},
    {CONST_DECL_MISSING_COLON, "Expected colon separator in variable declaration. To fix the issue, insert a colon between the variable name and the typename. Syntax: const name: type [= expr];"},
    {CONST_DECL_TYPE_ERR, "An error occurred while parsing the type of variable '%s'. Syntax: const name: type [= expr];"},
    {CONST_DECL_INIT_ERR, "Declaration of variable '%s' requires an initializer, but an error occurred while parsing the initializer. Syntax: const name: type [= expr];"},
    {CONST_DECL_MISSING_PUNCTOR, "Declaration of variable '%s' requires an initializer OR a semicolon punctuator, but neither was found. Make sure to terminate all statements with a semicolon. Syntax: const name: type [= expr];"},
    {STRUCT_DECL_MISSING_IDENTIFIER, "Expected identifier in struct declaration. To fix the issue, name your struct. Syntax: struct name;"},
    {STRUCT_FIELD_MISSING_IDENTIFIER, "Expected identifier in struct field declaration. To fix the issue, name your field. Syntax: name: type [= expr];"},
    {STRUCT_DEF_EXPECTED_OPEN_BRACE, "Expected an open brace after the struct name. To fix the issue, insert an open brace after the struct name. Syntax: struct name { ... };"},
    {STRUCT_DEF_EXPECTED_SEMICOLON, "Expected a semicolon after the struct definition. To fix the issue, insert a semicolon after the struct definition. Syntax: struct name { ... };"},
    {STRUCT_FIELD_MISSING_COLON, "Expected colon separator in struct field declaration. To fix the issue, insert a colon between the field name and the typename. Syntax: name: type [= expr];"},
    {STRUCT_FIELD_TYPE_ERR, "An error occurred while parsing the type of field '%s'. Syntax: name: type [= expr];"},
    {STRUCT_FIELD_INIT_ERR, "Declaration of field '%s' requires an initializer, but an error occurred while parsing the initializer. Syntax: name: type [= expr];"},
    {STRUCT_FIELD_MISSING_PUNCTOR, "Declaration of field '%s' requires an initializer OR a semicolon punctuator, but neither was found. Make sure to terminate all statements with a semicolon. Syntax: name: type [= expr];"},
    {TYPE_EXPECTED_TYPE, "Expected a type name after the open bracket. To fix the issue, insert a type name after the open bracket. Syntax: [type; size]"},
    {TYPE_EXPECTED_SEMICOLON, "Expected a semicolon after the type name. To fix the issue, insert a semicolon after the type name. Syntax: [type; size]"},
    {TYPE_EXPECTED_CONST_EXPR, "Expected a constant expression after the semicolon. To fix the issue, insert a constant expression after the semicolon. Syntax: [type; size]"},
    {TYPE_EXPECTED_CLOSE_BRACKET, "Expected a close bracket after the constant expression. To fix the issue, insert a close bracket after the constant expression. Syntax: [type; size]"}};