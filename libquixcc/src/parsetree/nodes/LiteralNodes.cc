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

#include <parsetree/nodes/AllNodes.h>

std::unordered_map<std::string, std::shared_ptr<libquixcc::StringNode>> libquixcc::StringNode::m_instances;
std::unordered_map<std::string, std::shared_ptr<libquixcc::CharNode>> libquixcc::CharNode::m_instances;
std::shared_ptr<libquixcc::BoolLiteralNode> libquixcc::BoolLiteralNode::m_true_instance;
std::shared_ptr<libquixcc::BoolLiteralNode> libquixcc::BoolLiteralNode::m_false_instance;
std::unordered_map<std::string, std::shared_ptr<libquixcc::FloatLiteralNode>> libquixcc::FloatLiteralNode::m_instances;
std::unordered_map<std::string, std::shared_ptr<libquixcc::IntegerNode>> libquixcc::IntegerNode::m_instances;
std::shared_ptr<libquixcc::NullLiteralNode> libquixcc::NullLiteralNode::m_instance;
std::shared_ptr<libquixcc::UndefLiteralNode> libquixcc::UndefLiteralNode::m_instance;

typedef unsigned int uint128_t __attribute__((mode(TI)));

uint128_t stringToUint128(const std::string &str)
{
    uint128_t result = 0;
    for (char c : str)
    {
        if (c < '0' || c > '9')
        {
            throw std::invalid_argument("Invalid character in input string");
        }
        result = result * 10 + (c - '0');
    }

    return result;
}

uint8_t get_numbits(std::string s)
{
    if (s == "0" || s == "1")
        return 1;

    if (s.find('.') != std::string::npos)
    {
        float f0;
        try
        {
            f0 = std::stof(s);
        }
        catch (const std::out_of_range &e)
        {
            return 64;
        }
        double f1 = std::stod(s);
        double delta = 0.0000001;

        return std::abs(f0 - f1) < delta ? 64 : 32;
    }

    uint128_t val = stringToUint128(s);

    uint8_t bits = 0;
    while (val)
    {
        val >>= 1;
        bits++;
    }

    if (bits > 64)
        return 128;
    else if (bits > 32)
        return 64;
    else if (bits > 16)
        return 32;
    else if (bits > 8)
        return 16;
    return 8;
}

libquixcc::IntegerNode::IntegerNode(const std::string &val)
{
    ntype = NodeType::IntegerNode;
    m_val = val;
    m_val_type = U64TypeNode::create();
}

libquixcc::FloatLiteralNode::FloatLiteralNode(const std::string &val)
{
    ntype = NodeType::FloatLiteralNode;
    m_val = val;
    m_value = std::stod(val);

    uint8_t numbits = get_numbits(val);

    if (numbits == 32)
        m_val_type = F32TypeNode::create();
    else if (numbits == 64)
        m_val_type = F64TypeNode::create();
}
