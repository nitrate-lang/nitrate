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

/*
    WARNING: WE ARE NOT ALLOWED TO USE THE LOGGER(lvl) SUBSYSTEM IN THIS FILE
*/

#include <lexer/Lex.h>
#include <cstdio>
#include <cctype>
#include <cassert>
#include <stdexcept>
#include <cstring>
#include <iomanip>
#include <unordered_map>
#include <iostream>
#include <mutex>
#include <regex>
#include <cmath>
#include <cache.h>

///=============================================================================

#define FLOATING_POINT_PRECISION 32

namespace libquixcc
{
    const std::unordered_map<std::string_view, libquixcc::Keyword> keyword_map = {
        {"subsystem", libquixcc::Keyword::Subsystem},
        {"import", libquixcc::Keyword::Import},
        {"pub", libquixcc::Keyword::Pub},

        {"type", libquixcc::Keyword::Type},
        {"let", libquixcc::Keyword::Let},
        {"var", libquixcc::Keyword::Var},
        {"const", libquixcc::Keyword::Const},
        {"static", libquixcc::Keyword::Static},

        {"struct", libquixcc::Keyword::Struct},
        {"region", libquixcc::Keyword::Region},
        {"group", libquixcc::Keyword::Group},
        {"union", libquixcc::Keyword::Union},
        {"opaque", libquixcc::Keyword::Opaque},
        {"enum", libquixcc::Keyword::Enum},

        {"fn", libquixcc::Keyword::Fn},
        {"noexcept", libquixcc::Keyword::Noexcept},
        {"foreign", libquixcc::Keyword::Foreign},
        {"impure", libquixcc::Keyword::Impure},
        {"tsafe", libquixcc::Keyword::Tsafe},
        {"pure", libquixcc::Keyword::Pure},
        {"quasipure", libquixcc::Keyword::Quasipure},
        {"retropure", libquixcc::Keyword::Retropure},
        {"crashpoint", libquixcc::Keyword::CrashPoint},
        {"inline", libquixcc::Keyword::Inline},

        {"if", libquixcc::Keyword::If},
        {"else", libquixcc::Keyword::Else},
        {"for", libquixcc::Keyword::For},
        {"while", libquixcc::Keyword::While},
        {"do", libquixcc::Keyword::Do},
        {"switch", libquixcc::Keyword::Switch},
        {"case", libquixcc::Keyword::Case},
        {"default", libquixcc::Keyword::Default},
        {"break", libquixcc::Keyword::Break},
        {"continue", libquixcc::Keyword::Continue},
        {"ret", libquixcc::Keyword::Return},
        {"retif", libquixcc::Keyword::Retif},
        {"retz", libquixcc::Keyword::Retz},
        {"retv", libquixcc::Keyword::Retv},

        {"__asm__", libquixcc::Keyword::__Asm__},

        {"void", libquixcc::Keyword::Void},
        {"undef", libquixcc::Keyword::Undef},
        {"null", libquixcc::Keyword::Null},
        {"true", libquixcc::Keyword::True},
        {"false", libquixcc::Keyword::False}};

    const std::unordered_map<libquixcc::Keyword, std::string_view> keyword_map_inverse = {
        {libquixcc::Keyword::Subsystem, "subsystem"},
        {libquixcc::Keyword::Import, "import"},
        {libquixcc::Keyword::Pub, "pub"},

        {libquixcc::Keyword::Type, "type"},
        {libquixcc::Keyword::Let, "let"},
        {libquixcc::Keyword::Var, "var"},
        {libquixcc::Keyword::Const, "const"},
        {libquixcc::Keyword::Static, "static"},

        {libquixcc::Keyword::Struct, "struct"},
        {libquixcc::Keyword::Region, "region"},
        {libquixcc::Keyword::Group, "group"},
        {libquixcc::Keyword::Union, "union"},
        {libquixcc::Keyword::Opaque, "opaque"},
        {libquixcc::Keyword::Enum, "enum"},

        {libquixcc::Keyword::Fn, "fn"},
        {libquixcc::Keyword::Noexcept, "noexcept"},
        {libquixcc::Keyword::Foreign, "foreign"},
        {libquixcc::Keyword::Impure, "impure"},
        {libquixcc::Keyword::Tsafe, "tsafe"},
        {libquixcc::Keyword::Pure, "pure"},
        {libquixcc::Keyword::Quasipure, "quasipure"},
        {libquixcc::Keyword::Retropure, "retropure"},
        {libquixcc::Keyword::CrashPoint, "crashpoint"},
        {libquixcc::Keyword::Inline, "inline"},

        {libquixcc::Keyword::If, "if"},
        {libquixcc::Keyword::Else, "else"},
        {libquixcc::Keyword::For, "for"},
        {libquixcc::Keyword::While, "while"},
        {libquixcc::Keyword::Do, "do"},
        {libquixcc::Keyword::Switch, "switch"},
        {libquixcc::Keyword::Case, "case"},
        {libquixcc::Keyword::Default, "default"},
        {libquixcc::Keyword::Break, "break"},
        {libquixcc::Keyword::Continue, "continue"},
        {libquixcc::Keyword::Return, "ret"},
        {libquixcc::Keyword::Retif, "retif"},
        {libquixcc::Keyword::Retz, "retz"},
        {libquixcc::Keyword::Retv, "retv"},

        {libquixcc::Keyword::__Asm__, "__asm__"},

        {libquixcc::Keyword::Void, "void"},
        {libquixcc::Keyword::Undef, "undef"},
        {libquixcc::Keyword::Null, "null"},
        {libquixcc::Keyword::True, "true"},
        {libquixcc::Keyword::False, "false"}};

