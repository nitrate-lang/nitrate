#include <conf/Parser.hh>
#include <yaml-cpp/yaml.h>
#include <core/Logger.hh>

std::optional<qpkg::conf::Config> qpkg::conf::YamlConfigParser::parse(const std::string &content)
{
    YAML::Node config;

    try
    {
        config = YAML::Load(content);
    }
    catch (YAML::ParserException &e)
    {
        LOG(core::ERROR) << "Failed to parse YAML configuration: " << e.what() << std::endl;
        return std::nullopt;
    }

    if (!config.IsMap())
    {
        LOG(core::ERROR) << "Invalid YAML configuration: root element must be a map" << std::endl;
        return std::nullopt;
    }

    ConfigGroup grp;

    for (auto it = config.begin(); it != config.end(); ++it)
    {
        if (it->second.IsScalar())
        {
            try
            {
                int64_t i = it->second.as<int64_t>();
                grp.set(it->first.as<std::string>(), i);
            }
            catch (YAML::TypedBadConversion<int64_t> &e)
            {
                try
                {
                    bool b = it->second.as<bool>();
                    grp.set(it->first.as<std::string>(), b);
                }
                catch (YAML::TypedBadConversion<bool> &e)
                {
                    grp.set(it->first.as<std::string>(), it->second.as<std::string>());
                }
            }
        }
        else if (it->second.IsSequence())
        {
            std::vector<std::string> v;

            for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
            {
                if (it2->IsScalar())
                    v.push_back(it2->as<std::string>());
                else
                {
                    LOG(core::ERROR) << "Invalid YAML configuration: unsupported value type" << std::endl;
                    return std::nullopt;
                }
            }

            grp.set(it->first.as<std::string>(), v);
        }
        else
        {
            LOG(core::ERROR) << "Invalid YAML configuration: unsupported value type" << std::endl;
            return std::nullopt;
        }
    }

    if (!grp.has<int64_t>("version"))
    {
        LOG(core::ERROR) << "Invalid YAML configuration: missing 'version' key" << std::endl;
        return std::nullopt;
    }

    return Config(grp, grp["version"].as<int64_t>());
}
