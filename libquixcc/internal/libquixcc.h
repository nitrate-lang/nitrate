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
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
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

#ifndef __QUIXCC_QUIXCCLIB_H__
#define __QUIXCC_QUIXCCLIB_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <map>
#include <mutex>
#include <atomic>
#include <llvm/LLVMWrapper.h>
#include <preprocessor/Preprocessor.h>
#include <quixcc.h>
#include <unordered_map>

typedef struct quixcc_options_t
{
    const char **m_options;
    uint32_t m_count;
} quixcc_options_t;

typedef struct quixcc_uuid_t
{
    uint8_t data[16];
} quixcc_uuid_t;

#define JOB_MAGIC 0x32b287410bbef790

struct quixcc_job_t
{
    volatile uint64_t m_magic;
    std::unique_ptr<libquixcc::PrepEngine> m_prep;
    libquixcc::LLVMContext m_inner;
    std::map<std::string, std::string> m_argset;
    std::unordered_map<quixcc_sid_t, char *> m_owned_strings;
    quixcc_sid_t m_sid_ctr;
    std::mutex m_lock;
    std::string m_triple;
    std::string m_cpu;
    quixcc_uuid_t m_id;
    quixcc_options_t m_options;
    quixcc_status_t m_result;
    FILE *m_in;
    FILE *m_out;
    char *m_filename;
    uint8_t m_priority;
    bool m_debug;
    bool m_tainted;
    bool m_running;
};

extern std::atomic<uint64_t> g_num_of_contexts;
extern std::mutex g_library_lock;

#endif // __QUIXCC_QUIXCCLIB_H__