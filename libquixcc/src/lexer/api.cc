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

#define QUIXCC_INTERNAL

#include <lexer/Lex.h>
#include <LibMacro.h>
#include <quixcc.h>
#include <stdexcept>

LIB_EXPORT void quixcc_lexconf(quixcc_job_t *job, quixcc_lexer_config_t config)
{
    /// TODO: Implement
    throw std::runtime_error("quixcc_lexconf Not implemented");
}

LIB_EXPORT quixcc_tok_t quixcc_next(quixcc_job_t *job)
{
    /// TODO: Implement
    throw std::runtime_error("quixcc_next Not implemented");
}

LIB_EXPORT quixcc_tok_t quixcc_peek(quixcc_job_t *job)
{
    /// TODO: Implement
    throw std::runtime_error("quixcc_peek Not implemented");
}

LIB_EXPORT const char *quixcc_getstr(quixcc_job_t *job, quixcc_sid_t voucher)
{
    /// TODO: Implement
    throw std::runtime_error("quixcc_getstr Not implemented");
}

LIB_EXPORT void quixcc_tok_release(quixcc_job_t *job, quixcc_tok_t *tok)
{
    /// TODO: Implement
    throw std::runtime_error("quixcc_tok_release Not implemented");
}

LIB_EXPORT size_t quixcc_tok_serialize(quixcc_job_t *job, const quixcc_tok_t *tok, char *buf, size_t len)
{
    /// TODO: Implement
    throw std::runtime_error("quixcc_tok_serialize Not implemented");
}

LIB_EXPORT char *quixcc_tok_humanize(quixcc_job_t *job, const quixcc_tok_t *tok)
{
    /// TODO: Implement
    throw std::runtime_error("quixcc_tok_humanize Not implemented");
}