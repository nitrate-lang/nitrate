////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .
///    | .
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '
///     '
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

/// TODO: Cleanup this code; it's a mess from refactoring.

#include <nitrate-lexer/Lexer.h>
#include <nitrate-lexer/Token.h>
#include <nitrate-parser/Node.h>

#include <descent/Recurse.hh>
#include <unordered_map>

#include "core/ParseReport.hh"
#include "nitrate-core/Error.h"

using namespace npar;

/// TODO: Source location

// static const std::unordered_map<std::string_view, Type *(*)()>
// primitive_types =
//     {{"u1", []() -> Type * { return U1::get(); }},
//      {"u8", []() -> Type * { return U8::get(); }},
//      {"u16", []() -> Type * { return U16::get(); }},
//      {"u32", []() -> Type * { return U32::get(); }},
//      {"u64", []() -> Type * { return U64::get(); }},
//      {"u128", []() -> Type * { return U128::get(); }},
//      {"i8", []() -> Type * { return I8::get(); }},
//      {"i16", []() -> Type * { return I16::get(); }},
//      {"i32", []() -> Type * { return I32::get(); }},
//      {"i64", []() -> Type * { return I64::get(); }},
//      {"i128", []() -> Type * { return I128::get(); }},
//      {"f16", []() -> Type * { return F16::get(); }},
//      {"f32", []() -> Type * { return F32::get(); }},
//      {"f64", []() -> Type * { return F64::get(); }},
//      {"f128", []() -> Type * { return F128::get(); }},
//      {"void", []() -> Type * { return VoidTy::get(); }}};

// Type *npar::recurse_type(npar_t &S, qlex_t &rd) {
//   using namespace std;

//   Type *type, *inner, *value_type;
//   type = inner = value_type = nullptr;
//   Stmt *fn = nullptr;
//   Expr *size = nullptr;
//   vector<Type *> types;
//   string name;

//   qlex_tok_t tok;

//   if ((tok = next()).ty == qKeyW) {
//     switch (tok.as<qlex_key_t>()) {
//       case qKFn: {
//         /** Nitrate FUNCTION TYPE
//          *
//          * @brief Parse a function type.
//          *
//          * @note We will reuse the function parser here. We expect
//          *       a function declaration node to be returned. A
//          *       will end with a semicolon. But a function type
//          *       will not end with a semicolon. We push a semicolon
//          *       to account for this.
//          */

//         fn = recurse_function(S, rd);

//         if (!fn->is<FnDecl>()) {
//           diagnostic
//               << tok
//               << "Expected a function declaration but got something else";
//           goto error_end;
//         }

//         inner = fn->as<FnDecl>()->get_type();

//         /* Push a semicolon to account for the above usage. */
//         // qlex_push(rd, qlex_tok_t(qPunc, qPuncSemi));

//         goto type_suffix;
//       }

//       case qKOpaque: {
//         /** Nitrate OPAQUE TYPE
//          *
//          * @brief Parse an opaque type.
//          *
//          * @note An opaque type is a type that is not defined in the
//          *       current scope. It is a placeholder for a type that
//          *       is distinguisable by its name.
//          */

//         if (!(tok = next()).is<qPuncLPar>()) {
//           diagnostic << tok << "Expected '(' after 'opaque'";
//           goto error_end;
//         }

//         if ((tok = next()).ty != qName) {
//           diagnostic << tok << "Expected a name after 'opaque('";
//           goto error_end;
//         }

//         name = tok.as_string(&rd); /* Save the name of the opaque type. */

//         if (!(tok = next()).is<qPuncRPar>()) {
//           diagnostic << tok << "Expected ')' after 'opaque(name'";
//           goto error_end;
//         }

//         inner = OpaqueTy::get(name);
//         goto type_suffix;
//       }

//       default: {
//         /*! We should not reach here in legal code. */
//         goto error_end;
//       }
//     }

//     __builtin_unreachable();
//   } else if (tok.is(qName)) {
//     if (primitive_types.contains(tok.as_string(&rd))) {
//       /** Nitrate PRIMITIVE TYPE
//        *
//        * @brief Parse a primitive type.
//        */

