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

#ifndef __QUIXCC_H__
#define __QUIXCC_H__

#include <stdint.h>
#include <stdbool.h>
#include <bits/types/FILE.h>

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

    struct quixcc_feedback_t
    {
        struct quixcc_msg_t **m_messages;
        uint32_t m_count;
    };

    struct quixcc_result_t
    {
        struct quixcc_feedback_t m_feedback;
        bool m_success;
    };

    typedef struct quixcc_options_t
    {
        const char **m_options;
        uint32_t m_count;
    } quixcc_options_t;

    typedef struct quixcc_uuid_t
    {
        uint64_t m_low;
        uint64_t m_high;
    } quixcc_uuid_t;

#if defined(__cplusplus) && defined(QUIXCC_INTERNAL)
}

#include <map>
#include <string>
#include <llvm/LLVMWrapper.h>

typedef struct quixcc_job_t
{
    quixcc_uuid_t m_id;
    quixcc_options_t m_options;
    quixcc_result_t *m_result;
    FILE *m_in;
    FILE *m_out;
    const char *m_filename;
    uint8_t m_priority;
    std::shared_ptr<libquixcc::LLVMContext> m_inner;
    std::map<std::string, std::string> *m_argset;
    bool m_debug;
    bool m_tainted;
} quixcc_job_t;

extern "C"
{
#else
typedef struct quixcc_job_t quixcc_job_t;
#endif

    /**
     * @brief Initialize the QUIX compiler library.
     * 
     * This function initializes the QUIX compiler library and must be called before any other functions.
     * 
     * @return true if the library was initialized successfully.
     * @note This function is thread-safe.
     * @note It is okay to call this function multiple times. All calls after the first are no-ops.
    */
    bool quixcc_init();

    /**
     * @brief Create a new compiler job.
     *
     * This function initializes and allocates memory for a new compiler job structure.
     *
     * @return A pointer to the newly created compiler job.
     *
     * @note The caller is responsible for disposing the structure.
     * @note The returned structure is in a valid state.
     * @note The returned structure is allocated dynamically.
     * @note This function is thread-safe.
     */
    quixcc_job_t *quixcc_new();

    /**
     * @brief Dispose a compiler job.
     *
     * This function releases all associated resources and deallocates memory for the specified compiler job.
     *
     * @param job The compiler job to be disposed.
     * @return true if the job was disposed successfully.
     *
     * @note The job is disposed, and ALL associated resources are released.
     * @note This function is thread-safe.
     * @note This function will return false if the job is in use.
     */
    bool quixcc_dispose(quixcc_job_t *job);

    /**
     * @brief Add an option to a compiler job.
     *
     * This function enables or disables the specified option for the given compiler job.
     *
     * @param job The compiler job.
     * @param option The option to add.
     * @param enabled Is the option enabled?
     *
     * @note This function is thread-safe.
     */
    void quixcc_add_option(quixcc_job_t *job, const char *option, bool enabled);

    /**
     * @brief Remove an option from a compiler job.
     *
     * This function removes the specified option from the given compiler job.
     *
     * @param job The compiler job.
     * @param opt The option to remove.
     *
     * @note This function is thread-safe.
     */
    void quixcc_remove_option(quixcc_job_t *job, const char *opt);

    /**
     * @brief Set the input stream for a compiler job.
     *
     * This function sets the input stream and filename for the given compiler job.
     *
     * @param job The compiler job.
     * @param in The input stream.
     * @param filename The input filename (required for error messages).
     *
     * @note This function is thread-safe.
     */
    void quixcc_set_input(quixcc_job_t *job, FILE *in, const char *filename);

    /**
     * @brief Set the output stream for a compiler job.
     *
     * This function sets the output stream for the given compiler job.
     *
     * @param job The compiler job.
     * @param out The output stream.
     *
     * @note This function is thread-safe.
     */
    void quixcc_set_output(quixcc_job_t *job, FILE *out);

    /**
     * @brief Run a compiler job.
     *
     * This function executes the compiler job and returns true if it completed successfully.
     *
     * @param job The compiler job.
     * @return true if the job completed successfully.
     *
     * @note This function is thread-safe.
     * @note Use `quixcc_status()` for a detailed result.
     */
    bool quixcc_run(quixcc_job_t *job);

    /**
     * @brief Get the result of a compiler job.
     *
     * This function retrieves the result of the specified compiler job.
     *
     * @param job The compiler job.
     * @return The result of the job, or NULL if the job is still running.
     *
     * @note This function is thread-safe.
     */
    const quixcc_result_t *quixcc_result(quixcc_job_t *job);

    /**
     * @brief Perform a one-shot compilation using QUIX.
     *
     * This function compiles input from an input stream, processes it according to specified options,
     * and writes the output to the provided output stream.
     *
     * @param in The input stream containing the source code to be compiled.
     * @param out The output stream where the compiled output will be written.
     * @param options An array of compiler options, terminated by a NULL element.
     * @return NULL if the compilation is successful, otherwise a string containing concatenated error messages.
     *
     * @note This function is thread-safe.
     * @note It is the caller's responsibility to free the returned string if it is not NULL.
     * @warning Debug and non-fatal messages will be discarded.
     * @note Compilation option parsing is handled internally.
     * @warning Ensure that the options array is properly NULL-terminated.
     * @note The FILE handles for input and output streams are owned by the caller.
     *       The caller may need to flush the streams.
     * @note The FILE handles are not required to support seeking.
     *       It is acceptable to use `fmemopen` and `open_memstream` to create the IO streams.
     */
    char *quixcc_compile(FILE *in, FILE *out, const char *options[]);

    ///===================================================================================================
    /// BEGIN: LANGUAGE stuff
    ///===================================================================================================
    /**
     * @brief Demangle a mangled symbol name into JSON.
     *
     * This function converts a mangled symbol name into a demangled JSON string.
     *
     * @param mangled The mangled symbol name to be demangled.
     * @return A dynamically allocated JSON string representing the demangled symbol name.
     *
     * @note The caller is responsible for `free()`ing the memory allocated for the returned JSON string.
     */
    char *quixcc_demangle_symbol(const char *mangled);

    ///===================================================================================================
    /// END: LANGUAGE stuff
    ///===================================================================================================

#ifdef __cplusplus
}
#endif

#endif // __QUIXCC_H__