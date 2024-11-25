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

#include <nitrate-ir/IRGraph.hh>
#include <passes/PassList.hh>

using namespace nr;

/**
 * -- Overview of the IR type nodes:
 * +========================================================+
 * | QIR_NODE_U1_TY     => 1-bit unsigned integer (boolean) |
 * | QIR_NODE_U8_TY     => 8-bit unsigned integer           |
 * | QIR_NODE_U16_TY    => 16-bit unsigned integer          |
 * | QIR_NODE_U32_TY    => 32-bit unsigned integer          |
 * | QIR_NODE_U64_TY    => 64-bit unsigned integer          |
 * | QIR_NODE_U128_TY   => 128-bit unsigned integer         |
 * | QIR_NODE_I8_TY     => 8-bit signed integer             |
 * | QIR_NODE_I16_TY    => 16-bit signed integer            |
 * | QIR_NODE_I32_TY    => 32-bit signed integer            |
 * | QIR_NODE_I64_TY    => 64-bit signed integer            |
 * | QIR_NODE_I128_TY   => 128-bit signed integer           |
 * | QIR_NODE_F16_TY    => 16-bit floating-point            |
 * | QIR_NODE_F32_TY    => 32-bit floating-point            |
 * | QIR_NODE_F64_TY    => 64-bit floating-point            |
 * | QIR_NODE_F128_TY   => 128-bit floating-point           |
 * | QIR_NODE_VOID_TY   => Void type                        |
 * | QIR_NODE_PTR_TY    => Pointer type                     |
 * | QIR_NODE_OPAQUE_TY => Opaque type                      |
 * | QIR_NODE_STRUCT_TY => Struct type                      |
 * | QIR_NODE_UNION_TY  => Union type                       |
 * | QIR_NODE_ARRAY_TY  => Array type                       |
 * | QIR_NODE_FN_TY     => Function type                    |
 * +========================================================+
 */

