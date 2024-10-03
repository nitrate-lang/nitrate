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

#include <sstream>

#include "quix-qxir/TypeDecl.h"
#define QXIR_USE_CPP_API
#include <quix-qxir/Inference.h>
#include <quix-qxir/Node.h>

#include <boost/bimap.hpp>
#include <transform/passes/Decl.hh>

/**
 * @brief Canonicalize the names of things in the module.
 *
 * @timecomplexity O(n)
 * @spacecomplexity O(n)
 * @reason: If every node must be mangled, the memory consumption is
 * proportional to the number of nodes. In practice, its quite small and bounded by language
 * semantics.
 */

using namespace qxir;

static void encode_ns_size_value(std::string_view input, std::ostream &ss) {
  int state = 0;
  std::string buf;

  for (size_t i = 0; i < input.size(); i++) {
    switch (state) {
      case 0: {
        if (input[i] == ':') {
          state = 1;
        } else {
          buf.push_back(input[i]);
        }
        break;
      }
      case 1: {
        if (input[i] != ':') {
          ss << buf.size() << buf;
          buf.clear();
          buf.push_back(input[i]);
          state = 0;
        }
        break;
      }
    }
  }

  if (!buf.empty()) {
    ss << buf.size() << buf;
  }
}

static void mangle_type(Type *n, std::ostream &ss) {
  /**
   * @brief Name mangling for QUIX is inspired by the Itanium C++ ABI.
   * @ref https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling
   *
   *  <builtin-type> ::= v	# void
   *                 ::= w	# wchar_t
   *                 ::= b	# bool
   *                 ::= c	# char
   *                 ::= a	# signed char
   *                 ::= h	# unsigned char
   *                 ::= s	# short
   *                 ::= t	# unsigned short
   *                 ::= i	# int
   *                 ::= j	# unsigned int
   *                 ::= l	# long
   *                 ::= m	# unsigned long
   *                 ::= x	# long long, __int64
   *                 ::= y	# unsigned long long, __int64
   *                 ::= n	# __int128
   *                 ::= o	# unsigned __int128
   *                 ::= f	# float
   *                 ::= d	# double
   *                 ::= e	# long double, __float80
   *                 ::= g	# __float128
   *                 ::= z	# ellipsis
   *                 ::= Dd # IEEE 754r decimal floating point (64 bits)
   *                 ::= De # IEEE 754r decimal floating point (128 bits)
   *                 ::= Df # IEEE 754r decimal floating point (32 bits)
   *                 ::= Dh # IEEE 754r half-precision floating point (16 bits)
   *                 ::= DF <number> _ # ISO/IEC TS 18661 binary floating point type _FloatN (N
   * bits), C++23 std::floatN_t
   *                 ::= DF <number> x # IEEE extended precision formats, C23 _FloatNx (N bits)
   *                 ::= DF16b # C++23 std::bfloat16_t
   *                 ::= DB <number> _        # C23 signed _BitInt(N)
   *                 ::= DB <instantiation-dependent expression> _ # C23 signed _BitInt(N)
   *                 ::= DU <number> _        # C23 unsigned _BitInt(N)
   *                 ::= DU <instantiation-dependent expression> _ # C23 unsigned _BitInt(N)
   *                 ::= Di # char32_t
   *                 ::= Ds # char16_t
   *                 ::= Du # char8_t
   *                 ::= Da # auto
   *                 ::= Dc # decltype(auto)
   *                 ::= Dn # std::nullptr_t (i.e., decltype(nullptr))
   *                 ::= [DS] DA  # N1169 fixed-point [_Sat] T _Accum
   *                 ::= [DS] DR  # N1169 fixed-point [_Sat] T _Fract
   *                 ::= u <source-name> [<template-args>] # vendor extended type
   *
   *  <fixed-point-size>
   *                 ::= s # short
   *                 ::= t # unsigned short
   *                 ::= i # plain
   *                 ::= j # unsigned
   *                 ::= l # long
   *                 ::= m # unsigned long
   */

  switch (n->getKind()) {
    case QIR_NODE_U1_TY: {
      ss << 'b';
      break;
    }

    case QIR_NODE_U8_TY: {
      ss << 'h';
      break;
    }

    case QIR_NODE_U16_TY: {
      ss << 't';
      break;
    }

    case QIR_NODE_U32_TY: {
      ss << 'j';
      break;
    }

    case QIR_NODE_U64_TY: {
      ss << 'm';
      break;
    }

    case QIR_NODE_U128_TY: {
      ss << 'o';
      break;
    }

    case QIR_NODE_I8_TY: {
      ss << 'a';
      break;
    }

    case QIR_NODE_I16_TY: {
      ss << 's';
      break;
    }

    case QIR_NODE_I32_TY: {
      ss << 'i';
      break;
    }

    case QIR_NODE_I64_TY: {
      ss << 'l';
      break;
    }

    case QIR_NODE_I128_TY: {
      ss << 'n';
      break;
    }

    case QIR_NODE_F16_TY: {
      ss << "Dh";
      break;
    }

    case QIR_NODE_F32_TY: {
      ss << "Df";
      break;
    }

    case QIR_NODE_F64_TY: {
      ss << "Dd";
      break;
    }

    case QIR_NODE_F128_TY: {
      ss << "De";
      break;
    }

    case QIR_NODE_VOID_TY: {
      ss << 'v';
      break;
    }

    case QIR_NODE_PTR_TY: {
      ss << 'P';
      mangle_type(n->as<PtrTy>()->getPointee(), ss);
      break;
    }

    case QIR_NODE_OPAQUE_TY: {
      ss << 'N';
      encode_ns_size_value(n->as<OpaqueTy>()->getName(), ss);
      ss << 'E';
      break;
    }

    case QIR_NODE_STRING_TY: {
      ss << 'S';
      break;
    }

    case QIR_NODE_STRUCT_TY: {
      /**
       * @brief Unlike C++, QUIX encodes field types into the name.
       * Making any changes to a struct will break ABI compatibility
       * at link time avoiding runtime UB.
       */

      ss << 'c';
      for (auto *field : n->as<StructTy>()->getFields()) {
        mangle_type(field, ss);
      }
      ss << 'E';
      break;
    }

    case QIR_NODE_UNION_TY: {
      /**
       * @brief Unlike C++, QUIX encodes field types into the name.
       * Making any changes to a union will break ABI compatibility
       * at link time avoiding runtime UB.
       */

      ss << 'u';
      for (auto *field : n->as<StructTy>()->getFields()) {
        mangle_type(field, ss);
      }
      ss << 'E';
      break;
    }

    case QIR_NODE_ARRAY_TY: {
      ss << 'A';
      ss << n->as<ArrayTy>()->getCount();
      ss << '_';
      mangle_type(n->as<ArrayTy>()->getElement(), ss);
      break;
    }

    case QIR_NODE_FN_TY: {
      /**
       * @brief Unlike C++, QUIX also encodes the parameter types
       * into the name. This is to avoid runtime UB when calling
       * functions with the wrong number of arguments.
       * These bugs will be caught at link time.
       */

      ss << 'F';
      auto *fn = n->as<FnTy>();
      mangle_type(fn->getReturn(), ss);
      for (auto *param : fn->getParams()) {
        mangle_type(param, ss);
      }
      if (fn->getAttrs().count(FnAttr::Variadic)) {
        ss << '_';
      }
      ss << 'E';
      break;
    }

    default: {
      qcore_panicf("Unknown type kind: %d", (int)n->getKind());
    }
  }
}

