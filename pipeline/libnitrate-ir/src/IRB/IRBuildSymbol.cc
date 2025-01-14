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
// #include <nitrate-ir/IRB/Builder.hh>
// #include <nitrate-ir/IR/Nodes.hh>
// #include <unordered_map>

// using namespace ncc::ir;

// Function *NRBuilder::createFunctionDefintion(std::string_view name,
//                                        std::span<FnParam> params, Type
//                                        *ret_ty, bool is_variadic, Vis
//                                        visibility, Purity purity, bool
//                                        thread_safe, bool foreign
//                                        SOURCE_LOCATION_PARAM) {
//   contract_enforce(m_state == SelfState::Constructed);
//   contract_enforce(m_root != nullptr);
//   contract_enforce(ret_ty != nullptr && static_cast<Expr
//   *>(ret_ty)->isType());

//   Params parameters(params.size());
//   std::unordered_map<size_t, Expr *> default_arguments;

//   for (size_t i = 0; i < params.size(); i++) {
//     contract_enforce(static_cast<Expr *>(std::get<1>(params[i]))->isType());
//     parameters[i] = {std::get<1>(params[i]), std::get<0>(params[i])};

//     if (std::get<2>(params[i]).has_value()) {
//       default_arguments[i] = std::get<2>(params[i]).value();
//     }
//   }

//   /// TODO: Do something useful with the metadata:
//   /// [visibility,purity,thread_safety,,foriegn]
//   (void)visibility;
//   (void)purity;
//   (void)thread_safe;
//   (void)foreign;

//   Function *fn = create<Function>(name, std::move(parameters), ret_ty,
//   std::nullopt,
//                       is_variadic, AbiTag::Default);

//   if (m_functions.contains(name)) [[unlikely]] {
//     m_duplicate_functions->insert(fn);
//   }

//   m_functions[name] = fn;
//   m_function_defaults[fn] = std::move(default_arguments);

//   return compiler_trace(debug_info(fn, DEBUG_INFO));
// }

// Function *NRBuilder::createFunctionDeclaration(std::string_view name,
//                                          std::span<FnParam> params,
//                                          FlowPtr<Type>ret_ty, bool
//                                          is_variadic, Vis visibility, Purity
//                                          purity, bool thread_safe, bool
//                                          foreign SOURCE_LOCATION_PARAM)
//                                          {
//   contract_enforce(m_state == SelfState::Constructed);
//   contract_enforce(m_root != nullptr);
//   contract_enforce(ret_ty != nullptr && static_cast<Expr
//   *>(ret_ty)->isType());

//   Params parameters(params.size());
//   std::unordered_map<size_t, Expr *> default_arguments;

//   for (size_t i = 0; i < params.size(); i++) {
//     contract_enforce(static_cast<Expr *>(std::get<1>(params[i]))->isType());
//     parameters[i] = {std::get<1>(params[i]), std::get<0>(params[i])};

//     if (std::get<2>(params[i]).has_value()) {
//       default_arguments[i] = std::get<2>(params[i]).value();
//     }
//   }

//   /// TODO: Do something useful with the metadata:
//   /// [visibility,purity,thread_safety,,foriegn]
//   (void)visibility;
//   (void)purity;
//   (void)thread_safe;
//   (void)foreign;

//   Function *fn = create<Function>(name, std::move(parameters), ret_ty,
//   std::nullopt,
//                       is_variadic, AbiTag::Default);

//   if (m_functions.contains(name)) [[unlikely]] {
//     m_duplicate_functions->insert(fn);
//   }

//   m_functions[name] = fn;
//   m_function_defaults[fn] = std::move(default_arguments);

//   return compiler_trace(debug_info(fn, DEBUG_INFO));
// }

// Function *NRBuilder::createOperatorOverload(Op op, std::span<FlowPtr<Type>>
// params,
//                                       FlowPtr<Type>ret_ty, Purity purity,
//                                       bool thread_safe SOURCE_LOCATION_PARAM)
//                                       {
//   /// TODO: Implement operator overloading

//   qcore_implement();
//   (void)op;
//   (void)params;
//   (void)ret_ty;
//   (void)purity;
//   (void)thread_safe;
//   ignore_caller_info();
// }

// Local *NRBuilder::createVariable(std::string_view name, FlowPtr<Type>ty,
//                                  Vis visibility, StorageClass storage,
//                                  bool is_readonly SOURCE_LOCATION_PARAM) {
//   contract_enforce(m_state == SelfState::Constructed);
//   contract_enforce(m_root != nullptr);
//   contract_enforce(ty != nullptr && static_cast<Expr *>(ty)->isType());

//   Local *local =
//       create<Local>(name, createIgn(), AbiTag::Default, is_readonly,
//       storage);

//   /// TODO: Set the visibility of the local variable
//   (void)visibility;

//   if (m_variables.contains(name)) [[unlikely]] {
//     m_duplicate_variables->insert(local);
//   }

//   m_variables[name] = local;

//   local = compiler_trace(debug_info(local, DEBUG_INFO));

//   return local;
// }
