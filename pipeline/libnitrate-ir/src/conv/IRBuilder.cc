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

#include <span>

#include "nitrate-core/Error.h"
#define IRBUILDER_IMPL

#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>

using namespace nr;

NRBuilder::NRBuilder(qlex_t &lexer_instance,
                     TargetInfo target_info SOURCE_LOCATION_PARAM) noexcept {
  ignore_caller_info();

  m_lex = &lexer_instance;
  m_target_info = target_info;

  m_state = SelfState::Constructed;
  m_result = std::nullopt;
  m_root = nullptr;

  m_current_scope = nullptr;
  m_current_function = std::nullopt;
  m_current_expr = std::nullopt;

  m_root = create<Seq>(SeqItems());
  m_current_scope = m_root;
}

NRBuilder::~NRBuilder() noexcept {
  m_state = SelfState::Destroyed;
  m_result = std::nullopt;
  m_root = nullptr;

  m_current_scope = nullptr;
  m_current_function = std::nullopt;
  m_current_expr = std::nullopt;
}

NRBuilder &NRBuilder::operator=(NRBuilder &&rhs) noexcept {
  this->m_state = std::move(rhs.m_state);
  this->m_result = std::move(rhs.m_result);
  this->m_root = std::move(rhs.m_root);

  this->m_current_scope = rhs.m_current_scope;
  this->m_current_function = rhs.m_current_function;
  this->m_current_expr = rhs.m_current_expr;

  rhs.m_state = SelfState::Destroyed;
  rhs.m_result = std::nullopt;
  rhs.m_root = nullptr;

  rhs.m_current_scope = nullptr;
  rhs.m_current_function = std::nullopt;
  rhs.m_current_expr = std::nullopt;

  return *this;
}

NRBuilder::NRBuilder(NRBuilder &&rhs) noexcept {
  this->m_state = std::move(rhs.m_state);
  this->m_result = std::move(rhs.m_result);
  this->m_root = std::move(rhs.m_root);

  this->m_current_scope = rhs.m_current_scope;
  this->m_current_function = rhs.m_current_function;
  this->m_current_expr = rhs.m_current_expr;

  rhs.m_state = SelfState::Destroyed;
  rhs.m_result = std::nullopt;
  rhs.m_root = nullptr;

  rhs.m_current_scope = nullptr;
  rhs.m_current_function = std::nullopt;
  rhs.m_current_expr = std::nullopt;
}

void NRBuilder::contract_enforce_(bool cond, std::string_view cond_str SOURCE_LOCATION_PARAM,
                                  std::experimental::source_location caller) const noexcept {
  if (cond) [[likely]] {
    return;
  }

#ifdef CALLEE_KNOWN

  qcore_panicf_(
      "IRBuilder contract violation:\n"
      "-----------------------------\n"
      "Condition: (%s);\n\n"

      "User File: %s\n"
      "User Line: %d\n"
      "User Fn: %s\n\n"

      "Lib File: %s\n"
      "Lib Line: %d\n"
      "Lib Fn: %s\n\n"

      "Errno: %s\n",

      cond_str.data(),  // Preprocessor stringification of the predicate; the string_view is always
                        // null terminated in this case.

      caller_info.file_name(),      // Original source file that invoked the external API
      caller_info.line(),           // Original source line that invoked the external API
      caller_info.function_name(),  // Original source function that invoked the external API

      caller.file_name(),      // Library source file that triggered the contract enforcement
      caller.line(),           // Library source line that triggered the contract enforcement
      caller.function_name(),  // Library source function that triggered the contract enforcement

      strerror((*__errno_location())));

#else

  qcore_panicf_(
      "IRBuilder contract violation:\n"
      "-----------------------------\n"
      "Condition: (%s);\n\n"

      "Lib File: %s\n"
      "Lib Line: %d\n"
      "Lib Fn: %s\n\n"

      "Errno: %s\n",

      cond_str.data(),  // Preprocessor stringification of the predicate; the string_view is always
                        // null terminated in this case.

      caller.file_name(),      // Library source file that triggered the contract enforcement
      caller.line(),           // Library source line that triggered the contract enforcement
      caller.function_name(),  // Library source function that triggered the contract enforcement

      strerror((*__errno_location())));

#endif
}

