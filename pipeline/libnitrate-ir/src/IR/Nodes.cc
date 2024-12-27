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

#define __IR_NODE_REFLECT_IMPL__  // Make private fields accessible

#include <openssl/sha.h>

#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstring>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/IR.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/Module.hh>

using namespace ncc;
using namespace ncc::ir;

///=============================================================================
namespace ncc::ir {
  thread_local std::unique_ptr<ncc::IMemory> nr_allocator =
      std::make_unique<ncc::dyn_arena>();

  namespace mem {
    Brk static_IR_eBRK;
    Cont static_IR_eSKIP;
    Expr static_IR_eIGN(IR_eIGN);

  }  // namespace mem
}  // namespace ncc::ir

// ///=============================================================================

// static bool isCyclicUtil(const Expr *const base,
//                          std::unordered_set<const Expr *> &visited,
//                          std::unordered_set<const Expr *> &recStack) {
//   bool has_cycle = false;

//   if (!visited.contains(base)) {
//     // Mark the current node as visited
//     // and part of recursion stack
//     visited.insert(base);
//     recStack.insert(base);

//     // Recurse for all the vertices adjacent
//     // to this vertex
//     iterate<IterMode::children>(
//         base, [&](const Expr *, const Expr *const *const cur) -> IterOp {
//           if (!visited.contains(*cur) && isCyclicUtil(*cur, visited,
//           recStack))
//               [[unlikely]] {
//             has_cycle = true;
//             return IterOp::Abort;
//           } else if (recStack.contains(*cur)) [[unlikely]] {
//             has_cycle = true;
//             return IterOp::Abort;
//           }

//           return IterOp::Proceed;
//         });
//   }

//   // Remove the vertex from recursion stack
//   recStack.erase(base);
//   return has_cycle;
// }

// CPP_EXPORT bool Expr::isAcyclic() const {
//   std::unordered_set<const Expr *> visited, recStack;
//   bool has_cycle = false;

//   iterate<IterMode::children>(
//       this, [&](const Expr *const, const Expr *const *const cur) -> IterOp {
//         if (!visited.contains(*cur) && isCyclicUtil(*cur, visited, recStack))
//             [[unlikely]] {
//           has_cycle = true;
//           return IterOp::Abort;
//         }

//         return IterOp::Proceed;
//       });

//   return !has_cycle;
// }

// CPP_EXPORT void Expr::dump(std::ostream &os, bool isForDebug) const {
//   (void)isForDebug;

//   char *cstr = nullptr;
//   size_t len = 0;

//   FILE *fmembuf = open_memstream(&cstr, &len);
//   if (!nr_write(nullptr, this, IR_SERIAL_CODE, fmembuf, nullptr, 0)) {
//     qcore_panic("Failed to dump expression");
//   }
//   fflush(fmembuf);

//   os.write(cstr, len);

//   fclose(fmembuf);
//   free(cstr);
// }

// #pragma clang diagnostic push
// #pragma clang diagnostic ignored "-Wdeprecated-declarations"

// CPP_EXPORT boost::uuids::uuid Expr::hash() {
//   std::array<uint8_t, 20> hash;

//   SHA_CTX ctx;
//   SHA1_Init(&ctx);

//   Expr *ptr = this;
//   iterate<IterMode::dfs_pre>(ptr, [&ctx](Expr *, Expr **_cur) -> IterOp {
//     Expr *cur = *_cur;
//     uint8_t kind = static_cast<uint8_t>(cur->getKind());

//     if (SHA1_Update(&ctx, &kind, sizeof(kind)) != 1) {
//       qcore_panic("Failed to update EVP_MD_CTX");
//     }

// #define MIXIN_PRIMITIVE(x) SHA1_Update(&ctx, &x, sizeof(x))
// #define MIXIN_STRING(x) SHA1_Update(&ctx, x.data(), x.size())

