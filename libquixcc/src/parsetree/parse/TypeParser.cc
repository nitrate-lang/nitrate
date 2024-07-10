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

#define QUIXCC_INTERNAL

#include <LibMacro.h>
#include <core/Logger.h>
#include <parsetree/Parser.h>

#include <utility>

using namespace libquixcc;
using namespace libquixcc::core;

template <typename T>
using shared = std::shared_ptr<T>;

#define ERRORS(__type) LOG(ERROR) << feedback[__type] << tok << std::endl

static std::map<std::string, shared<TypeNode>> primitives = {
    {"u8", std::make_shared<U8TypeNode>()},
    {"u16", std::make_shared<U16TypeNode>()},
    {"u32", std::make_shared<U32TypeNode>()},
    {"u64", std::make_shared<U64TypeNode>()},
    {"u128", std::make_shared<U128TypeNode>()},
    {"i8", std::make_shared<I8TypeNode>()},
    {"i16", std::make_shared<I16TypeNode>()},
    {"i32", std::make_shared<I32TypeNode>()},
    {"i64", std::make_shared<I64TypeNode>()},
    {"i128", std::make_shared<I128TypeNode>()},
    {"f32", std::make_shared<F32TypeNode>()},
    {"f64", std::make_shared<F64TypeNode>()},
    {"i1", std::make_shared<BoolTypeNode>()},
    {"string", std::make_shared<StringTypeNode>()},
    {"void", std::make_shared<VoidTypeNode>()},
    {"null", std::make_shared<NullTypeNode>()}};

