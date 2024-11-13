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

NRBuilder &NRBuilder::insertAfter(Expr *last SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(last != nullptr);

  if (m_state == SelfState::FailEarly) {
    return *this;
  }

  /// TODO: Implement
  qcore_implement();

  return *this;
}

NRBuilder &NRBuilder::insertAfterVariable(std::string_view name SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(!name.empty());

  if (m_state == SelfState::FailEarly) {
    return *this;
  }

  /// TODO: Implement
  qcore_implement();

  return *this;
}

NRBuilder &NRBuilder::insertAfterFunction(std::string_view name SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(!name.empty());

  if (m_state == SelfState::FailEarly) {
    return *this;
  }

  /// TODO: Implement
  qcore_implement();

  return *this;
}

NRBuilder &NRBuilder::insertBefore(Expr *last SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(last != nullptr);

  if (m_state == SelfState::FailEarly) {
    return *this;
  }

  /// TODO: Implement
  qcore_implement();

  return *this;
}

NRBuilder &NRBuilder::insertBeforeVariable(std::string_view name SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(!name.empty());

  if (m_state == SelfState::FailEarly) {
    return *this;
  }

  /// TODO: Implement
  qcore_implement();

  return *this;
}

NRBuilder &NRBuilder::insertBeforeFunction(std::string_view name SOURCE_LOCATION_PARAM) noexcept {
  contract_enforce(m_state == SelfState::Constructed || m_state == SelfState::FailEarly);
  contract_enforce(!name.empty());

  if (m_state == SelfState::FailEarly) {
    return *this;
  }

  /// TODO: Implement
  qcore_implement();

  return *this;
}

///=============================================================================

std::optional<Local *> NRBuilder::lookup_global(std::string_view global) noexcept {
  /// TODO: Implement
  qcore_implement();
  (void)global;
}

std::optional<Local *> NRBuilder::lookup_local(std::string_view local) noexcept {
  /// TODO: Implement
  qcore_implement();
  (void)local;
}

std::optional<Fn *> NRBuilder::lookup_function(std::string_view function) noexcept {
  /// TODO: Implement
  qcore_implement();
  (void)function;
}
