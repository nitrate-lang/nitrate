////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <parser/Parse.h>

#include <unordered_map>

#include "nitrate-lexer/Lexer.h"
#include "nitrate-lexer/Token.h"
#include "nitrate-parser/Node.h"

using namespace qparse;
using namespace qparse::parser;
using namespace qparse::diag;

/// TODO: Source location

static const std::unordered_map<std::string_view, Type *(*)()> primitive_types =
    {{"u1", []() -> Type * { return U1::get(); }},
     {"u8", []() -> Type * { return U8::get(); }},
     {"u16", []() -> Type * { return U16::get(); }},
     {"u32", []() -> Type * { return U32::get(); }},
     {"u64", []() -> Type * { return U64::get(); }},
     {"u128", []() -> Type * { return U128::get(); }},
     {"i8", []() -> Type * { return I8::get(); }},
     {"i16", []() -> Type * { return I16::get(); }},
     {"i32", []() -> Type * { return I32::get(); }},
     {"i64", []() -> Type * { return I64::get(); }},
     {"i128", []() -> Type * { return I128::get(); }},
     {"f16", []() -> Type * { return F16::get(); }},
     {"f32", []() -> Type * { return F32::get(); }},
     {"f64", []() -> Type * { return F64::get(); }},
     {"f128", []() -> Type * { return F128::get(); }},
     {"void", []() -> Type * { return VoidTy::get(); }}};

