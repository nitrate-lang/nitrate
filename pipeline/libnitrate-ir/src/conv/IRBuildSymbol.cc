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

#define IRBUILDER_IMPL

#include <nitrate-core/Error.h>

#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>

using namespace nr;

Fn *NRBuilder::createFunctionDefintion(std::string_view name, std::span<Type *> params,
                                       Type *ret_ty, bool is_variadic, Vis visibility,
                                       Purity purity, bool thread_safe, bool is_noexcept,
                                       bool foreign SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement();
  (void)name;
  (void)params;
  (void)ret_ty;
  (void)is_variadic;
  (void)visibility;
  (void)purity;
  (void)thread_safe;
  (void)is_noexcept;
  (void)foreign;
  ignore_caller_info();
}

Fn *NRBuilder::createFunctionDeclaration(std::string_view name, std::span<Type *> params,
                                         Type *ret_ty, bool is_variadic, Vis visibility,
                                         Purity purity, bool thread_safe, bool is_noexcept,
                                         bool foreign SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement();
  (void)name;
  (void)params;
  (void)ret_ty;
  (void)is_variadic;
  (void)visibility;
  (void)purity;
  (void)thread_safe;
  (void)is_noexcept;
  (void)foreign;
  ignore_caller_info();
}

Fn *NRBuilder::createAnonymousFunction(std::span<Type *> params, Type *ret_ty, bool is_variadic,
                                       Purity purity, bool thread_safe,
                                       bool is_noexcept SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement();
  (void)params;
  (void)ret_ty;
  (void)is_variadic;
  (void)purity;
  (void)thread_safe;
  (void)is_noexcept;
  ignore_caller_info();
}

Fn *NRBuilder::createOperatorOverload(Op op, std::span<Type *> params, Type *ret_ty, Purity purity,
                                      bool thread_safe,
                                      bool is_noexcept SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement();
  (void)op;
  (void)params;
  (void)ret_ty;
  (void)purity;
  (void)thread_safe;
  (void)is_noexcept;
  ignore_caller_info();
}

Fn *NRBuilder::createTemplateFunction(std::string_view name,
                                      std::span<std::string_view> template_params,
                                      std::span<Type *> params, Type *ret_ty, bool is_variadic,
                                      Vis visibility, Purity purity, bool thread_safe,
                                      bool is_noexcept SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement();
  (void)name;
  (void)template_params;
  (void)params;
  (void)ret_ty;
  (void)is_variadic;
  (void)visibility;
  (void)purity;
  (void)thread_safe;
  (void)is_noexcept;
  ignore_caller_info();
}

Local *NRBuilder::createVariable(std::string_view name, Type *ty, Vis visibility,
                                 StorageClass storage,
                                 bool is_readonly SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement();
  (void)name;
  (void)ty;
  (void)visibility;
  (void)storage;
  (void)is_readonly;
  ignore_caller_info();
}
