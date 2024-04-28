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

#ifndef __QUIXCC_H__
#define __QUIXCC_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

    enum quixcc_msg_level_t
    {
        /// @brief Raw message
        QUIXCC_RAW,

        /// @brief Debug message
        QUIXCC_DEBUG,

        /// @brief Success message
        QUIXCC_SUCCESS,

        /// @brief Information message
        QUIXCC_INFO,

        /// @brief Warning message
        QUIXCC_WARN,

        /// @brief Error message
        QUIXCC_ERROR,

        /// @brief Internal compiler error
        QUIXCC_FATAL
    };

    struct quixcc_msg_t
    {
        uint64_t line;
        uint64_t column;
        const char *message;
        enum quixcc_msg_level_t m_level;
    };

    struct quixcc_status_t
    {
        struct quixcc_msg_t **m_messages;
        uint32_t m_count;
        bool m_success;
    };

    typedef struct quixcc_job_t quixcc_job_t;

    /**
     * @brief Initialize the QUIX compiler library.
     *
     * Initializes the QUIX compiler library and must be called before all
     * other QUIX library functions.
     *
     * @return true if the library was initialized successfully.
     * @note This function is thread-safe.
     * @note Initialization more than once is a no-op.
     */
    bool quixcc_init();

    /**
     * @brief Create a new compiler job with default settings.
     *
     * @return A pointer to the newly created compiler job.
     *
     * @note The caller is responsible for disposing of the job using
     *       `quixcc_dispose()`.
     * @note This function is thread-safe.
     */
    quixcc_job_t *quixcc_new();

    /**
     * @brief Dispose a compiler job.
     *
     * Releases all associated resources and deallocates memory for the
     * specified compiler job.
     *
     * @param job The compiler job to be disposed.
     * @return true if the job was disposed successfully.
     *
     * @note The job is disposed, and ALL associated resources are released.
     * @note This function is thread-safe.
     * @note This function will return false if the job is locked.
     * @note If `!job`, this function is a no-op.
     */
    bool quixcc_dispose(quixcc_job_t *job);

    /**
     * @brief Set an option for a compiler job.
     *
     * @param job The compiler job.
     * @param option The option to add.
     * @param enabled Is the option enabled?
     *
     * @note This function is thread-safe.
     * @note The option string is copied internally.
     * @note It is okay to set the same option multiple times.
     *       The last setting will be used.
     * @note If `!job || !option`, this function is a no-op.
     * @note This function will block until the job is unlocked.
     */
    void quixcc_option(quixcc_job_t *job, const char *option, bool enable);

    /**
     * @brief Set the input stream for a compiler job.
     *
     * @param job The compiler job.
     * @param in The input stream.
     * @param filename The input filename (required for error messages).
     *
     * @note This function is thread-safe.
     * @note The filename string is copied internally.
     * @note If `!job || !in || !filename`, this function is a no-op.
     * @note The FILE handle for the input stream is owned by the caller.
     * @warning The caller must ensure that the input stream is open and
     *          readable for the ENTIRE duration of the job.
     * @note This function will block until the job is unlocked.
     */
    void quixcc_source(quixcc_job_t *job, FILE *in, const char *filename);

    /**
     * @brief Set the LLVM Target Triple for a compiler job.
     *
     * @param job The compiler job.
     * @param triple The LLVM Target Triple.
     * @return true if the triple was set successfully. false if the triple
     *         is invalid or unknown.
     * @warning An empty string is a special case and will use the Host Target
     *          Triple returned by `llvm::sys::getDefaultTargetTriple()`.
     * @note This function will validate the triple before setting it and will
     *       check if it is supported.
     * @note It is okay to set the triple multiple times. The last **VALID**
     *       triple will be used.
     * @note If `!job || !triple`, this function is a no-op.
     * @note This function is thread-safe.
     * @note This function will block until the job is unlocked.
     */
    bool quixcc_target(quixcc_job_t *job, const char *llvm_triple);

    /**
     * @brief Set the LLVM Target CPU for a compiler job.
     *
     * @param job The compiler job.
     * @param cpu The LLVM Target CPU.
     * @return true if the CPU was set successfully. false if the CPU
     *         is invalid or unknown.
     * @note It is okay to set the CPU multiple times. The last **VALID**
     *       CPU will be used.
     * @note This function is thread-safe.
     * @note If `!job || !cpu`, this function is a no-op.
     * @warning Currently, the CPU is not validated for correctness or backend
     *          support.
     * @note This function will block until the job is unlocked.
     */
    bool quixcc_cpu(quixcc_job_t *job, const char *cpu);

    /**
     * @brief Set the output stream for a compiler job.
     *
     * @param job The compiler job.
     * @param out The output stream.
     * @param[out] old_out The previous output stream.
     *
     * @note The FILE handle for the output stream is owned by the caller.
     * @note If `!job || !out`, this function is a no-op.
     * @note The caller must ensure that the output stream is open and
     *       writable for the ENTIRE duration of the job.
     * @note If `old_out` is not NULL, the previous output stream will be
     *       returned in it.
     * @note This function is thread-safe.
     * @note This function will block until the job is unlocked.
     */
    void quixcc_output(quixcc_job_t *job, FILE *out, FILE **old_out);

    /**
     * @brief Run a compiler job.
     *
     * @param job The compiler job.
     * @return true if the job completed without errors.
     *
     * @note This function is thread-safe.
     * @note Use `quixcc_status()` for a detailed result.
     * @note If `!job`, this function is a no-op.
     * @note This function will block until the job is unlocked.
     */
    bool quixcc_run(quixcc_job_t *job);

    /**
     * @brief Get the result of a compiler job.
     *
     * This function retrieves the result of the specified compiler job.
     *
     * @param job The compiler job.
     * @return The result of the job, or NULL.
     *
     * @note This function is thread-safe.
     * @note The result is owned by the job and should not be modified.
     * @note If the job is locked, this function will return NULL.
     * @note The result is valid until the job is disposed.
     * @note If `!job`, this function is a no-op.
     */
    const quixcc_status_t *quixcc_status(quixcc_job_t *job);

    /**
     * @brief Perform a one-shot compilation.
     *
     * This function compiles input from an input stream, processes it
     * according to specified options, and writes the output to the
     * provided output stream.
     *
     * @param in The input stream containing the source code to be compiled.
     * @param out The output stream where the compiled output will be written.
     * @param options An array of compiler options, terminated by a NULL
     *                element.
     * @return NULL if the compilation is successful, otherwise a string array
     *         containing status messages.
     *
     * @note This function is thread-safe.
     * @note It is the caller's responsibility to free the returned array
     *       if it is not NULL.
     * @warning Non-error messages will be discarded.
     * @note Compilation option parsing is handled internally.
     * @warning Ensure that the options array is properly NULL-terminated.
     * @note The FILE handles for input and output streams are owned by the
     *       caller.
     * @warning The caller must ensure that the input stream is open and
     *          readable for the ENTIRE duration of the compilation.
     * @warning The caller must ensure that the output stream is open and
     *          writable for the ENTIRE duration of the compilation.
     * @note If `!in || !out`, this function is a no-op and returns NULL.
     */
    char **quixcc_compile(FILE *in, FILE *out, const char *options[]);

    ///===================================================================================================
    /// BEGIN: LANGUAGE STUFF
    ///===================================================================================================

    /**
     * @brief Demangle a mangled symbol name into a JSON representation.
     *
     * @param mangled The mangled symbol name to be demangled.
     * @return A malloc'd JSON string representing the demangled symbol name.
     *
     * @note This function is thread-safe.
     * @note If `!mangled`, this function is a no-op and returns NULL.
     */
    char *quixcc_demangle(const char *mangled);

    ///===================================================================================================
    /// END: LANGUAGE STUFF
    ///===================================================================================================

    /**
     * @brief Reset and free the internal cache memory
     *
     * @brief This function is thread-safe.
     * @return true if the cache was reset successfully. false otherwise.
     *
     * @note This function requires all jobs to be disposed before calling.
     * @note This function is a no-op if any jobs are still in use.
     * @warning Although this will decrease memory usage, it will also
     *          decrease performance significantly.
     */
    bool quixcc_cache_reset();

#ifdef __cplusplus
}
#endif

#endif // __QUIXCC_H__