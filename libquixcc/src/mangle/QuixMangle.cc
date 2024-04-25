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
///     * Copyright (C) 2020-2024 Wesley C. Jones                                ///
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

#include <LibMacro.h>
#include <mangle/Symbol.h>
#include <parse/nodes/AllNodes.h>
#include <core/Logger.h>

/// TODO: Add a construct to the mangling system that indicates mutability.

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

static std::string serialize_type(const libquixcc::TypeNode *type, std::set<const libquixcc::TypeNode *> visited)
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
        {NodeType::StringTypeNode, "x"},
        {NodeType::VoidTypeNode, "v"},
    };

    if (basic_typesmap.contains(type->ntype))
        return basic_typesmap[type->ntype];

    if (visited.contains(type))
    {
        switch (type->ntype)
        {
        case NodeType::StructTypeNode:
            return "t" + wrap_tag(static_cast<const StructTypeNode *>(type)->m_name);
        case NodeType::RegionTypeNode:
            return "j" + wrap_tag(static_cast<const RegionTypeNode *>(type)->m_name);
        case NodeType::UnionTypeNode:
            return "u" + wrap_tag(static_cast<const UnionTypeNode *>(type)->m_name);
        default:
            throw std::runtime_error("Unknown type: " + std::to_string((int)type->ntype));
        }
    }

    if (type->is<StructTypeNode>())
    {
        visited.insert(type);

        const libquixcc::StructTypeNode *st = static_cast<const StructTypeNode *>(type);
        std::string s = wrap_tag(st->m_name);
        for (auto &field : st->m_fields)
            s += wrap_tag(serialize_type(field, visited));

        return "t" + s;
    }
    else if (type->is<RegionTypeNode>())
    {
        visited.insert(type);

        const libquixcc::RegionTypeNode *st = static_cast<const RegionTypeNode *>(type);
        std::string s = wrap_tag(st->m_name);
        for (auto &field : st->m_fields)
            s += wrap_tag(serialize_type(field, visited));

        return "j" + s;
    }
    else if (type->is<UnionTypeNode>())
    {
        visited.insert(type);

        const libquixcc::UnionTypeNode *st = static_cast<const UnionTypeNode *>(type);
        std::string s = wrap_tag(st->m_name);
        for (auto &field : st->m_fields)
            s += wrap_tag(serialize_type(field, visited));

        return "u" + s;
    }
    else if (type->is<EnumTypeNode>())
    {
        const libquixcc::EnumTypeNode *st = static_cast<const EnumTypeNode *>(type);
        std::string s = wrap_tag(serialize_type(st->m_member_type, visited)) + wrap_tag(st->m_name);
        return "k" + s;
    }
    else if (type->is<ArrayTypeNode>())
    {
        const libquixcc::ArrayTypeNode *st = static_cast<const ArrayTypeNode *>(type);
        std::string s;
        s += wrap_tag(serialize_type(st->m_type, visited));
        s += wrap_tag("x" + st->m_size->reduce<IntegerLiteralNode>()->m_val);
        return "a" + s;
    }
    else if (type->is<PointerTypeNode>())
    {
        const libquixcc::PointerTypeNode *st = static_cast<const PointerTypeNode *>(type);
        std::string s;
        s += wrap_tag(serialize_type(st->m_type, visited));
        return "p" + s;
    }
    else if (type->is<FunctionTypeNode>())
    {
        const libquixcc::FunctionTypeNode *st = static_cast<const FunctionTypeNode *>(type);
        std::string s;
        s += wrap_tag(serialize_type(st->m_return_type, visited));
        for (auto &param : st->m_params)
        {
            s += wrap_tag(serialize_ns(param.first));
            s += wrap_tag(serialize_type(param.second, visited));
        }

        std::string prop;

        if (st->m_foreign)
            prop += "f";
        if (st->m_nothrow)
            prop += "n";
        if (st->m_pure)
            prop += "p";
        if (st->m_thread_safe)
            prop += "t";
        if (st->m_variadic)
            prop += "v";

        s += wrap_tag(prop);

        return "f" + s;
    }
    else if (type->is<MutTypeNode>())
    {
        const libquixcc::MutTypeNode *st = static_cast<const MutTypeNode *>(type);
        std::string s;
        s += wrap_tag(serialize_type(st->m_type, visited));
        return "m" + s;
    }
    else if (type->is<OpaqueTypeNode>())
    {
        LOG(FATAL) << "Opaque types are not supported in the mangling system." << std::endl;
    }

    throw std::runtime_error("Unknown type: " + std::to_string((int)type->ntype));
}