    const std::unordered_map<std::string_view, libquixcc::Punctor> punctor_map = {
        {"(", libquixcc::Punctor::OpenParen},
        {")", libquixcc::Punctor::CloseParen},
        {"{", libquixcc::Punctor::OpenBrace},
        {"}", libquixcc::Punctor::CloseBrace},
        {"[", libquixcc::Punctor::OpenBracket},
        {"]", libquixcc::Punctor::CloseBracket},
        {".", libquixcc::Punctor::Dot},
        {",", libquixcc::Punctor::Comma},
        {":", libquixcc::Punctor::Colon},
        {";", libquixcc::Punctor::Semicolon}};

    const std::unordered_map<libquixcc::Punctor, std::string_view> punctor_map_inverse = {
        {libquixcc::Punctor::OpenParen, "("},
        {libquixcc::Punctor::CloseParen, ")"},
        {libquixcc::Punctor::OpenBrace, "{"},
        {libquixcc::Punctor::CloseBrace, "}"},
        {libquixcc::Punctor::OpenBracket, "["},
        {libquixcc::Punctor::CloseBracket, "]"},
        {libquixcc::Punctor::Dot, "."},
        {libquixcc::Punctor::Comma, ","},
        {libquixcc::Punctor::Colon, ":"},
        {libquixcc::Punctor::Semicolon, ";"}};

    const std::unordered_map<std::string_view, libquixcc::Operator> operator_map = {
        {"<", libquixcc::Operator::LessThan},
        {">", libquixcc::Operator::GreaterThan},
        {"=", libquixcc::Operator::Assign},
        {"@", libquixcc::Operator::At},
        {"=>", libquixcc::Operator::Arrow},
        {"-", libquixcc::Operator::Minus},
        {"+", libquixcc::Operator::Plus},
        {"*", libquixcc::Operator::Multiply},
        {"/", libquixcc::Operator::Divide},
        {"%", libquixcc::Operator::Modulo},
        {"&", libquixcc::Operator::BitwiseAnd},
        {"|", libquixcc::Operator::BitwiseOr},
        {"^", libquixcc::Operator::BitwiseXor},
        {"~", libquixcc::Operator::BitwiseNot},
        {"!", libquixcc::Operator::LogicalNot},
        {"?", libquixcc::Operator::Question},
        {"+=", libquixcc::Operator::PlusAssign},
        {"-=", libquixcc::Operator::MinusAssign},
        {"*=", libquixcc::Operator::MultiplyAssign},
        {"/=", libquixcc::Operator::DivideAssign},
        {"%=", libquixcc::Operator::ModuloAssign},
        {"|=", libquixcc::Operator::BitwiseOrAssign},
        {"&=", libquixcc::Operator::BitwiseAndAssign},
        {"^=", libquixcc::Operator::BitwiseXorAssign},
        {"<<", libquixcc::Operator::LeftShift},
        {">>", libquixcc::Operator::RightShift},
        {"==", libquixcc::Operator::Equal},
        {"!=", libquixcc::Operator::NotEqual},
        {"&&", libquixcc::Operator::LogicalAnd},
        {"||", libquixcc::Operator::LogicalOr},
        {"^^", libquixcc::Operator::LogicalXor},
        {"<=", libquixcc::Operator::LessThanEqual},
        {">=", libquixcc::Operator::GreaterThanEqual},
        {"++", libquixcc::Operator::Increment},
        {"--", libquixcc::Operator::Decrement},
        {"^^=", libquixcc::Operator::XorAssign},
        {"||=", libquixcc::Operator::OrAssign},
        {"&&=", libquixcc::Operator::AndAssign},
        {"<<=", libquixcc::Operator::LeftShiftAssign},
        {">>=", libquixcc::Operator::RightShiftAssign}};

