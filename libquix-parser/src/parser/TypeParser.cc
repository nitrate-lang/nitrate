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

#include <utility>

#include "LibMacro.h"
#include "parser/Parse.h"

using namespace qparse;
using namespace qparse::parser;

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
   * @return true if the type was parsed successfully, false otherwise.
   */

  using namespace std;

  Type *type, *inner, *value_type;
  type = inner = value_type = nullptr;
  Stmt *fn = nullptr;
  ConstExpr *size = nullptr;
  vector<Type *> types;
  string name;

  qlex_tok_t tok;

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

        if (!parse_function(job, rd, &fn)) {
          /// TODO: Write error message
          goto error_end;
        }

        if (!fn->is<FnDecl>()) {
          /// TODO: Write error message
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
          /// TODO: Write error message
          goto error_end;
        }

        if ((tok = qlex_next(rd)).ty != qName) {
          /// TODO: Write error message
          goto error_end;
        }

        name = tok.as_string(rd); /* Save the name of the opaque type. */

        if (!(tok = qlex_next(rd)).is<qPuncRPar>()) {
          /// TODO: Write error message
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
      /// TODO: Write error message
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
        /// TODO: Write error message
        goto error_end;
      }

      if (!parse_type(job, rd, &value_type)) {
        /// TODO: Write error message
        goto error_end;
      }

      if (!(tok = qlex_next(rd)).is<qPuncRBrk>()) {
        /// TODO: Write error message
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
      /// TODO: Write error message
      goto error_end;
    }

    {
      Expr *_size = nullptr;
      if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncRBrk)}, &_size)) {
        /// TODO: Write error message
        goto error_end;
      }
      size = ConstExpr::get(_size);
    }

    if (!(tok = qlex_next(rd)).is<qPuncRBrk>()) {
      /// TODO: Write error message
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
      /// TODO: Write error message
      goto error_end;
    }

    if (!(tok = qlex_next(rd)).is<qPuncRCur>()) {
      /// TODO: Write error message
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
        /// TODO: Write error message
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
      /// TODO: Write error message
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
      /// TODO: Write error message
      goto error_end;
    }

    inner = MutTy::get(type);
    goto type_suffix;
  } else {
    /// TODO: Write error message
    goto error_end;
  }

type_suffix: {
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
