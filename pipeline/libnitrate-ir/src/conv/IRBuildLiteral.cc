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

Int *NRBuilder::createBool(bool value SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement(__func__);
  (void)value;
  ignore_caller_info();
}

Int *NRBuilder::createFixedInteger(uint128_t value SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement(__func__);
  (void)value;
  ignore_caller_info();
}

Float *NRBuilder::createFixedFloat(bigfloat_t value SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement(__func__);
  (void)value;
  ignore_caller_info();
}

ArrayTy *NRBuilder::createStringDataArray(std::string_view value,
                                          ABIStringStyle style SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement(__func__);
  (void)value;
  (void)style;

  ignore_caller_info();
}

List *NRBuilder::createList(std::span<Expr *> items, StorageClass storage,

                            /* Require assert(typeof(result)==typeof(array<result.element,
                             * result.size>)) ? Reason: It has to do with type inference and
                             * implicit conversions of the elements in the list.
                             */
                            bool cast_homogenous SOURCE_LOCATION_PARAM) noexcept {
  /// TODO: Implement
  qcore_implement(__func__);
  (void)items;
  (void)storage;
  (void)cast_homogenous;
  ignore_caller_info();
}
