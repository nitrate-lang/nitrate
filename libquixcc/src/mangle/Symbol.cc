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

#include <LibMacro.h>
#include <mangle/Symbol.h>
#include <parse/nodes/AllNodes.h>

const std::string abiprefix = "_ZJ0";

static std::string join_namespace(const std::string &a, const std::string &b)
{
    if (a.empty())
        return b;
    if (b.empty())
        return a;
    return a + "::" + b;
}

static std::string wrap_tag(const std::string &tag)
{
    return std::to_string(tag.size()) + tag;
}

static bool unwrap_tags(const std::string &input, std::vector<std::string> &out)
{
    size_t i = 0;
    while (i < input.size())
    {
        std::string len;
        while (std::isdigit(input[i]))
        {
            len += input[i];
            i++;
        }

        if (len.empty())
            return false;

        size_t l = std::stoi(len);
        out.push_back(input.substr(i, l));
        i += l;
    }

    return true;
}

static std::string serialize_ns(const std::string &ns)
{
    std::string s;

    for (size_t i = 0; i < ns.size(); i++)
    {
        if (ns[i] == ':')
        {
            s += "_";
            i++;
        }
        else if (ns[i] == '_')
        {
            s += "__";
        }
        else
        {
            s += ns[i];
        }
    }

    return s;
}

static std::string deserialize_ns(const std::string &ns)
{
    std::string s;

    for (size_t i = 0; i < ns.size(); i++)
    {
        if (ns[i] == '_')
        {
            if (ns[i + 1] == '_')
            {
                s += "_";
                i++;
            }
            else
            {
                s += "::";
            }
        }
        else
        {
            s += ns[i];
        }
    }

    return s;
}

static std::string serialize_type(const libquixcc::TypeNode *type)
{
    using namespace libquixcc;

    static std::map<libquixcc::NodeType, std::string> basic_typesmap = {
        {NodeType::U8TypeNode, "b"},
        {NodeType::U16TypeNode, "w"},
        {NodeType::U32TypeNode, "d"},
        {NodeType::U64TypeNode, "q"},
        {NodeType::I8TypeNode, "c"},
        {NodeType::I16TypeNode, "s"},
        {NodeType::I32TypeNode, "i"},
        {NodeType::I64TypeNode, "l"},
        {NodeType::F32TypeNode, "g"},
        {NodeType::F64TypeNode, "e"},
        {NodeType::BoolTypeNode, "y"},
        {NodeType::CharTypeNode, "c"},
        {NodeType::StringTypeNode, "x"},
        {NodeType::VoidTypeNode, "v"},
    };

    if (basic_typesmap.contains(type->ntype))
        return basic_typesmap[type->ntype];

    if (type->ntype == NodeType::StructTypeNode)
    {
        const libquixcc::StructTypeNode *st = static_cast<const StructTypeNode *>(type);
        std::string s;
        for (auto &field : st->m_fields)
            s += wrap_tag(serialize_type(field));

        return "t" + s;
    }
    else if (type->ntype == NodeType::UnionTypeNode)
    {
        const libquixcc::UnionTypeNode *st = static_cast<const UnionTypeNode *>(type);
        std::string s;
        for (auto &field : st->m_fields)
            s += wrap_tag(serialize_type(field));

        return "u" + s;
    }
    else if (type->ntype == NodeType::ArrayTypeNode)
    {
        const libquixcc::ArrayTypeNode *st = static_cast<const ArrayTypeNode *>(type);
        std::string s;
        s += wrap_tag(serialize_type(st->m_type));
        s += wrap_tag("x" + std::to_string(st->m_size->GetInt64()));
        return "a" + s;
    }
    else if (type->ntype == NodeType::PointerTypeNode)
    {
        const libquixcc::PointerTypeNode *st = static_cast<const PointerTypeNode *>(type);
        std::string s;
        s += wrap_tag(serialize_type(st->m_type));
        return "p" + s;
    }

    throw std::runtime_error("Unknown type");
}

