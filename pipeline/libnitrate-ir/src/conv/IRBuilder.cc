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

#define IRBUILDER_IMPL

#include <cstddef>
#include <nitrate-core/Logger.hh>
#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Report.hh>
#include <string_view>
#include <unordered_set>

using namespace nr;

NRBuilder::NRBuilder(std::string module_name,
                     TargetInfo target_info SOURCE_LOCATION_PARAM) {
  ignore_caller_info();

  m_module_name = module_name;
  m_target_info = target_info;

  m_state = SelfState::Constructed;
  m_result = std::nullopt;
  m_root = nullptr;

  m_root = create<Seq>(SeqItems());

  m_duplicate_functions = std::unordered_set<Fn *>();
  m_duplicate_variables = std::unordered_set<Local *>();
  m_duplicate_named_types = std::unordered_set<std::string_view>();
  m_duplicate_named_constants = std::unordered_set<std::string_view>();
}

NRBuilder::~NRBuilder() {
  m_state = SelfState::Destroyed;
  m_result = std::nullopt;
  m_root = nullptr;
}

NRBuilder &NRBuilder::operator=(NRBuilder &&rhs) {
  this->m_module_name = std::move(rhs.m_module_name);
  this->m_target_info = std::move(rhs.m_target_info);

  this->m_state = std::move(rhs.m_state);
  this->m_result = std::move(rhs.m_result);
  this->m_root = std::move(rhs.m_root);

  rhs.m_state = SelfState::Destroyed;
  rhs.m_result = std::nullopt;
  rhs.m_root = nullptr;

  return *this;
}

NRBuilder::NRBuilder(NRBuilder &&rhs) {
  this->m_module_name = std::move(rhs.m_module_name);
  this->m_target_info = std::move(rhs.m_target_info);

  this->m_state = std::move(rhs.m_state);
  this->m_result = std::move(rhs.m_result);
  this->m_root = std::move(rhs.m_root);

  rhs.m_state = SelfState::Destroyed;
  rhs.m_result = std::nullopt;
  rhs.m_root = nullptr;
}

void NRBuilder::contract_enforce_(
    bool cond, std::string_view cond_str SOURCE_LOCATION_PARAM,
    std::experimental::source_location caller) const {
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

      cond_str.data(),  // Preprocessor stringification of the predicate; the
                        // string_view is always null terminated in this case.

      caller_info
          .file_name(),    // Original source file that invoked the external API
      caller_info.line(),  // Original source line that invoked the external API
      caller_info.function_name(),  // Original source function that invoked the
                                    // external API

      caller.file_name(),  // Library source file that triggered the contract
                           // enforcement
      caller.line(),       // Library source line that triggered the contract
                           // enforcement
      caller.function_name(),  // Library source function that triggered the
                               // contract enforcement

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

      cond_str.data(),  // Preprocessor stringification of the predicate; the
                        // string_view is always null terminated in this case.

      caller.file_name(),  // Library source file that triggered the contract
                           // enforcement
      caller.line(),       // Library source line that triggered the contract
                           // enforcement
      caller.function_name(),  // Library source function that triggered the
                               // contract enforcement

      strerror((*__errno_location())));

#endif
}

NRBuilder NRBuilder::deep_clone(SOURCE_LOCATION_PARAM_ONCE) const {
  contract_enforce(
      m_state == SelfState::Constructed || m_state == SelfState::Finished ||
      m_state == SelfState::Verified || m_state == SelfState::Emitted ||
      m_state == SelfState::Destroyed);

  NRBuilder r(m_module_name, m_target_info);

  r.m_state = SelfState::Destroyed;

  if (m_state == SelfState::Destroyed) {
    contract_enforce(m_result == std::nullopt);
    contract_enforce(m_root == nullptr);
  } else {
    contract_enforce(m_root != nullptr);

    Expr *out_expr = static_cast<Expr *>(nr_clone(m_root));

    contract_enforce(out_expr->getKind() == NR_NODE_SEQ);
    r.m_root = out_expr->as<Seq>();
  }

  return r;
}