//       inner = primitive_types.at(tok.as_string(&rd))();
//       goto type_suffix;
//     } else {
//       /** Nitrate ANY NAMED TYPE
//        *
//        * @brief Parse a named type.
//        *
//        * @note A named type is a type that is referenced by its name.
//        *       It is a placeholder for a type that is defined elsewhere.
//        */

//       inner = NamedTy::get(tok.as_string(&rd));
//       goto type_suffix;
//     }

//     __builtin_unreachable();
//   } else if (tok.is<qPuncLBrk>()) {
//     /** THIS COULD BE A VECTOR, MAP, OR ARRAY TYPE
//      *
//      * @brief Parse a vector, map, or array type.
//      */

//     type = recurse_type(S, rd);

//     if ((tok = next()).is<qPuncRBrk>()) {
//       /** Nitrate VECTOR TYPE
//        *
//        * @brief Parse a vector type.
//        */

//       inner = TemplType::get(NamedTy::get("__builtin_vec"),
//                              TemplTypeArgs{TypeExpr::get(type)});
//       goto type_suffix;
//     }

//     if (tok.is<qOpMinus>()) {
//       /** Nitrate MAP TYPE
//        *
//        * @brief Parse a map type.
//        */

//       if (!(tok = next()).is<qOpGT>()) {
//         diagnostic << tok << "Expected '>' after '-' in map type";
//         goto error_end;
//       }

//       value_type = recurse_type(S, rd);

//       if (!(tok = next()).is<qPuncRBrk>()) {
//         diagnostic << tok << "Expected ']' after map type";
//         goto error_end;
//       }

//       inner = TemplType::get(
//           NamedTy::get("__builtin_umap"),
//           TemplTypeArgs{TypeExpr::get(type), TypeExpr::get(value_type)});
//       goto type_suffix;
//     }

//     /** Nitrate ARRAY TYPE
//      *
//      * @brief Parse an array type.
//      */

//     if (!tok.is<qPuncSemi>()) {
//       diagnostic << tok << "Expected ';' separator in array type before
//       size"; goto error_end;
//     }

//     size = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncRBrk)});

//     if (!(tok = next()).is<qPuncRBrk>()) {
//       diagnostic << tok << "Expected ']' after array size";
//       goto error_end;
//     }

//     inner = ArrayTy::get(type, size);
//     goto type_suffix;
//   } else if (tok.is<qPuncLCur>()) {
//     /** Nitrate SET TYPE
//      *
//      * @brief Parse a set type.
//      */

//     type = recurse_type(S, rd);

//     if (!(tok = next()).is<qPuncRCur>()) {
//       diagnostic << tok << "Expected '}' after set type";
//       goto error_end;
//     }

//     inner = TemplType::get(NamedTy::get("__builtin_uset"),
//                            TemplTypeArgs{TypeExpr::get(type)});
//     goto type_suffix;
//   } else if (tok.is<qPuncLPar>()) {
//     /** Nitrate TUPLE TYPE
//      *
//      * @brief Parse a tuple type.
//      */

//     while (true) {
//       if (peek().is(qEofF)) {
//         diagnostic << current() << "Unexpected EOF in tuple type";
//         goto error_end;
//       }

//       if ((tok = peek()).is<qPuncRPar>()) {
//         next();
//         break;
//       }

//       type = recurse_type(S, rd);

//       types.push_back(type);

//       tok = peek();
//       if (tok.is<qPuncComa>()) {
//         next();
//       }
//     }

//     inner = TupleTy::get(TupleTyItems(types.begin(), types.end()));
//     goto type_suffix;
//   } else if (tok.is<qOpTimes>()) {
//     /** Nitrate POINTER TYPE
//      *
//      * @brief Parse a pointer type.
//      */

//     type = recurse_type(S, rd);

//     inner = PtrTy::get(type);
//     goto type_suffix;
//   } else if (tok.is<qOpBitAnd>()) {
//     /** Nitrate MUTABLE TYPE
//      *
//      * @brief Parse a mutable type.
//      */

//     type = recurse_type(S, rd);

//     inner = RefTy::get(type);
//     goto type_suffix;
//   } else if (tok.is<qOpTernary>()) {
//     /** Nitrate INFERRED TYPE
//      *
//      * @brief Parse an inferred type.
//      */

//     inner = InferTy::get();
//     goto type_suffix;
//   } else {
//     diagnostic << tok << "Expected a type";
//     goto error_end;
//   }