static libquixcc::TypeNode *deserialize_type(const std::string &type)
{
    using namespace libquixcc;

    static std::map<std::string, TypeNode *> basic_typesmap = {
        {"b", U8TypeNode::create()},
        {"w", U16TypeNode::create()},
        {"d", U32TypeNode::create()},
        {"q", U64TypeNode::create()},
        {"c", I8TypeNode::create()},
        {"s", I16TypeNode::create()},
        {"i", I32TypeNode::create()},
        {"l", I64TypeNode::create()},
        {"g", F32TypeNode::create()},
        {"e", F64TypeNode::create()},
        {"y", BoolTypeNode::create()},
        {"x", StringTypeNode::create()},
        {"v", VoidTypeNode::create()},
        {"c", CharTypeNode::create()},
        {"s", StringTypeNode::create()},
        {"v", VoidTypeNode::create()}};

    if (basic_typesmap.contains(type))
        return basic_typesmap[type];

    if (type[0] == 't')
    {
        std::vector<std::string> fields;
        if (!unwrap_tags(type.substr(1), fields))
            return nullptr;

        std::vector<TypeNode *> fields_nodes;
        for (auto &field : fields)
        {
            TypeNode *t;
            if ((t = deserialize_type(field)) == nullptr)
                return nullptr;
            fields_nodes.push_back(t);
        }

        return StructTypeNode::create(fields_nodes);
    }
    else if (type[0] == 'u')
    {
        std::vector<std::string> fields;
        if (!unwrap_tags(type.substr(1), fields))
            return nullptr;

        std::vector<TypeNode *> fields_nodes;
        for (auto &field : fields)
        {
            TypeNode *t;
            if ((t = deserialize_type(field)) == nullptr)
                return nullptr;
            fields_nodes.push_back(t);
        }

        return UnionTypeNode::create(fields_nodes);
    }
    else if (type[0] == 'a')
    {
        std::vector<std::string> fields;
        if (!unwrap_tags(type.substr(1), fields))
            return nullptr;

        TypeNode *t;
        if ((t = deserialize_type(fields[0])) == nullptr)
            return nullptr;

        return ArrayTypeNode::create(t, libquixcc::IntegerLiteralNode::create(fields[1].substr(1)));
    }
    else if (type[0] == 'p')
    {
        std::vector<std::string> fields;
        if (!unwrap_tags(type.substr(1), fields))
            return nullptr;

        TypeNode *t;
        if ((t = deserialize_type(fields[0])) == nullptr)
            return nullptr;

        return PointerTypeNode::create(t);
    }

    throw std::runtime_error("Unknown type");
}

std::string libquixcc::Symbol::mangle(std::shared_ptr<libquixcc::DeclNode> node, const std::string &prefix)
{
    return mangle(node, prefix);
}

std::string libquixcc::Symbol::mangle(const libquixcc::DeclNode *node, const std::string &prefix)
{
    std::string res = abiprefix;

    switch (node->ntype)
    {
    case libquixcc::NodeType::VarDeclNode:
    {
        res += "v";
        auto var = static_cast<const libquixcc::VarDeclNode *>(node);
        res += wrap_tag(serialize_ns(join_namespace(prefix, var->m_name)));
        res += wrap_tag(serialize_type(var->m_type));

        std::string flags;
        if (var->m_is_mut)
            flags += "m";
        if (var->m_is_thread_local)
            flags += "t";
        if (var->m_is_static)
            flags += "s";
        if (var->m_is_deprecated)
            flags += "d";
        res += wrap_tag(flags);

        return res;
    }
    case libquixcc::NodeType::LetDeclNode:
    {
        res += "l";
        auto var = static_cast<const libquixcc::LetDeclNode *>(node);
        res += wrap_tag(serialize_ns(join_namespace(prefix, var->m_name)));
        res += wrap_tag(serialize_type(var->m_type));

        std::string flags;
        if (var->m_is_mut)
            flags += "m";
        if (var->m_is_thread_local)
            flags += "t";
        if (var->m_is_static)
            flags += "s";
        if (var->m_is_deprecated)
            flags += "d";
        res += wrap_tag(flags);

        return res;
    }
    case libquixcc::NodeType::ConstDeclNode:
    {
        res += "c";
        auto var = static_cast<const libquixcc::ConstDeclNode *>(node);
        res += wrap_tag(serialize_ns(join_namespace(prefix, var->m_name)));
        res += wrap_tag(serialize_type(var->m_type));

        std::string flags;
        if (var->m_is_deprecated)
            flags += "d";
        res += wrap_tag(flags);

        return res;
    }

    default:
        break;
    }

    return "";
}

