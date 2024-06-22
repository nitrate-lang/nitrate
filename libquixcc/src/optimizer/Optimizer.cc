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
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <optimizer/Optimizer.h>

#include <algorithm>
#include <boost/uuid/detail/sha1.hpp>
#include <boost/uuid/uuid_hash.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <stdexcept>

using namespace libquixcc::optimizer;

void OptPass::derive_uuid() {
  boost::uuids::detail::sha1 m_hasher;
  m_hasher.process_bytes(m_name.data(), m_name.size());
  m_hasher.process_bytes(&m_version, sizeof(m_version));

  boost::uuids::detail::sha1::digest_type digest;
  m_hasher.get_digest(digest);

  boost::uuids::uuid ret;
  auto p = ret.begin();
  for (std::size_t i{}; i != 4; p += 4, ++i) {
    auto const d = boost::endian::native_to_big(digest[i]);
    std::memcpy(p, &d, 4);
  }
}

OptPassRegistry &OptPassRegistry::register_pass(const OptPass &pass) {
  m_passes.push_back(pass);
  return *this;
}

OptPassRegistry &OptPassRegistry::unregister_pass(const OptPass &info) {
  std::remove(m_passes.begin(), m_passes.end(), info);
  return *this;
}

OptPass &OptPassRegistry::get_pass_by_uuid(const OptPassUUID &uuid) {
  auto it = std::find_if(
      m_passes.begin(), m_passes.end(),
      [&uuid](const OptPass &pass) { return pass.uuid() == uuid; });

  if (it != m_passes.end()) {
    return *it;
  }

  throw std::out_of_range("Optimizer pass not found");
}

OptPass &OptPassRegistry::get_pass_by_name(std::string_view name) {
  auto it = std::find_if(
      m_passes.begin(), m_passes.end(),
      [&name](const OptPass &pass) { return pass.name() == name; });

  if (it != m_passes.end()) {
    return *it;
  }

  throw std::out_of_range("Optimizer pass not found");
}

std::shared_ptr<OptPassRegistry> OptPassRegistry::GetBuiltinRegistry() {
  auto reg = std::make_shared<OptPassRegistry>();

  reg->register_pass(
      OptPass("test-pass", "Test pass",
              [](quixcc_job_t &job, std::unique_ptr<ir::q::QModule> &ir) {
                LOG(INFO) << "Running test pass" << std::endl;
                return true;
              }));

  reg->register_pass(
      OptPass("llvm-reincarnate", "Reincarnate LLVM",
              [](quixcc_job_t &job, std::unique_ptr<ir::q::QModule> &ir) {
                LOG(INFO) << "Reincarnating LLVM" << std::endl;
                return true;
              }));

  return reg;
}

OptPassManager &OptPassManager::append_pass(std::string_view name) {
  auto pass = m_registry->get_pass_by_name(name);
  m_phase.push_back(pass.uuid());
  return *this;
}

OptPassManager &OptPassManager::prepend_pass(std::string_view name) {
  auto pass = m_registry->get_pass_by_name(name);
  m_phase.insert(m_phase.begin(), pass.uuid());
  return *this;
}

OptPassManager &OptPassManager::add_after(std::string_view name,
                                          std::string_view after) {
  auto pass = m_registry->get_pass_by_name(name);
  auto after_pass = m_registry->get_pass_by_name(after);
  auto after_it = std::find(m_phase.begin(), m_phase.end(), after_pass.uuid());
  if (after_it == m_phase.end()) {
    throw std::out_of_range("After pass not found");
  }
  m_phase.insert(after_it + 1, pass.uuid());

  return *this;
}

OptPassManager &OptPassManager::add_before(std::string_view name,
                                           std::string_view before) {
  auto pass = m_registry->get_pass_by_name(name);
  auto before_pass = m_registry->get_pass_by_name(before);
  auto before_it =
      std::find(m_phase.begin(), m_phase.end(), before_pass.uuid());
  if (before_it == m_phase.end()) {
    throw std::out_of_range("Before pass not found");
  }
  m_phase.insert(before_it, pass.uuid());
  return *this;
}

OptPassManager &OptPassManager::remove_pass(std::string_view name) {
  auto pass = m_registry->get_pass_by_name(name);
  auto it = std::find(m_phase.begin(), m_phase.end(), pass.uuid());
  if (it == m_phase.end()) {
    throw std::out_of_range("Optimizer pass not found in phase");
  }
  m_phase.erase(it);
  return *this;
}

