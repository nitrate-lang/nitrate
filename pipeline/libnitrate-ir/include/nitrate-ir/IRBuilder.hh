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

#ifndef __NITRATE_IR_IRBUILDER_H__
#define __NITRATE_IR_IRBUILDER_H__

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/uuid/uuid.hpp>
#include <cassert>
#include <cmath>
#include <experimental/source_location>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-ir/IRFwd.hh>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Module.hh>
#include <nitrate-ir/Report.hh>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

namespace ncc::ir {
  enum class ABIStringStyle {
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
    /** Implicit copying is not allowed */
    NRBuilder(const NRBuilder &) = delete;
    NRBuilder &operator=(const NRBuilder &) = delete;

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
    std::optional<qmodule_t *> m_result;
    Seq *m_root;

    std::unordered_map<std::string_view, Type *> m_named_types;
    std::unordered_map<std::string_view,
                       std::unordered_map<std::string_view, Expr *>>
        m_named_constant_group;
    std::unordered_map<std::string_view, Fn *> m_functions;
    std::unordered_map<Fn *, std::unordered_map<size_t, Expr *>>
        m_function_defaults;
    std::unordered_map<std::string_view, Local *> m_variables;

    std::optional<std::unordered_set<Fn *>> m_duplicate_functions;
    std::optional<std::unordered_set<Local *>> m_duplicate_variables;
    std::optional<std::unordered_set<std::string_view>> m_duplicate_named_types;
    std::optional<std::unordered_set<std::string_view>>
        m_duplicate_named_constants;

    ///**************************************************************************///
    // Builder helper methods
    ///**************************************************************************///

    std::optional<std::pair<Expr *, std::string_view>> resolve_name(
        std::string_view name, Kind kind);

    void try_transform_alpha(Expr *root);
    void try_transform_beta(Expr *root);
    void try_transform_gamma(Expr *root);
    void connect_nodes(Seq *root);
    void flatten_symbols(Seq *root);
    void remove_garbage(Seq *root);

    bool check_acyclic(Seq *root, IReport *I);
    bool check_duplicates(Seq *root, IReport *I);
    bool check_symbols_exist(Seq *root, IReport *I);
    bool check_function_calls(Seq *root, IReport *I);
    bool check_returns(Seq *root, IReport *I);
    bool check_scopes(Seq *root, IReport *I);
    bool check_mutability(Seq *root, IReport *I);
    bool check_control_flow(Seq *root, IReport *I);
    bool check_types(Seq *root, IReport *I);
    bool check_safety_claims(Seq *root, IReport *I);

#if defined(NDEBUG)
#define SOURCE_LOCATION_PARAM
#define SOURCE_LOCATION_PARAM_ONCE

    void contract_enforce_(
        bool cond, std::string_view cond_str,
        std::experimental::source_location caller =
            std::experimental::source_location::current()) const;
#define contract_enforce(cond) contract_enforce_(cond, #cond)
#else
#define SOURCE_LOCATION_PARAM                        \
  , std::experimental::source_location caller_info = \
        std::experimental::source_location::current()
#define SOURCE_LOCATION_PARAM_ONCE                 \
  std::experimental::source_location caller_info = \
      std::experimental::source_location::current()

    void contract_enforce_(
        bool cond, std::string_view cond_str SOURCE_LOCATION_PARAM,
        std::experimental::source_location caller =
            std::experimental::source_location::current()) const;
#define contract_enforce(cond) contract_enforce_(cond, #cond, caller_info)

#endif

#define DEBUG_INFO 1, 1

  public:
    NRBuilder(std::string module_name,
              TargetInfo target_info SOURCE_LOCATION_PARAM);
    ~NRBuilder();

    /* Moving the module is permitted */
    NRBuilder &operator=(NRBuilder &&);
    NRBuilder(NRBuilder &&);

    /** @warning: This is a slow and resource heavy operation for
     * most programs. */
    NRBuilder deep_clone(SOURCE_LOCATION_PARAM_ONCE) const;

    /** @brief Get an approximate figure of how much memory the
     * builder is currently using. The returned value is a lower bound. */
    size_t approx_memory_usage(SOURCE_LOCATION_PARAM_ONCE);

    /** @brief Count *ALL* nodes currently in the builder. This includes
     * temporary nodes. */
    size_t node_count(SOURCE_LOCATION_PARAM_ONCE);

    /**
     * @brief Finialize the module build
     * @note After the builder is finalized, it can't be updated anymore.
     * @note This function is idempotent, without any overhead from additional
     * calls.
     */
    void finish(SOURCE_LOCATION_PARAM_ONCE);

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
    bool verify(std::optional<IReport *> sink SOURCE_LOCATION_PARAM);

    /**
     * @brief Return the build module.
     * @note `verify()` must be called first.
     */
    qmodule_t *get_module(SOURCE_LOCATION_PARAM_ONCE);

    void appendToRoot(Expr *node SOURCE_LOCATION_PARAM);

    ///**************************************************************************///
    // Create linkable symbols

    using FnParam = std::tuple<std::string_view, Type *, std::optional<Expr *>>;

    Fn *createFunctionDefintion(std::string_view name,
                                std::span<FnParam> params, Type *ret_ty,
                                bool is_variadic = false,
                                Vis visibility = Vis::Sec,
                                Purity purity = Purity::Impure,
                                bool thread_safe = false,
                                bool foreign = true SOURCE_LOCATION_PARAM);

