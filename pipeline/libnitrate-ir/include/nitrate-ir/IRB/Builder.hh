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

#ifndef __NITRATE_IR_tIRBUILDER_H__
#define __NITRATE_IR_tIRBUILDER_H__

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/uuid/uuid.hpp>
#include <cassert>
#include <cmath>
#include <experimental/source_location>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-ir/IR/Fwd.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/Module.hh>
#include <nitrate-ir/diagnostic/Report.hh>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace ncc::ir {
  enum class ABIStringStyle : uint8_t {
    CStr, /* Only supported variant */
  };

  enum class Kind {
    TypeDef,
    ScopedEnum,
    Function,
    Variable,
  };

  using bigfloat_t = boost::multiprecision::cpp_dec_float_100;
  using boost::multiprecision::uint128_t;

  class __attribute__((visibility("default"))) NRBuilder {
    ///**************************************************************************///
    // Builder properties
    ///**************************************************************************///

    std::string m_module_name;
    TargetInfo m_target_info;

    ///**************************************************************************///
    // Builder state variables
    ///**************************************************************************///

    enum class SelfState {
      Constructed,  // start => [Finished, Destroyed]
      Finished,     // => [Verified, Destroyed]
      Verified,     // => [Emitted, Destroyed]
      Emitted,      // => [Destroyed]
      Destroyed,    // exit => []
    };

    SelfState m_state;
    std::optional<IRModule *> m_result;
    FlowPtr<Seq> m_root;

    std::unordered_map<std::string_view, FlowPtr<Type>> m_named_types;
    std::unordered_map<std::string_view, std::unordered_map<std::string_view, FlowPtr<Expr>>> m_named_constant_group;
    std::unordered_map<std::string_view, Function *> m_functions;
    std::unordered_map<Function *, std::unordered_map<size_t, FlowPtr<Expr>>> m_function_defaults;
    std::unordered_map<std::string_view, Local *> m_variables;

    std::optional<std::unordered_set<Function *>> m_duplicate_functions;
    std::optional<std::unordered_set<Local *>> m_duplicate_variables;
    std::optional<std::unordered_set<std::string_view>> m_duplicate_named_types;
    std::optional<std::unordered_set<std::string_view>> m_duplicate_named_constants;

    ///**************************************************************************///
    // Builder helper methods
    ///**************************************************************************///

    auto ResolveName(std::string_view name, Kind kind) -> std::optional<std::pair<FlowPtr<Expr>, std::string_view>>;

    void TryTransformAlpha(FlowPtr<Expr> root);
    void TryTransformBeta(FlowPtr<Expr> root);
    void TryTransformGamma(FlowPtr<Expr> root);
    void ConnectNodes(FlowPtr<Seq> root);
    void FlattenSymbols(FlowPtr<Seq> root);
    void RemoveGarbage(FlowPtr<Seq> root);

    auto CheckAcyclic(FlowPtr<Seq> root, IReport *i) -> bool;
    auto CheckDuplicates(FlowPtr<Seq> root, IReport *i) -> bool;
    auto CheckSymbolsExist(FlowPtr<Seq> root, IReport *i) -> bool;
    auto CheckFunctionCalls(FlowPtr<Seq> root, IReport *i) -> bool;
    auto CheckReturns(FlowPtr<Seq> root, IReport *i) -> bool;
    auto CheckScopes(FlowPtr<Seq> root, IReport *i) -> bool;
    auto CheckMutability(FlowPtr<Seq> root, IReport *i) -> bool;
    auto CheckControlFlow(FlowPtr<Seq> root, IReport *i) -> bool;
    auto CheckTypes(FlowPtr<Seq> root, IReport *i) -> bool;
    auto CheckSafetyClaims(FlowPtr<Seq> root, IReport *i) -> bool;

#if defined(NDEBUG)
#define SOURCE_LOCATION_PARAM
#define SOURCE_LOCATION_PARAM_ONCE

    void contract_enforce_(
        bool cond, std::string_view cond_str,
        std::experimental::source_location caller = std::experimental::source_location::current()) const;
#define contract_enforce(cond) contract_enforce_(cond, #cond)
#else
#define SOURCE_LOCATION_PARAM \
  , std::experimental::source_location caller_info = std::experimental::source_location::current()
#define SOURCE_LOCATION_PARAM_ONCE \
  std::experimental::source_location caller_info = std::experimental::source_location::current()

    void ContractEnforce(
        bool cond, std::string_view cond_str SOURCE_LOCATION_PARAM,
        std::experimental::source_location caller = std::experimental::source_location::current()) const;
#define contract_enforce(cond) contract_enforce_(cond, #cond, caller_info)

#endif

#define DEBUG_INFO 1, 1

  public:
    NRBuilder(const NRBuilder &) = delete;
    auto operator=(const NRBuilder &) -> NRBuilder & = delete;

    NRBuilder(std::string module_name, TargetInfo target_info SOURCE_LOCATION_PARAM);
    ~NRBuilder();

    /* Moving the module is permitted */
    auto operator=(NRBuilder &&) noexcept -> NRBuilder &;
    NRBuilder(NRBuilder &&) noexcept;

    /** @warning: This is a slow and resource heavy operation for
     * most programs. */
    auto DeepClone(SOURCE_LOCATION_PARAM_ONCE) const -> NRBuilder;

    /** @brief Count *ALL* nodes currently in the builder. This includes
     * temporary nodes. */
    auto NodeCount(SOURCE_LOCATION_PARAM_ONCE) -> size_t;

    /**
     * @brief Finialize the module build
     * @note After the builder is finalized, it can't be updated anymore.
     * @note This function is idempotent, without any overhead from additional
     * calls.
     */
    void Finish(SOURCE_LOCATION_PARAM_ONCE);

    /**
     * @brief Run basic checks on the module:
     * @param sink The diagnostic engine to use.
     * @return True if the module is usable, false otherwise.
     *
     * Usability means that the module is in a state where all data-structure
     * invariants are intact, such that it can be used for further processing
     * as-if it were fully correct.
     *
     * An example of something is is semantically erroronous, but still "usable"
     * is an out-of-bounds array access. `verify()` may report an error to the
     * diagnostic sink regarding the out-of-bounds access, but it may return
     * true because the module's data-structure invariants are verified as
     * correct.
     *
     *  - Check for cyclic references in the internal data-structure;
     *  - Ensure that all symbols are resolved;
     *  - Check for duplicate identifiers;
     *  - Ensure that all types are resolved;
     *  - Check range and type of initial values;
     *  - Type check entire module;
     *  - Function calls have the correct number of arguments;
     *  - Ensure that return statements are present;
     *  - Ensure that all scopes are obeyed:
     *      Variables exist by the time they are accessed;
     *      Functions exist by the time they are called;
     *  - Verify mutability rules are obeyed;
     *  - Verify usage and presence of control flow nodes;
     *  - Do complex safety checks to verify proper usage of `safe` and
     * `unsafe`.
     *
     * @note This function calls `finish()`.
     */
    auto Verify(std::optional<IReport *> sink SOURCE_LOCATION_PARAM) -> bool;

    /**
     * @brief Return the build module.
     * @note `verify()` must be called first.
     */
    auto GetModule(SOURCE_LOCATION_PARAM_ONCE) -> IRModule *;

    void AppendToRoot(FlowPtr<Expr> node SOURCE_LOCATION_PARAM);

    ///**************************************************************************///
    // Create linkable symbols

    using FnParam = std::tuple<std::string_view, FlowPtr<Type>, std::optional<FlowPtr<Expr>>>;

    auto CreateFunctionDefintion(std::string_view name, std::span<FnParam> params, FlowPtr<Type> ret_ty,
                                 bool is_variadic = false, Vis visibility = Vis::Sec, Purity purity = Purity::Impure,
                                 bool thread_safe = false, bool foreign = true SOURCE_LOCATION_PARAM) -> Function *;

    auto CreateFunctionDeclaration(std::string_view name, std::span<FnParam> params, FlowPtr<Type> ret_ty,
                                   bool is_variadic = false, Vis visibility = Vis::Sec, Purity purity = Purity::Impure,
                                   bool thread_safe = false, bool foreign = true SOURCE_LOCATION_PARAM) -> Function *;

    /* This is the only intended way to overload operaters */
    auto CreateOperatorOverload(lex::Operator op, std::span<FlowPtr<Type>> params, FlowPtr<Type> ret_ty,
                                Purity purity = Purity::Impure,
                                bool thread_safe = false SOURCE_LOCATION_PARAM) -> Function *;

    /* Works for both local and global variables FlowPtr<Type> */
    auto CreateVariable(std::string_view name, FlowPtr<Type> ty, Vis visibility = Vis::Sec,
                        StorageClass storage = StorageClass::LLVM_StackAlloa,
                        bool is_readonly = false SOURCE_LOCATION_PARAM) -> Local *;

    ///**************************************************************************///
    // Create expressions

    auto CreateCall(FlowPtr<Expr> target, std::span<std::pair<std::string_view, FlowPtr<Expr>>> arguments
                                              SOURCE_LOCATION_PARAM) -> FlowPtr<Expr>;

    auto CreateMethodCall(FlowPtr<Expr> object, std::string_view name,
                          std::span<std::pair<std::string_view, FlowPtr<Expr>>> arguments SOURCE_LOCATION_PARAM)
        -> FlowPtr<Expr>;

    ///**************************************************************************///
    // Create literals

    auto CreateBool(bool value SOURCE_LOCATION_PARAM) -> Int *;

    auto CreateFixedInteger(boost::multiprecision::cpp_int value, uint8_t width SOURCE_LOCATION_PARAM) -> Int *;

    auto CreateFixedFloat(bigfloat_t value, uint8_t width SOURCE_LOCATION_PARAM) -> Float *;

    auto CreateStringDataArray(std::string_view value,
                               ABIStringStyle style = ABIStringStyle::CStr SOURCE_LOCATION_PARAM) -> List *;

    auto CreateList(std::span<FlowPtr<Expr>> items,

                    /* Require assert(typeof(result)==typeof(array<result.element,
                     * result.size>)) ? Reason: It has to do with type inference and
                     * implicit conversions of the elements in the list.
                     */
                    bool cast_homogenous SOURCE_LOCATION_PARAM) -> List *;

    ///**************************************************************************///
    // Create values

    auto GetDefaultValue(FlowPtr<Type> src_loc SOURCE_LOCATION_PARAM) -> std::optional<FlowPtr<Expr>>;

    ///**************************************************************************///
    // Create types

    auto GetU1Ty(SOURCE_LOCATION_PARAM_ONCE) -> U1Ty *;
    auto GetU8Ty(SOURCE_LOCATION_PARAM_ONCE) -> U8Ty *;
    auto GetU16Ty(SOURCE_LOCATION_PARAM_ONCE) -> U16Ty *;
    auto GetU32Ty(SOURCE_LOCATION_PARAM_ONCE) -> U32Ty *;
    auto GetU64Ty(SOURCE_LOCATION_PARAM_ONCE) -> U64Ty *;
    auto GetU128Ty(SOURCE_LOCATION_PARAM_ONCE) -> U128Ty *;
    auto GetI8Ty(SOURCE_LOCATION_PARAM_ONCE) -> I8Ty *;
    auto GetI16Ty(SOURCE_LOCATION_PARAM_ONCE) -> I16Ty *;
    auto GetI32Ty(SOURCE_LOCATION_PARAM_ONCE) -> I32Ty *;
    auto GetI64Ty(SOURCE_LOCATION_PARAM_ONCE) -> I64Ty *;
    auto GetI128Ty(SOURCE_LOCATION_PARAM_ONCE) -> I128Ty *;
    auto GetF16Ty(SOURCE_LOCATION_PARAM_ONCE) -> F16Ty *;
    auto GetF32Ty(SOURCE_LOCATION_PARAM_ONCE) -> F32Ty *;
    auto GetF64Ty(SOURCE_LOCATION_PARAM_ONCE) -> F64Ty *;
    auto GetF128Ty(SOURCE_LOCATION_PARAM_ONCE) -> F128Ty *;
    auto GetVoidTy(SOURCE_LOCATION_PARAM_ONCE) -> VoidTy *;

    /* Type inference unknowns; Converted to proper type upon resolution */
    auto GetUnknownTy(SOURCE_LOCATION_PARAM_ONCE) -> OpaqueTy *;

    auto GetUnknownNamedTy(std::string_view name SOURCE_LOCATION_PARAM) -> FlowPtr<Type>;

    auto GetPtrTy(FlowPtr<Type> pointee SOURCE_LOCATION_PARAM) -> PtrTy *;

    auto GetOpaqueTy(std::string_view name SOURCE_LOCATION_PARAM) -> OpaqueTy *;

    auto GetStructTy(std::span<std::tuple<std::string_view, FlowPtr<Type>, FlowPtr<Expr>>> fields SOURCE_LOCATION_PARAM)
        -> StructTy *;

    auto GetStructTy(std::span<FlowPtr<Type>> fields SOURCE_LOCATION_PARAM) -> StructTy *;

    auto GetUnionTy(std::span<FlowPtr<Type>> fields SOURCE_LOCATION_PARAM) -> UnionTy *;

    auto GetArrayTy(FlowPtr<Type> element_ty, size_t count SOURCE_LOCATION_PARAM) -> ArrayTy *;

    auto GetFnTy(std::span<FlowPtr<Type>> params, FlowPtr<Type> ret_ty, bool is_variadic = false,
                 Purity purity = Purity::Impure, bool thread_safe = false,
                 bool foreign = true SOURCE_LOCATION_PARAM) -> FnTy *;

    void CreateNamedConstantDefinition(
        std::string_view name, const std::unordered_map<std::string_view, FlowPtr<Expr>> &values SOURCE_LOCATION_PARAM);

    void CreateNamedTypeAlias(FlowPtr<Type> type, std::string_view name SOURCE_LOCATION_PARAM);

    ///**************************************************************************///

#undef SOURCE_LOCATION_PARAM
#undef SOURCE_LOCATION_PARAM_ONCE

#if defined(IRBUILDER_IMPL)

#if defined(NDEBUG)
#define SOURCE_LOCATION_PARAM
#define SOURCE_LOCATION_PARAM_ONCE
#define CALLER_INFO 0
#define ignore_caller_info()
#define compiler_trace(x) x
#else
#define SOURCE_LOCATION_PARAM , std::experimental::source_location caller_info
#define SOURCE_LOCATION_PARAM_ONCE std::experimental::source_location caller_info
#define CALLER_INFO caller_info
#define CALLEE_KNOWN
#define ignore_caller_info() (void)caller_info;
#define compiler_trace(x) x
#endif

#else
#undef DEBUG_INFO
#endif
  };
}  // namespace ncc::ir

#endif