static libquixcc::TypeNode *deserialize_type_inner(const std::string &type, std::map<std::string, libquixcc::TypeNode *> &prev)
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
        {"v", VoidTypeNode::create()}};

    if (basic_typesmap.contains(type))
        return basic_typesmap[type];

    if (type[0] == 't')
    {
        std::vector<std::string> fields;
        if (!unwrap_tags(type.substr(1), fields))
            return nullptr;

        std::string name = deserialize_ns(fields[0]);

        if (fields.size() == 1)
            return UserTypeNode::create(name);

        std::vector<TypeNode *> fields_nodes;
        for (size_t i = 1; i < fields.size(); i++)
        {
            TypeNode *t;
            if ((t = deserialize_type_inner(fields[i], prev)) == nullptr)
                return nullptr;
            fields_nodes.push_back(t);
        }

        return prev[name] = StructTypeNode::create(fields_nodes, name);
    }
    else if (type[0] == 'j')
    {
        std::vector<std::string> fields;
        if (!unwrap_tags(type.substr(1), fields))
            return nullptr;

        std::string name = deserialize_ns(fields[0]);

        if (fields.size() == 1)
            return UserTypeNode::create(name);

        std::vector<TypeNode *> fields_nodes;
        for (size_t i = 1; i < fields.size(); i++)
        {
            TypeNode *t;
            if ((t = deserialize_type_inner(fields[i], prev)) == nullptr)
                return nullptr;
            fields_nodes.push_back(t);
        }

        return prev[name] = RegionTypeNode::create(fields_nodes, name);
    }
    else if (type[0] == 'u')
    {
        std::vector<std::string> fields;
        if (!unwrap_tags(type.substr(1), fields))
            return nullptr;

        std::string name = deserialize_ns(fields[0]);

        if (fields.size() == 1)
            return UserTypeNode::create(name);

        std::vector<TypeNode *> fields_nodes;
        for (size_t i = 1; i < fields.size(); i++)
        {
            TypeNode *t;
            if ((t = deserialize_type_inner(fields[i], prev)) == nullptr)
                return nullptr;
            fields_nodes.push_back(t);
        }

        return prev[name] = UnionTypeNode::create(fields_nodes, name);
    }
    else if (type[0] == 'k')
    {
        TypeNode *m_type;
        std::string name;

        std::vector<std::string> fields;
        if (!unwrap_tags(type.substr(1), fields))
            return nullptr;

        if (fields.size() != 2)
            return nullptr;

        if ((m_type = deserialize_type_inner(fields[0], prev)) == nullptr)
            return nullptr;

        name = fields[1];

        return EnumTypeNode::create(name, m_type);
    }
    else if (type[0] == 'a')
    {
        std::vector<std::string> fields;
        if (!unwrap_tags(type.substr(1), fields))
            return nullptr;

        TypeNode *t;
        if ((t = deserialize_type_inner(fields[0], prev)) == nullptr)
            return nullptr;

        return ArrayTypeNode::create(t, libquixcc::IntegerLiteralNode::create(fields[1].substr(1)));
    }
    else if (type[0] == 'p')
    {
        std::vector<std::string> fields;
        if (!unwrap_tags(type.substr(1), fields))
            return nullptr;

        TypeNode *t;
        if ((t = deserialize_type_inner(fields[0], prev)) == nullptr)
            return nullptr;

        return PointerTypeNode::create(t);
    }
    else if (type[0] == 'f')
    {
        std::vector<std::string> fields;
        if (!unwrap_tags(type.substr(1), fields))
            return nullptr;

        TypeNode *ret;
        if ((ret = deserialize_type_inner(fields[0], prev)) == nullptr)
            return nullptr;

        std::vector<std::pair<std::string, TypeNode *>> params;
        for (size_t i = 1; i < fields.size() - 1; i += 2)
        {
            std::string name = fields[i];
            TypeNode *t;
            if ((t = deserialize_type_inner(fields[i + 1], prev)) == nullptr)
                return nullptr;

            params.push_back({name, t});
        }

        std::string prop = fields.back();
        bool foreign = false, nothrow = false, pure = false, thread_safe = false, variadic = false;
        for (size_t i = 0; i < prop.size(); i++)
        {
            switch (prop[i])
            {
            case 'f':
                foreign = true;
                break;
            case 'n':
                nothrow = true;
                break;
            case 'p':
                pure = true;
                break;
            case 't':
                thread_safe = true;
                break;
            case 'v':
                variadic = true;
                break;
            default:
                return nullptr;
            }
        }

        return FunctionTypeNode::create(ret, params, variadic, pure, thread_safe, foreign, nothrow);
    }
    else if (type[0] == 'm')
    {
        std::vector<std::string> fields;
        if (!unwrap_tags(type.substr(1), fields))
            return nullptr;

        TypeNode *t;
        if ((t = deserialize_type_inner(fields[0], prev)) == nullptr)
            return nullptr;

        return MutTypeNode::create(t);
    }

    throw std::runtime_error("Unknown type 2");
}

