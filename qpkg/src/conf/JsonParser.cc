#include <conf/Parser.hh>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <core/Logger.hh>

std::optional<qpkg::conf::Config> qpkg::conf::JsonConfigParser::parse(const std::string &content)
{
    rapidjson::Document doc;
    doc.Parse(content.c_str());

    if (doc.HasParseError())
    {
        LOG(core::ERROR) << "Failed to parse JSON configuration: " << rapidjson::GetParseError_En(doc.GetParseError()) << std::endl;
        return std::nullopt;
    }

    if (!doc.IsObject())
    {
        LOG(core::ERROR) << "Invalid JSON configuration: root element must be an object" << std::endl;
        return std::nullopt;
    }

    ConfigGroup grp;

    for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it)
    {
        if (it->value.IsString())
            grp.set(it->name.GetString(), it->value.GetString());
        else if (it->value.IsInt64())
            grp.set(it->name.GetString(), it->value.GetInt64());
        else if (it->value.IsBool())
            grp.set(it->name.GetString(), it->value.GetBool());
        else
        {
            LOG(core::ERROR) << "Invalid JSON configuration: unsupported value type" << std::endl;
            return std::nullopt;
        }
    }

    if (!grp.has<int64_t>("version"))
    {
        LOG(core::ERROR) << "Invalid JSON configuration: missing 'version' key" << std::endl;
        return std::nullopt;
    }

    return Config(grp, grp["version"].as<int64_t>());
}