    const std::unordered_map<libquixcc::Operator, std::string_view> operator_map_inverse = {
        {libquixcc::Operator::LessThan, "<"},
        {libquixcc::Operator::GreaterThan, ">"},
        {libquixcc::Operator::Assign, "="},
        {libquixcc::Operator::At, "@"},
        {libquixcc::Operator::Arrow, "=>"},
        {libquixcc::Operator::Minus, "-"},
        {libquixcc::Operator::Plus, "+"},
        {libquixcc::Operator::Multiply, "*"},
        {libquixcc::Operator::Divide, "/"},
        {libquixcc::Operator::Modulo, "%"},
        {libquixcc::Operator::BitwiseAnd, "&"},
        {libquixcc::Operator::BitwiseOr, "|"},
        {libquixcc::Operator::BitwiseXor, "^"},
        {libquixcc::Operator::BitwiseNot, "~"},
        {libquixcc::Operator::LogicalNot, "!"},
        {libquixcc::Operator::Question, "?"},
        {libquixcc::Operator::PlusAssign, "+="},
        {libquixcc::Operator::MinusAssign, "-="},
        {libquixcc::Operator::MultiplyAssign, "*="},
        {libquixcc::Operator::DivideAssign, "/="},
        {libquixcc::Operator::ModuloAssign, "%="},
        {libquixcc::Operator::BitwiseOrAssign, "|="},
        {libquixcc::Operator::BitwiseAndAssign, "&="},
        {libquixcc::Operator::BitwiseXorAssign, "^="},
        {libquixcc::Operator::LeftShift, "<<"},
        {libquixcc::Operator::RightShift, ">>"},
        {libquixcc::Operator::Equal, "=="},
        {libquixcc::Operator::NotEqual, "!="},
        {libquixcc::Operator::LogicalAnd, "&&"},
        {libquixcc::Operator::LogicalOr, "||"},
        {libquixcc::Operator::LogicalXor, "^^"},
        {libquixcc::Operator::LessThanEqual, "<="},
        {libquixcc::Operator::GreaterThanEqual, ">="},
        {libquixcc::Operator::Increment, "++"},
        {libquixcc::Operator::Decrement, "--"},
        {libquixcc::Operator::XorAssign, "^^="},
        {libquixcc::Operator::OrAssign, "||="},
        {libquixcc::Operator::AndAssign, "&&="},
        {libquixcc::Operator::LeftShiftAssign, "<<="},
        {libquixcc::Operator::RightShiftAssign, ">>="}};

    const std::set<char> operator_chars = {'<', '>', '=', '@',
                                           '-', '+', '*', '/',
                                           '%', '&', '|', '^',
                                           '~', '!', '?'};
}

// Precomputed lookup table for hex char to byte conversion
static constexpr std::array<uint8_t, 256> hexLookup = []()
{
    std::array<uint8_t, 256> hexLookup = {};
    hexLookup['0'] = 0;
    hexLookup['1'] = 1;
    hexLookup['2'] = 2;
    hexLookup['3'] = 3;
    hexLookup['4'] = 4;
    hexLookup['5'] = 5;
    hexLookup['6'] = 6;
    hexLookup['7'] = 7;
    hexLookup['8'] = 8;
    hexLookup['9'] = 9;
    hexLookup['A'] = 10;
    hexLookup['B'] = 11;
    hexLookup['C'] = 12;
    hexLookup['D'] = 13;
    hexLookup['E'] = 14;
    hexLookup['F'] = 15;
    hexLookup['a'] = 10;
    hexLookup['b'] = 11;
    hexLookup['c'] = 12;
    hexLookup['d'] = 13;
    hexLookup['e'] = 14;
    hexLookup['f'] = 15;
    return hexLookup;
}();