//     switch (kind) {
//       case IR_eBIN: {
//         MIXIN_PRIMITIVE(cur->as<BinExpr>()->m_op);
//         break;
//       }
//       case IR_eUNARY: {
//         MIXIN_PRIMITIVE(cur->as<Unary>()->m_op);
//         break;
//       }
//       case IR_eINT: {
//         uint128_t x = cur->as<Int>()->getValue();
//         MIXIN_PRIMITIVE(x);
//         break;
//       }
//       case IR_eFLOAT: {
//         double v = cur->as<Float>()->getValue();
//         FloatSize s = cur->as<Float>()->getSize();
//         MIXIN_PRIMITIVE(v);
//         MIXIN_PRIMITIVE(s);
//         break;
//       }
//       case IR_eLIST: {
//         break;
//       }
//       case IR_eCALL: {
//         break;
//       }
//       case IR_eSEQ: {
//         break;
//       }
//       case IR_eINDEX: {
//         break;
//       }
//       case IR_eIDENT: {
//         MIXIN_STRING(cur->as<Ident>()->getName());
//         break;
//       }
//       case IR_eEXTERN: {
//         MIXIN_STRING(cur->as<Extern>()->getAbiName());
//         break;
//       }
//       case IR_eLOCAL: {
//         MIXIN_STRING(cur->as<Local>()->getName());
//         break;
//       }
//       case IR_eRET: {
//         break;
//       }
//       case IR_eBRK: {
//         break;
//       }
//       case IR_eSKIP: {
//         break;
//       }
//       case IR_eIF: {
//         break;
//       }
//       case IR_eWHILE: {
//         break;
//       }
//       case IR_eFOR: {
//         break;
//       }
//       case IR_eCASE: {
//         break;
//       }
//       case IR_eSWITCH: {
//         break;
//       }
//       case IR_eIGN: {
//         break;
//       }
//       case IR_eFUNCTION: {
//         MIXIN_STRING(cur->as<Function>()->m_name);
//         break;
//       }
//       case IR_eASM: {
//         qcore_implement();
//         break;
//       }
//       case IR_tU1: {
//         break;
//       }
//       case IR_tU8: {
//         break;
//       }
//       case IR_tU16: {
//         break;
//       }
//       case IR_tU32: {
//         break;
//       }
//       case IR_tU64: {
//         break;
//       }
//       case IR_tU128: {
//         break;
//       }
//       case IR_tI8: {
//         break;
//       }
//       case IR_tI16: {
//         break;
//       }
//       case IR_tI32: {
//         break;
//       }
//       case IR_tI64: {
//         break;
//       }
//       case IR_tI128: {
//         break;
//       }
//       case IR_tF16_TY: {
//         break;
//       }
//       case IR_tF32_TY: {
//         break;
//       }
//       case IR_tF64_TY: {
//         break;
//       }
//       case IR_tF128_TY: {
//         break;
//       }
//       case IR_tVOID: {
//         break;
//       }
//       case IR_tPTR: {
//         break;
//       }
//       case IR_tCONST: {
//         break;
//       }
//       case IR_tOPAQUE: {
//         MIXIN_STRING(cur->as<OpaqueTy>()->getName());
//         break;
//       }
//       case IR_tSTRUCT: {
//         break;
//       }
//       case IR_tUNION: {
//         break;
//       }
//       case IR_tARRAY: {
//         break;
//       }
//       case IR_tFUNC: {
//         std::set<FnAttr> tags;
//         for (auto &tag : cur->as<FnTy>()->m_attrs) {
//           tags.insert(tag);
//         }
//         for (auto tag : tags) {
//           MIXIN_PRIMITIVE(tag);
//         }
//         break;
//       }
//       case IR_tTMP: {
//         MIXIN_PRIMITIVE(cur->as<Tmp>()->m_type);

//         if (std::holds_alternative<CallArgsTmpNodeCradle>(
//                 cur->as<Tmp>()->m_data)) {
//           const CallArgsTmpNodeCradle &data =
//               std::get<CallArgsTmpNodeCradle>(cur->as<Tmp>()->m_data);
//           if (data.base != nullptr) {
//             MIXIN_STRING(data.base->getStateUUID());
//           }
//           for (const auto &arg : data.args) {
//             MIXIN_STRING(arg.first);
//             MIXIN_STRING(arg.second->getStateUUID());
//           }
//         } else if (std::holds_alternative<std::string_view>(
//                        cur->as<Tmp>()->m_data)) {
//           std::string_view &data =
//               std::get<std::string_view>(cur->as<Tmp>()->m_data);
//           MIXIN_STRING(data);
//         } else {
//           qcore_panic("Unknown TmpNodeCradle inner type");
//         }
//         break;
//       }
//     }

//     return IterOp::Proceed;
//   });

//   if (SHA1_Final(hash.data(), &ctx) != 1) {
//     qcore_panic("Failed to finalize EVP_MD_CTX");
//   }

//   boost::uuids::uuid uuid;
//   std::memcpy(uuid.data, hash.data(), uuid.size());
//   boost::uuids::name_generator gen(uuid);
//   return gen("nr");
// }

// #pragma clang diagnostic pop

// ///=============================================================================

// CPP_EXPORT uint128_t Int::str2u128(std::string_view s) {
//   uint128_t x = 0;

//   for (char c : s) {
//     if (!std::isdigit(c)) {
//       qcore_panicf("Failed to convert string `%s` to uint128_t", s.data());
//     }

//     // Check for overflow
//     if (x > (std::numeric_limits<uint128_t>::max() - (c - '0')) / 10) {
//       qcore_panicf("Overflow when converting string `%s` to uint128_t",
//                    s.data());
//     }

//     x = x * 10 + (c - '0');
//   }

//   return x;
// }

// CPP_EXPORT std::string Int::getValueString() const {
//   return ((uint128_t)m_value).str();
// }

// std::unordered_map<std::pair<uint128_t, uint8_t>, Int *, Int::map_hash>
//     Int::m_cache;

// static std::mutex m_cache_mtx;

// CPP_EXPORT Int *Int::get(uint128_t val, uint8_t size) {
//   std::lock_guard<std::mutex> lock(m_cache_mtx);

//   auto it = m_cache.find({val, size});
//   if (it != m_cache.end()) [[likely]] {
//     return it->second;
//   }

//   return m_cache[{val, size}] = new Int(val, size);
// }

// ///=============================================================================

CPP_EXPORT FlowPtr<Expr> ir::createIgn() {
  return new (Arena<Expr>().allocate(1)) Expr(IR_eIGN);
}
