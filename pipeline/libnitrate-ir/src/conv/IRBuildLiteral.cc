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

#include <limits>
#define IRBUILDER_IMPL

#include <nitrate-core/Error.h>

#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>

using namespace nr;

Int *NRBuilder::createBool(bool value SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  return compiler_trace(debug_info(create<Int>(value, IntSize::U1), DEBUG_INFO));
}

Int *NRBuilder::createFixedInteger(uint128_t value, IntSize width SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  switch (width) {
    case nr::IntSize::U1: {
      contract_enforce(value >= std::numeric_limits<bool>::min() &&
                       value <= std::numeric_limits<bool>::max());
      break;
    }
    case nr::IntSize::U8: {
      contract_enforce(value >= std::numeric_limits<uint8_t>::min() &&
                       value <= std::numeric_limits<uint8_t>::max());
      break;
    }
    case nr::IntSize::U16: {
      contract_enforce(value >= std::numeric_limits<uint16_t>::min() &&
                       value <= std::numeric_limits<uint16_t>::max());
      break;
    }
    case nr::IntSize::U32: {
      contract_enforce(value >= std::numeric_limits<uint32_t>::min() &&
                       value <= std::numeric_limits<uint32_t>::max());
      break;
    }
    case nr::IntSize::U64: {
      contract_enforce(value >= std::numeric_limits<uint64_t>::min() &&
                       value <= std::numeric_limits<uint64_t>::max());
      break;
    }
    case nr::IntSize::U128: {
      contract_enforce(value >= std::numeric_limits<uint128_t>::min() &&
                       value <= std::numeric_limits<uint128_t>::max());
      break;
    }
  }

  return compiler_trace(debug_info(create<Int>(value, width), DEBUG_INFO));
}

Float *NRBuilder::createFixedFloat(bigfloat_t value,
                                   FloatSize width SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  switch (width) {
    case nr::FloatSize::F16: {
      contract_enforce(value >= -65504 && value <= 65504 && "This might be a bug?");
      break;
    }
    case nr::FloatSize::F32: {
      contract_enforce(value >= std::numeric_limits<_Float32>::min() &&
                       value <= std::numeric_limits<_Float32>::max());
      break;
    }
    case nr::FloatSize::F64: {
      contract_enforce(value >= std::numeric_limits<_Float64>::min() &&
                       value <= std::numeric_limits<_Float64>::max());
      break;
    }
    case nr::FloatSize::F128: {
      /// FIXME: Find out how to verify
      break;
    }
  }

  return compiler_trace(debug_info(create<Float>(value, width), DEBUG_INFO));
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