std::string libquixcc::Scanner::escape_string(std::string_view str)
{
    std::ostringstream output;

    for (char c : str)
    {
        switch (c)
        {
        case '"':
            output << "\\\"";
            break;
        case '\\':
            output << "\\\\";
            break;
        case '/':
            output << "\\/";
            break;
        case '\b':
            output << "\\b";
            break;
        case '\f':
            output << "\\f";
            break;
        case '\n':
            output << "\\n";
            break;
        case '\r':
            output << "\\r";
            break;
        case '\t':
            output << "\\t";
            break;
        default:
            if (std::isprint(c))
            {
                output << c;
            }
            else
            {
                output << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c;
            }
        }
    }

    return output.str();
}

///=============================================================================

thread_local std::map<std::string, std::unique_ptr<char[]>> libquixcc::TLCString::m_data;

libquixcc::StreamLexer::StreamLexer()
{
    m_src = nullptr;
    m_buf_pos = GETC_BUFFER_SIZE;
    m_tok = std::nullopt;
    added_newline = false;
}

char libquixcc::StreamLexer::getc()
{
    if (m_buf_pos > GETC_BUFFER_SIZE) [[unlikely]]
        return EOF;

    /* Fill buffer if it's empty */
    if (m_buf_pos >= GETC_BUFFER_SIZE)
    {
        size_t read = fread(m_buffer.data(), 1, GETC_BUFFER_SIZE, m_src);

        if (read == GETC_BUFFER_SIZE) [[likely]]
        {
            m_buf_pos = 0;
        }
        else if (read < GETC_BUFFER_SIZE)
        {
            memset(m_buffer.data() + read, '\n', GETC_BUFFER_SIZE - read);

            if (read != 0)
            {
                m_buf_pos = 0;
            }
            else
            {
                m_buf_pos = GETC_BUFFER_SIZE + 1;
                return EOF;
            }
        }
    }

    /* Update the current location */
    m_loc = m_loc_curr;

    char c = m_buffer[m_buf_pos++];
    if (c != '\n')
    {
        m_loc_curr.col++;
    }
    else
    {
        m_loc_curr.line++;
        m_loc_curr.col = 1;
    }

    return c;
}

bool libquixcc::StreamLexer::set_source(FILE *src, const std::string &filename)
{
    if (src == nullptr)
        return false;

    /* Test if the file is 'usable' */
    if (fseek(m_src = src, 0, SEEK_SET) != 0)
        return false;

    m_filename = filename;
    m_loc_curr = Loc(1, 1, TLCString::get(filename));

    return true;
}

static bool validate_identifier(std::string_view id)
{
    /*
     * This state machine checks if the identifier looks
     * like 'a::b::c::d_::e::f'.
     */

    int state = 0;

    for (const auto &c : id)
    {
        switch (state)
        {
        case 0:
            if (std::isalnum(c) || c == '_')
                continue;
            if (c == ':')
            {
                state = 1;
                continue;
            }
            return false;
        case 1:
            if (c == ':')
            {
                state = 0;
                continue;
            }
            return false;
        }
    }

    return state == 0;
}

static bool canonicalize_identifier(std::string &str)
{
    /* Canonicalize the identifier */

    /* Create a cache */
    static std::unordered_map<std::string, std::string> g_canonicalize_identifier_cache;
    if (g_canonicalize_identifier_cache.contains(str))
        return str = g_canonicalize_identifier_cache[str], true;

    // Replace all `name::<string>` syntax should be replaced with `name_::string`
    size_t first, last;
    while ((first = str.find('<')) != std::string::npos && (last = str.find('>')) != std::string::npos)
        str.replace(first, last - first + 1, "::_" + str.substr(first + 1, last - first - 1));

    if (!validate_identifier(str))
        return false;

    /* Cache the result */
    return g_canonicalize_identifier_cache[str] = str, true;
}

enum class NumType
{
    Invalid,
    Decimal,
    DecimalExplicit,
    Hexadecimal,
    Binary,
    Octal,
    Floating,
};

