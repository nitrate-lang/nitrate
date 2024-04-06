#include <conf/Parser.hh>
#include <rapidxml/rapidxml.hpp>
#include <core/Logger.hh>

std::optional<qpkg::conf::Config> qpkg::conf::XmlConfigParser::parse(const std::string &content)
{
    rapidxml::xml_document doc;

    try
    {
        doc.parse<0>(const_cast<char *>(content.c_str()));
    }
    catch (rapidxml::parse_error &e)
    {
        LOG(core::ERROR) << "Failed to parse XML configuration: " << e.what() << std::endl;
        return std::nullopt;
    }

    if (!doc.first_node())
    {
        LOG(core::ERROR) << "Invalid XML configuration: root element must be an object" << std::endl;
        return std::nullopt;
    }

    if (doc.first_node()->name() != std::string("config"))
    {
        LOG(core::ERROR) << "Invalid XML configuration: root element must be named 'config'" << std::endl;
        return std::nullopt;
    }

    ConfigGroup grp;

    for (auto it = doc.first_node()->first_node(); it; it = it->next_sibling())
    {
        if (it->type() != rapidxml::node_type::node_element)
        {
            LOG(core::ERROR) << "Invalid XML configuration: unsupported value type" << std::endl;
            return std::nullopt;
        }

        if (it->value_size() == 0)
        {
            LOG(core::ERROR) << "Invalid XML configuration: empty value" << std::endl;
            return std::nullopt;
        }

        if (it->value_size() == 1 && it->value()[0] == '\0')
        {
            LOG(core::ERROR) << "Invalid XML configuration: empty value" << std::endl;
            return std::nullopt;
        }

        if (it->value_size() == 4 && std::string(it->value()) == "true")
            grp.set(it->name(), true);
        else if (it->value_size() == 5 && std::string(it->value()) == "false")
            grp.set(it->name(), false);
        else
        {
            try
            {
                int64_t i = std::stoll(it->value());
                grp.set(it->name(), i);
            }
            catch (std::invalid_argument &e)
            {
                grp.set(it->name(), it->value());
            }
        }
    }

    if (!grp.has<int64_t>("version"))
    {
        LOG(core::ERROR) << "Invalid JSON configuration: missing 'version' key" << std::endl;
        return std::nullopt;
    }

    return Config(grp, grp["version"].as<int64_t>());
}