// type_suffix: {
//   /** Nitrate TEMPLATE TYPES
//    *
//    */

//   tok = peek();
//   if (tok.is<qOpLT>()) {
//     next();

//     TemplTypeArgs args;

//     while (true) {
//       tok = peek();
//       if (tok.is<qOpGT>() || tok.is<qOpRShift>() || tok.is<qOpROTR>() ||
//           tok.ty == qEofF) {
//         break;
//       }

//       Type *arg = recurse_type(S, rd);

//       args.push_back(TypeExpr::get(arg));

//       tok = peek();
//       if (tok.is<qPuncComa>()) {
//         next();
//       }
//     }

//     tok = next();

//     if (tok.is<qOpGT>()) {
//     } else if (tok.is<qOpRShift>()) {
//       tok.v.op = qOpGT;
//       qlex_insert(&rd, tok);
//     } else if (tok.is<qOpROTR>()) {
//       tok.v.op = qOpRShift;
//       qlex_insert(&rd, tok);
//     } else {
//       diagnostic << tok << "Expected '>' after template type arguments";
//       goto error_end;
//     }

//     peek();

//     inner = TemplType::get(inner, args);
//   }

//   /** Nitrate TYPE SUFFIXES
//    *
//    * @brief Parse type suffixes (syntax sugar).
//    */

//   while (true) {
//     tok = peek();

//     if (tok.is<qPuncColn>()) { /* Parse bit-field width or confinement range
//     */
//       next();
//       tok = peek();

//       if (tok.is<qPuncLBrk>()) { /* Parse confinement range */
//         next();

//         Expr *start = nullptr, *end = nullptr;

//         tok = peek();
//         if (tok.is<qPuncColn>()) {
//           start = nullptr;
//         } else {
//           start = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncColn)});
//         }
//         next();
//         tok = peek();

//         if (tok.is<qPuncRBrk>()) {
//           end = nullptr;
//         } else {
//           end = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncRBrk)});
//         }
//         next();

//         inner->set_range(start, end);
//       } else { /* Parse bit-field width */
//         Expr *expr = recurse_expr(S, rd,
//                                   {
//                                       qlex_tok_t(qPunc, qPuncRPar),  //
//                                       qlex_tok_t(qPunc, qPuncRBrk),  //
//                                       qlex_tok_t(qPunc, qPuncLCur),  //
//                                       qlex_tok_t(qPunc, qPuncRCur),  //
//                                       qlex_tok_t(qPunc, qPuncComa),  //
//                                       qlex_tok_t(qPunc, qPuncColn),  //
//                                       qlex_tok_t(qPunc, qPuncSemi),  //
//                                       qlex_tok_t(qOper, qOpSet),     //
//                                       qlex_tok_t(qOper, qOpMinus),   //
//                                       qlex_tok_t(qOper, qOpGT),      //
//                                   });

//         inner->set_width(expr);
//       }

//       continue;
//     }

//     if (tok.is<qOpTernary>()) { /* Parse optional type */
//       next();
//       inner = TemplType::get(NamedTy::get("__builtin_result"),
//                              TemplTypeArgs{TypeExpr::get(inner)});
//       continue;
//     }

//     break;
//   }
// }

//   assert(inner != nullptr);

//   return inner;

// error_end:
//   return mock_type(QAST_NODE_VOID_TY);
// }

static std::optional<Expr *> recurse_type_range_start(npar_t &S, qlex_t &rd) {
  if (next_if(qPuncColn)) {
    return std::nullopt;
  }

  let min_value = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncColn)});

  if (!next_if(qPuncColn)) {
    diagnostic << current() << "Expected ':' after range start";
  }

  return min_value;
}

static std::optional<Expr *> recurse_type_range_end(npar_t &S, qlex_t &rd) {
  if (next_if(qPuncRBrk)) {
    return std::nullopt;
  }

  let max_val = recurse_expr(S, rd, {qlex_tok_t(qPunc, qPuncRBrk)});

  if (!next_if(qPuncRBrk)) {
    diagnostic << current() << "Expected ']' after range";
  }

  return max_val;
}