static bool verify_cast_as(qmodule_t* M, IReport* log, Expr* N, Type* L,
                           Type* R) {
  static constexpr std::array<std::string_view, 13> texts = {
      "`as` expected both struct types to have the same number of fields",
      "Field cast failed in recursive struct cast",
      "`as` expected both types to have the same number of entries",
      "Field cast failed in recursive array to struct cast",
      "Bad cast to/from struct type",
      "Illegal direct cast of opaque in `as`",
      "Bad cast from non-void type into void type",
      "`as` prohibits pointer type casts",
      "`as` prohibits union type casts",
      "`as` prohibits array type casts",
      "`as` prohibits function type casts",
      "Casting between numeric and non-numeric types is not allowed in `as`",
      "Invalid cast in `as` expression",
  };

  if (L->isSame(R)) {
    return true;
  }

  const auto prepare = [](std::string_view msg, Type* L,
                          Type* R) -> std::string {
    return std::string(msg) + ": " + std::string(L->getKindName()) + " -> " +
           std::string(R->getKindName()) + ".";
  };

  /* Recursively check struct field casting */
  if (L->getKind() == QIR_NODE_STRUCT_TY &&
      R->getKind() == QIR_NODE_STRUCT_TY) {
    StructTy *LS = L->as<StructTy>(), *RS = R->as<StructTy>();

    if (LS->getFields().size() != RS->getFields().size()) {
      log->report(BadCast, IC::Error, prepare(texts[0], L, R), N->getLoc());

      return false;
    }

    for (size_t i = 0; i < LS->getFields().size(); i++) {
      Type *LT = LS->getFields()[i], *RT = RS->getFields()[i];

      if (!verify_cast_as(M, log, N, LT, RT)) {
        log->report(BadCast, IC::Error, prepare(texts[1], L, R), N->getLoc());

        return false;
      }
    }

    return true;
  }
  if (L->getKind() == QIR_NODE_ARRAY_TY && R->getKind() == QIR_NODE_STRUCT_TY) {
    ArrayTy* LS = L->as<ArrayTy>();
    StructTy* RS = R->as<StructTy>();

    if (LS->getCount() != RS->getFields().size()) {
      log->report(BadCast, IC::Error, prepare(texts[2], L, R), N->getLoc());

      return false;
    }

    for (size_t i = 0; i < LS->getCount(); i++) {
      Type* RT = RS->getFields()[i];

      if (!verify_cast_as(M, log, N, LS->getElement(), RT)) {
        log->report(BadCast, IC::Error, prepare(texts[3], L, R), N->getLoc());

        return false;
      }
    }

    return true;
  } else if (L->getKind() == QIR_NODE_STRUCT_TY ||
             R->getKind() == QIR_NODE_STRUCT_TY) {
    log->report(BadCast, IC::Error, prepare(texts[4], L, R), N->getLoc());

    return false;
  }

  /* Opaque types cannot be casted to anything */
  if (L->getKind() == QIR_NODE_OPAQUE_TY ||
      R->getKind() == QIR_NODE_OPAQUE_TY) {
    log->report(BadCast, IC::Error, prepare(texts[5], L, R), N->getLoc());

    return false;
  }

  if (!L->is_void() && R->is_void()) {
    log->report(BadCast, IC::Error, prepare(texts[6], L, R), N->getLoc());

    return false;
  }

  /* `cast_as` does not support pointer casts */
  if (L->is_pointer() || R->is_pointer()) {
    log->report(BadCast, IC::Error, prepare(texts[7], L, R), N->getLoc());

    return false;
  }

  if (L->getKind() == QIR_NODE_UNION_TY || R->getKind() == QIR_NODE_UNION_TY) {
    log->report(BadCast, IC::Error, prepare(texts[8], L, R), N->getLoc());

    return false;
  }

  if (L->getKind() == QIR_NODE_ARRAY_TY || R->getKind() == QIR_NODE_ARRAY_TY) {
    log->report(BadCast, IC::Error, prepare(texts[9], L, R), N->getLoc());

    return false;
  }

  if (L->getKind() == QIR_NODE_FN_TY || R->getKind() == QIR_NODE_FN_TY) {
    log->report(BadCast, IC::Error, prepare(texts[10], L, R), N->getLoc());

    return false;
  }

  /* All number <=> number casts are legal */
  if (L->is_numeric() && R->is_numeric()) {
    return true;
  } else if (L->is_numeric() || R->is_numeric()) {
    log->report(BadCast, IC::Error, prepare(texts[11], L, R), N->getLoc());

    return false;
  }

  log->report(BadCast, IC::Error, prepare(texts[12], L, R), N->getLoc());

  return false;
}

bool nr::pass::chk_bad_cast(qmodule_t* M, IReport* log) {
  /**
   * Perform validation checks on `cast_as` expressions to ensure that the cast
   * is indeed accecptable.
   */

  iterate<dfs_pre>(M->getRoot(),
                   [&](Expr* /* parent */, Expr** C /* current */) -> IterOp {
                     Expr* N = *C;

                     /**
                      * If the node is not a binary expression OR the binary
                      * expression operator is not a cast_as, the node is not
                      * subject to scrutiny by this analysis pass.
                      */
                     if (N->getKind() != QIR_NODE_BINEXPR ||
                         N->as<BinExpr>()->getOp() != Op::CastAs) {
                       return IterOp::Proceed;
                     }

                     BinExpr* BE = N->as<BinExpr>();
                     Type *L = BE->getLHS()->getType().value_or(nullptr),
                          *R = BE->getRHS()->asType();

                     /* No inference was possible on the left-hand side of the
                      * cast, therefore no cast is possible */
                     if (L == nullptr) {
                       return IterOp::Proceed;
                     }

                     /* Casting to the same type is always legal */
                     (void)verify_cast_as(M, log, N, L, R);

                     return IterOp::Proceed;
                   });

  return true;
}
