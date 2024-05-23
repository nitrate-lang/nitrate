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
        QUIXCC_RAW = 0,
        QUIXCC_DEBUG = 1,
        QUIXCC_SUCCESS = 2,
        QUIXCC_INFO = 3,
        QUIXCC_WARN = 4,
        QUIXCC_ERROR = 5,
        QUIXCC_FATAL = 6
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

    /// @brief Opaque compiler job context
    /// @note It is opaque for a reason, treat it with respect.
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

    typedef enum
    {
        QUIXCC_LEX_EOF,
        QUIXCC_LEX_UNK,
        QUIXCC_LEX_ERROR = -1,

        QUIXCC_LEX_IDENT,
        QUIXCC_LEX_KW,
        QUIXCC_LEX_OP,
        QUIXCC_LEX_PUNCT,
        QUIXCC_LEX_INT,
        QUIXCC_LEX_FLOAT,
        QUIXCC_LEX_STR,
        QUIXCC_LEX_CHAR,
        QUIXCC_LEX_METABLK,
        QUIXCC_LEX_METASEG,
        QUIXCC_LEX_NOTE,
    } quixcc_lex_type_t;

    typedef enum
    {
        QUIXCC_KW_SUBSYSTEM,
        QUIXCC_KW_IMPORT,
        QUIXCC_KW_TYPE,
        QUIXCC_KW_LET,
        QUIXCC_KW_VAR,
        QUIXCC_KW_CONST,
        QUIXCC_KW_STATIC,
        QUIXCC_KW_STRUCT,
        QUIXCC_KW_REGION,
        QUIXCC_KW_GROUP,
        QUIXCC_KW_UNION,
        QUIXCC_KW_OPAQUE,
        QUIXCC_KW_FN,
        QUIXCC_KW_NOTHROW,
        QUIXCC_KW_FOREIGN,
        QUIXCC_KW_IMPURE,
        QUIXCC_KW_TSAFE,
        QUIXCC_KW_ENUM,
        QUIXCC_KW_PUB,
        QUIXCC_KW_IF,
        QUIXCC_KW_ELSE,
        QUIXCC_KW_FOR,
        QUIXCC_KW_WHILE,
        QUIXCC_KW_DO,
        QUIXCC_KW_SWITCH,
        QUIXCC_KW_CASE,
        QUIXCC_KW_DEFAULT,
        QUIXCC_KW_BREAK,
        QUIXCC_KW_CONTINUE,
        QUIXCC_KW_RETURN,
        QUIXCC_KW_RETIF,
        QUIXCC_KW_RETZ,
        QUIXCC_KW_RETV,
        QUIXCC_KW_ASM,
        QUIXCC_KW_VOID,
        QUIXCC_KW_UNDEF,
        QUIXCC_KW_NULL,
        QUIXCC_KW_TRUE,
        QUIXCC_KW_FALSE,
    } quixcc_lex_kw_t;

    typedef enum
    {
        QUIXCC_PUNCT_OPEN_PAREN,
        QUIXCC_PUNCT_CLOSE_PAREN,
        QUIXCC_PUNCT_OPEN_BRACE,
        QUIXCC_PUNCT_CLOSE_BRACE,
        QUIXCC_PUNCT_OPEN_BRACKET,
        QUIXCC_PUNCT_CLOSE_BRACKET,
        QUIXCC_PUNCT_DOT,
        QUIXCC_PUNCT_COMMA,
        QUIXCC_PUNCT_COLON,
        QUIXCC_PUNCT_SEMICOLON,
    } quixcc_lex_punct_t;

    typedef enum
    {
        QUIXCC_OP_AT,
        QUIXCC_OP_TERNARY,
        QUIXCC_OP_ARROW,
        QUIXCC_OP_PLUS,
        QUIXCC_OP_MINUS,
        QUIXCC_OP_MUL,
        QUIXCC_OP_DIV,
        QUIXCC_OP_MOD,
        QUIXCC_OP_BIT_AND,
        QUIXCC_OP_BIT_OR,
        QUIXCC_OP_BIT_XOR,
        QUIXCC_OP_BIT_NOT,
        QUIXCC_OP_SHL,
        QUIXCC_OP_SHR,
        QUIXCC_OP_INC,
        QUIXCC_OP_DEC,
        QUIXCC_OP_ASSIGN,
        QUIXCC_OP_PLUS_ASSIGN,
        QUIXCC_OP_MINUS_ASSIGN,
        QUIXCC_OP_MUL_ASSIGN,
        QUIXCC_OP_DIV_ASSIGN,
        QUIXCC_OP_MOD_ASSIGN,
        QUIXCC_OP_BIT_OR_ASSIGN,
        QUIXCC_OP_BIT_AND_ASSIGN,
        QUIXCC_OP_BIT_XOR_ASSIGN,
        QUIXCC_OP_XOR_ASSIGN,
        QUIXCC_OP_OR_ASSIGN,
        QUIXCC_OP_AND_ASSIGN,
        QUIXCC_OP_SHL_ASSIGN,
        QUIXCC_OP_SHR_ASSIGN,
        QUIXCC_OP_NOT,
        QUIXCC_OP_AND,
        QUIXCC_OP_OR,
        QUIXCC_OP_XOR,
        QUIXCC_OP_LT,
        QUIXCC_OP_GT,
        QUIXCC_OP_LE,
        QUIXCC_OP_GE,
        QUIXCC_OP_EQ,
        QUIXCC_OP_NE,
    } quixcc_lex_op_t;

    typedef uint32_t quixcc_sid_t;

    typedef struct
    {
        uint32_t line;
        uint32_t column : 24;
        quixcc_sid_t voucher;
    } __attribute__((packed)) quixcc_lex_loc_t;

    typedef struct
    {
        quixcc_lex_loc_t loc;
        union
        {
            quixcc_lex_op_t op;
            quixcc_lex_punct_t punct;
            quixcc_lex_kw_t kw;
            quixcc_sid_t voucher;
        } val;
        quixcc_lex_type_t ty : 8;
    } __attribute__((packed)) quixcc_tok_t;