    Fn *createFunctionDeclaration(std::string_view name,
                                  std::span<FnParam> params, Type *ret_ty,
                                  bool is_variadic = false,
                                  Vis visibility = Vis::Sec,
                                  Purity purity = Purity::Impure,
                                  bool thread_safe = false,
                                  bool foreign = true SOURCE_LOCATION_PARAM);

    /* This is the only intended way to overload operaters */
    Fn *createOperatorOverload(Op op, std::span<Type *> params, Type *ret_ty,
                               Purity purity = Purity::Impure,
                               bool thread_safe = false SOURCE_LOCATION_PARAM);

    /* Works for both local and global variables */
    Local *createVariable(std::string_view name, Type *ty,
                          Vis visibility = Vis::Sec,
                          StorageClass storage = StorageClass::LLVM_StackAlloa,
                          bool is_readonly = false SOURCE_LOCATION_PARAM);

    ///**************************************************************************///
    // Create expressions

    Expr *createCall(Expr *target,
                     std::span<std::pair<std::string_view, Expr *>> arguments
                         SOURCE_LOCATION_PARAM);

    Expr *createMethodCall(Expr *object, std::string_view name,
                           std::span<std::pair<std::string_view, Expr *>>
                               arguments SOURCE_LOCATION_PARAM);

    ///**************************************************************************///
    // Create literals

    Int *createBool(bool value SOURCE_LOCATION_PARAM);

    Int *createFixedInteger(boost::multiprecision::cpp_int value,
                            uint8_t width SOURCE_LOCATION_PARAM);

    Float *createFixedFloat(bigfloat_t value,
                            FloatSize width SOURCE_LOCATION_PARAM);

    List *createStringDataArray(
        std::string_view value,
        ABIStringStyle style = ABIStringStyle::CStr SOURCE_LOCATION_PARAM);

    List *createList(
        std::span<Expr *> items,

        /* Require assert(typeof(result)==typeof(array<result.element,
         * result.size>)) ? Reason: It has to do with type inference and
         * implicit conversions of the elements in the list.
         */
        bool cast_homogenous SOURCE_LOCATION_PARAM);

    ///**************************************************************************///
    // Create values

    std::optional<Expr *> getDefaultValue(Type *_for SOURCE_LOCATION_PARAM);

    ///**************************************************************************///
    // Create types

    U1Ty *getU1Ty(SOURCE_LOCATION_PARAM_ONCE);
    U8Ty *getU8Ty(SOURCE_LOCATION_PARAM_ONCE);
    U16Ty *getU16Ty(SOURCE_LOCATION_PARAM_ONCE);
    U32Ty *getU32Ty(SOURCE_LOCATION_PARAM_ONCE);
    U64Ty *getU64Ty(SOURCE_LOCATION_PARAM_ONCE);
    U128Ty *getU128Ty(SOURCE_LOCATION_PARAM_ONCE);
    I8Ty *getI8Ty(SOURCE_LOCATION_PARAM_ONCE);
    I16Ty *getI16Ty(SOURCE_LOCATION_PARAM_ONCE);
    I32Ty *getI32Ty(SOURCE_LOCATION_PARAM_ONCE);
    I64Ty *getI64Ty(SOURCE_LOCATION_PARAM_ONCE);
    I128Ty *getI128Ty(SOURCE_LOCATION_PARAM_ONCE);
    F16Ty *getF16Ty(SOURCE_LOCATION_PARAM_ONCE);
    F32Ty *getF32Ty(SOURCE_LOCATION_PARAM_ONCE);
    F64Ty *getF64Ty(SOURCE_LOCATION_PARAM_ONCE);
    F128Ty *getF128Ty(SOURCE_LOCATION_PARAM_ONCE);
    VoidTy *getVoidTy(SOURCE_LOCATION_PARAM_ONCE);

    /* Type inference unknowns; Converted to proper type upon resolution */
    OpaqueTy *getUnknownTy(SOURCE_LOCATION_PARAM_ONCE);

    Type *getUnknownNamedTy(std::string_view name SOURCE_LOCATION_PARAM);

    PtrTy *getPtrTy(Type *pointee SOURCE_LOCATION_PARAM);

    OpaqueTy *getOpaqueTy(std::string_view name SOURCE_LOCATION_PARAM);

    StructTy *getStructTy(
        std::span<std::tuple<std::string_view, Type *, Expr *>> fields
            SOURCE_LOCATION_PARAM);

    StructTy *getStructTy(std::span<Type *> fields SOURCE_LOCATION_PARAM);

    UnionTy *getUnionTy(std::span<Type *> fields SOURCE_LOCATION_PARAM);

    ArrayTy *getArrayTy(Type *element_ty, size_t count SOURCE_LOCATION_PARAM);

    FnTy *getFnTy(std::span<Type *> params, Type *ret_ty,
                  bool is_variadic = false, Purity purity = Purity::Impure,
                  bool thread_safe = false,
                  bool foreign = true SOURCE_LOCATION_PARAM);

    void createNamedConstantDefinition(
        std::string_view name,
        const std::unordered_map<std::string_view, Expr *> &values
            SOURCE_LOCATION_PARAM);

    void createNamedTypeAlias(Type *type,
                              std::string_view name SOURCE_LOCATION_PARAM);

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
#define SOURCE_LOCATION_PARAM_ONCE \
  std::experimental::source_location caller_info
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
