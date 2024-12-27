
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

// #define IRBUILDER_IMPL

// #include <nitrate-core/Logger.hh>
// #include <nitrate-ir/IRBuilder.hh>
// #include <nitrate-ir/IRGraph.hh>

// using namespace ncc::ir;

// Expr *NRBuilder::createCall(Expr *target,
//                             std::span<std::pair<std::string_view, Expr *>>
//                                 arguments SOURCE_LOCATION_PARAM) {
//   contract_enforce(m_state == SelfState::Constructed);
//   contract_enforce(m_root != nullptr);
//   contract_enforce(target != nullptr);
//   contract_enforce(std::all_of(arguments.begin(), arguments.end(),
//                                [](auto x) { return x.second != nullptr; }));

//   CallArgsTmpNodeCradle call;

//   std::vector<std::pair<std::string_view, Expr *>,
//               Arena<std::pair<std::string_view, Expr *>>>
//       copy;
//   copy.resize(arguments.size());

//   for (size_t i = 0; i < copy.size(); i++) {
//     copy[i] = arguments[i];
//   }

//   call.base = target;
//   call.args = std::move(copy);

//   Tmp *R = create<Tmp>(TmpType::CALL, std::move(call));

//   return compiler_trace(debug_info(R, DEBUG_INFO));
// }

// Expr *NRBuilder::createMethodCall(Expr *object, std::string_view name,
//                                   std::span<std::pair<std::string_view, Expr
//                                   *>>
//                                       arguments SOURCE_LOCATION_PARAM) {
//   contract_enforce(m_state == SelfState::Constructed);
//   contract_enforce(m_root != nullptr);
//   contract_enforce(object != nullptr);
//   contract_enforce(std::all_of(arguments.begin(), arguments.end(),
//                                [](auto x) { return x.second != nullptr; }));

//   CallArgsTmpNodeCradle call;

//   std::vector<std::pair<std::string_view, Expr *>,
//               Arena<std::pair<std::string_view, Expr *>>>
//       copy;
//   copy.resize(arguments.size());

//   for (size_t i = 0; i < copy.size(); i++) {
//     copy[i] = arguments[i];
//   }

//   call.base = create<Index>(object, create<Ident>(name, nullptr));
//   call.args = std::move(copy);

//   Tmp *R = create<Tmp>(TmpType::CALL, std::move(call));

//   return compiler_trace(debug_info(R, DEBUG_INFO));
// }