static void recurse_type_metadata(npar_t &S, qlex_t &rd, Type *base) {
  static let bit_width_terminaters = {
      qlex_tok_t(qPunc, qPuncRPar), qlex_tok_t(qPunc, qPuncRBrk),
      qlex_tok_t(qPunc, qPuncLCur), qlex_tok_t(qPunc, qPuncRCur),
      qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncColn),
      qlex_tok_t(qPunc, qPuncSemi), qlex_tok_t(qOper, qOpSet),
      qlex_tok_t(qOper, qOpMinus),  qlex_tok_t(qOper, qOpGT)};

  std::pair<std::optional<Expr *>, std::optional<Expr *>> range;
  std::optional<Expr *> width;

  if (next_if(qPuncColn)) {
    if (next_if(qPuncLBrk)) {
      range.first = recurse_type_range_start(S, rd);
      range.second = recurse_type_range_end(S, rd);

      if (next_if(qPuncColn)) {
        width = recurse_expr(S, rd, bit_width_terminaters);
      }
    } else {
      width = recurse_expr(S, rd, bit_width_terminaters);
    }
  }

  base->set_range(range.first.value_or(nullptr),
                  range.second.value_or(nullptr));
  base->set_width(width.value_or(nullptr));
}

static Type *recurse_type_by_keyword(npar_t &S, qlex_t &rd, qlex_key_t key) {
  switch (key) {
    case qKFn: {
      /// TODO:
      qcore_implement();
    }

    case qKOpaque: {
      /// TODO:
      qcore_implement();
    }

    default: {
      diagnostic << current() << "Keyword is not valid in this context";
      return mock_type();
    }
  }
}

static Type *recurse_type_by_operator(npar_t &S, qlex_t &rd, qlex_op_t op) {
  switch (op) {
    case qOpTimes: {
      /// TODO:
      qcore_implement();
    }

    case qOpBitAnd: {
      /// TODO:
      qcore_implement();
    }

    default: {
      diagnostic << current() << "Operator is not valid in this context";
      return mock_type();
    }
  }
}

static Type *recurse_type_by_punctuation(npar_t &S, qlex_t &rd,
                                         qlex_punc_t punc) {
  switch (punc) {
    case qPuncLBrk: {
      /// TODO:
      qcore_implement();
    }

    case qPuncLCur: {
      /// TODO:
      qcore_implement();
    }

    case qPuncLPar: {
      /// TODO:
      qcore_implement();
    }

    default: {
      diagnostic << current() << "Punctuation is not valid in this context";
      return mock_type();
    }
  }
}

static Type *recurse_type_by_name(npar_t &, qlex_t &, std::string_view name) {
  if (name == "u1") {
    return U1::get();
  } else if (name == "u8") {
    return U8::get();
  } else if (name == "u16") {
    return U16::get();
  } else if (name == "u32") {
    return U32::get();
  } else if (name == "u64") {
    return U64::get();
  } else if (name == "u128") {
    return U128::get();
  } else if (name == "i8") {
    return I8::get();
  } else if (name == "i16") {
    return I16::get();
  } else if (name == "i32") {
    return I32::get();
  } else if (name == "i64") {
    return I64::get();
  } else if (name == "i128") {
    return I128::get();
  } else if (name == "f16") {
    return F16::get();
  } else if (name == "f32") {
    return F32::get();
  } else if (name == "f64") {
    return F64::get();
  } else if (name == "f128") {
    return F128::get();
  } else if (name == "void") {
    return VoidTy::get();
  } else {
    return NamedTy::get(name);
  }
}

Type *npar::recurse_type(npar_t &S, qlex_t &rd) {
  switch (let tok = next(); tok.ty) {
    case qKeyW: {
      let type = recurse_type_by_keyword(S, rd, tok.v.key);
      recurse_type_metadata(S, rd, type);

      return type;
    }

    case qOper: {
      let type = recurse_type_by_operator(S, rd, tok.v.op);
      recurse_type_metadata(S, rd, type);

      return type;
    }

    case qPunc: {
      let type = recurse_type_by_punctuation(S, rd, tok.v.punc);
      recurse_type_metadata(S, rd, type);

      return type;
    }

    case qName: {
      let type = recurse_type_by_name(S, rd, tok.as_string(&rd));
      recurse_type_metadata(S, rd, type);

      return type;
    }

    default: {
      diagnostic << current() << "Expected a type";

      let type = mock_type();
      recurse_type_metadata(S, rd, type);

      return type;
    }
  }
}