OptPassManager &OptPassManager::clear_passes() {
  m_phase.clear();
  return *this;
}

bool OptPassManager::run_passes(
    quixcc_job_t &job, std::unique_ptr<libquixcc::ir::q::QModule> &ir) {
  for (const auto &uuid : m_phase) {
    auto pass = m_registry->get_pass_by_uuid(uuid);

    if (!pass(job, ir)) {
      return false;
    }

    ir->acknowledge_pass(ir::q::QPassType::Optimizer, pass.name());
  }
  return true;
}

OptPassManager &OptPassManager::dump(std::ostream &os) {
  os << "[";
  for (size_t i = 0; i < m_phase.size(); i++) {
    auto pass = m_registry->get_pass_by_uuid(m_phase[i]);
    os << pass.name();

    if (i < m_phase.size() - 1) {
      os << ", ";
    }
  }
  os << "]";
  return *this;
}

bool OptPassManager::verify_phase_order() {
  /// TODO: Get my PhD in compiler theory and implement this,
  ///       or find someone who can.
  return true;
}

void OptPassManager::optimize_phase_order() {
  /// TODO: Get my PhD in compiler theory and implement this,
  ///       or find someone who can.
}

std::unique_ptr<OptPassManager> OptPassMgrFactory::create(OptLevel level,
                                                          OptType type) {
  switch (type) {
    case OptType::General:
      switch (level) {
        case OptLevel::O0:
          return create_gen_0();
        case OptLevel::O1:
          return create_gen_1();
        case OptLevel::O2:
          return create_gen_2();
        case OptLevel::O3:
          return create_gen_3();
        default:
          throw std::invalid_argument("Invalid optimization level");
      }
    case OptType::Behavior:
      switch (level) {
        case OptLevel::O0:
          return create_beh_0();
        case OptLevel::O1:
          return create_beh_1();
        case OptLevel::O2:
          return create_beh_2();
        case OptLevel::O3:
          return create_beh_3();
        default:
          throw std::invalid_argument("Invalid optimization level");
      }
    case OptType::Architecture:
      switch (level) {
        case OptLevel::O0:
          return create_arch_0();
        case OptLevel::O1:
          return create_arch_1();
        case OptLevel::O2:
          return create_arch_2();
        case OptLevel::O3:
          return create_arch_3();
        default:
          throw std::invalid_argument("Invalid optimization level");
      }
    default:
      throw std::invalid_argument("Invalid optimization type");
  }
}

#define PLACEHOLDER                        \
  return std::make_unique<OptPassManager>( \
      OptPassRegistry::GetBuiltinRegistry());

std::unique_ptr<OptPassManager> OptPassMgrFactory::create_arch_0() {
  PLACEHOLDER;
}

std::unique_ptr<OptPassManager> OptPassMgrFactory::create_arch_1() {
  PLACEHOLDER;
}

std::unique_ptr<OptPassManager> OptPassMgrFactory::create_arch_2() {
  PLACEHOLDER;
}

std::unique_ptr<OptPassManager> OptPassMgrFactory::create_arch_3() {
  // PLACEHOLDER;

  auto mgr =
      std::make_unique<OptPassManager>(OptPassRegistry::GetBuiltinRegistry());

  mgr->append_pass("test-pass");
  mgr->add_before("llvm-reincarnate", "test-pass");

  if (!mgr->verify_phase_order()) {
    throw std::runtime_error("Invalid phase order");
  }

  return mgr;
}

std::unique_ptr<OptPassManager> OptPassMgrFactory::create_beh_0() {
  PLACEHOLDER;
}

std::unique_ptr<OptPassManager> OptPassMgrFactory::create_beh_1() {
  PLACEHOLDER;
}

std::unique_ptr<OptPassManager> OptPassMgrFactory::create_beh_2() {
  PLACEHOLDER;
}

std::unique_ptr<OptPassManager> OptPassMgrFactory::create_beh_3() {
  PLACEHOLDER;
}

std::unique_ptr<OptPassManager> OptPassMgrFactory::create_gen_0() {
  PLACEHOLDER;
}

std::unique_ptr<OptPassManager> OptPassMgrFactory::create_gen_1() {
  PLACEHOLDER;
}

std::unique_ptr<OptPassManager> OptPassMgrFactory::create_gen_2() {
  PLACEHOLDER;
}

std::unique_ptr<OptPassManager> OptPassMgrFactory::create_gen_3() {
  PLACEHOLDER;
}

#undef PLACEHOLDER