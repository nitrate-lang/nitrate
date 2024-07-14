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

#ifndef __QUIXCC_MUTATE_ROUTINES_H__
#define __QUIXCC_MUTATE_ROUTINES_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <quixcc/Library.h>
#include <quixcc/parsetree/Parser.h>

#include <functional>
#include <memory>
#include <vector>

namespace libquixcc {
  namespace mutate {
    void DiscoverNamedConstructs(quixcc_cc_job_t *job,
                                 const std::shared_ptr<libquixcc::Ptree> ptree);
    void ExtrapolateEnumFields(quixcc_cc_job_t *job, const std::shared_ptr<libquixcc::Ptree> ptree);
    void ResolveNamedConstructs(quixcc_cc_job_t *job,
                                const std::shared_ptr<libquixcc::Ptree> ptree);
    void MethodToFunc(quixcc_cc_job_t *job, const std::shared_ptr<libquixcc::Ptree> ptree);
    void SubsystemCollapse(quixcc_cc_job_t *job, const std::shared_ptr<libquixcc::Ptree> ptree);
  } // namespace mutate

  typedef std::function<void(quixcc_cc_job_t *job, std::shared_ptr<libquixcc::Ptree>)>
      PtreeMutateRoutine;

  class Mutation {
public:
    /// @brief Run rountines in the order they were added.
    void run(quixcc_cc_job_t *job, std::shared_ptr<libquixcc::Ptree> ptree) {
      for (auto routine : m_routines) {
        routine(job, ptree);
      }
    }

    void add_routine(PtreeMutateRoutine routine) { m_routines.push_back(routine); }

    std::vector<PtreeMutateRoutine> m_routines;
  };
} // namespace libquixcc

#endif // __QUIXCC_MUTATE_ROUTINES_H__