////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_MESSAGES_HPP__
#define __QUIXCC_MESSAGES_HPP__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <unordered_map>

namespace libquixcc {
enum Msg {
  PARSER_EXPECTED_KEYWORD,
  PARSER_ILLEGAL_KEYWORD,
  PARSER_EXPECTED_LEFT_BRACE,
  PARSER_EXPECTED_RIGHT_BRACE,
  PARSER_EXPECTED_SEMICOLON,
  PARSER_UNKNOWN_LANGUAGE,

  EXPECTED_TYPE,
  TYPE_EXPECTED_FUNCTION,
  TYPE_OPAQUE_EXPECTED_IDENTIFIER,
  TYPE_OPAQUE_EXPECTED_PAREN,
  TYPE_OPAQUE_EXPECTED_CLOSE_PAREN,

  LET_DECL_MISSING_IDENTIFIER,
  LET_DECL_MISSING_COLON,
  LET_DECL_TYPE_ERR,
  LET_DECL_INIT_ERR,
  LET_DECL_MISSING_PUNCTOR,

  CONST_DECL_TYPE_ERR,

  VAR_DECL_MISSING_IDENTIFIER,
  VAR_DECL_MISSING_COLON,
  VAR_DECL_TYPE_ERR,
  VAR_DECL_INIT_ERR,
  VAR_DECL_MISSING_PUNCTOR,

  CONST_DECL_MISSING_IDENTIFIER,
  CONST_DECL_MISSING_COLON,
  CONST_DECL_INIT_ERR,
  CONST_DECL_MISSING_PUNCTOR,

  STRUCT_DECL_MISSING_IDENTIFIER,
  STRUCT_FIELD_MISSING_IDENTIFIER,
  STRUCT_DEF_EXPECTED_OPEN_BRACE,
  STRUCT_FIELD_MISSING_COLON,
  STRUCT_FIELD_TYPE_ERR,
  STRUCT_FIELD_INIT_ERR,
  STRUCT_FIELD_MISSING_PUNCTOR,
  STRUCT_DEF_EXPECTED_FN,

  REGION_DECL_MISSING_IDENTIFIER,
  REGION_FIELD_MISSING_IDENTIFIER,
  REGION_DEF_EXPECTED_OPEN_BRACE,
  REGION_FIELD_MISSING_COLON,
  REGION_FIELD_TYPE_ERR,
  REGION_FIELD_INIT_ERR,
  REGION_FIELD_MISSING_PUNCTOR,
  REGION_DEF_EXPECTED_FN,

  GROUP_DECL_MISSING_IDENTIFIER,
  GROUP_FIELD_MISSING_IDENTIFIER,
  GROUP_DEF_EXPECTED_OPEN_BRACE,
  GROUP_FIELD_MISSING_COLON,
  GROUP_FIELD_TYPE_ERR,
  GROUP_FIELD_INIT_ERR,
  GROUP_FIELD_MISSING_PUNCTOR,
  GROUP_DEF_EXPECTED_FN,

  UNION_DECL_MISSING_IDENTIFIER,
  UNION_DEF_EXPECTED_OPEN_BRACE,
  UNION_FIELD_MISSING_IDENTIFIER,
  UNION_DEF_EXPECTED_SEMICOLON,
  UNION_FIELD_MISSING_COLON,
  UNION_FIELD_TYPE_ERR,
  UNION_FIELD_INIT_ERR,
  UNION_FIELD_MISSING_PUNCTOR,
  UNION_DEF_EXPECTED_FN,

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
  FN_PARAM_PARSE_ERROR,
  FN_PARAM_EXPECTED_IDENTIFIER,
  FN_PARAM_EXPECTED_COLON,
  FN_PARAM_TYPE_ERR,
  FN_PARAM_INIT_ERR,
  FN_EXPECTED_VARARG,

  FN_NOEXCEPT_MULTIPLE,
  FN_FOREIGN_MULTIPLE,
  FN_IMPURE_MULTIPLE,
  FN_TSAFE_MULTIPLE,
  FN_PURE_MULTIPLE,
  FN_QUASIPURE_MULTIPLE,
  FN_RETROPURE_MULTIPLE,
  FN_INLINE_MULTIPLE,
  FN_PURE_IMPURE_MIX,
  FN_PURE_MIX,

  PREP_DUPLICATE_IMPORT,

  VAR_NAME_DUPLICATE,
  LET_NAME_DUPLICATE,
  CONST_NAME_DUPLICATE,
  STRUCT_NAME_DUPLICATE,
  REGION_NAME_DUPLICATE,
  GROUP_NAME_DUPLICATE,
  UNION_NAME_DUPLICATE,
  ENUM_NAME_DUPLICATE,
  FUNC_NAME_DUPLICATE,
  STRUCT_FIELD_DUPLICATE,
  REGION_FIELD_DUPLICATE,
  GROUP_FIELD_DUPLICATE,
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

  TYPEDEF_EXPECTED_IDENTIFIER,
  TYPEDEF_EXPECTED_ASSIGN,
  TYPEDEF_INVALID_TYPE,
  TYPEDEF_EXPECTED_SEMICOLON,
  TYPEDEF_NAME_DUPLICATE,

  FOR_EXPECTED_SEMICOLON,
  FOR_EXPECTED_CLOSING_PARANTHESIS,

  FORM_EXPECTED_IDENTIFIER,
  FORM_EXPECTED_IN,
  FORM_EXPECTED_EXPR,
  FORM_EXPECTED_BLOCK,
  FORM_EXPECTED_CLOSE_PAREN,

  FOREACH_EXPECTED_IDENTIFIER,
  FOREACH_EXPECTED_IN,
  FOREACH_EXPECTED_EXPR,
  FOREACH_EXPECTED_BLOCK,
  FOREACH_EXPECTED_CLOSE_PAREN,

  ASM_EXPECTED_LEFT_PAREN,
  ASM_EXPECTED_STRING_LITERAL,
  ASM_EXPECTED_COMMA,
  ASM_EXPECTED_OUTPUTS,
  ASM_EXPECTED_INPUTS,
  ASM_EXPECTED_CLOBBERS,
  ASM_EXPECTED_RIGHT_PAREN,
  ASM_EXPECTED_SEMICOLON,
  ASM_EXPECTED_LEFT_BRACE,
  ASM_PARAM_EXPECTED_STRING_LITERAL,
  ASM_PARAM_EXPECTED_COLON,
  ASM_PARAM_EXPECTED_EXPR,

  SWITCH_EXPECTED_LEFT_BRACE,
  SWITCH_MULTIPLE_DEFAULT,
  SWITCH_EXPECTED_COLON,
  SWITCH_EXPECTED_CASE,
  SWITCH_EXPECTED_RIGHT_BRACE,

  UNRESOLVED_TYPE,
  UNRESOLVED_FUNCTION,
  RESOLVED_TYPE,

  GENERIC_ERROR,

  MAX_MSG
};
}

namespace libquixcc::core {

class Feedback {
  std::unordered_map<Msg, const char *> m_feedback;

 public:
  const char *operator[](Msg msg) const { return m_feedback.at(msg); }
  Feedback(const std::unordered_map<Msg, const char *> &feedback)
      : m_feedback(feedback) {}
};

extern Feedback feedback;

};  // namespace libquixcc::core

#endif