static void mangle_function(Fn *n) {
  switch (n->getAbiTag()) {
    case qxir::AbiTag::C: {
      std::string s = std::string(n->getName());
      std::replace(s.begin(), s.end(), ':', '_');
      n->setName(n->getModule()->internString(s));
      break;
    }
    case qxir::AbiTag::Internal:
    case qxir::AbiTag::QUIX: {
      std::stringstream ss;
      ss << "_Q";

      encode_ns_size_value(n->getName(), ss);
      mangle_type(n->getType(), ss);

      ss << "_0";

      n->setName(n->getModule()->internString(ss.str()));
      break;
    }
  }
}

static void mangle_local(Local *n) {
  switch (n->getAbiTag()) {
    case qxir::AbiTag::C: {
      std::string s = std::string(n->getName());
      std::replace(s.begin(), s.end(), ':', '_');
      n->setName(n->getModule()->internString(s));
      break;
    }
    case qxir::AbiTag::Internal:
    case qxir::AbiTag::QUIX: {
      std::stringstream ss;
      ss << "_Q";

      encode_ns_size_value(n->getName(), ss);
      mangle_type(n->getType(), ss);

      ss << "_0";

      n->setName(n->getModule()->internString(ss.str()));
      break;
    }
  }
}

bool qxir::passes::impl::nm_premangle(qmodule_t *mod) {
  iterate<dfs_pre, IterMP::none>(mod->getRoot(), [](Expr *, Expr **cur) -> IterOp {
    if ((*cur)->getKind() == QIR_NODE_FN) {
      mangle_function((*cur)->as<Fn>());
    } else if ((*cur)->getKind() == QIR_NODE_LOCAL) {
      mangle_local((*cur)->as<Local>());
    }

    return IterOp::Proceed;
  });

  /* Update identifiers to use the new names */
  iterate<dfs_pre, IterMP::none>(mod->getRoot(), [](Expr *, Expr **cur) -> IterOp {
    if ((*cur)->getKind() != QIR_NODE_IDENT) {
      return IterOp::Proceed;
    }

    Ident *ident = (*cur)->as<Ident>();
    if (!ident->getWhat()) {
      return IterOp::Proceed;
    }

    ident->setName(ident->getWhat()->getName());

    return IterOp::Proceed;
  });

  return true;
}