#define QUIXCC_TOK_SIZE sizeof(quixcc_tok_t)

    typedef enum
    {
        QUIXCC_LEXCONF_IGN_WS = 1 << 0,
        QUIXCC_LEXCONF_IGN_COM = 1 << 1,
    } quixcc_lexer_config_t;

    /// @brief Set the lexer configuration for a compiler job.
    /// @param job The compiler job.
    /// @param config The lexer configuration.
    /// @note This function is thread-safe.
    void quixcc_lexconf(quixcc_job_t *job, quixcc_lexer_config_t config);

    /// @brief Get the next token from the lexer.
    /// @param job The compiler job.
    /// @return The next token from the lexer.
    /// @warning This function is not thread-safe on the same job context, but is thread-safe across different job contexts.
    quixcc_tok_t quixcc_next(quixcc_job_t *job);

    /// @brief Peek at the next token from the lexer.
    /// @param job The compiler job.
    /// @return The next token from the lexer.
    /// @warning This function is not thread-safe on the same job context, but is thread-safe across different job contexts.
    quixcc_tok_t quixcc_peek(quixcc_job_t *job);

    /// @brief Get the value of a string given its String ID.
    /// @param job The compiler job.
    /// @param voucher The String ID.
    /// @return The string value or NULL if the voucher does not exist.
    /// @note This function is thread-safe.
    /// @warning The returned string is owned by the job. Its lifetime is tied to the job and the lifetime of the token which created it.
    ///          If the token which created the string is released via `quixcc_tok_release()`, the string will be deallocated and therefore invalid.
    ///          To prevent this, ensure either to never release the token or dispose of the job before releasing the token. Or copy the string.
    /// @note This exists to save memory and decrease the size of the `quixcc_tok_t` structure.
    /// @note This function is very fast and simply indexes into a string table.
    const char *quixcc_getstr(quixcc_job_t *job, quixcc_sid_t voucher);

    /// @brief Check if a token is valid (no error occurred).
    /// @param tok The token to check.
    /// @return true if the token is valid, false otherwise.
    /// @note This function is thread-safe.
    static inline bool quixcc_lex_ok(const quixcc_tok_t *tok) { return tok->ty < QUIXCC_LEX_ERROR; }

    /// @brief Check if a token is of a specific type.
    /// @param tok The token to check.
    /// @param ty The type to check against.
    /// @return true if the token is of the specified type, false otherwise.
    /// @note This function is thread-safe.
    static inline bool quixcc_lex_is(const quixcc_tok_t *tok, quixcc_lex_type_t ty) { return tok->ty == ty; }

    /// @brief Signal to the job that a token's internal string memory (if any) is no longer needed.
    /// @param job The compiler job.
    /// @param tok The token to release.
    /// @note This function is thread-safe.
    void quixcc_tok_release(quixcc_job_t *job, quixcc_tok_t *tok);

    /// @brief Get raw string representation of a token.
    /// @param job The compiler job.
    /// @param tok The token to serialize.
    /// @param buf The buffer to write the string representation to.
    /// @param len The length of the buffer.
    /// @note This function is thread-safe.
    size_t quixcc_tok_serialize(quixcc_job_t *job, const quixcc_tok_t *tok, char *buf, size_t len);

    /// @brief Get the human-readable string representation of a token.
    /// @param job The compiler job.
    /// @param tok The token to serialize.
    /// @return A malloc'd human-readable string representation of the token.
    /// @note This function is thread-safe.
    /// @note The returned string is owned by the caller and must be freed.
    char *quixcc_tok_humanize(quixcc_job_t *job, const quixcc_tok_t *tok);

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
     * @warning Although this will decrease memory usage, it may also
     *          decrease pipeline performance significantly.
     * @note This function will return false if any jobs are still active.
     */
    bool quixcc_cache_reset();

#ifdef __cplusplus
}
#endif

#endif // __QUIXCC_H__