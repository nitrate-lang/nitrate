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

#include <cctype>
#define IRBUILDER_IMPL

#include <nitrate-core/Error.h>

#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>

using namespace nr;

U1Ty *NRBuilder::getU1Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<U1Ty>(), DEBUG_INFO));
}

U8Ty *NRBuilder::getU8Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<U8Ty>(), DEBUG_INFO));
}

U16Ty *NRBuilder::getU16Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<U16Ty>(), DEBUG_INFO));
}

U32Ty *NRBuilder::getU32Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<U32Ty>(), DEBUG_INFO));
}

U64Ty *NRBuilder::getU64Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<U64Ty>(), DEBUG_INFO));
}

U128Ty *NRBuilder::getU128Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<U128Ty>(), DEBUG_INFO));
}

I8Ty *NRBuilder::getI8Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<I8Ty>(), DEBUG_INFO));
}

I16Ty *NRBuilder::getI16Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<I16Ty>(), DEBUG_INFO));
}

I32Ty *NRBuilder::getI32Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<I32Ty>(), DEBUG_INFO));
}

I64Ty *NRBuilder::getI64Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<I64Ty>(), DEBUG_INFO));
}

I128Ty *NRBuilder::getI128Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<I128Ty>(), DEBUG_INFO));
}

F16Ty *NRBuilder::getF16Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<F16Ty>(), DEBUG_INFO));
}

F32Ty *NRBuilder::getF32Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<F32Ty>(), DEBUG_INFO));
}

F64Ty *NRBuilder::getF64Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<F64Ty>(), DEBUG_INFO));
}

F128Ty *NRBuilder::getF128Ty(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<F128Ty>(), DEBUG_INFO));
}

VoidTy *NRBuilder::getVoidTy(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<VoidTy>(), DEBUG_INFO));
}

OpaqueTy *NRBuilder::getUnknownTy(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  // Use the '?' name to indiciate an unknown subject to later resolution by type inference.
  return compiler_trace(debug_info(create<OpaqueTy>("?"), DEBUG_INFO));
}

PtrTy *NRBuilder::getPtrTy(Type *pointee SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);
  contract_enforce(pointee != nullptr && static_cast<Expr *>(pointee)->isType());

  PtrTy *ptr_ty = create<PtrTy>(compiler_trace(pointee));

  return compiler_trace(debug_info(ptr_ty, DEBUG_INFO));
}

OpaqueTy *NRBuilder::getOpaqueTy(std::string_view name SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);
  contract_enforce(!name.empty() && std::isalnum(name[0]) &&
                   "Non alphanumeric starter characters are reserved internally");

  OpaqueTy *opaque_ty = create<OpaqueTy>(intern(name));

  return compiler_trace(debug_info(opaque_ty, DEBUG_INFO));
}

StructTy *NRBuilder::getStructTy(std::span<Type *> fields SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);
  contract_enforce(std::all_of(fields.begin(), fields.end(), [](Type *ty) {
    return ty != nullptr && static_cast<Expr *>(ty)->isType();
  }));

  if (fields.empty()) {
    return compiler_trace(debug_info(create<StructTy>(StructFields()), DEBUG_INFO));
  }

  StructFields fields_copy;
  fields_copy.resize(fields.size());
  for (size_t i = 0; i < fields.size(); i++) {
    fields_copy[i] = compiler_trace(fields[i]);
  }

  StructTy *struct_ty = create<StructTy>(std::move(fields_copy));

  return compiler_trace(debug_info(struct_ty, DEBUG_INFO));
}

UnionTy *NRBuilder::getUnionTy(std::span<Type *> fields SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);
  contract_enforce(std::all_of(fields.begin(), fields.end(), [](Type *ty) {
    return ty != nullptr && static_cast<Expr *>(ty)->isType();
  }));

  if (fields.empty()) {
    return compiler_trace(debug_info(create<UnionTy>(UnionFields()), DEBUG_INFO));
  }

  UnionFields fields_copy;
  fields_copy.resize(fields.size());
  for (size_t i = 0; i < fields.size(); i++) {
    fields_copy[i] = compiler_trace(fields[i]);
  }

  UnionTy *struct_ty = create<UnionTy>(std::move(fields_copy));

  return compiler_trace(debug_info(struct_ty, DEBUG_INFO));
}

ArrayTy *NRBuilder::getArrayTy(Type *element_ty, size_t count SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);
  contract_enforce(element_ty != nullptr && static_cast<Expr *>(element_ty)->isType());

  ArrayTy *array_ty = create<ArrayTy>(compiler_trace(element_ty), count);

  return compiler_trace(debug_info(array_ty, DEBUG_INFO));
}

FnTy *getFnTy(std::span<Type *> params, Type *ret_ty, bool is_variadic, Purity purity,
              bool thread_safe, bool is_noexcept, bool foreign SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement(__func__);
  (void)params;
  (void)ret_ty;
  (void)is_variadic;
  (void)purity;
  (void)thread_safe;
  (void)is_noexcept;
  (void)foreign;
  ignore_caller_info();
}

StructTy *createStructTemplateDefintion(std::string_view name,
                                        std::span<std::string_view> template_params,
                                        StructTy *ty SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement(__func__);
  (void)name;
  (void)template_params;
  (void)ty;
  ignore_caller_info();
}

UnionTy *createUnionTemplateDefintion(std::string_view name,
                                      std::span<std::string_view> template_params,
                                      UnionTy *ty SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement(__func__);
  (void)name;
  (void)template_params;
  (void)ty;
  ignore_caller_info();
}
