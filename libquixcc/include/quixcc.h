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

#define QUIXCC_VERSION "0.1.0"
#define QUIXCC_VERSION_MAJOR 0
#define QUIXCC_VERSION_MINOR 1
#define QUIXCC_VERSION_PATCH 0

#define QUIXCC_OK 0

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

        /// @brief Message level
        enum quixcc_msg_level_t m_level;

        /// @brief Is the message allocated dynamically?
        bool m_allocated;
    };

    struct quixcc_feedback_t
    {
        /// @brief Messages
        struct quixcc_msg_t **m_messages;

        /// @brief Message count
        uint32_t m_count;
    };

    struct quixcc_result_t
    {
        /// @brief Feedback
        struct quixcc_feedback_t m_feedback;

        /// @brief Internal flags
        uint32_t m_internal_flags;
    };

    typedef struct quixcc_option_t
    {
        /// @brief Identifier
        const char *opt;
    } quixcc_option_t;

    typedef struct quixcc_options_t
    {
        /// @brief Options
        quixcc_option_t **m_options;

        /// @brief Option count
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

    /// @brief Options
    quixcc_options_t m_options;

    /// @brief Result
    quixcc_result_t *m_result;

    FILE *m_in;
    FILE *m_out;

    const char *m_filename;

    /// @brief Job priority
    uint8_t m_priority;

    /// @brief Internal flags
    uint32_t m_internal_flags;

    /// @brief Internal data
    // void *m_inner;
    std::shared_ptr<libquixcc::LLVMContext> m_inner;

    std::map<std::string, std::string> *m_argset;

    /// @brief User may set to true to enable debug messages
    bool m_debug;

    bool m_tainted;
} quixcc_job_t;

extern "C"
{
#else
typedef struct quixcc_job_t quixcc_job_t;
#endif

    /// @brief Create a new compiler job
    /// @return A new compiler job
    /// @note The caller is responsible for disposing the structure
    /// @note The returned structure is in a valid state
    /// @note The returned structure is allocated dynamically
    /// @note This function is thread-safe
    quixcc_job_t *quixcc_new();

    /// @brief Dispose a compiler job
    /// @param job The compiler job
    /// @note The job is disposed and ALL associated resources are released
    /// @note This function is thread-safe
    /// @note This function is safe to call with NULL
    /// @note This function will return false if the job is in use
    /// @return True if the job was disposed
    bool quixcc_dispose(quixcc_job_t *job);

    /// @brief Add an option to a compiler job
    /// @param job The compiler job
    /// @param option The option
    /// @param enabled Is the option enabled?
    /// @note This function is thread-safe
    /// @note This function is safe to call with NULL
    void quixcc_add_option(quixcc_job_t *job, const char *option, bool enabled);

    /// @brief Remove an option from a compiler job
    /// @param job The compiler job
    /// @param opt The option
    /// @note This function is thread-safe
    /// @note This function is safe to call with NULL
    void quixcc_remove_option(quixcc_job_t *job, const char *opt);

    /// @brief Set the input stream for a compiler job
    /// @param job The compiler job
    /// @param in The input stream
    /// @param filename The input filename (required for error messages)
    /// @note This function is thread-safe
    /// @note This function is safe to call with NULL
    void quixcc_set_input(quixcc_job_t *job, FILE *in, const char *filename);

    /// @brief Set the output stream for a compiler job
    /// @param job The compiler job
    /// @param out The output stream
    /// @note This function is thread-safe
    /// @note This function is safe to call with NULL
    void quixcc_set_output(quixcc_job_t *job, FILE *out);

    /// @brief Run a compiler job
    /// @param job The compiler job
    /// @return The result of the job
    /// @note This function is thread-safe
    /// @note This function is safe to call with NULL
    /// @return true if the job completed successfully (the output stream is usable and correct).
    /// @note Use `quixcc_status()` for a detailed result
    bool quixcc_run(quixcc_job_t *job);

    /// @brief Get the result of a compiler job
    /// @param job The compiler job
    /// @return The result of the job
    /// @note This function is thread-safe
    /// @note This function is safe to call with NULL
    /// @note This function will return NULL if the job is still running
    const quixcc_result_t *quixcc_result(quixcc_job_t *job);

    /// @brief One shot compile
    /// @param in The input stream
    /// @param out The output stream
    /// @param options The compiler options
    /// @return NULL if successful, otherwise all messages concatenated with newlines
    /// @note This function is thread-safe
    /// @note Make sure to free the returned string
    char *quixcc_compile(FILE *in, FILE *out, const char *options[]);

    ///===================================================================================================
    /// BEGIN: LANGUAGE stuff
    ///===================================================================================================

    /// @brief Demangle a mangled symbol name into JSON.
    /// @param mangled The mangled symbol name.
    /// @return malloc'd JSON string.
    char *quixcc_demangle_symbol(const char *mangled);

    ///===================================================================================================
    /// END: LANGUAGE stuff
    ///===================================================================================================

#ifdef __cplusplus
}
#endif

#endif // __QUIXCC_H__