NRBuilder NRBuilder::deep_clone(SOURCE_LOCATION_PARAM_ONCE) const noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::Finished ||
                   m_state == SelfState::Verified || m_state == SelfState::Emitted ||
                   m_state == SelfState::FailEarly || m_state == SelfState::Destroyed);

  NRBuilder r(*m_lex, m_target_info);

  r.m_state = SelfState::Destroyed;

  if (m_state == SelfState::Destroyed) {
    contract_enforce(m_result == std::nullopt);
    contract_enforce(m_root == nullptr);
    contract_enforce(m_current_scope == nullptr);
    contract_enforce(m_current_function == std::nullopt);
    contract_enforce(m_current_expr == std::nullopt);
  } else {
    /// TODO: Implement deep clone and resolve state variables to cloned counterparts
    qcore_implement(__func__);
  }

  return r;
}

size_t NRBuilder::approx_memory_usage(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::Finished ||
                   m_state == SelfState::Verified || m_state == SelfState::Emitted ||
                   m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  size_t lower_bound = 0;

  { /* Only nodes reachable from root are counted */
    Expr *expr_ptr = m_root;
    iterate<dfs_pre>(expr_ptr, [&lower_bound](Expr *, Expr **C) -> IterOp {
      lower_bound += Expr::getKindSize((*C)->getKind());
      /// TODO: Take into account dynamic data!

      return IterOp::Proceed;
    });
  }

  return lower_bound;
}

size_t NRBuilder::node_count(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::Finished ||
                   m_state == SelfState::Verified || m_state == SelfState::Emitted ||
                   m_state == SelfState::FailEarly);
  contract_enforce(m_root != nullptr);

  if (m_state == SelfState::FailEarly) {
    return 1;
  }

  size_t count = 0;

  { /* Only nodes reachable from root are counted */
    Expr *expr_ptr = m_root;
    iterate<dfs_pre>(expr_ptr, [&count](Expr *, Expr **) -> IterOp {
      count++;
      return IterOp::Proceed;
    });
  }

  return count;
}

void NRBuilder::finish(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::Finished ||
                   m_state == SelfState::FailEarly);
  contract_enforce(m_result == std::nullopt);
  contract_enforce(m_root != nullptr);

  if (m_state == SelfState::FailEarly) {
    return;
  }

  m_state = SelfState::Finished;

  m_current_scope = nullptr;
  m_current_function = std::nullopt;
  m_current_expr = std::nullopt;
}

bool NRBuilder::verify(diag::IDiagnosticEngine *sink SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Finished || m_state == SelfState::Verified ||
                   m_state == SelfState::FailEarly);
  contract_enforce(m_result == std::nullopt);
  contract_enforce(m_root != nullptr);
  contract_enforce(m_current_scope == nullptr);
  contract_enforce(m_current_function == std::nullopt);
  contract_enforce(m_current_expr == std::nullopt);

  if (m_state == SelfState::FailEarly) {
    return false;
  } else if (m_state == SelfState::Verified) {
    return true;
  }

  bool is_valid = true;

  /// TODO: Implement verification logic
  (void)sink;
  qcore_implement(__func__);

  m_state = SelfState::Verified;

  return is_valid;
}

qmodule_t *NRBuilder::get_module(SOURCE_LOCATION_PARAM_ONCE) noexcept {
  contract_enforce(m_state == SelfState::Verified || m_state == SelfState::Emitted);
  contract_enforce(m_result != std::nullopt);
  contract_enforce(m_root != nullptr);
  contract_enforce(m_current_scope == nullptr);
  contract_enforce(m_current_function == std::nullopt);
  contract_enforce(m_current_expr == std::nullopt);

  m_state = SelfState::Emitted;

  return m_result.value();
}
