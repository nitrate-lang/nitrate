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

#ifndef __QUIXCC_MESSAGES_HPP__
#define __QUIXCC_MESSAGES_HPP__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <map>

namespace libquixcc
{
    enum Msg
    {
        PARSER_EXPECTED_KEYWORD,
        PARSER_EXPECTED_LEFT_BRACE,
        PARSER_EXPECTED_RIGHT_BRACE,
        PARSER_EXPECTED_SEMICOLON,
        PARSER_UNKNOWN_LANGUAGE,

        TYPE_EXPECTED_IDENTIFIER_OR_BRACKET,
        TYPE_EXPECTED_FUNCTION,

        LET_DECL_MISSING_IDENTIFIER,
        LET_DECL_MISSING_COLON,
        LET_DECL_TYPE_ERR,
        LET_DECL_INIT_ERR,
        LET_DECL_MISSING_PUNCTOR,

        VAR_DECL_MISSING_IDENTIFIER,
        VAR_DECL_MISSING_COLON,
        VAR_DECL_TYPE_ERR,
        VAR_DECL_INIT_ERR,
        VAR_DECL_MISSING_PUNCTOR,

        CONST_DECL_MISSING_IDENTIFIER,
        CONST_DECL_MISSING_COLON,
        CONST_DECL_TYPE_ERR,
        CONST_DECL_INIT_ERR,
        CONST_DECL_MISSING_PUNCTOR,

        STRUCT_DECL_MISSING_IDENTIFIER,
        STRUCT_FIELD_MISSING_IDENTIFIER,
        STRUCT_DEF_EXPECTED_OPEN_BRACE,
        STRUCT_DEF_EXPECTED_SEMICOLON,
        STRUCT_FIELD_MISSING_COLON,
        STRUCT_FIELD_TYPE_ERR,
        STRUCT_FIELD_INIT_ERR,
        STRUCT_FIELD_MISSING_PUNCTOR,

        UNION_DECL_MISSING_IDENTIFIER,
        UNION_DEF_EXPECTED_OPEN_BRACE,
        UNION_FIELD_MISSING_IDENTIFIER,
        UNION_DEF_EXPECTED_SEMICOLON,
        UNION_FIELD_MISSING_COLON,
        UNION_FIELD_TYPE_ERR,
        UNION_FIELD_MISSING_PUNCTOR,

        TYPE_EXPECTED_TYPE,
        TYPE_EXPECTED_SEMICOLON,
        TYPE_EXPECTED_CONST_EXPR,
        TYPE_EXPECTED_CLOSE_BRACKET,

        SUBSYSTEM_MISSING_IDENTIFIER,
        SUBSYSTEM_EXPECTED_IDENTIFIER,
        SUBSYSTEM_EXPECTED_SEMICOLON,

        ENUM_EXPECTED_IDENTIFIER,
        ENUM_EXPECTED_COLON,
        ENUM_EXPECTED_LEFT_BRACE,
        ENUM_DEF_EXPECTED_SEMICOLON,
        ENUM_FIELD_EXPECTED_IDENTIFIER,
        ENUM_FIELD_EXPECTED_CONST_EXPR,
        ENUM_FIELD_EXPECTED_SEMICOLON,

        FN_EXPECTED_IDENTIFIER,
        FN_EXPECTED_OPEN_PAREN,
        FN_EXPECTED_CLOSE_PAREN_OR_COMMA,
        FN_EXPECTED_OPEN_BRACE,
        FN_NO_THROW_ALREADY_SPECIFIED,
        FN_FOREIGN_ALREADY_SPECIFIED,
        FN_THREAD_SAFE_ALREADY_SPECIFIED,
        FN_PURE_ALREADY_SPECIFIED,
        FN_IMPURE_ALREADY_SPECIFIED,
        FN_PURE_AND_IMPURE_SPECIFIED,

        PREP_DUPLICATE_IMPORT,

        VAR_NAME_DUPLICATE,
        LET_NAME_DUPLICATE,
        CONST_NAME_DUPLICATE,
        STRUCT_NAME_DUPLICATE,
        UNION_NAME_DUPLICATE,
        ENUM_NAME_DUPLICATE,
        FUNC_NAME_DUPLICATE,
        STRUCT_FIELD_DUPLICATE,
        UNION_FIELD_DUPLICATE,
        ENUM_FIELD_DUPLICATE,
        PARAM_NAME_DUPLICATE,
        SUBSYSTEM_NAME_DUPLICATE,

        RETIF_MISSING_SEMICOLON,
        RETIF_MISSING_COMMA,
        RETZ_MISSING_SEMICOLON,
        RETZ_MISSING_COMMA,
        RETV_MISSING_SEMICOLON,
        RETV_MISSING_COMMA,

        UNRESOLVED_TYPE,
        RESOLVED_TYPE,

        GENERIC_ERROR,

        MAX_MSG
    };

    extern std::map<Msg, const char *> feedback;
};

#endif