static NumType check_number_literal_type(std::string input)
{
    /* Create a cache */
    static std::unordered_map<std::string, NumType> g_check_number_literal_type_cache;
    if (g_check_number_literal_type_cache.contains(input))
        return g_check_number_literal_type_cache[input];

    if (input.empty())
        return g_check_number_literal_type_cache[input] = NumType::Invalid;

    /* Check if it's a single digit */
    if (input.size() < 3)
    {
        if (std::isdigit(input[0]))
            return g_check_number_literal_type_cache[input] = NumType::Decimal;
        else
            return g_check_number_literal_type_cache[input] = NumType::Invalid;
    }

    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
    std::erase(input, '_');

    std::string prefix = input.substr(0, 2);
    size_t i;

    if (prefix == "0x")
    {
        for (i = 2; i < input.size(); i++)
            if (!((input[i] >= '0' && input[i] <= '9') || (input[i] >= 'a' && input[i] <= 'f')))
                return g_check_number_literal_type_cache[input] = NumType::Invalid;

        return g_check_number_literal_type_cache[input] = NumType::Hexadecimal;
    }
    else if (prefix == "0b")
    {
        for (i = 2; i < input.size(); i++)
            if (!(input[i] == '0' || input[i] == '1'))
                return g_check_number_literal_type_cache[input] = NumType::Invalid;

        return g_check_number_literal_type_cache[input] = NumType::Binary;
    }
    else if (prefix == "0o")
    {
        for (i = 2; i < input.size(); i++)
            if (!(input[i] >= '0' && input[i] <= '7'))
                return g_check_number_literal_type_cache[input] = NumType::Invalid;

        return g_check_number_literal_type_cache[input] = NumType::Octal;
    }
    else if (prefix == "0d")
    {
        for (i = 2; i < input.size(); i++)
            if (!(input[i] >= '0' && input[i] <= '9'))
                return g_check_number_literal_type_cache[input] = NumType::Invalid;

        return g_check_number_literal_type_cache[input] = NumType::DecimalExplicit;
    }
    else
    {
        for (i = 0; i < input.size(); i++)
        {
            if (!(input[i] >= '0' && input[i] <= '9'))
            {
                static const auto regexpFloat = std::regex("^([0-9]+(\\.[0-9]+)?)?(e[+-]?([0-9]+(\\.?[0-9]+)?)+)*$");

                // slow operation
                if (std::regex_match(input, regexpFloat))
                    return g_check_number_literal_type_cache[input] = NumType::Floating;

                return g_check_number_literal_type_cache[input] = NumType::Invalid;
            }
        }

        return g_check_number_literal_type_cache[input] = NumType::Decimal;
    }
}

static std::string canonicalize_float(std::string_view input)
{
    double mantissa = 0, exponent = 0, x = 0;
    size_t e_pos = 0;

    if ((e_pos = input.find('e')) == std::string::npos)
        return input.data();

    mantissa = std::stod(input.substr(0, e_pos).data());
    exponent = std::stod(input.substr(e_pos + 1).data());

    x = mantissa * std::pow(10.0, exponent);

    std::stringstream ss;
    ss << std::setprecision(FLOATING_POINT_PRECISION) << x;
    return ss.str();
}

bool canonicalize_number(std::string &number, std::string &norm, NumType type)
{
    /* Create a cache */
    static std::unordered_map<std::string, std::string> g_canonicalize_number_cache;
    if (g_canonicalize_number_cache.contains(number))
        return norm = g_canonicalize_number_cache[number], true;
    typedef unsigned int uint128_t __attribute__((mode(TI)));

    uint128_t x = 0, i = 0;

    /* Convert to lowercase */
    std::transform(number.begin(), number.end(), number.begin(), ::tolower);
    std::erase(number, '_');

    switch (type)
    {
    case NumType::Hexadecimal:
        for (i = 2; i < number.size(); ++i)
        {
            // Check for overflow
            if ((x >> 64) & 0xF000000000000000)
                return false;

            if (number[i] >= '0' && number[i] <= '9')
                x = (x << 4) + (number[i] - '0');
            else if (number[i] >= 'a' && number[i] <= 'f')
                x = (x << 4) + (number[i] - 'a' + 10);
            else
                return false;
        }
        break;
    case NumType::Binary:
        for (i = 2; i < number.size(); ++i)
        {
            // Check for overflow
            if (x & 0x8000000000000000)
                return false;

            if (number[i] != '0' && number[i] != '1')
                return false;

            x = (x << 1) + (number[i] - '0');
        }
        break;
    case NumType::Octal:
        for (i = 2; i < number.size(); ++i)
        {
            // Check for overflow
            if (x & 0xE000000000000000)
                return false;

            if (number[i] < '0' || number[i] > '7')
                return false;

            x = (x << 3) + (number[i] - '0');
        }
        break;
    case NumType::DecimalExplicit:
        try
        {
            x = std::stoull(number.substr(2));
        }
        catch (...)
        {
            return false;
        }
        break;
    case NumType::Decimal:
        try
        {
            x = std::stoull(number);
        }
        catch (...)
        {
            return false;
        }
    default:
        break;
    }

    /* Convert back to string and cache the result */
    std::stringstream ss;
    if (x == 0)
        ss << '0';

    for (i = x; i; i /= 10)
        ss << (char)('0' + i % 10);

    std::string s = ss.str();
    std::reverse(s.begin(), s.end());

    return g_canonicalize_number_cache[number] = (norm = s), true;
}

