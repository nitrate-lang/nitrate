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

#define __QUIX_IMPL__

#include <parser/Parse.h>

#include <utility>

using namespace qparse;
using namespace qparse::parser;
using namespace qparse::diag;

// Lifetime integrity requires the primitives to be thread-local because the Node Arena allocator is
// thread-local.
static thread_local std::map<std::string_view, Type *> primitives = {
    {"u8", U8::get()},     {"u16", U16::get()},         {"u32", U32::get()},    {"u64", U64::get()},
    {"u128", U128::get()}, {"i8", I8::get()},           {"i16", I16::get()},    {"i32", I32::get()},
    {"i64", I64::get()},   {"i128", I128::get()},       {"f32", F32::get()},    {"f64", F64::get()},
    {"i1", U1::get()},     {"string", StringTy::get()}, {"void", VoidTy::get()}};

bool qparse::parser::parse_type(qparse_t &job, qlex_t *rd, Type **node) {
  /** QUIX TYPE PARSER
   *
   * @brief Given a Scanner, parse tokens into a QUIX type node.
   *
   * @note No validation is done here. This is just a parser.
   *
   * @return true if it is okay to proceed, false otherwise.
   */

  using namespace std;

  Type *type, *inner, *value_type;
  type = inner = value_type = nullptr;
  Stmt *fn = nullptr;
  ConstExpr *size = nullptr;
  vector<Type *> types;
  string name;

  qlex_tok_t tok;

  *node = nullptr;

  if ((tok = qlex_next(rd)).ty == qKeyW) {
    switch (tok.as<qlex_key_t>()) {
      case qKVoid: {
        /** QUIX VOID TYPE
         *
         * @brief Parse a void type.
         */

        inner = VoidTy::get();
        goto type_suffix;
      }

      case qKFn: {
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

        if (!parse_function(job, rd, &fn) || !fn) {
          syntax(tok, "Malformed function type");
          goto error_end;
        }

        if (!fn->is<FnDecl>()) {
          syntax(tok, "Expected a function declaration but got something else");
          goto error_end;
        }

        inner = fn->as<FnDecl>()->get_type();

        /* Push a semicolon to account for the above usage. */
        qlex_push(rd, qlex_tok_t(qPunc, qPuncSemi));

        goto type_suffix;
      }

      case qKOpaque: {
        /** QUIX OPAQUE TYPE
         *
         * @brief Parse an opaque type.
         *
         * @note An opaque type is a type that is not defined in the
         *       current scope. It is a placeholder for a type that
         *       is distinguisable by its name.
         */

        if (!(tok = qlex_next(rd)).is<qPuncLPar>()) {
          syntax(tok, "Expected '(' after 'opaque'");
          goto error_end;
        }

        if ((tok = qlex_next(rd)).ty != qName) {
          syntax(tok, "Expected a name after 'opaque('");
          goto error_end;
        }

        name = tok.as_string(rd); /* Save the name of the opaque type. */

        if (!(tok = qlex_next(rd)).is<qPuncRPar>()) {
          syntax(tok, "Expected ')' after 'opaque(name'");
          goto error_end;
        }

        inner = OpaqueTy::get(name);
        goto type_suffix;
      }

      default: {
        /*! We should not reach here in legal code. */
        goto error_end;
      }
    }

    __builtin_unreachable();
  } else if (tok.is(qName)) {
    if (primitives.contains(tok.as_string(rd))) {
      /** QUIX PRIMITIVE TYPE
       *
       * @brief Parse a primitive type.
       */

      inner = primitives[tok.as_string(rd)];
      goto type_suffix;
    } else {
      /** QUIX ANY NAMED TYPE
       *
       * @brief Parse a named type.
       *
       * @note A named type is a type that is referenced by its name.
       *       It is a placeholder for a type that is defined elsewhere.
       */

      inner = UnresolvedType::get(tok.as_string(rd));
      goto type_suffix;
    }

    __builtin_unreachable();
  } else if (tok.is<qPuncLBrk>()) {
    /** THIS COULD BE A VECTOR, MAP, OR ARRAY TYPE
     *
     * @brief Parse a vector, map, or array type.
     */

    if (!parse_type(job, rd, &type)) {
      syntax(tok, "Expected a type after '['");
      goto error_end;
    }

    if ((tok = qlex_next(rd)).is<qPuncRBrk>()) {
      /** QUIX VECTOR TYPE
       *
       * @brief Parse a vector type.
       */

      inner = VectorTy::get(type);
      goto type_suffix;
    }

    if (tok.is<qOpMinus>()) {
      /** QUIX MAP TYPE
       *
       * @brief Parse a map type.
       */

      if (!(tok = qlex_next(rd)).is<qOpGT>()) {
        syntax(tok, "Expected '>' after '-' in map type");
        goto error_end;
      }

      if (!parse_type(job, rd, &value_type)) {
        syntax(tok, "Expected value type after '>' in map type");
        goto error_end;
      }

      if (!(tok = qlex_next(rd)).is<qPuncRBrk>()) {
        syntax(tok, "Expected ']' after map type");
        goto error_end;
      }

      inner = MapTy::get(type, value_type);
      goto type_suffix;
    }

    /** QUIX ARRAY TYPE
     *
     * @brief Parse an array type.
     */

    if (!tok.is<qPuncSemi>()) {
      syntax(tok, "Expected ';' separator in array type before size");
      goto error_end;
    }

    {
      Expr *_size = nullptr;
      if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncRBrk)}, &_size)) {
        syntax(tok, "Expected array size after ';'");
        goto error_end;
      }
      size = ConstExpr::get(_size);
    }

    if (!(tok = qlex_next(rd)).is<qPuncRBrk>()) {
      syntax(tok, "Expected ']' after array size");
      goto error_end;
    }

    inner = ArrayTy::get(type, size);
    goto type_suffix;
  } else if (tok.is<qPuncLCur>()) {
    /** QUIX SET TYPE
     *
     * @brief Parse a set type.
     */

    if (!parse_type(job, rd, &type)) {
      syntax(tok, "Expected a type after '{'");
      goto error_end;
    }

    if (!(tok = qlex_next(rd)).is<qPuncRCur>()) {
      syntax(tok, "Expected '}' after set type");
      goto error_end;
    }

    inner = SetTy::get(type);
    goto type_suffix;
  } else if (tok.is<qPuncLPar>()) {
    /** QUIX TUPLE TYPE
     *
     * @brief Parse a tuple type.
     */

    while (true) {
      if ((tok = qlex_peek(rd)).is<qPuncRPar>()) {
        qlex_next(rd);
        break;
      }

      if (!parse_type(job, rd, &type)) {
        syntax(tok, "Expected a type in tuple type");
        goto error_end;
      }

      types.push_back(type);

      tok = qlex_peek(rd);
      if (tok.is<qPuncComa>()) {
        qlex_next(rd);
      }
    }

    inner = TupleTy::get(TupleTyItems(types.begin(), types.end()));
    goto type_suffix;
  } else if (tok.is<qOpTimes>()) {
    /** QUIX POINTER TYPE
     *
     * @brief Parse a pointer type.
     */

    if (!parse_type(job, rd, &type)) {
      syntax(tok, "Expected a type after '*'");
      goto error_end;
    }

    inner = PtrTy::get(type);
    goto type_suffix;
  } else if (tok.is<qOpLogicNot>()) {
    /** QUIX MUTABLE TYPE
     *
     * @brief Parse a mutable type.
     */

    if (!parse_type(job, rd, &type)) {
      syntax(tok, "Expected a type after '!' mutable type");
      goto error_end;
    }

    inner = MutTy::get(type);
    goto type_suffix;
  } else if (tok.is<qOpTernary>()) {
    /** QUIX INFERRED TYPE
     *
     * @brief Parse an inferred type.
     */

    inner = InferType::get();
    goto type_suffix;
  } else {
    syntax(tok, "Expected a type");
    goto error_end;
  }