size_t NRBuilder::approx_memory_usage(SOURCE_LOCATION_PARAM_ONCE) {
  contract_enforce(
      m_state == SelfState::Constructed || m_state == SelfState::Finished ||
      m_state == SelfState::Verified || m_state == SelfState::Emitted);
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

size_t NRBuilder::node_count(SOURCE_LOCATION_PARAM_ONCE) {
  contract_enforce(
      m_state == SelfState::Constructed || m_state == SelfState::Finished ||
      m_state == SelfState::Verified || m_state == SelfState::Emitted);
  contract_enforce(m_root != nullptr);

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

void NRBuilder::finish(SOURCE_LOCATION_PARAM_ONCE) {
  contract_enforce(m_state == SelfState::Constructed ||
                   m_state == SelfState::Finished);
  contract_enforce(m_result == std::nullopt);
  contract_enforce(m_root != nullptr);

  connect_nodes(m_root);

  m_state = SelfState::Finished;
}

static thread_local class NullLog : public IReport {
public:
  virtual void report(IssueCode, IC, std::vector<std::string_view> = {},
                      std::tuple<uint32_t, uint32_t> = {
                          QLEX_EOFF, QLEX_NOFILE}) override {}

  virtual void erase_reports() override {}

  virtual void stream_reports(
      std::function<void(const ReportData &)>) override{};
} g_null_log;

bool NRBuilder::verify(std::optional<IReport *> the_log SOURCE_LOCATION_PARAM) {
  contract_enforce(m_state == SelfState::Finished ||
                   m_state == SelfState::Verified);
  contract_enforce(m_result == std::nullopt);
  contract_enforce(m_root != nullptr);

  if (m_state == SelfState::Verified) {
    return true;
  }

  if (!the_log.has_value()) {
    the_log = &g_null_log;
  }

  IReport *I = the_log.value();

  if (!check_acyclic(m_root, I)) {
    return false;
  }

  bool ok = true;

  ok &= check_duplicates(m_root, I);
  ok &= check_symbols_exist(m_root, I);
  ok &= check_function_calls(m_root, I);
  ok &= check_returns(m_root, I);
  ok &= check_scopes(m_root, I);
  ok &= check_mutability(m_root, I);
  ok &= check_control_flow(m_root, I);
  ok &= check_types(m_root, I);
  ok &= check_safety_claims(m_root, I);

  if (!ok) {
    return false;
  }

  m_state = SelfState::Verified;

  contract_enforce(m_state == SelfState::Verified);
  contract_enforce(m_result == std::nullopt);
  contract_enforce(m_root != nullptr);

  return true;
}

qmodule_t *NRBuilder::get_module(SOURCE_LOCATION_PARAM_ONCE) {
  contract_enforce(m_state == SelfState::Verified ||
                   m_state == SelfState::Emitted);
  contract_enforce(m_root != nullptr);

  if (m_state == SelfState::Emitted) {
    contract_enforce(m_result != std::nullopt);
    return m_result.value();
  } else {
    contract_enforce(m_result == std::nullopt);

    qmodule_t *new_mod = createModule(m_module_name);

    { /* Clone the IRGraph into the module */
      std::swap(nr::nr_arena.get(), new_mod->getNodeArena());
      new_mod->setRoot(static_cast<Seq *>(nr_clone(m_root)));
      std::swap(nr::nr_arena.get(), new_mod->getNodeArena());
    }

    m_result = new_mod;

    m_state = SelfState::Emitted;

    return m_result.value();
  }
}

void NRBuilder::appendToRoot(Expr *root SOURCE_LOCATION_PARAM) {
  contract_enforce(m_state == SelfState::Constructed);
  contract_enforce(m_root != nullptr);

  m_root->getItems().push_back(root);
}