const libquixcc::Token &libquixcc::StreamLexer::read_token()
{
    enum class LexState
    {
        Start,
        Identifier,
        String,
        Integer,
        CommentStart,
        CommentSingleLine,
        CommentMultiLine,
        MacroStart,
        SingleLineMacro,
        BlockMacro,
        Other,
    };

    static std::string buf;
    buf.clear();

    LexState state = LexState::Start;
    uint32_t state_parens = 0;
    char c;

    try
    {
        while (true)
        {
            /* If the Lexer overshot, we will return the saved character */
            if (!m_pushback.empty())
            {
                c = m_pushback.front();
                m_pushback.pop();
            }
            else
            {
                if ((c = getc()) == EOF)
                    break;
            }

            switch (state)
            {
            case LexState::Start:
            {
                // Skip whitespace
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
                {
                    continue;
                }
                else if (std::isalpha(c) || c == '_') /* Identifier or keyword */
                {
                    buf += c, state = LexState::Identifier;
                    continue;
                }
                else if (c == '/') /* Comment or operator */
                {
                    state = LexState::CommentStart;
                    continue;
                }
                else if (std::isdigit(c))
                {
                    buf += c, state = LexState::Integer;
                    continue;
                }
                else if (c == '"' || c == '\'')
                {
                    buf += c, state = LexState::String;
                    continue;
                }
                else if (c == '@')
                {
                    state = LexState::MacroStart;
                    continue;
                }
                else /* Operator or punctor or invalid */
                {
                    buf += c;
                    state = LexState::Other;
                    continue;
                }
            }
            case LexState::Identifier:
            {
                while (std::isalnum(c) || c == '_' || c == ':' || c == '<' || c == '>')
                {
                    buf += c;

                    if ((c = getc()) == EOF)
                        return reset_state(), (m_tok = Token(TT::Unknown, buf, m_loc - buf.size())).value();
                }

                /* We overshot; this must be a punctor ':' */
                if (buf.size() > 0 && buf.back() == ':')
                {
                    char tc;
                    m_pushback.push((tc = buf.back(), buf.pop_back(), tc));
                }
                m_pushback.push(c);

                /* Determine if it's a keyword or an identifier */
                for (const auto &kw : keyword_map)
                    if (buf == kw.first)
                        return (m_tok = Token(TT::Keyword, keyword_map.at(buf), m_loc - buf.size())).value();

                /* Check if it's a valid identifier */
                if (!canonicalize_identifier(buf))
                    return reset_state(), (m_tok = Token(TT::Unknown, buf, m_loc - buf.size())).value();

                /* Canonicalize the identifier to the correct format */
                return (m_tok = Token(TT::Identifier, buf, m_loc - buf.size())).value();
            }
            case LexState::Integer:
            {
                while (std::isxdigit(c) || c == '_' || c == '.' || c == 'x' || c == 'b' || c == 'd' || c == 'o' || c == 'e')
                {
                    buf += c;

                    if ((c = getc()) == EOF)
                        return reset_state(), (m_tok = Token(TT::Unknown, buf, m_loc - buf.size())).value();
                }

                NumType type;

                m_pushback.push(c);

                /* Check if it's a floating point number */
                if ((type = check_number_literal_type(buf)) == NumType::Floating)
                    return (m_tok = Token(TT::Float, canonicalize_float(buf), m_loc - buf.size())).value();

                /* Check if it's a valid number */
                if (type == NumType::Invalid)
                    return reset_state(), (m_tok = Token(TT::Unknown, buf, m_loc - buf.size())).value();

                /* Canonicalize the number */
                std::string norm;
                if (canonicalize_number(buf, norm, type))
                    return (m_tok = Token(TT::Integer, norm, m_loc - buf.size())).value();

                /* Invalid number */
                std::cerr << "Tokenization error: Numeric literal is too large to fit in an integer type: '" << buf << "'" << std::endl;
                return reset_state(), (m_tok = Token(TT::Unknown, buf, m_loc - buf.size())).value();
            }
            case LexState::CommentStart:
            {
                if (c == '/')
                {
                    /* Single line comment */
                    state = LexState::CommentSingleLine;
                    continue;
                }
                else if (c == '*')
                {
                    /* Multi-line comment */
                    state = LexState::CommentMultiLine;
                    continue;
                }
                else
                {
                    /* Divide operator */
                    return (m_tok = Token(TT::Operator, Operator::Divide, m_loc)).value();
                }
            }
            case LexState::CommentSingleLine:
            {
                /* Automota for single-line comments */
                while (c != '\n')
                {
                    buf += c;

                    if ((c = getc()) == EOF)
                        return reset_state(), (m_tok = Token(TT::Unknown, buf, m_loc - buf.size())).value();
                }

                return (m_tok = Token(TT::Comment, buf, m_loc - buf.size())).value();
            }
            case LexState::CommentMultiLine:
            {
                /* Automota for multi-line comments */
            loop:
                while (c != '*')
                {
                    buf += c;

                    if ((c = getc()) == EOF)
                        return reset_state(), (m_tok = Token(TT::Unknown, buf, m_loc - buf.size())).value();
                }

                if ((c = getc()) == '/')
                    return (m_tok = Token(TT::Comment, buf, m_loc - buf.size())).value();
                else if (c == EOF)
                    return reset_state(), (m_tok = Token(TT::Unknown, buf, m_loc - buf.size())).value();

                buf += '*';
                goto loop;
            }
            case LexState::String:
            {
                if (c != buf[0])
                {
                    /* Normal character */
                    if (c != '\\')
                    {
                        buf += c;
                        continue;
                    }

                    /* String escape sequences */
                    c = getc();
                    switch (c)
                    {
                    case 'n':
                        buf += '\n';
                        break;
                    case 't':
                        buf += '\t';
                        break;
                    case 'r':
                        buf += '\r';
                        break;
                    case '0':
                        buf += '\0';
                        break;
                    case '\\':
                        buf += '\\';
                        break;
                    case '\'':
                        buf += '\'';
                        break;
                    case '\"':
                        buf += '\"';
                        break;
                    case 'x':
                    {
                        char hex[2] = {getc(), getc()};
                        buf += (hexLookup[(uint8_t)hex[0]] << 4) | hexLookup[(uint8_t)hex[1]];
                        break;
                    }
                    case 'u':
                    {
                        char hex[4] = {getc(), getc(), getc(), getc()};
                        uint32_t codepoint = 0;
                        codepoint |= hexLookup[(uint8_t)hex[0]] << 12;
                        codepoint |= hexLookup[(uint8_t)hex[1]] << 8;
                        codepoint |= hexLookup[(uint8_t)hex[2]] << 4;
                        codepoint |= hexLookup[(uint8_t)hex[3]];
                        buf += codepoint;
                        break;
                    }
                    case 'o':
                    {
                        char oct[4] = {getc(), getc(), getc(), 0};
                        buf += std::stoi(oct, nullptr, 8);
                        break;
                    }
                    default:
                        buf += c;
                        break;
                    }
                    continue;
                }

                /* Character or string */
                if (buf.front() == '\'' && buf.size() == 2)
                    return (m_tok = Token(TT::Char, std::string(1, buf[1]), m_loc - 2)).value();
                else
                    return (m_tok = Token(TT::String, buf.substr(1, buf.size() - 1), m_loc - buf.size())).value();
            }
            case LexState::MacroStart:
            {
                /*
                 * Macros start with '@' and can be either single-line or block macros.
                 * Block macros are enclosed in parentheses.
                 * Single-line macros end with a newline character or a special cases
                 */
                if (c == '(')
                {
                    state = LexState::BlockMacro, state_parens = 1;
                    continue;
                }
                else
                {
                    state = LexState::SingleLineMacro, state_parens = 0;
                    buf += c;
                    continue;
                }
                break;
            }
            case LexState::SingleLineMacro:
            {
                /*
                Format:
                    ... @macro_name(arg1, arg2, arg3, ...) ...
                */
                switch (state_parens)
                {
                case 0:
                {
                    if (c == '\n')
                    {
                        return (m_tok = Token(TT::MacroSingleLine, buf, m_loc - buf.size() - 1)).value();
                    }
                    else if (c != '(')
                    {
                        buf += c;
                        continue;
                    }
                    else if (c == '(')
                    {
                        buf += c, state_parens++;
                        continue;
                    }
                    else
                    {
                        return reset_state(), (m_tok = Token(TT::Unknown, buf, m_loc - buf.size())).value();
                    }
                }
                default:
                {
                    if (c == '(')
                        buf += c, state_parens++;
                    else if (c == ')')
                        buf += c, state_parens--;

                    if (state_parens == 0)
                        return (m_tok = Token(TT::MacroSingleLine, buf, m_loc - buf.size())).value();

                    buf += c;
                    continue;
                }
                }
            }
            case LexState::BlockMacro:
            {
                if (c == '(')
                    state_parens++;
                else if (c == ')')
                    state_parens--;

                if (state_parens == 0)
                    return (m_tok = Token(TT::MacroBlock, buf, m_loc - buf.size() - 1)).value();

                buf += c;
                continue;
            }
            case LexState::Other:
            {
                /* Check if it's a punctor */
                for (const char punc : punctors)
                {
                    if (punc == buf[0])
                    {
                        m_pushback.push(c);
                        return (m_tok = Token(TT::Punctor, punctor_map.at(buf), m_loc - buf.size())).value();
                    }
                }

                /* Special case for a comment */
                if ((buf[0] == '~' && c == '>'))
                {
                    state = LexState::CommentSingleLine;
                    continue;
                }

                /* Special case for a comment */
                if (buf[0] == '#' && std::isspace(c))
                {
                    state = LexState::CommentSingleLine;
                    continue;
                }

                while (1)
                {
                    if (!operator_map.contains(buf))
                    {
                        m_pushback.push(buf.back());
                        m_pushback.push(c);
                        return (m_tok = Token(TT::Operator, operator_map.at(buf.substr(0, buf.size() - 1)), m_loc - buf.size())).value();
                    }

                    buf += c;

                    if (buf.size() > 4)
                    { /* Handle infinite error case */
                        return reset_state(), (m_tok = Token(TT::Unknown, buf, m_loc - buf.size())).value();
                    }

                    if ((c = getc()) == EOF)
                        return (m_tok = Token(TT::Unknown, buf, m_loc - buf.size())).value();
                }
            }
            }
        }
        m_tok = Token(TT::Eof, "", m_loc);
        return m_tok.value();
    }
    catch (...)
    {
        reset_state();
        m_tok = Token(TT::Unknown, buf, m_loc - buf.size());

        return m_tok.value();
    }
}