bool libquixcc::parse_type(quixcc_job_t &job, Scanner *src,
                           shared<TypeNode> &node) {
  /** QUIX TYPE PARSER
   *
   * @brief Given a Scanner, parse tokens into a QUIX type node.
   *
   * @note No validation is done here. This is just a parser.
   *
   * @return true if the type was parsed successfully, false otherwise.
   */

  using namespace std;

  shared<TypeNode> type, inner, value_type;
  shared<StmtNode> fn;
  shared<ConstExprNode> size;
  vector<shared<TypeNode>> types;
  string name;

  Token tok;

  if ((tok = src->next()).type == TT::Keyword) {
    switch (tok.as<Keyword>()) {
      case Keyword::Void: {
        /** QUIX VOID TYPE
         *
         * @brief Parse a void type.
         */

        inner = make_shared<VoidTypeNode>();
        goto type_suffix;
      }

      case Keyword::Null: {
        /** QUIX NULL TYPE
         *
         * @brief Parse a null type.
         */

        inner = make_shared<NullTypeNode>();
        goto type_suffix;
      }

      case Keyword::Fn: {
        /** QUIX FUNCTION TYPE
         *
         * @brief Parse a function type.
         *
         * @note We will reuse the function parser here. We expect
         *       a function declaration node to be returned. A
         *       will end with a semicolon. But a function type
         *       will not end with a semicolon. We push a semicolon
         *       to account for this.
         */

        if (!parse_function(job, src, fn)) {
          ERRORS(TYPE_EXPECTED_FUNCTION);
          goto error_end;
        }

        if (!fn->is<FunctionDeclNode>()) {
          ERRORS(TYPE_EXPECTED_FUNCTION);
          goto error_end;
        }

        inner = static_pointer_cast<FunctionDeclNode>(fn)->m_type;

        /* Push a semicolon to account for the above usage. */
        src->push(Token(TT::Punctor, Punctor::Semicolon));

        goto type_suffix;
      }

      case Keyword::Opaque: {
        /** QUIX OPAQUE TYPE
         *
         * @brief Parse an opaque type.
         *
         * @note An opaque type is a type that is not defined in the
         *       current scope. It is a placeholder for a type that
         *       is distinguisable by its name.
         */

        if (!(tok = src->next()).is<Punctor>(Punctor::OpenParen)) {
          ERRORS(TYPE_OPAQUE_EXPECTED_PAREN);
          goto error_end;
        }

        if ((tok = src->next()).type != TT::Identifier) {
          ERRORS(TYPE_OPAQUE_EXPECTED_IDENTIFIER);
          goto error_end;
        }

        name = tok.as<string>(); /* Save the name of the opaque type. */

        if (!(tok = src->next()).is<Punctor>(Punctor::CloseParen)) {
          ERRORS(TYPE_OPAQUE_EXPECTED_CLOSE_PAREN);
          goto error_end;
        }

        inner = make_shared<OpaqueTypeNode>(name);
        goto type_suffix;
      }

      default: {
        /*! We should not reach here in legal code. */
        goto error_end;
      }
    }

    __builtin_unreachable();
  } else if (tok.type == TT::Identifier) {
    if (primitives.contains(tok.as<string>())) {
      /** QUIX PRIMITIVE TYPE
       *
       * @brief Parse a primitive type.
       */

      inner = primitives[tok.as<string>()];
      goto type_suffix;
    } else {
      /** QUIX ANY NAMED TYPE
       *
       * @brief Parse a named type.
       *
       * @note A named type is a type that is referenced by its name.
       *       It is a placeholder for a type that is defined elsewhere.
       */

      inner = make_shared<UserTypeNode>(tok.as<string>());
      goto type_suffix;
    }

    __builtin_unreachable();
  } else if (tok.is<Punctor>(Punctor::OpenBracket)) {
    /** THIS COULD BE A VECTOR, MAP, OR ARRAY TYPE
     *
     * @brief Parse a vector, map, or array type.
     */

    if (!parse_type(job, src, type)) {
      ERRORS(TYPE_EXPECTED_TYPE);
      goto error_end;
    }

    if ((tok = src->next()).is<Punctor>(Punctor::CloseBracket)) {
      /** QUIX VECTOR TYPE
       *
       * @brief Parse a vector type.
       */

      inner = make_shared<VectorTypeNode>(type);
      goto type_suffix;
    }

    if (tok.is<Operator>(Operator::Minus)) {
      /** QUIX MAP TYPE
       *
       * @brief Parse a map type.
       */

      if (!(tok = src->next()).is<Operator>(Operator::GreaterThan)) {
        ERRORS(TYPE_EXPECTED_MAP_ARROW);
        goto error_end;
      }

      if (!parse_type(job, src, value_type)) {
        ERRORS(TYPE_EXPECTED_TYPE);
        goto error_end;
      }

      if (!(tok = src->next()).is<Punctor>(Punctor::CloseBracket)) {
        ERRORS(TYPE_EXPECTED_CLOSE_BRACKET);
        goto error_end;
      }

      inner = make_shared<MapTypeNode>(type, value_type);
      goto type_suffix;
    }

    /** QUIX ARRAY TYPE
     *
     * @brief Parse an array type.
     */

    if (!tok.is<Punctor>(Punctor::Semicolon)) {
      ERRORS(TYPE_EXPECTED_SEMICOLON);
      goto error_end;
    }

    if (!parse_const_expr(job, src, Token(TT::Punctor, Punctor::CloseBracket),
                          size)) {
      ERRORS(TYPE_EXPECTED_CONST_EXPR);
      goto error_end;
    }

    if (!(tok = src->next()).is<Punctor>(Punctor::CloseBracket)) {
      ERRORS(TYPE_EXPECTED_CLOSE_BRACKET);
      goto error_end;
    }

    inner = make_shared<ArrayTypeNode>(type, size);
    goto type_suffix;
  } else if (tok.is<Punctor>(Punctor::OpenBrace)) {
    /** QUIX SET TYPE
     *
     * @brief Parse a set type.
     */

    if (!parse_type(job, src, type)) {
      ERRORS(TYPE_EXPECTED_TYPE);
      goto error_end;
    }

    if (!(tok = src->next()).is<Punctor>(Punctor::CloseBrace)) {
      ERRORS(TYPE_EXPECTED_CLOSE_BRACE);
      goto error_end;
    }

    inner = make_shared<SetTypeNode>(type);
    goto type_suffix;
  } else if (tok.is<Punctor>(Punctor::OpenParen)) {
    /** QUIX TUPLE TYPE
     *
     * @brief Parse a tuple type.
     */

    while (true) {
      if ((tok = src->peek()).is<Punctor>(Punctor::CloseParen)) {
        src->next();
        break;
      }

      if (!parse_type(job, src, type)) {
        ERRORS(TYPE_EXPECTED_TYPE);
        goto error_end;
      }

      types.push_back(type);

      tok = src->peek();
      if (tok.is<Punctor>(Punctor::Comma)) {
        src->next();
      }
    }

    inner = make_shared<TupleTypeNode>(types);
    goto type_suffix;
  } else if (tok.is<Operator>(Operator::Multiply)) {
    /** QUIX POINTER TYPE
     *
     * @brief Parse a pointer type.
     */

    if (!parse_type(job, src, type)) {
      ERRORS(TYPE_EXPECTED_TYPE);
      goto error_end;
    }

    inner = make_shared<PointerTypeNode>(type);
    goto type_suffix;
  } else if (tok.is<Operator>(Operator::LogicalNot)) {
    /** QUIX MUTABLE TYPE
     *
     * @brief Parse a mutable type.
     */

    if (!parse_type(job, src, type)) {
      ERRORS(TYPE_EXPECTED_TYPE);
      goto error_end;
    }

    inner = make_shared<MutTypeNode>(type);
    goto type_suffix;
  } else {
    ERRORS(EXPECTED_TYPE);
    goto error_end;
  }

type_suffix: {
  /** QUIX TYPE SUFFIXES
   *
   * @brief Parse type suffixes (syntax sugar).
   */

  while (true) {
    if ((tok = src->peek()).is<Operator>(Operator::Question)) {
      src->next();
      inner = make_shared<ResultTypeNode>(inner);
      continue;
    }

    break;
  }
}

  assert(inner != nullptr);

  node = inner;
  return true;

error_end:
  return false;
}