type_suffix: {
  /** QUIX TEMPLATE TYPES
   *
   */

  tok = qlex_peek(rd);
  if (tok.is<qOpLT>()) {
    qlex_next(rd);
    
    TemplTypeArgs args;

    while (true) {
      tok = qlex_peek(rd);
      if (tok.is<qOpGT>() || tok.ty == qEofF) {
        break;
      }

      Expr *arg = nullptr;
      if (!parse_expr(job, rd, {qlex_tok_t(qOper, qOpGT), qlex_tok_t(qPunc, qPuncComa)}, &arg) ||
          !arg) {
        syntax(tok, "Expected a template type argument");
        goto error_end;
      }

      args.push_back(arg);

      tok = qlex_peek(rd);
      if (tok.is<qPuncComa>()) {
        qlex_next(rd);
      }
    }

    if (!(tok = qlex_next(rd)).is<qOpGT>()) {
      syntax(tok, "Expected '>' after template type arguments");
      goto error_end;
    }

    inner = TemplType::get(inner, args);
  }

  /** QUIX TYPE SUFFIXES
   *
   * @brief Parse type suffixes (syntax sugar).
   */

  while (true) {
    if ((tok = qlex_peek(rd)).is<qOpTernary>()) {
      qlex_next(rd);
      inner = OptionalTy::get(inner);
      continue;
    }

    break;
  }
}

  assert(inner != nullptr);

  *node = inner;
  return true;

error_end:
  return false;
}
