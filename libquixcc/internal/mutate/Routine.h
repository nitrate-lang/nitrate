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
///     * Copyright (c) 2024, Wesley C. Jones. All rights reserved.              ///
///     * License terms may be found in the LICENSE file.                        ///
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
        void DiscoverNamedConstructs(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast);
        void ResolveNamedConstructs(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast);
        inline void ReplaceNamedTypes(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast) {}
        inline void FillInConstants(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast) {}
        void FoldConstExpr(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast);
        inline void ConvertTypes(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast) {}
        inline void InferTypes(quixcc_job_t *job, const std::shared_ptr<libquixcc::AST> ast) {}
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