bool qparse::parser::parse_type(qparse_t &job, qlex_t *rd, Type **node) {
  /** Nitrate TYPE PARSER
   *
   * @brief Given a Scanner, parse tokens into a Nitrate type node.
   *
   * @note No validation is done here. This is just a parser.
   *
   * @return true if it is okay to proceed, false otherwise.
   */

  using namespace std;

  Type *type, *inner, *value_type;
  type = inner = value_type = nullptr;
  Stmt *fn = nullptr;
  Expr *size = nullptr;
  vector<Type *> types;
  string name;

  qlex_tok_t tok;

  *node = nullptr;

  if ((tok = qlex_next(rd)).ty == qKeyW) {
    switch (tok.as<qlex_key_t>()) {
      case qKFn: {
        /** Nitrate FUNCTION TYPE
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
        // qlex_push(rd, qlex_tok_t(qPunc, qPuncSemi));

        goto type_suffix;
      }

      case qKOpaque: {
        /** Nitrate OPAQUE TYPE
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
    if (primitive_types.contains(tok.as_string(rd))) {
      /** Nitrate PRIMITIVE TYPE
       *
       * @brief Parse a primitive type.
       */

      inner = primitive_types.at(tok.as_string(rd))();
      goto type_suffix;
    } else {
      /** Nitrate ANY NAMED TYPE
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
      /** Nitrate VECTOR TYPE
       *
       * @brief Parse a vector type.
       */

      inner = TemplType::get(UnresolvedType::get("__builtin_vec"),
                             TemplTypeArgs{TypeExpr::get(type)});
      goto type_suffix;
    }

    if (tok.is<qOpMinus>()) {
      /** Nitrate MAP TYPE
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

      inner = TemplType::get(
          UnresolvedType::get("__builtin_umap"),
          TemplTypeArgs{TypeExpr::get(type), TypeExpr::get(value_type)});
      goto type_suffix;
    }

    /** Nitrate ARRAY TYPE
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
      size = _size;
    }

    if (!(tok = qlex_next(rd)).is<qPuncRBrk>()) {
      syntax(tok, "Expected ']' after array size");
      goto error_end;
    }

    inner = ArrayTy::get(type, size);
    goto type_suffix;
  } else if (tok.is<qPuncLCur>()) {
    /** Nitrate SET TYPE
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

    inner = TemplType::get(UnresolvedType::get("__builtin_uset"),
                           TemplTypeArgs{TypeExpr::get(type)});
    goto type_suffix;
  } else if (tok.is<qPuncLPar>()) {
    /** Nitrate TUPLE TYPE
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
    /** Nitrate POINTER TYPE
     *
     * @brief Parse a pointer type.
     */

    if (!parse_type(job, rd, &type)) {
      syntax(tok, "Expected a type after '*'");
      goto error_end;
    }

    inner = PtrTy::get(type);
    goto type_suffix;
  } else if (tok.is<qOpBitAnd>()) {
    /** Nitrate MUTABLE TYPE
     *
     * @brief Parse a mutable type.
     */

    if (!parse_type(job, rd, &type)) {
      syntax(tok, "Expected a type after '!' mutable type");
      goto error_end;
    }

    inner = RefTy::get(type);
    goto type_suffix;
  } else if (tok.is<qOpTernary>()) {
    /** Nitrate INFERRED TYPE
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
  /** Nitrate TEMPLATE TYPES
   *
   */

  tok = qlex_peek(rd);
  if (tok.is<qOpLT>()) {
    qlex_next(rd);

    TemplTypeArgs args;

    while (true) {
      tok = qlex_peek(rd);
      if (tok.is<qOpGT>() || tok.is<qOpRShift>() || tok.is<qOpROTR>() ||
          tok.ty == qEofF) {
        break;
      }

      Type *arg = nullptr;
      if (!parse_type(job, rd, &arg)) {
        syntax(tok, "Expected a template type argument");
        goto error_end;
      }

      args.push_back(TypeExpr::get(arg));

      tok = qlex_peek(rd);
      if (tok.is<qPuncComa>()) {
        qlex_next(rd);
      }
    }

    tok = qlex_next(rd);

    if (tok.is<qOpGT>()) {
    } else if (tok.is<qOpRShift>()) {
      tok.v.op = qOpGT;
      qlex_insert(rd, tok);
    } else if (tok.is<qOpROTR>()) {
      tok.v.op = qOpRShift;
      qlex_insert(rd, tok);
    } else {
      syntax(tok, "Expected '>' after template type arguments");
      goto error_end;
    }

    qlex_peek(rd);

    inner = TemplType::get(inner, args);
  }

  /** Nitrate TYPE SUFFIXES
   *
   * @brief Parse type suffixes (syntax sugar).
   */

  while (true) {
    tok = qlex_peek(rd);

    if (tok.is<qPuncColn>()) { /* Parse bit-field width or confinement range */
      qlex_next(rd);
      tok = qlex_peek(rd);

      if (tok.is<qPuncLBrk>()) { /* Parse confinement range */
        qlex_next(rd);

        Expr *start = nullptr, *end = nullptr;

        tok = qlex_peek(rd);
        if (tok.is<qPuncColn>()) {
          start = nullptr;
        } else {
          if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncColn)}, &start)) {
            syntax(tok, "Expected start of confinement range");
            goto error_end;
          }
        }
        qlex_next(rd);
        tok = qlex_peek(rd);

        if (tok.is<qPuncRBrk>()) {
          end = nullptr;
        } else {
          if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncRBrk)}, &end)) {
            syntax(tok, "Expected end of confinement range");
            goto error_end;
          }
        }
        qlex_next(rd);

        inner->set_range(start, end);
      } else { /* Parse bit-field width */
        Expr *expr = nullptr;
        if (!parse_expr(job, rd,
                        {
                            qlex_tok_t(qPunc, qPuncRPar),  //
                            qlex_tok_t(qPunc, qPuncRBrk),  //
                            qlex_tok_t(qPunc, qPuncLCur),  //
                            qlex_tok_t(qPunc, qPuncRCur),  //
                            qlex_tok_t(qPunc, qPuncComa),  //
                            qlex_tok_t(qPunc, qPuncColn),  //
                            qlex_tok_t(qPunc, qPuncSemi),  //
                            qlex_tok_t(qOper, qOpSet),     //
                            qlex_tok_t(qOper, qOpMinus),   //
                            qlex_tok_t(qOper, qOpGT),      //
                        },
                        &expr)) {
          syntax(tok, "Expected expression for bit-field width");
          goto error_end;
        }

        inner->set_width(expr);
      }

      continue;
    }

    if (tok.is<qOpTernary>()) { /* Parse optional type */
      qlex_next(rd);
      inner = TemplType::get(UnresolvedType::get("__builtin_result"),
                             TemplTypeArgs{TypeExpr::get(inner)});
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
