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

#define QUIXCC_INTERNAL

#include <lexer/Lex.h>
#include <cstdio>
#include <cctype>
#include <stdexcept>
#include <cstring>
#include <iomanip>
#include <unordered_map>
#include <mutex>
#include <regex>
#include <cmath>

///=============================================================================

#define FLOATING_POINT_LITERAL_ROUND_DIGITS 32

namespace libquixcc
{
    std::map<std::string, libquixcc::Keyword> keyword_map = {
        {"subsystem", libquixcc::Keyword::Subsystem},
        {"import", libquixcc::Keyword::Import},
        {"type", libquixcc::Keyword::Type},
        {"let", libquixcc::Keyword::Let},
        {"var", libquixcc::Keyword::Var},
        {"struct", libquixcc::Keyword::Struct},
        {"union", libquixcc::Keyword::Union},
        {"fn", libquixcc::Keyword::Fn},
        {"nothrow", libquixcc::Keyword::Nothrow},
        {"foreign", libquixcc::Keyword::Foreign},
        {"impure", libquixcc::Keyword::Impure},
        {"tsafe", libquixcc::Keyword::Tsafe},
        {"const", libquixcc::Keyword::Const},
        {"enum", libquixcc::Keyword::Enum},
        {"pub", libquixcc::Keyword::Pub},
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
        {"return", libquixcc::Keyword::Return},
        {"retif", libquixcc::Keyword::Retif},
        {"retz", libquixcc::Keyword::Retz},
        {"retv", libquixcc::Keyword::Retv},
        {"void", libquixcc::Keyword::Void},
        {"undef", libquixcc::Keyword::Undef},
        {"null", libquixcc::Keyword::Null},
        {"true", libquixcc::Keyword::True},
        {"false", libquixcc::Keyword::False}};

    std::map<libquixcc::Keyword, std::string> keyword_map_inverse = {
        {libquixcc::Keyword::Subsystem, "subsystem"},
        {libquixcc::Keyword::Import, "import"},
        {libquixcc::Keyword::Type, "type"},
        {libquixcc::Keyword::Let, "let"},
        {libquixcc::Keyword::Var, "var"},
        {libquixcc::Keyword::Struct, "struct"},
        {libquixcc::Keyword::Union, "union"},
        {libquixcc::Keyword::Fn, "fn"},
        {libquixcc::Keyword::Nothrow, "nothrow"},
        {libquixcc::Keyword::Foreign, "foreign"},
        {libquixcc::Keyword::Impure, "impure"},
        {libquixcc::Keyword::Tsafe, "tsafe"},
        {libquixcc::Keyword::Const, "const"},
        {libquixcc::Keyword::Enum, "enum"},
        {libquixcc::Keyword::Pub, "pub"},
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
        {libquixcc::Keyword::Return, "return"},
        {libquixcc::Keyword::Retif, "retif"},
        {libquixcc::Keyword::Retz, "retz"},
        {libquixcc::Keyword::Retv, "retv"},
        {libquixcc::Keyword::Void, "void"},
        {libquixcc::Keyword::Undef, "undef"},
        {libquixcc::Keyword::Null, "null"},
        {libquixcc::Keyword::True, "true"},
        {libquixcc::Keyword::False, "false"}};

