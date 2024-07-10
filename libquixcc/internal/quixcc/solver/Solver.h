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

#ifndef __QUIXCC_SOLVER_SOLVER_H__
#define __QUIXCC_SOLVER_SOLVER_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/IR/Q/QIR.h>
#include <quixcc/optimizer/Optimizer.h>

#include <boost/uuid/uuid.hpp>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace libquixcc {
namespace solver {
typedef bool (*SolPassFunc)(quixcc_job_t &job,
                            std::unique_ptr<ir::q::QModule> &ir);

struct SolPassVersion {
  uint16_t major;
  uint16_t minor;
  uint32_t patch;

  SolPassVersion(uint16_t major, uint16_t minor, uint32_t patch)
      : major(major), minor(minor), patch(patch) {}
};

typedef boost::uuids::uuid SolPassUUID;

class SolPass {
  SolPassUUID m_uuid;
  std::string m_name;
  std::string m_description;
  std::string m_author;
  SolPassVersion m_version;
  std::vector<SolPassUUID> m_prepass_dependencies;
  std::vector<SolPassUUID> m_postpass_dependencies;
  std::vector<SolPassUUID> m_prepass_suggests;
  std::vector<SolPassUUID> m_postpass_suggests;
  SolPassFunc m_func;

  void derive_uuid();

 public:
  SolPass(std::string name, std::string description, SolPassFunc func,
          std::string author = "unknown", SolPassVersion version = {1, 0, 0},
          std::vector<SolPassUUID> prepass_dependencies = {},
          std::vector<SolPassUUID> postpass_dependencies = {},
          std::vector<SolPassUUID> prepass_suggests = {},
          std::vector<SolPassUUID> postpass_suggests = {})
      : m_name(name),
        m_description(description),
        m_author(author),
        m_version(version),
        m_prepass_dependencies(prepass_dependencies),
        m_postpass_dependencies(postpass_dependencies),
        m_prepass_suggests(prepass_suggests),
        m_postpass_suggests(postpass_suggests),
        m_func(func) {
    derive_uuid();
  }

  inline const SolPassUUID &uuid() const { return m_uuid; }
  inline const std::string &name() const { return m_name; }
  inline const std::string &description() const { return m_description; }
  inline const std::string &author() const { return m_author; }
  inline const SolPassVersion &version() const { return m_version; }
  inline const std::vector<SolPassUUID> &prepass_dependencies() const {
    return m_prepass_dependencies;
  }
  inline const std::vector<SolPassUUID> &postpass_dependencies() const {
    return m_postpass_dependencies;
  }
  inline const std::vector<SolPassUUID> &prepass_suggests() const {
    return m_prepass_suggests;
  }
  inline const std::vector<SolPassUUID> &postpass_suggests() const {
    return m_postpass_suggests;
  }

  bool operator()(quixcc_job_t &job, std::unique_ptr<ir::q::QModule> &ir) {
    return m_func(job, ir);
  }

  bool operator==(const SolPass &other) const { return m_uuid == other.m_uuid; }
};

class SolPassRegistry {
  std::vector<SolPass> m_passes;

 public:
  SolPassRegistry &register_pass(const SolPass &pass);
  SolPassRegistry &unregister_pass(const SolPass &info);

  SolPass &get_pass_by_uuid(const SolPassUUID &uuid);
  SolPass &get_pass_by_name(std::string_view name);

  static std::shared_ptr<SolPassRegistry> GetBuiltinRegistry();
};

class SolPassManager {
  std::vector<SolPassUUID> m_phase;
  const std::shared_ptr<SolPassRegistry> m_registry;

 public:
  SolPassManager(std::shared_ptr<SolPassRegistry> registry)
      : m_registry(registry) {}

  SolPassManager &append_pass(std::string_view name);
  SolPassManager &prepend_pass(std::string_view name);
  SolPassManager &add_after(std::string_view name, std::string_view after);
  SolPassManager &add_before(std::string_view name, std::string_view before);
  SolPassManager &remove_pass(std::string_view name);
  SolPassManager &clear_passes();

  SolPassManager &dump(std::ostream &os = std::cout);
  std::vector<SolPassUUID> &current_phase() { return m_phase; }
  bool verify_phase_order();
  void optimize_phase_order();
  bool run_passes(quixcc_job_t &job, std::unique_ptr<ir::q::QModule> &ir);
};

class SolPassMgrFactory {
 public:
  static std::unique_ptr<SolPassManager> CreateStandard();
};

}  // namespace solver
}  // namespace libquixcc

#endif  // __QUIXCC_SOLVER_SOLVER_H__