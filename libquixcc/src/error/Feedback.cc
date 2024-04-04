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

#include <error/Messages.h>

using namespace libquixcc;

std::map<libquixcc::Msg, const char *> libquixcc::feedback = {
    {PARSER_EXPECTED_KEYWORD, "Parser failed because a keyword was expected, but the token {} was found"},
    {PARSER_EXPECTED_LEFT_BRACE, "Parser failed because an open brace was expected, but the token {} was found"},
    {PARSER_EXPECTED_RIGHT_BRACE, "Parser failed because a close brace was expected, but the token {} was found"},
    {PARSER_EXPECTED_SEMICOLON, "Parser failed because a semicolon was expected, but the token {} was found"},
    {PARSER_UNKNOWN_LANGUAGE, "Parser failed because the language type {} is unknown"},

    {LET_DECL_MISSING_IDENTIFIER, "Expected identifier in variable declaration. To fix the issue, name your variable. Syntax: let name: type [= expr];"},
    {LET_DECL_MISSING_COLON, "Expected colon separator in variable declaration. To fix the issue, insert a colon between the variable name and the typename. Syntax: let name: type [= expr];"},
    {LET_DECL_TYPE_ERR, "An error occurred while parsing the type of variable '{}'. Syntax: let name: type [= expr];"},
    {LET_DECL_INIT_ERR, "Declaration of variable '{}' requires an initializer, but an error occurred while parsing the initializer. Syntax: let name: type [= expr];"},
    {LET_DECL_MISSING_PUNCTOR, "Declaration of variable '{}' requires an initializer OR a semicolon punctuator, but neither was found. Make sure to terminate all statements with a semicolon. Syntax: let name: type [= expr];"},

    {VAR_DECL_MISSING_IDENTIFIER, "Expected identifier in variable declaration. To fix the issue, name your variable. Syntax: var name: type [= expr];"},
    {VAR_DECL_MISSING_COLON, "Expected colon separator in variable declaration. To fix the issue, insert a colon between the variable name and the typename. Syntax: var name: type [= expr];"},
    {VAR_DECL_TYPE_ERR, "An error occurred while parsing the type of variable '{}'. Syntax: var name: type [= expr];"},
    {VAR_DECL_INIT_ERR, "Declaration of variable '{}' requires an initializer, but an error occurred while parsing the initializer. Syntax: var name: type [= expr];"},
    {VAR_DECL_MISSING_PUNCTOR, "Declaration of variable '{}' requires an initializer OR a semicolon punctuator, but neither was found. Make sure to terminate all statements with a semicolon. Syntax: var name: type [= expr];"},

    {CONST_DECL_MISSING_IDENTIFIER, "Expected identifier in variable declaration. To fix the issue, name your variable. Syntax: const name: type [= expr];"},
    {CONST_DECL_MISSING_COLON, "Expected colon separator in variable declaration. To fix the issue, insert a colon between the variable name and the typename. Syntax: const name: type [= expr];"},
    {CONST_DECL_TYPE_ERR, "An error occurred while parsing the type of variable '{}'. Syntax: const name: type [= expr];"},
    {CONST_DECL_INIT_ERR, "Declaration of variable '{}' requires an initializer, but an error occurred while parsing the initializer. Syntax: const name: type [= expr];"},
    {CONST_DECL_MISSING_PUNCTOR, "Declaration of variable '{}' requires an initializer OR a semicolon punctuator, but neither was found. Make sure to terminate all statements with a semicolon. Syntax: const name: type [= expr];"},

    {STRUCT_DECL_MISSING_IDENTIFIER, "Expected identifier in struct declaration. To fix the issue, name your struct. Syntax: struct name;"},
    {STRUCT_FIELD_MISSING_IDENTIFIER, "Expected identifier in struct field declaration. To fix the issue, name your field. Syntax: name: type [= expr];"},
    {STRUCT_DEF_EXPECTED_OPEN_BRACE, "Expected an open brace after the struct name. To fix the issue, insert an open brace after the struct name. Syntax: struct name { ... };"},
    {STRUCT_DEF_EXPECTED_SEMICOLON, "Expected a semicolon after the struct definition. To fix the issue, insert a semicolon after the struct definition. Syntax: struct name { ... };"},
    {STRUCT_FIELD_MISSING_COLON, "Expected colon separator in struct field declaration. To fix the issue, insert a colon between the field name and the typename. Syntax: name: type [= expr];"},
    {STRUCT_FIELD_TYPE_ERR, "An error occurred while parsing the type of field '{}'. Syntax: name: type [= expr];"},
    {STRUCT_FIELD_INIT_ERR, "Declaration of field '{}' requires an initializer, but an error occurred while parsing the initializer. Syntax: name: type [= expr];"},
    {STRUCT_FIELD_MISSING_PUNCTOR, "Declaration of field '{}' requires an initializer OR a semicolon punctuator, but neither was found. Make sure to terminate all statements with a semicolon. Syntax: name: type [= expr];"},

    {UNION_DECL_MISSING_IDENTIFIER, "Expected identifier in union declaration. To fix the issue, name your union. Syntax: union name;"},
    {UNION_DEF_EXPECTED_OPEN_BRACE, "Expected an open brace after the union name. To fix the issue, insert an open brace after the union name. Syntax: union name { ... };"},
    {UNION_FIELD_MISSING_IDENTIFIER, "Expected identifier in union field declaration. To fix the issue, name your field. Syntax: name: type [= expr];"},
    {UNION_DEF_EXPECTED_SEMICOLON, "Expected a semicolon after the union definition. To fix the issue, insert a semicolon after the union definition. Syntax: union name { ... };"},
    {UNION_FIELD_MISSING_COLON, "Expected colon separator in union field declaration. To fix the issue, insert a colon between the field name and the typename. Syntax: name: type [= expr];"},
    {UNION_FIELD_TYPE_ERR, "An error occurred while parsing the type of field '{}'. Syntax: name: type [= expr];"},
    {UNION_FIELD_MISSING_PUNCTOR, "Declaration of field '{}' requires an initializer OR a semicolon punctuator, but neither was found. Make sure to terminate all statements with a semicolon. Syntax: name: type [= expr];"},

    {TYPE_EXPECTED_TYPE, "Expected a type name after the open bracket. To fix the issue, insert a type name after the open bracket. Syntax: [type; size]"},
    {TYPE_EXPECTED_SEMICOLON, "Expected a semicolon after the type name. To fix the issue, insert a semicolon after the type name. Syntax: [type; size]"},
    {TYPE_EXPECTED_CONST_EXPR, "Expected a constant expression after the semicolon. To fix the issue, insert a constant expression after the semicolon. Syntax: [type; size]"},
    {TYPE_EXPECTED_CLOSE_BRACKET, "Expected a close bracket after the constant expression. To fix the issue, insert a close bracket after the constant expression. Syntax: [type; size]"},
    {TYPE_EXPECTED_FUNCTION, "Expected a function name after the keyword 'fn'. To fix the issue, insert a function name after the keyword 'fn'"},

    {SUBSYSTEM_MISSING_IDENTIFIER, "Expected identifier after the keyword 'subsystem'. To fix the issue, insert an identifier after the keyword 'subsystem'"},
    {SUBSYSTEM_EXPECTED_IDENTIFIER, "Expected identifier after the colon in the subsystem declaration. To fix the issue, insert an identifier after the colon in the subsystem declaration"},
    {SUBSYSTEM_EXPECTED_SEMICOLON, "Expected semicolon after the subsystem block. To fix the issue, insert a semicolon after the subsystem block. Syntax: subsystem name[: dep1, dep2, ...] { ... };"},

    {ENUM_EXPECTED_IDENTIFIER, "Expected identifier after the keyword 'enum'. To fix the issue, insert an identifier after the keyword 'enum'"},
    {ENUM_EXPECTED_COLON, "Expected colon after the identifier in the enum declaration. To fix the issue, insert a colon after the identifier in the enum declaration"},
    {ENUM_EXPECTED_LEFT_BRACE, "Expected an open brace after the enum declaration. To fix the issue, insert an open brace after the enum declaration"},
    {ENUM_DEF_EXPECTED_SEMICOLON, "Expected a semicolon after the enum definition. To fix the issue, insert a semicolon after the enum definition"},
    {ENUM_FIELD_EXPECTED_IDENTIFIER, "Expected identifier in enum field declaration. To fix the issue, name your field. Syntax: name [= expr];"},
    {ENUM_FIELD_EXPECTED_CONST_EXPR, "Expected a constant expression after the equals sign. To fix the issue, insert a constant expression after the equals sign. Syntax: name [= expr];"},
    {ENUM_FIELD_EXPECTED_SEMICOLON, "Expected a semicolon after the enum field definition. To fix the issue, insert a semicolon after the enum field definition. Syntax: name [= expr];"},

    {FN_EXPECTED_IDENTIFIER, "Expected identifier after the keyword 'fn'. To fix the issue, insert an identifier after the keyword 'fn'"},
    {FN_EXPECTED_OPEN_PAREN, "Expected an open parenthesis after the function name. To fix the issue, insert an open parenthesis after the function name"},
    {FN_EXPECTED_CLOSE_PAREN_OR_COMMA, "Expected a close parenthesis or a comma after the function parameter. To fix the issue, insert a close parenthesis or a comma after the function parameter"},
    {FN_EXPECTED_OPEN_BRACE, "Expected an open brace after the function parameter list. To fix the issue, insert an open brace after the function parameter list"},
    {FN_NO_THROW_ALREADY_SPECIFIED, "The 'nothrow' specifier was already specified for this function. To fix the issue, remove the duplicate 'nothrow' specifier"},
    {FN_FOREIGN_ALREADY_SPECIFIED, "The 'foreign' specifier was already specified for this function. To fix the issue, remove the duplicate 'foreign' specifier"},
    {FN_THREAD_SAFE_ALREADY_SPECIFIED, "The 'tsafe' specifier was already specified for this function. To fix the issue, remove the duplicate 'thread_safe' specifier"},
    {FN_PURE_ALREADY_SPECIFIED, "The 'pure' specifier was already specified for this function. To fix the issue, remove the duplicate 'pure' specifier"},
    {FN_IMPURE_ALREADY_SPECIFIED, "The 'impure' specifier was already specified for this function. To fix the issue, remove the duplicate 'impure' specifier"},
    {FN_PURE_AND_IMPURE_SPECIFIED, "The 'pure' and 'impure' specifiers were both specified for this function. To fix the issue, remove one of the duplicate specifiers"},
    {FN_PARAM_PARSE_ERROR, "An error occurred while parsing the function parameter. To fix the issue, check the syntax of the function parameter"},
    {FN_PARAM_EXPECTED_IDENTIFIER, "Expected identifier in function parameter declaration. To fix the issue, name your parameter. Syntax: name: type [= expr];"},
    {FN_PARAM_EXPECTED_COLON, "Expected colon separator in function parameter declaration. To fix the issue, insert a colon between the parameter name and the typename. Syntax: name: type [= expr];"},
    {FN_PARAM_TYPE_ERR, "An error occurred while parsing the type of parameter '{}'. Syntax: name: type [= expr];"},
    {FN_PARAM_INIT_ERR, "Declaration of parameter '{}' requires an initializer, but an error occurred while parsing the initializer. Syntax: name: type [= expr];"},
    {FN_EXPECTED_VARARG, "Expected '...' after the last parameter in the function parameter list. To fix the issue, insert '...' after the last parameter in the function parameter list"},

    {PREP_DUPLICATE_IMPORT, "Source file \"{}\" contains duplicate import of \"{}\""},

    {VAR_NAME_DUPLICATE, "Variable name '{}' is already in use. To fix the issue, rename the variable to something unique"},
    {LET_NAME_DUPLICATE, "Variable name '{}' is already in use. To fix the issue, rename the variable to something unique"},
    {CONST_NAME_DUPLICATE, "Variable name '{}' is already in use. To fix the issue, rename the variable to something unique"},
    {STRUCT_NAME_DUPLICATE, "Struct name '{}' is already in use. To fix the issue, rename the struct to something unique"},
    {UNION_NAME_DUPLICATE, "Union name '{}' is already in use. To fix the issue, rename the union to something unique"},
    {ENUM_NAME_DUPLICATE, "Enum name '{}' is already in use. To fix the issue, rename the enum to something unique"},
    {FUNC_NAME_DUPLICATE, "Function name '{}' is already in use. To fix the issue, rename the function to something unique"},
    {STRUCT_FIELD_DUPLICATE, "Field name '{}' is already in use. To fix the issue, rename the field to something unique"},
    {UNION_FIELD_DUPLICATE, "Field name '{}' is already in use. To fix the issue, rename the field to something unique"},
    {ENUM_FIELD_DUPLICATE, "Field name '{}' is already in use. To fix the issue, rename the field to something unique"},
    {PARAM_NAME_DUPLICATE, "Parameter name '{}' is already in use. To fix the issue, rename the parameter to something unique"},
    {SUBSYSTEM_NAME_DUPLICATE, "Subsystem name '{}' is already in use. To fix the issue, rename the subsystem to something unique"},

    {RETIF_MISSING_COMMA, "Expected a comma after the condition in the return-if statement. To fix the issue, insert a comma after the condition in the return-if statement"},
    {RETIF_MISSING_SEMICOLON, "Expected a semicolon after the return-if statement. To fix the issue, insert a semicolon after the return-if statement"},
    {RETZ_MISSING_SEMICOLON, "Expected a semicolon after the return-zero statement. To fix the issue, insert a semicolon after the return-zero statement"},
    {RETZ_MISSING_COMMA, "Expected a comma after the condition in the return-zero statement. To fix the issue, insert a comma after the condition in the return-zero statement"},
    {RETV_MISSING_SEMICOLON, "Expected a semicolon after the return-void statement. To fix the issue, insert a semicolon after the return-void statement"},
    {RETV_MISSING_COMMA, "Expected a comma after the condition in the return-void statement. To fix the issue, insert a comma after the condition in the return-void statement"},

    {TYPEDEF_EXPECTED_IDENTIFIER, "Expected identifier in type definition. To fix the issue, name your type. Syntax: type name = type;"},
    {TYPEDEF_EXPECTED_ASSIGN, "Expected an assignment operator in type definition. To fix the issue, insert an assignment operator after the type name. Syntax: type name = type;"},
    {TYPEDEF_INVALID_TYPE, "An error occurred while parsing the type of type '{}'. Syntax: type name = type;"},
    {TYPEDEF_EXPECTED_SEMICOLON, "Expected a semicolon after the type definition. To fix the issue, insert a semicolon after the type definition. Syntax: type name = type;"},
    {TYPEDEF_NAME_DUPLICATE, "Type name '{}' is already in use. To fix the issue, rename the type to something unique"},
    
    {FOR_EXPECTED_SEMICOLON, "Expected a semicolon after the for loop condition. To fix the issue, insert a semicolon after the for loop condition"},
    {FOR_EXPECTED_CLOSING_PARANTHESIS, "Expected a closing parenthesis after the for loop condition. To fix the issue, insert a closing parenthesis after the for loop condition"},

    {UNRESOLVED_FUNCTION, "Function '{}' is not defined in the current scope. To fix the issue, define the function or import the module that defines the function"},
    {UNRESOLVED_TYPE, "Type '{}' is not defined in the current scope. To fix the issue, define the type or import the module that defines the type"},
    {RESOLVED_TYPE, "Resolved type '{}' into '{}'"},
};
