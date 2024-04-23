////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (C) 2020-2024 Wesley C. Jones                                ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_MUTATE_ROUTINES_H__
#define __QUIXCC_MUTATE_ROUTINES_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <memory>
#include <functional>
#include <vector>
#include <parse/Parser.h>
#include <quixcc.h>

namespace libquixcc
{
    namespace mutate
    {
        void StripUnderscoreNames(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast);
        void DiscoverNamedConstructs(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast);
        void ExtrapolateEnumFields(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast);
        void ResolveNamedConstructs(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast);
        void FoldConstExpr(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast);
        void ConvertTypes(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast);
        inline void InferTypes(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast) {}
        void MethodToFunc(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast);
        inline void ObjectConstruction(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast) {}
        inline void ObjectDestruction(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast) {}
        void SubsystemCollapse(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast);
    }

    typedef std::function<void(quixcc_job_t *job, std::shared_ptr<libquixcc::AST>)> ASTMutateRoutine;

    class Mutation
    {
    public:
        /// @brief Run rountines in the order they were added.
        void run(quixcc_job_t *job, std::shared_ptr<libquixcc::AST> ast)
        {
            for (auto routine : m_routines)
            {
                routine(job, ast);
            }
        }

        void add_routine(ASTMutateRoutine routine)
        {
            m_routines.push_back(routine);
        }

        std::vector<ASTMutateRoutine> m_routines;
    };
}

#endif // __QUIXCC_MUTATE_ROUTINES_H__