// ////////////////////////////////////////////////////////////////////////////////
// /// ///
// ///     .-----------------.    .----------------.     .----------------. ///
// ///    | .--------------. |   | .--------------. |   | .--------------. | ///
// ///    | | ____  _____  | |   | |     ____     | |   | |    ______    | | ///
// ///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | | ///
// ///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | | ///
// ///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | | ///
// ///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | | ///
// ///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | | ///
// ///    | |              | |   | |              | |   | |              | | ///
// ///    | '--------------' |   | '--------------' |   | '--------------' | ///
// ///     '----------------'     '----------------'     '----------------' ///
// /// ///
// ///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language.
// ///
// ///   * Copyright (C) 2024 Wesley C. Jones ///
// /// ///
// ///   The Nitrate Toolchain is free software; you can redistribute it or ///
// ///   modify it under the terms of the GNU Lesser General Public ///
// ///   License as published by the Free Software Foundation; either ///
// ///   version 2.1 of the License, or (at your option) any later version. ///
// /// ///
// ///   The Nitrate Toolcain is distributed in the hope that it will be ///
// ///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// ///
// ///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU ///
// ///   Lesser General Public License for more details. ///
// /// ///
// ///   You should have received a copy of the GNU Lesser General Public ///
// ///   License along with the Nitrate Toolchain; if not, see ///
// ///   <https://www.gnu.org/licenses/>. ///
// /// ///
// ////////////////////////////////////////////////////////////////////////////////

// #include <limits>
// #define IRBUILDER_IMPL

// #include <nitrate-core/Logger.hh>
// #include <nitrate-ir/IRB/Builder.hh>
// #include <nitrate-ir/IR/Nodes.hh>

// using namespace ncc::ir;

// Int *NRBuilder::createBool(bool value SOURCE_LOCATION_PARAM) {
//   contract_enforce(m_state == SelfState::Constructed);
//   contract_enforce(m_root != nullptr);

//   return compiler_trace(debug_info(create<Int>(value, 1), DEBUG_INFO));
// }

// Int *NRBuilder::createFixedInteger(boost::multiprecision::cpp_int value,
//                                    uint8_t width SOURCE_LOCATION_PARAM) {
//   contract_enforce(m_state == SelfState::Constructed);
//   contract_enforce(m_root != nullptr);

//   contract_enforce(width > 0 && width <= std::exp2(width) - 1);

//   return compiler_trace(debug_info(
//       create<Int>(value.convert_to<unsigned __int128>(), width),
//       DEBUG_INFO));
// }

// Float *NRBuilder::createFixedFloat(bigfloat_t value,
//                                    uint8_t width SOURCE_LOCATION_PARAM) {
//   contract_enforce(m_state == SelfState::Constructed);
//   contract_enforce(m_root != nullptr);

//   // switch (width) {
//   //   case 16: {
//   //     contract_enforce(value >= -65504 && value <= 65504 &&
//   //                      "This might be a bug?");
//   //     break;
//   //   }
//   //   case 32: {
//   //     contract_enforce(value >= std::numeric_limits<_Float32>::min() &&
//   //                      value <= std::numeric_limits<_Float32>::max());
//   //     break;
//   //   }
//   //   case 64: {
//   //     contract_enforce(value >= std::numeric_limits<_Float64>::min() &&
//   //                      value <= std::numeric_limits<_Float64>::max());
//   //     break;
//   //   }
//   //   case 128: {
//   //     /// FIXME: Find out how to verify
//   //     break;
//   //   }
//   // }

//   return compiler_trace(debug_info(
//       create<Float>(value.convert_to<long double>(), width), DEBUG_INFO));
// }

// List *NRBuilder::createStringDataArray(
//     std::string_view value, ABIStringStyle style SOURCE_LOCATION_PARAM) {
//   contract_enforce(m_state == SelfState::Constructed);
//   contract_enforce(m_root != nullptr);

//   // Only C-strings are currently supported
//   contract_enforce(style == ABIStringCStr);

//   std::vector<Expr *> items(value.size() + 1);

//   for (size_t i = 0; i < value.size(); i++) {
//     items[i] = compiler_trace(createFixedInteger(value[i], 8));
//   }

//   /* Add null byte at end */
//   items[value.size()] = compiler_trace(createFixedInteger(0, 8));

//   return compiler_trace(debug_info(createList(items, true), DEBUG_INFO));
// }

// List *NRBuilder::createList(
//     std::span<Expr *> items,
//     /* Require assert(typeof(result)==typeof(array<result.element,
//      * result.size>)) ? Reason: It has to do with type inference and
//      * implicit conversions of the elements in the list.
//      */
//     bool cast_homogenous SOURCE_LOCATION_PARAM) {
//   contract_enforce(m_state == SelfState::Constructed);
//   contract_enforce(m_root != nullptr);

//   ListItems items_copy;
//   items_copy.resize(items.size());

//   for (size_t i = 0; i < items.size(); i++) {
//     items_copy[i] = compiler_trace(items[i]);
//   }

//   List *R = create<List>(std::move(items_copy), cast_homogenous);

//   return compiler_trace(debug_info(R, DEBUG_INFO));
// }
