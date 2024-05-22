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

#ifndef __QUIXCC_LEX_H__
#define __QUIXCC_LEX_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <stdio.h>
#include <vector>
#include <string>
#include <optional>
#include <variant>
#include <deque>
#include <array>
#include <unordered_map>
#include <set>
#include <queue>
#include <lexer/Token.h>

namespace libquixcc
{
    constexpr std::array<char, 10> punctors = {
        '(', ')', '{', '}', '[', ']', '.', ',', ':', ';'};

    extern std::unordered_map<std::string, libquixcc::Keyword> keyword_map;
    extern std::unordered_map<libquixcc::Keyword, std::string> keyword_map_inverse;
    extern std::unordered_map<std::string, libquixcc::Punctor> punctor_map;
    extern std::unordered_map<libquixcc::Punctor, std::string> punctor_map_inverse;
    extern std::unordered_map<std::string, libquixcc::Operator> operator_map;
    extern std::unordered_map<libquixcc::Operator, std::string> operator_map_inverse;

    class Scanner
    {
    public:
        Scanner() = default;
        ~Scanner() = default;

        virtual Token next() = 0;
        virtual Token peek() = 0;

        static std::string escape_string(const std::string &str);

        virtual void push(Token tok) = 0;
    };

    class StreamLexer : public Scanner
    {
    private:
        FILE *m_src = nullptr;

    protected:
        /// @brief C FILE* source. Object is owned by the caller.
        /// @note The caller is responsible for closing the file.
        std::string m_filename;
        std::vector<char> m_buffer;
        std::queue<char> m_pushback;
        std::optional<Token> m_tok;
        size_t m_buf_pos = 0;
        char m_last = 0;
        Loc m_loc_curr;
        Loc m_loc;
        bool added_newline = false;

        virtual char getc();
        virtual libquixcc::Token read_token();

        inline void pushback(char c) { m_pushback.push(c); }

    public:
        StreamLexer();
        virtual ~StreamLexer() = default;

        /// @brief Set the source file
        /// @param src C FILE pointer
        /// @return true if the source file is set successfully
        virtual bool set_source(FILE *src, const std::string &filename);

        Token next() override;
        Token peek() override;

        inline void push(Token tok) override { m_tok = tok; }
    };

    class StringLexer : public StreamLexer
    {
    private:
        std::string m_src;
        FILE *m_file;

    public:
        StringLexer() : m_file(nullptr) {}
        StringLexer(const std::string &source_code) { set_source(source_code, "stringlexer"); }
        virtual ~StringLexer();

        using StreamLexer::set_source;

        /// @brief Set the source file
        /// @param src C FILE pointer
        /// @return true if the source file is set successfully
        bool set_source(const std::string &source_code, const std::string &filename);

        /// @brief Lex the source code
        /// @param source_code
        /// @param tokens
        /// @return Returns false if the source code is invalid
        static bool QuickLex(const std::string &source_code, std::vector<Token> &tokens, const std::string &filename = "quicklex");
    };
};

#endif // __QUIXCC_LEX_H__