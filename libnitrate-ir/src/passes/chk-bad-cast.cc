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

#include <nitrate-ir/IRGraph.hh>
#include <passes/PassList.hh>

using namespace qxir;
using namespace qxir::diag;

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

static bool verify_cast_as(qmodule_t* M, Expr* N, Type* L, Type* R) {
  if (L->cmp_eq(R)) {
    return true;
  }

  const auto prepare = [](std::string msg, Type* L, Type* R) -> std::string {
    return msg + ": " + L->getKindName() + " -> " + R->getKindName() + ".";
  };

  /* Recursively check struct field casting */
  if (L->getKind() == QIR_NODE_STRUCT_TY && R->getKind() == QIR_NODE_STRUCT_TY) {
    StructTy *LS = L->as<StructTy>(), *RS = R->as<StructTy>();

    if (LS->getFields().size() != RS->getFields().size()) {
      report(IssueCode::BadCast, IssueClass::Error,
             prepare("`as` expected both struct types to have the same number of fields", L, R),
             N->locBeg(), N->locEnd());
      M->setFailbit(true);

      return false;
    }

    for (size_t i = 0; i < LS->getFields().size(); i++) {
      Type *LT = LS->getFields()[i], *RT = RS->getFields()[i];

      if (!verify_cast_as(M, N, LT, RT)) {
        report(IssueCode::BadCast, IssueClass::Error,
               prepare("Field cast failed in recursive struct cast", L, R), N->locBeg(),
               N->locEnd());
        M->setFailbit(true);
        return false;
      }
    }

    return true;
  }
  if (L->getKind() == QIR_NODE_ARRAY_TY && R->getKind() == QIR_NODE_STRUCT_TY) {
    ArrayTy* LS = L->as<ArrayTy>();
    StructTy* RS = R->as<StructTy>();

    if (LS->getCount() != RS->getFields().size()) {
      report(IssueCode::BadCast, IssueClass::Error,
             prepare("`as` expected both types to have the same number of entries", L, R),
             N->locBeg(), N->locEnd());
      M->setFailbit(true);

      return false;
    }

    for (size_t i = 0; i < LS->getCount(); i++) {
      Type* RT = RS->getFields()[i];

      if (!verify_cast_as(M, N, LS->getElement(), RT)) {
        report(IssueCode::BadCast, IssueClass::Error,
               prepare("Field cast failed in recursive array to struct cast", L, R), N->locBeg(),
               N->locEnd());
        M->setFailbit(true);
        return false;
      }
    }

    return true;
  } else if (L->getKind() == QIR_NODE_STRUCT_TY || R->getKind() == QIR_NODE_STRUCT_TY) {
    report(IssueCode::BadCast, IssueClass::Error, prepare("Bad cast to/from struct type", L, R),
           N->locBeg(), N->locEnd());
    M->setFailbit(true);
    return false;
  }

  /* Opaque types cannot be casted to anything */
  if (L->getKind() == QIR_NODE_OPAQUE_TY || R->getKind() == QIR_NODE_OPAQUE_TY) {
    report(IssueCode::BadCast, IssueClass::Error,
           prepare("Illegal direct cast of opaque in `as`", L, R), N->locBeg(), N->locEnd());
    M->setFailbit(true);
    return false;
  }

  if (!L->is_void() && R->is_void()) {
    report(IssueCode::BadCast, IssueClass::Error,
           prepare("Bad cast from non-void type into void type", L, R), N->locBeg(), N->locEnd());
    M->setFailbit(true);
    return false;
  }

  /* `cast_as` does not support pointer casts */
  if (L->is_pointer() || R->is_pointer()) {
    report(IssueCode::BadCast, IssueClass::Error,
           prepare("`as` prohibits pointer type casts", L, R), N->locBeg(), N->locEnd());
    M->setFailbit(true);
    return false;
  }

  if (L->getKind() == QIR_NODE_UNION_TY || R->getKind() == QIR_NODE_UNION_TY) {
    report(IssueCode::BadCast, IssueClass::Error, prepare("`as` prohibits union type casts", L, R),
           N->locBeg(), N->locEnd());
    M->setFailbit(true);
    return false;
  }

  if (L->getKind() == QIR_NODE_ARRAY_TY || R->getKind() == QIR_NODE_ARRAY_TY) {
    report(IssueCode::BadCast, IssueClass::Error, prepare("`as` prohibits array type casts", L, R),
           N->locBeg(), N->locEnd());
    M->setFailbit(true);
    return false;
  }

  if (L->getKind() == QIR_NODE_FN_TY || R->getKind() == QIR_NODE_FN_TY) {
    report(IssueCode::BadCast, IssueClass::Error,
           prepare("`as` prohibits function type casts", L, R), N->locBeg(), N->locEnd());
    M->setFailbit(true);
    return false;
  }

  /* All number <=> number casts are legal */
  if (L->is_numeric() && R->is_numeric()) {
    return true;
  } else if (L->is_numeric() || R->is_numeric()) {
    report(IssueCode::BadCast, IssueClass::Error,
           prepare("Casting between numeric and non-numeric types is not allowed in `as`", L, R),
           N->locBeg(), N->locEnd());
    M->setFailbit(true);
    return false;
  }

  report(IssueCode::BadCast, IssueClass::Error, prepare("Invalid cast in `as` expression", L, R),
         N->locBeg(), N->locEnd());
  M->setFailbit(true);

  return false;
}

bool qxir::pass::chk_bad_cast(qmodule_t* M) {
  /**
   * Perform validation checks on `cast_as` expressions to ensure that the cast is indeed
   * accecptable.
   */

  iterate<dfs_pre>(M->getRoot(), [&](Expr* /* parent */, Expr** C /* current */) -> IterOp {
    Expr* N = *C;

    /**
     * If the node is not a binary expression OR the binary expression operator is not a cast_as,
     * the node is not subject to scrutiny by this analysis pass.
     */
    if (N->getKind() != QIR_NODE_BINEXPR || N->as<BinExpr>()->getOp() != Op::CastAs) {
      return IterOp::Proceed;
    }

    BinExpr* BE = N->as<BinExpr>();
    Type *L = BE->getLHS()->getType().value_or(nullptr), *R = BE->getRHS()->asType();

    /* No inference was possible on the left-hand side of the cast, therefore no cast is possible */
    if (L == nullptr) {
      return IterOp::Proceed;
    }

    /* Casting to the same type is always legal */
    (void)verify_cast_as(M, N, L, R);

    return IterOp::Proceed;
  });

  return true;
}