    std::map<std::string, libquixcc::Punctor> punctor_map = {
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

    std::map<libquixcc::Punctor, std::string> punctor_map_inverse = {
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

    std::map<std::string, libquixcc::Operator> operator_map = {
        {"<", libquixcc::Operator::LessThan},
        {">", libquixcc::Operator::GreaterThan},
        {"=", libquixcc::Operator::Assign},
        {"@", libquixcc::Operator::At},
        {"-", libquixcc::Operator::Minus},
        {"+", libquixcc::Operator::Plus},
        {"*", libquixcc::Operator::Multiply},
        {"/", libquixcc::Operator::Divide},
        {"%", libquixcc::Operator::Modulo},
        {"&", libquixcc::Operator::BitwiseAnd},
        {"|", libquixcc::Operator::BitwiseOr},
        {"^", libquixcc::Operator::BitwiseXor},
        {"~", libquixcc::Operator::BitwiseNot},
        {"!", libquixcc::Operator::Not},
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
        {"&&", libquixcc::Operator::And},
        {"||", libquixcc::Operator::Or},
        {"^^", libquixcc::Operator::Xor},
        {"<=", libquixcc::Operator::LessThanEqual},
        {">=", libquixcc::Operator::GreaterThanEqual},
        {"++", libquixcc::Operator::Increment},
        {"--", libquixcc::Operator::Decrement},
        {"^^=", libquixcc::Operator::XorAssign},
        {"||=", libquixcc::Operator::OrAssign},
        {"&&=", libquixcc::Operator::AndAssign},
        {"<<=", libquixcc::Operator::LeftShiftAssign},
        {">>=", libquixcc::Operator::RightShiftAssign}};

    std::map<libquixcc::Operator, std::string> operator_map_inverse = {
        {libquixcc::Operator::LessThan, "<"},
        {libquixcc::Operator::GreaterThan, ">"},
        {libquixcc::Operator::Assign, "="},
        {libquixcc::Operator::At, "@"},
        {libquixcc::Operator::Minus, "-"},
        {libquixcc::Operator::Plus, "+"},
        {libquixcc::Operator::Multiply, "*"},
        {libquixcc::Operator::Divide, "/"},
        {libquixcc::Operator::Modulo, "%"},
        {libquixcc::Operator::BitwiseAnd, "&"},
        {libquixcc::Operator::BitwiseOr, "|"},
        {libquixcc::Operator::BitwiseXor, "^"},
        {libquixcc::Operator::BitwiseNot, "~"},
        {libquixcc::Operator::Not, "!"},
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
        {libquixcc::Operator::And, "&&"},
        {libquixcc::Operator::Or, "||"},
        {libquixcc::Operator::Xor, "^^"},
        {libquixcc::Operator::LessThanEqual, "<="},
        {libquixcc::Operator::GreaterThanEqual, ">="},
        {libquixcc::Operator::Increment, "++"},
        {libquixcc::Operator::Decrement, "--"},
        {libquixcc::Operator::XorAssign, "^^="},
        {libquixcc::Operator::OrAssign, "||="},
        {libquixcc::Operator::AndAssign, "&&="},
        {libquixcc::Operator::LeftShiftAssign, "<<="},
        {libquixcc::Operator::RightShiftAssign, ">>="}};
}

std::string libquixcc::Scanner::escape_string(const std::string &str)
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

libquixcc::StreamLexer::StreamLexer()
{
    m_src = nullptr;
    m_buf_pos = 1024;
    m_buffer = std::vector<char>(m_buf_pos);
    m_tok = std::nullopt;
    m_last = 0;
    added_newline = false;
}

char libquixcc::StreamLexer::getc()
{
    if (!m_pushback.empty())
    {
        char c = m_pushback.front();
        m_pushback.pop();
        return c;
    }

    if (m_buf_pos >= m_buffer.size())
    {
        size_t read;
        if ((read = fread(m_buffer.data(), 1, m_buffer.size(), m_src)) == 0)
        {
            if (added_newline)
                return EOF;

            m_buffer[0] = '\n';
            read = 1;
            added_newline = true;
        }

        if (m_buffer.size() != read)
            m_buffer.resize(read);

        m_buf_pos = 0;
    }

    char c = m_buffer[m_buf_pos++];

    m_loc = m_loc_curr;

    if (c == '\n')
    {
        m_loc_curr.line++;
        m_loc_curr.col = 1;
    }
    else
    {
        m_loc_curr.col++;
    }

    return c;
}

bool libquixcc::StreamLexer::set_source(FILE *src, const std::string &filename)
{
    if (src == nullptr)
        return false;

    m_src = src;

    if (fseek(m_src, 0, SEEK_SET) != 0)
        return false;

    m_filename = filename;
    m_loc_curr = Loc(1, 1, filename);

    return true;
}

libquixcc::Token libquixcc::StreamLexer::next()
{
    Token tok = peek();
    m_tok = std::nullopt;
    return tok;
}

void libquixcc::StreamLexer::push(libquixcc::Token tok)
{
    m_tok = tok;
}

static bool validate_identifier_type_1(const std::string &id)
{
    // check if its a a::b::c::d::e::f

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

static bool validate_identifier_type_2(const std::string &id)
{
    int state = 0;

    for (const auto &c : id)
    {
        switch (state)
        {
        case 0:
            if (std::isalnum(c) || c == '_')
                continue;
            if (c == '.')
            {
                state = 1;
                continue;
            }
            return false;
        case 1:
            if (std::isalnum(c) || c == '_')
            {
                state = 0;
                continue;
            }
            return false;
        }
    }

    return state == 0;
}

static bool reduce_identifier(std::string &str)
{
    static std::unordered_map<std::string, std::string> cache;

    if (cache.contains(str))
    {
        str = cache[str];
        return true;
    }

    if (validate_identifier_type_1(str))
    {
        cache[str] = str;
        return true;
    }

    /*
        a.b.c.d.e.f
        becomes -> a::b::c::d::e::f
    */

    if (validate_identifier_type_2(str))
    {
        size_t index = 0;
        while (true)
        {
            /* Locate the substring to replace. */
            index = str.find(".", index);
            if (index == std::string::npos)
                break;

            /* Make the replacement. */
            str.replace(index, 1, "::");

            /* Advance index forward so the next iteration doesn't pick it up as well. */
            index++;
        }

        cache[str] = str;

        return true;
    }

    return false;
}

enum class NumberLiteralType
{
    Invalid,
    Decimal,
    DecimalExplicit,
    Hexadecimal,
    Binary,
    Octal,
    Floating,
};

static NumberLiteralType check_number_literal_type(std::string input)
{
    static std::unordered_map<std::string, NumberLiteralType> cache;

    if (cache.contains(input))
        return cache[input];

    if (input[0] == '-')
        input = input.substr(1);

    if (input.length() < 3)
    {
        if (std::isdigit(input[0]))
        {
            cache[input] = NumberLiteralType::Decimal;
            return NumberLiteralType::Decimal;
        }
        else
        {
            cache[input] = NumberLiteralType::Invalid;
            return NumberLiteralType::Invalid;
        }
    }

    std::string prefix = input.substr(0, 2);

    if (prefix == "0x")
    {
        for (size_t i = 2; i < input.length(); i++)
        {
            if (!((input[i] >= '0' && input[i] <= '9') || (input[i] >= 'a' && input[i] <= 'f')))
            {
                cache[input] = NumberLiteralType::Invalid;
                return NumberLiteralType::Invalid;
            }
        }
        cache[input] = NumberLiteralType::Hexadecimal;
        return NumberLiteralType::Hexadecimal;
    }
    else if (prefix == "0b")
    {
        for (size_t i = 2; i < input.length(); i++)
        {
            if (!(input[i] == '0' || input[i] == '1'))
            {
                cache[input] = NumberLiteralType::Invalid;
                return NumberLiteralType::Invalid;
            }
        }
        cache[input] = NumberLiteralType::Binary;
        return NumberLiteralType::Binary;
    }
    else if (prefix == "0o")
    {
        for (size_t i = 2; i < input.length(); i++)
        {
            if (!(input[i] >= '0' && input[i] <= '7'))
            {
                cache[input] = NumberLiteralType::Invalid;
                return NumberLiteralType::Invalid;
            }
        }
        cache[input] = NumberLiteralType::Octal;
        return NumberLiteralType::Octal;
    }
    else if (prefix == "0d")
    {
        for (size_t i = 2; i < input.length(); i++)
        {
            if (!(input[i] >= '0' && input[i] <= '9'))
            {
                cache[input] = NumberLiteralType::Invalid;
                return NumberLiteralType::Invalid;
            }
        }
        cache[input] = NumberLiteralType::DecimalExplicit;
        return NumberLiteralType::DecimalExplicit;
    }
    else
    {
        for (size_t i = 0; i < input.length(); i++)
        {
            if (!(input[i] >= '0' && input[i] <= '9'))
            {
                goto test_float;
            }
        }
        cache[input] = NumberLiteralType::Decimal;
        return NumberLiteralType::Decimal;
    }

test_float:
    static const auto regexpFloat = std::regex("^([0-9]+(\\.[0-9]+)?)?(e[+-]?([0-9]+(\\.?[0-9]+)?)+)*$");

    // slow operation
    if (std::regex_match(input, regexpFloat))
    {
        cache[input] = NumberLiteralType::Floating;
        return NumberLiteralType::Floating;
    }

    cache[input] = NumberLiteralType::Invalid;
    return NumberLiteralType::Invalid;
}

static std::string normalize_float(const std::string &input)
{
    double mantissa = 0;
    double exponent = 0;
    double x = 0;

    size_t e_pos = input.find('e');

    if (e_pos == std::string::npos)
        return input;

    mantissa = std::stod(input.substr(0, e_pos));
    exponent = std::stod(input.substr(e_pos + 1));

    x = mantissa * std::pow(10.0, exponent);
    std::stringstream ss;
    ss << std::setprecision(FLOATING_POINT_LITERAL_ROUND_DIGITS) << x;
    return ss.str();
}

bool normalize_number_literal(std::string &number, std::string &norm, NumberLiteralType type)
{
    static std::unordered_map<std::string, std::string> cache;

    if (cache.contains(number))
    {
        norm = cache[number];
        return true;
    }

    uint64_t x = 0;

    for (size_t i = 0; i < number.length(); i++)
    {
        number[i] = std::tolower(number[i]);
    }

    switch (type)
    {
    case NumberLiteralType::Hexadecimal:
        for (size_t i = 2; i < number.length(); ++i)
        {
            // check for overflow
            if (x & 0xF000000000000000)
                return false;

            if (number[i] >= '0' && number[i] <= '9')
                x = (x << 4) + (number[i] - '0');
            else if (number[i] >= 'a' && number[i] <= 'f')
                x = (x << 4) + (number[i] - 'a' + 10);
            else
                return false;
        }
        break;
    case NumberLiteralType::Binary:
        for (size_t i = 2; i < number.length(); ++i)
        {
            // check for overflow
            if (x & 0x8000000000000000)
                return false;

            x = (x << 1) + (number[i] - '0');
        }
        break;
    case NumberLiteralType::Octal:
        for (size_t i = 2; i < number.length(); ++i)
        {
            // check for overflow
            if (x & 0xE000000000000000)
                return false;

            x = (x << 3) + (number[i] - '0');
        }
        break;
    case NumberLiteralType::DecimalExplicit:
        x = std::stoull(number.substr(2));
        break;
    case NumberLiteralType::Decimal:
        x = std::stoull(number);
        break;
    default:
        break;
    }

    norm = std::to_string(x);
    cache[number] = norm;
    return true;
}

libquixcc::Token libquixcc::StreamLexer::read_token()
{
    enum class LexState
    {
        Start,
        Identifier,
        StringLiteral,
        IntegerLiteral,
        CommentStart,
        CommentSingleLine,
        CommentMultiLine,
        MacroStart,
        SingleLineMacro,
        BlockMacro,
        Other,
    };

    std::string buffer;
    LexState state = LexState::Start;
    size_t state_parens = 0;
    char c;

    while (true)
    {
        if (m_last != 0)
        {
            c = m_last;
            m_last = 0;
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
            else if (c == '/')
            {
                state = LexState::CommentStart;

                // it could also be an operator
                continue;
            }
            else if (std::isalpha(c) || c == '_')
            {
                buffer += c;
                state = LexState::Identifier;

                // it can also be a keyword
                continue;
            }
            else if (std::isdigit(c))
            {
                // Number literal
                buffer += c;
                state = LexState::IntegerLiteral;
                continue;
            }
            else if (c == '"' || c == '\'')
            {
                // String literal
                buffer += c;
                state = LexState::StringLiteral;
                continue;
            }
            else if (c == '@')
            {
                // Macro
                state = LexState::MacroStart;
                continue;
            }
            else
            {
                // Operator or punctor or comment or invalid
                buffer += c;
                state = LexState::Other;
                continue;
            }
        }
        case LexState::Identifier:
        {
            if (std::isalnum(c) || c == '_' || c == '.' || c == ':')
            {
                buffer += c;
                continue;
            }

            if (buffer.size() > 0 && buffer.back() == ':')
            {
                m_last = c;
                buffer.pop_back();

                pushback(':');
                continue;
            }

            m_last = c;

            // check if it's a keyword
            for (const auto &kw : keywords)
            {
                if ((int)buffer.size() == kw.second && memcmp(buffer.c_str(), kw.first, kw.second) == 0)
                {
                    m_tok = Token(TokenType::Keyword, keyword_map[buffer], m_loc - buffer.size());
                    return m_tok.value();
                }
            }

            if (!reduce_identifier(buffer))
            {
                m_tok = Token(TokenType::Unknown, buffer, m_loc - buffer.size());
                return m_tok.value();
            }

            // it's an identifier
            m_tok = Token(TokenType::Identifier, buffer, m_loc - buffer.size());
            return m_tok.value();
        }
        case LexState::IntegerLiteral:
        {
            // match [0-9.]
            if (std::isxdigit(c) || c == '.' || c == 'x' || c == 'b' || c == 'd' || c == 'o' || c == 'e')
            {
                buffer += c;
                continue;
            }

            auto type = check_number_literal_type(buffer);

            if (type == NumberLiteralType::Floating)
            {
                m_tok = Token(TokenType::FloatingLiteral, normalize_float(buffer), m_loc - buffer.size());
            }
            else if (type != NumberLiteralType::Invalid)
            {
                std::string norm;
                if (!normalize_number_literal(buffer, norm, type))
                {
                    m_tok = Token(TokenType::Unknown, buffer, m_loc - buffer.size());
                }

                m_tok = Token(TokenType::IntegerLiteral, norm, m_loc - buffer.size());
            }

            m_last = c;
            return m_tok.value();
        }
        case LexState::CommentStart:
        {
            if (c == '/')
            {
                state = LexState::CommentSingleLine;
                continue;
            }
            else if (c == '*')
            {
                state = LexState::CommentMultiLine;
                continue;
            }
            else
            {
                // it's an operator
                m_last = c;
                m_tok = Token(TokenType::Operator, Operator::Divide, m_loc);
                return m_tok.value();
            }
        }
        case LexState::CommentSingleLine:
        {
            if (c != '\n')
            {
                buffer += c;
                continue;
            }

            m_tok = Token(TokenType::Comment, buffer, m_loc - buffer.size());
            return m_tok.value();
        }
        case LexState::CommentMultiLine:
        {
            if (c != '*')
            {
                buffer += c;
                continue;
            }

            if ((c = getc()) == '/')
            {
                m_tok = Token(TokenType::Comment, buffer, m_loc - buffer.size());
                return m_tok.value();
            }

            buffer += '*';
            buffer += c;
            continue;
        }
        case LexState::StringLiteral:
        {
            if (c != buffer[0])
            {
                // buffer += c;
                // continue;
                if (c == '\\')
                {
                    // escape sequence
                    c = getc();
                    switch (c)
                    {
                    case 'n':
                        buffer += '\n';
                        break;
                    case 't':
                        buffer += '\t';
                        break;
                    case 'r':
                        buffer += '\r';
                        break;
                    case '0':
                        buffer += '\0';
                        break;
                    case '\\':
                        buffer += '\\';
                        break;
                    case '\'':
                        buffer += '\'';
                        break;
                    case '\"':
                        buffer += '\"';
                        break;
                    case 'x':
                    {
                        std::string hex;
                        hex += getc();
                        hex += getc();
                        buffer += std::stoi(hex, nullptr, 16);
                        break;
                    }
                    case 'u':
                    {
                        std::string hex;
                        hex += getc();
                        hex += getc();
                        hex += getc();
                        hex += getc();
                        buffer += std::stoi(hex, nullptr, 16);
                        break;
                    }
                    case 'o':
                    {
                        std::string oct;
                        oct += getc();
                        oct += getc();
                        oct += getc();
                        buffer += std::stoi(oct, nullptr, 8);
                        break;
                    }
                    default:
                        buffer += c;
                        break;
                    }
                }
                else
                {
                    buffer += c;
                }
                continue;
            }

            if (buffer.front() == '\'' && buffer.size() == 2)
            {
                m_tok = Token(TokenType::CharLiteral, std::string(1, buffer[1]), m_loc - 2);
            }
            else
            {
                m_tok = Token(TokenType::StringLiteral, buffer.substr(1, buffer.size() - 1), m_loc - buffer.size());
            }

            return m_tok.value();
        }
        case LexState::MacroStart:
            // macros begin with '@'
            // There are two types. One is a single line macro, the other is a block macro.
            // block macros begin with a '@(' and end with a ');'
            if (c == '(')
            {
                state = LexState::BlockMacro;
                state_parens = 1;
                continue;
            }
            else
            {
                state = LexState::SingleLineMacro;
                state_parens = 0;
                buffer += c;
                continue;
            }
            break;
        case LexState::SingleLineMacro:
        {
            /*
            Format:
                ... @macro_name(arg1, arg2, arg3, ...) ...
            */
            switch (state_parens)
            {
            case 0:
                if (c == '\n')
                {
                    m_tok = Token(TokenType::MacroSingleLine, buffer, m_loc - buffer.size());
                    return m_tok.value();
                }
                else if (c != '(')
                {
                    buffer += c;
                    continue;
                }
                else if (c == '(')
                {
                    buffer += c;
                    state_parens++;
                    continue;
                }
                else
                {
                    m_tok = Token(TokenType::Unknown, buffer, m_loc - buffer.size());
                    return m_tok.value();
                }
            default:
                if (c == '(')
                {
                    buffer += c;
                    state_parens++;
                }
                else if (c == ')')
                {
                    buffer += c;
                    state_parens--;
                }

                if (state_parens == 0)
                {
                    m_tok = Token(TokenType::MacroSingleLine, buffer, m_loc - buffer.size());
                    return m_tok.value();
                }

                buffer += c;
                continue;
            }
        }
        case LexState::BlockMacro:
        {
            // block macros can contain anything
            // therefore, we must keep track of how many open and close parens we have
            // and stop when we have a matching pair
            if (c == '(')
            {
                state_parens++;
            }
            else if (c == ')')
            {
                state_parens--;
            }

            if (state_parens == 0)
            {
                m_tok = Token(TokenType::MacroBlock, buffer, m_loc - buffer.size());
                return m_tok.value();
            }

            buffer += c;
            continue;
        }

        case LexState::Other:
        {
            if (buffer.size() == 1)
            {
                for (const char punc : punctors)
                {
                    if (punc == buffer[0])
                    {
                        m_last = c;
                        m_tok = Token(TokenType::Punctor, punctor_map.at(buffer), m_loc - buffer.size());
                        return m_tok.value();
                    }
                }

                if ((buffer[0] == '~' && c == '>'))
                {
                    state = LexState::CommentSingleLine;
                    continue;
                }

                if (buffer[0] == '#' && std::isspace(c))
                {
                    state = LexState::CommentSingleLine;
                    continue;
                }
            }
            if (!operator_map.contains(buffer))
            {
                pushback(buffer.back());
                pushback(c);
                m_tok = Token(TokenType::Operator, operator_map.at(buffer.substr(0, buffer.size() - 1)), m_loc - buffer.size());
                return m_tok.value();
            }

            buffer += c;
            continue;
        }
        }
    }

    m_tok = Token(TokenType::Eof, "", m_loc);
    return m_tok.value();
}

libquixcc::Token libquixcc::StreamLexer::peek()
{
    Token tok;
    if (m_tok.has_value())
        return m_tok.value();

    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    while (true)
    {
        tok = read_token();
        if (tok.type() == TokenType::Comment)
            m_tok = std::nullopt;
        else
            return tok;
    }
}

bool libquixcc::StringLexer::set_source(const std::string &source_code, const std::string &filename)
{
    m_src = source_code;

    m_file = fmemopen((void *)m_src.c_str(), m_src.size(), "r");
    if (m_file == nullptr)
        return false;

    return StreamLexer::set_source(m_file, filename);
}

libquixcc::StringLexer::~StringLexer()
{
    if (m_file != nullptr)
        fclose(m_file);
}

bool libquixcc::StringLexer::QuickLex(const std::string &source_code, std::vector<libquixcc::Token> &tokens, const std::string &filename)
{
    try
    {
        StringLexer lex;
        lex.set_source(source_code + "\n# .", filename);
        Token tok;
        tokens.clear();
        while ((tok = lex.next()).type() != TokenType::Eof)
        {
            tokens.push_back(tok);
        }
    }
    catch (std::exception &e)
    {
        return false;
    }

    return true;
}