static libquixcc::TypeNode *deserialize_type(const std::string &input)
{
    std::map<std::string, libquixcc::TypeNode *> prev;

    auto type = deserialize_type_inner(input, prev);
    if (!type)
        return nullptr;

    type->dfs_preorder(libquixcc::ParseNodePreorderVisitor(
        [&](const std::vector<std::string> &_namespace, libquixcc::ParseNode *parent, libquixcc::TraversePtr current)
        {
            if (current.first != libquixcc::TraversePtrType::Raw)
                return;
            
            libquixcc::TypeNode **t = reinterpret_cast<libquixcc::TypeNode **>(std::get<libquixcc::ParseNode **>(current.second));
            if ((*t)->ntype != libquixcc::NodeType::UserTypeNode)
                return;

            auto ut = static_cast<libquixcc::UserTypeNode *>(*t);

            if (!prev.contains(ut->m_name))
                return;

            *t = prev[ut->m_name];
        },
        {}));

    return type;
}

std::string libquixcc::Symbol::mangle_quix(const libquixcc::DeclNode *node, const std::string &prefix)
{
    std::string res = quix_abiprefix;
    std::set<const libquixcc::TypeNode *> visited;

    switch (node->ntype)
    {
    case libquixcc::NodeType::VarDeclNode:
    {
        res += "v";
        auto var = static_cast<const libquixcc::VarDeclNode *>(node);
        res += wrap_tag(serialize_ns(join(prefix, var->m_name)));
        res += wrap_tag(serialize_type(var->m_type, visited));

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
        res += wrap_tag(serialize_ns(join(prefix, var->m_name)));
        res += wrap_tag(serialize_type(var->m_type, visited));

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
    case libquixcc::NodeType::FunctionDeclNode:
    {
        res += "f";
        auto var = static_cast<const libquixcc::FunctionDeclNode *>(node);
        auto tp = static_cast<const libquixcc::FunctionTypeNode *>(var->m_type);
        res += wrap_tag(serialize_ns(join(prefix, var->m_name)));
        res += wrap_tag(serialize_type(var->m_type, visited));

        std::string flags;
        if (tp->m_foreign)
            flags += "f";
        if (tp->m_nothrow)
            flags += "n";
        if (tp->m_pure)
            flags += "p";
        if (tp->m_thread_safe)
            flags += "t";
        if (tp->m_variadic)
            flags += "v";
        res += wrap_tag(flags);

        return res;
    }

    default:
        break;
    }

    return "";
}

std::shared_ptr<libquixcc::DeclNode> libquixcc::Symbol::demangle_quix(std::string input)
{
    try
    {
        input = input.substr(quix_abiprefix.size());

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
        case 'f':
        {
            if (!unwrap_tags(input.substr(1), parts))
                return nullptr;

            auto fun = std::make_shared<libquixcc::FunctionDeclNode>();
            fun->m_name = deserialize_ns(parts[0]);
            if ((fun->m_type = static_cast<libquixcc::FunctionTypeNode *>(deserialize_type(parts[1]))) == nullptr)
                return nullptr;

            auto tp = static_cast<libquixcc::FunctionTypeNode *>(fun->m_type);

            for (auto p : tp->m_params)
            {
                fun->m_params.push_back(std::make_shared<libquixcc::FunctionParamNode>(p.first, p.second, nullptr));
            }

            std::string flags = parts[2];
            for (size_t i = 0; i < flags.size(); i++)
            {
                switch (flags[i])
                {
                case 'f':
                    tp->m_foreign = true;
                    break;
                case 'n':
                    tp->m_nothrow = true;
                    break;
                case 'p':
                    tp->m_pure = true;
                    break;
                case 't':
                    tp->m_thread_safe = true;
                    break;
                case 'v':
                    tp->m_variadic = true;
                    break;
                default:
                    return nullptr;
                }
            }

            return fun;
        }

        default:
            return nullptr;
        }
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Error demangling: " + std::string(e.what()));
        return nullptr;
    }
}