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

#ifndef __QUIXCC_OPTIMIZER_OPTIMIZER_H__
#define __QUIXCC_OPTIMIZER_OPTIMIZER_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/IR/Q/QIR.h>

#include <boost/uuid/uuid.hpp>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace libquixcc {
namespace optimizer {
typedef bool (*OptPassFunc)(quixcc_job_t &job,
                            std::unique_ptr<ir::q::QModule> &ir);

struct OptPassVersion {
  uint16_t major;
  uint16_t minor;
  uint32_t patch;

  OptPassVersion(uint16_t major, uint16_t minor, uint32_t patch)
      : major(major), minor(minor), patch(patch) {}
};

typedef boost::uuids::uuid OptPassUUID;

class OptPass {
  OptPassUUID m_uuid;
  std::string m_name;
  std::string m_description;
  std::string m_author;
  OptPassVersion m_version;
  std::vector<OptPassUUID> m_prepass_dependencies;
  std::vector<OptPassUUID> m_postpass_dependencies;
  std::vector<OptPassUUID> m_prepass_suggests;
  std::vector<OptPassUUID> m_postpass_suggests;
  OptPassFunc m_func;

  void derive_uuid();

 public:
  OptPass(std::string name, std::string description, OptPassFunc func,
          std::string author = "unknown", OptPassVersion version = {1, 0, 0},
          std::vector<OptPassUUID> prepass_dependencies = {},
          std::vector<OptPassUUID> postpass_dependencies = {},
          std::vector<OptPassUUID> prepass_suggests = {},
          std::vector<OptPassUUID> postpass_suggests = {})
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

  inline const OptPassUUID &uuid() const { return m_uuid; }
  inline const std::string &name() const { return m_name; }
  inline const std::string &description() const { return m_description; }
  inline const std::string &author() const { return m_author; }
  inline const OptPassVersion &version() const { return m_version; }
  inline const std::vector<OptPassUUID> &prepass_dependencies() const {
    return m_prepass_dependencies;
  }
  inline const std::vector<OptPassUUID> &postpass_dependencies() const {
    return m_postpass_dependencies;
  }
  inline const std::vector<OptPassUUID> &prepass_suggests() const {
    return m_prepass_suggests;
  }
  inline const std::vector<OptPassUUID> &postpass_suggests() const {
    return m_postpass_suggests;
  }

  bool operator()(quixcc_job_t &job, std::unique_ptr<ir::q::QModule> &ir) {
    return m_func(job, ir);
  }

  bool operator==(const OptPass &other) const { return m_uuid == other.m_uuid; }
};

class OptPassRegistry {
  std::vector<OptPass> m_passes;

 public:
  OptPassRegistry &register_pass(const OptPass &pass);
  OptPassRegistry &unregister_pass(const OptPass &info);

  OptPass &get_pass_by_uuid(const OptPassUUID &uuid);
  OptPass &get_pass_by_name(std::string_view name);

  static std::shared_ptr<OptPassRegistry> GetBuiltinRegistry();
};

class OptPassManager {
  std::vector<OptPassUUID> m_phase;
  const std::shared_ptr<OptPassRegistry> m_registry;

 public:
  OptPassManager(std::shared_ptr<OptPassRegistry> registry)
      : m_registry(registry) {}

  OptPassManager &append_pass(std::string_view name);
  OptPassManager &prepend_pass(std::string_view name);
  OptPassManager &add_after(std::string_view name, std::string_view after);
  OptPassManager &add_before(std::string_view name, std::string_view before);
  OptPassManager &remove_pass(std::string_view name);
  OptPassManager &clear_passes();

  OptPassManager &dump(std::ostream &os = std::cout);
  std::vector<OptPassUUID> &current_phase() { return m_phase; }
  bool verify_phase_order();
  void optimize_phase_order();
  bool run_passes(quixcc_job_t &job, std::unique_ptr<ir::q::QModule> &ir);
};

enum class OptLevel {
  O0,
  O1,
  O2,
  O3,
  Os,
  Oz,
};

enum class OptType {
  General,
  Behavior,
  Architecture,
};

class OptPassMgrFactory {
  static std::unique_ptr<OptPassManager> create_arch_0();
  static std::unique_ptr<OptPassManager> create_arch_1();
  static std::unique_ptr<OptPassManager> create_arch_2();
  static std::unique_ptr<OptPassManager> create_arch_3();

  static std::unique_ptr<OptPassManager> create_beh_0();
  static std::unique_ptr<OptPassManager> create_beh_1();
  static std::unique_ptr<OptPassManager> create_beh_2();
  static std::unique_ptr<OptPassManager> create_beh_3();

  static std::unique_ptr<OptPassManager> create_gen_0();
  static std::unique_ptr<OptPassManager> create_gen_1();
  static std::unique_ptr<OptPassManager> create_gen_2();
  static std::unique_ptr<OptPassManager> create_gen_3();

 public:
  static std::unique_ptr<OptPassManager> create(OptLevel level, OptType type);
};

}  // namespace optimizer
}  // namespace libquixcc

#endif  // __QUIXCC_OPTIMIZER_OPTIMIZER_H__