const libquixcc::Token &libquixcc::StreamLexer::peek()
{
    /* If we have a token, return it */
    if (m_tok.has_value())
        return std::move(m_tok.value());

    /* This lock makes the entire Lexer thread-safe */
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    while (true)
    {
        read_token();

        /* Skip comments */
        if (m_tok->type != TT::Comment)
            return std::move(m_tok.value());

        /* We will ignore comments */
    }
}

libquixcc::Token libquixcc::StreamLexer::next()
{
    Token tok = peek();
    m_tok = std::nullopt;
    return tok;
}

bool libquixcc::StringLexer::set_source(const std::string &source_code, const std::string &filename)
{
    /* Copy the source internally */
    m_src = source_code;

    /* Open a file stream from the string */
    m_file = fmemopen((void *)m_src.c_str(), m_src.size(), "r");
    if (m_file == nullptr)
        return false;

    /* Set the source using the memory buffer */
    return StreamLexer::set_source(m_file, filename);
}

libquixcc::StringLexer::~StringLexer()
{
    if (m_file != nullptr)
    {
        fclose(m_file);
        m_file = nullptr;
    }
}

bool libquixcc::StringLexer::QuickLex(const std::string &source_code, std::vector<libquixcc::Token> &tokens, const std::string &filename)
{
    tokens.clear();

    try
    {
        /* Parse the source code "as-is" */
        StringLexer lex;
        if (!lex.set_source(source_code, filename))
            return false;

        Token tok;
        while ((tok = lex.next()).type != TT::Eof)
            tokens.push_back(tok);

        return true;
    }
    catch (std::exception &e)
    {
        return false;
    }
}