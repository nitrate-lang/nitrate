#include <conf/Parser.hh>
#include <sstream>
#include <fstream>

static std::string JsonEscapeString(const std::string &str)
{
    std::stringstream ss;

    for (char c : str)
    {
        switch (c)
        {
        case '"':
            ss << "\\\"";
            break;
        case '\\':
            ss << "\\\\";
            break;
        case '/':
            ss << "\\/";
            break;
        case '\b':
            ss << "\\b";
            break;
        case '\f':
            ss << "\\f";
            break;
        case '\n':
            ss << "\\n";
            break;
        case '\r':
            ss << "\\r";
            break;
        case '\t':
            ss << "\\t";
            break;
        default:
            ss << c;
            break;
        }
    }

    return ss.str();
}

std::string qpkg::conf::ConfigGroup::dump(qpkg::conf::ConfigItemSerializationTarget target) const
{
    if (target != ConfigItemSerializationTarget::JSON)
        throw ConfigSerializerException("Unsupported serialization target");

    std::stringstream ss;

    ss << "{";

    for (auto it = m_items.begin(); it != m_items.end(); ++it)
    {
        ss << "\"" << it->first << "\":";
        if (it->second.is<std::string>())
        {
            ss << "\"" << JsonEscapeString(it->second.as<std::string>()) << "\"";
        }
        else if (it->second.is<int64_t>())
        {
            ss << it->second.as<int64_t>();
        }
        else if (it->second.is<bool>())
        {
            ss << (it->second.as<bool>() ? "true" : "false");
        }

        if (std::next(it) != m_items.end())
            ss << ",";
    }

    ss << "}";

    return ss.str();
}

std::string qpkg::conf::Config::dump(qpkg::conf::ConfigItemSerializationTarget target) const
{
    if (target != ConfigItemSerializationTarget::JSON)
        throw ConfigSerializerException("Unsupported serialization target");

    std::stringstream ss;

    ss << "{";
    ss << "\"config\":{";
    ss << "\"version\":" << m_version << ",";
    ss << "\"root\":" << m_root.dump(target);
    ss << "}}";

    return ss.str();
}

std::optional<qpkg::conf::Config> qpkg::conf::IParser::parsef(const std::string &path)
{
    try
    {
        std::ifstream file(path);
        if (!file.is_open())
            return std::nullopt;

        std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return parse(data);
    }
    catch (...)
    {
        return std::nullopt;
    }
}