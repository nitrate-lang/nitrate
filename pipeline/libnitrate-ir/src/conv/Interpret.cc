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

#include <core/LibMacro.h>
#include <nitrate-ir/IR.h>

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <nitrate-ir/IRGraph.hh>

using namespace nr;

#define intern(__str) x->getModule()->internString(__str)

nr::Expr *nr::evaluate_to_literal(nr::Expr *x) noexcept {
  Expr *ANS = nullptr;

  nr::current = x->getModule();

  // switch (x->getKind()) {
  //   case QIR_NODE_BINEXPR: {
  //     Op op = x->as<BinExpr>()->getOp();
  //     Expr *L = evaluate_to_literal(x->as<BinExpr>()->getLHS());
  //     if (!L) {
  //       break;
  //     }
  //     Expr *R = evaluate_to_literal(x->as<BinExpr>()->getRHS());
  //     if (!R) {
  //       break;
  //     }

  //     switch (op) {
  //       case Op::Plus: {
  //         /// TODO:
  //         break;
  //       }

  //       case Op::Minus: {
  //         /// TODO:
  //         break;
  //       }

  //       case Op::Times: {
  //         /// TODO:
  //         break;
  //       }

  //       case Op::Slash: {
  //         /// TODO:
  //         break;
  //       }

  //       case Op::Percent: {
  //         /// TODO:
  //         break;
  //       }

  //       case Op::BitAnd: {
  //         /// TODO:
  //         break;
  //       }

  //       case Op::BitOr: {
  //         /// TODO:
  //         break;
  //       }

  //       case Op::BitXor: {
  //         /// TODO:
  //         break;
  //       }

  //       case Op::LogicAnd: {
  //         /// TODO:
  //         break;
  //       }

  //       case Op::LogicOr: {
  //         /// TODO:
  //         break;
  //       }

  //       case Op::LShift: {
  //         if (L->getKind() != QIR_NODE_INT || R->getKind() != QIR_NODE_INT) {
  //           break;
  //         }

  //         uint64_t rhs = 0;

  //         {  // Only works if rhs is native
  //           boost::multiprecision::cpp_int tmp(R->as<Int>()->getValue());

  //           if (tmp < UINT64_MAX) {
  //             rhs = tmp.convert_to<uint64_t>();
  //           } else {
  //             break;
  //           }
  //         }

  //         boost::multiprecision::cpp_int lhs(L->as<Int>()->getValue());
  //         boost::multiprecision::cpp_int RES = lhs << rhs;
  //         /// TODO: Mask the result
  //         ANS = create<Int>(intern(RES.str()));
  //         break;
  //       }

  //       case Op::RShift: {
  //         if (L->getKind() != QIR_NODE_INT || R->getKind() != QIR_NODE_INT) {
  //           break;
  //         }

  //         uint64_t rhs = 0;

  //         {  // Only works if rhs is native
  //           boost::multiprecision::cpp_int tmp(R->as<Int>()->getValue());

  //           if (tmp < UINT64_MAX) {
  //             rhs = tmp.convert_to<uint64_t>();
  //           } else {
  //             break;
  //           }
  //         }

  //         boost::multiprecision::cpp_int lhs(L->as<Int>()->getValue());
  //         boost::multiprecision::cpp_int RES = lhs >> rhs;
  //         /// TODO: Mask the result
  //         ANS = create<Int>(intern(RES.str()));
  //         break;
  //       }

  //       case Op::ROTR: {
  //         if (L->getKind() != QIR_NODE_INT || R->getKind() != QIR_NODE_INT) {
  //           break;
  //         }

  //         uint64_t rhs = 0;

  //         {  // Only works if rhs is native
  //           boost::multiprecision::cpp_int tmp(R->as<Int>()->getValue());

  //           if (tmp < UINT64_MAX) {
  //             rhs = tmp.convert_to<uint64_t>();
  //           } else {
  //             break;
  //           }
  //         }

  //         Type *T = L->getType().value_or(nullptr);
  //         if (!T) {
  //           break;
  //         }
  //         size_t w = T->getSizeBits();
  //         boost::multiprecision::cpp_int lhs(L->as<Int>()->getValue());
  //         boost::multiprecision::cpp_int RES = (lhs >> rhs) | (lhs << (w - rhs));
  //         /// TODO: Mask the result?
  //         ANS = create<Int>(intern(RES.str()));
  //         break;
  //       }

  //       case Op::ROTL: {
  //         if (L->getKind() != QIR_NODE_INT || R->getKind() != QIR_NODE_INT) {
  //           break;
  //         }

  //         uint64_t rhs = 0;

  //         {  // Only works if rhs is native
  //           boost::multiprecision::cpp_int tmp(R->as<Int>()->getValue());

  //           if (tmp < UINT64_MAX) {
  //             rhs = tmp.convert_to<uint64_t>();
  //           } else {
  //             break;
  //           }
  //         }

  //         Type *T = L->getType().value_or(nullptr);
  //         if (!T) {
  //           break;
  //         }
  //         size_t w = T->getSizeBits();
  //         boost::multiprecision::cpp_int lhs(L->as<Int>()->getValue());
  //         boost::multiprecision::cpp_int RES = (lhs << rhs) | (lhs >> (w - rhs));
  //         /// TODO: Mask the result?
  //         ANS = create<Int>(intern(RES.str()));
  //         break;
  //       }

  //       case Op::LT: {
  //         boost::multiprecision::cpp_dec_float<128> lhs, rhs;

  //         if (L->getKind() == QIR_NODE_INT) {
  //           lhs = static_cast<unsigned __int128>(L->as<Int>()->getValue());
  //         } else if (L->getKind() == QIR_NODE_FLOAT) {
  //           lhs = L->as<Float>()->getValue();
  //         } else {
  //           break;
  //         }

  //         if (R->getKind() == QIR_NODE_INT) {
  //           rhs = static_cast<unsigned __int128>(L->as<Int>()->getValue());
  //         } else if (R->getKind() == QIR_NODE_FLOAT) {
  //           rhs = R->as<Float>()->getValue().c_str();
  //         } else {
  //           break;
  //         }

  //         ANS = create<Int>(lhs.compare(rhs) < 0);
  //         break;
  //       }

  //       case Op::GT: {
  //         boost::multiprecision::cpp_dec_float<128> lhs, rhs;

  //         if (L->getKind() == QIR_NODE_INT) {
  //           lhs = static_cast<unsigned __int128>(L->as<Int>()->getValue());
  //         } else if (L->getKind() == QIR_NODE_FLOAT) {
  //           lhs = L->as<Float>()->getValue().c_str();
  //         } else {
  //           break;
  //         }

  //         if (R->getKind() == QIR_NODE_INT) {
  //           rhs = R->as<Int>()->getValue().c_str();
  //         } else if (R->getKind() == QIR_NODE_FLOAT) {
  //           rhs = R->as<Float>()->getValue().c_str();
  //         } else {
  //           break;
  //         }

  //         ANS = create<Int>(lhs.compare(rhs) > 0);
  //         break;
  //       }

  //       case Op::LE: {
  //         boost::multiprecision::cpp_dec_float<128> lhs, rhs;

  //         if (L->getKind() == QIR_NODE_INT) {
  //           lhs = static_cast<unsigned __int128>(L->as<Int>()->getValue());
  //         } else if (L->getKind() == QIR_NODE_FLOAT) {
  //           lhs = L->as<Float>()->getValue().c_str();
  //         } else {
  //           break;
  //         }

  //         if (R->getKind() == QIR_NODE_INT) {
  //           rhs = R->as<Int>()->getValue().c_str();
  //         } else if (R->getKind() == QIR_NODE_FLOAT) {
  //           rhs = R->as<Float>()->getValue().c_str();
  //         } else {
  //           break;
  //         }

  //         ANS = create<Int>(lhs.compare(rhs) <= 0);
  //         break;
  //       }

  //       case Op::GE: {
  //         boost::multiprecision::cpp_dec_float<128> lhs, rhs;

  //         if (L->getKind() == QIR_NODE_INT) {
  //           lhs = static_cast<unsigned __int128>(L->as<Int>()->getValue());
  //         } else if (L->getKind() == QIR_NODE_FLOAT) {
  //           lhs = L->as<Float>()->getValue().c_str();
  //         } else {
  //           break;
  //         }

  //         if (R->getKind() == QIR_NODE_INT) {
  //           rhs = R->as<Int>()->getValue().c_str();
  //         } else if (R->getKind() == QIR_NODE_FLOAT) {
  //           rhs = R->as<Float>()->getValue().c_str();
  //         } else {
  //           break;
  //         }

  //         ANS = create<Int>(lhs.compare(rhs) >= 0);
  //         break;
  //       }

  //       case Op::Eq: {
  //         boost::multiprecision::cpp_dec_float<128> lhs, rhs;

  //         if (L->getKind() == QIR_NODE_INT) {
  //           lhs = static_cast<unsigned __int128>(L->as<Int>()->getValue());
  //         } else if (L->getKind() == QIR_NODE_FLOAT) {
  //           lhs = L->as<Float>()->getValue().c_str();
  //         } else {
  //           break;
  //         }

  //         if (R->getKind() == QIR_NODE_INT) {
  //           rhs = R->as<Int>()->getValue().c_str();
  //         } else if (R->getKind() == QIR_NODE_FLOAT) {
  //           rhs = R->as<Float>()->getValue().c_str();
  //         } else {
  //           break;
  //         }

  //         ANS = create<Int>(lhs.compare(rhs) == 0);
  //         break;
  //       }

  //       case Op::NE: {
  //         boost::multiprecision::cpp_dec_float<128> lhs, rhs;

  //         if (L->getKind() == QIR_NODE_INT) {
  //           lhs = static_cast<unsigned __int128>(L->as<Int>()->getValue());
  //         } else if (L->getKind() == QIR_NODE_FLOAT) {
  //           lhs = L->as<Float>()->getValue().c_str();
  //         } else {
  //           break;
  //         }

  //         if (R->getKind() == QIR_NODE_INT) {
  //           rhs = R->as<Int>()->getValue().c_str();
  //         } else if (R->getKind() == QIR_NODE_FLOAT) {
  //           rhs = R->as<Float>()->getValue().c_str();
  //         } else {
  //           break;
  //         }

  //         ANS = create<Int>(lhs.compare(rhs) != 0);
  //         break;
  //       }

  //       case Op::BitcastAs: {
  //         /// TODO:
  //         break;
  //       }

  //       case Op::CastAs: {
  //         /// TODO:
  //         break;
  //       }

  //       default: {
  //         qcore_panic("Unknown binary operator in expression evaluation");
  //         break;
  //       }
  //     }
  //     break;
  //   }

  //   case QIR_NODE_UNEXPR: {
  //     Op op = x->as<UnExpr>()->getOp();
  //     Expr *E = evaluate_to_literal(x->as<UnExpr>()->getExpr());
  //     if (!E) {
  //       break;
  //     }

  //     switch (op) {
  //       case Op::Plus: {
  //         if (E->getKind() != QIR_NODE_INT && E->getKind() != QIR_NODE_FLOAT) {
  //           break;
  //         }

  //         ANS = E;
  //         break;
  //       }

  //       case Op::Minus: {
  //         if (E->getKind() != QIR_NODE_INT && E->getKind() != QIR_NODE_FLOAT) {
  //           break;
  //         }
  //         // Unsupported.

  //         break;
  //       }

  //       case Op::BitNot: {
  //         boost::multiprecision::cpp_int val;

  //         if (E->getKind() == QIR_NODE_INT) {
  //           val = ~boost::multiprecision::cpp_int(E->as<Int>()->getValue());

  //           /// TODO: Mask the result to the correct size.
  //         } else {
  //           break;
  //         }

  //         ANS = create<Int>(intern(val.str()));
  //         break;
  //       }

  //       case Op::LogicNot: {
  //         if (E->getKind() == QIR_NODE_FLOAT) {
  //           ANS = create<Int>(E->as<Float>()->getValue() == "0");
  //           break;
  //         }

  //         boost::multiprecision::cpp_int val;

  //         if (E->getKind() == QIR_NODE_INT) {
  //           val = !boost::multiprecision::cpp_int(E->as<Int>()->getValue());
  //         } else {
  //           break;
  //         }

  //         ANS = create<Int>(intern(val.str()));
  //         break;
  //       }

  //       case Op::Alignof: {
  //         Type *T = E->getType().value_or(nullptr);
  //         if (!T) {
  //           break;
  //         }
  //         ANS = create<Int>(T->getAlignBytes());
  //         break;
  //       }

  //       case Op::Bitsizeof: {
  //         Type *T = E->getType().value_or(nullptr);
  //         if (!T) {
  //           break;
  //         }
  //         ANS = create<Int>(T->getSizeBits());
  //         break;
  //       }
  //       default: {
  //         qcore_panic("Unknown unary operator in expression evaluation");
  //         break;
  //       }
  //     }
  //     break;
  //   }

  //   case QIR_NODE_INT: {
  //     ANS = x;
  //     break;
  //   }

  //   case QIR_NODE_FLOAT: {
  //     ANS = x;
  //     break;
  //   }

  //   case QIR_NODE_LIST: {
  //     ANS = x;
  //     break;
  //   }

  //   case QIR_NODE_CALL: {
  //     // Unsupported.
  //     break;
  //   }

  //   case QIR_NODE_SEQ: {
  //     Seq *seq = x->as<Seq>();
  //     if (seq->getItems().empty()) {
  //       return nullptr;
  //     }

  //     ANS = evaluate_to_literal(seq->getItems().back());
  //     break;
  //   }

  //   case QIR_NODE_INDEX: {
  //     Index *index = x->as<Index>();

  //     Expr *base = evaluate_to_literal(index->getExpr());
  //     if (!base) {
  //       break;
  //     }

  //     Expr *idx = evaluate_to_literal(index->getIndex());
  //     if (!idx) {
  //       break;
  //     }

  //     auto base_kind = base->getKind();
  //     if (base_kind != QIR_NODE_LIST) {
  //       break;
  //     }

  //     auto idx_kind = idx->getKind();
  //     if (idx_kind != QIR_NODE_INT) {
  //       break;
  //     }

  //     auto idx_val_str = idx->as<Int>()->getValue();
  //     uint64_t idx_val = 0;
  //     try {
  //       idx_val = std::stoull(idx_val_str);
  //     } catch (...) {
  //       break;
  //     }

  //     if (base_kind == QIR_NODE_LIST) {
  //       List *list = base->as<List>();
  //       if (idx_val >= list->getItems().size()) {
  //         break;
  //       }

  //       ANS = evaluate_to_literal(list->getItems()[idx_val]);
  //     }

  //     break;
  //   }

  //   case QIR_NODE_TMP: {
  //     qcore_panicf("Unexpected temporary node in expression evaluation: %s", x->getKindName());
  //   }

  //   default: {
  //     qcore_panicf("Unknown node kind in expression evaluation: %s", x->getKindName());
  //     break;
  //   }
  // }

  return ANS;
}