std::shared_ptr<libquixcc::DeclNode> libquixcc::Symbol::demangle(const std::string &mangled)
{
    std::string input;

    if (!mangled.starts_with(abiprefix))
        return nullptr;

    input = mangled.substr(abiprefix.size());

    try
    {
        std::vector<std::string> parts;

        switch (input[0])
        {
        case 'v':
        {
            if (!unwrap_tags(input.substr(1), parts))
                return nullptr;

            auto var = std::make_shared<libquixcc::VarDeclNode>();
            var->m_name = deserialize_ns(parts[0]);
            if ((var->m_type = deserialize_type(parts[1])) == nullptr)
                return nullptr;

            std::string flags = parts[2];
            for (size_t i = 0; i < flags.size(); i++)
            {
                switch (flags[i])
                {
                case 'm':
                    var->m_is_mut = true;
                    break;
                case 't':
                    var->m_is_thread_local = true;
                    break;
                case 's':
                    var->m_is_static = true;
                    break;
                case 'd':
                    var->m_is_deprecated = true;
                    break;
                default:
                    return nullptr;
                }
            }

            return var;
        }
        case 'l':
        {
            if (!unwrap_tags(input.substr(1), parts))
                return nullptr;

            auto let = std::make_shared<libquixcc::LetDeclNode>();
            let->m_name = deserialize_ns(parts[0]);
            if ((let->m_type = deserialize_type(parts[1])) == nullptr)
                return nullptr;

            std::string flags = parts[2];
            for (size_t i = 0; i < flags.size(); i++)
            {
                switch (flags[i])
                {
                case 'm':
                    let->m_is_mut = true;
                    break;
                case 't':
                    let->m_is_thread_local = true;
                    break;
                case 's':
                    let->m_is_static = true;
                    break;
                case 'd':
                    let->m_is_deprecated = true;
                    break;
                default:
                    return nullptr;
                }
            }

            return let;
        }
        case 'c':
        {
            if (!unwrap_tags(input.substr(1), parts))
                return nullptr;

            auto con = std::make_shared<libquixcc::ConstDeclNode>();
            con->m_name = deserialize_ns(parts[0]);
            if ((con->m_type = deserialize_type(parts[1])) == nullptr)
                return nullptr;

            std::string flags = parts[2];
            for (size_t i = 0; i < flags.size(); i++)
            {
                switch (flags[i])
                {
                case 'd':
                    con->m_is_deprecated = true;
                    break;
                default:
                    return nullptr;
                }
            }

            return con;
        }

        default:
            break;
        }
    }
    catch (std::exception &e)
    {
        return nullptr;
    }

    return nullptr;
}

bool libquixcc::Symbol::demangle_tojson(const std::string &mangled, std::string &output)
{
    auto node = demangle(mangled);
    if (node == nullptr)
        return false;

    output = node->to_json(ParseNodeJsonSerializerVisitor());
    return true;
}

LIB_EXPORT char *quixcc_demangle_symbol(const char *mangled)
{
    std::string output;
    if (!libquixcc::Symbol::demangle_tojson(mangled, output))
        return nullptr;

    return strdup(output.c_str());
}