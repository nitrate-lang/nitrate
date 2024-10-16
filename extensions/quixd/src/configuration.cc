#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <server.hh>
#include <string>

std::optional<Configuration> parse_config(const std::string& path) {
  rapidjson::Document doc;

  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    LOG(ERROR) << "Failed to open file: " << path;
    return std::nullopt;
  }

  rapidjson::IStreamWrapper isw(ifs);

  doc.ParseStream(isw);

  if (doc.HasParseError()) {
    LOG(ERROR) << "Failed to parse JSON config file: " << path;
    return std::nullopt;
  }

  if (!doc.IsObject()) {
    LOG(ERROR) << "Expected object at JSON root, got " << doc.GetType();
    return std::nullopt;
  }

  if (!doc.HasMember("version")) {
    LOG(ERROR) << "Expected 'version' field in the config file, not found";
    return std::nullopt;
  }

  if (!doc["version"].IsInt()) {
    LOG(ERROR) << "Expected 'version' field in the config file to be an integer";
    return std::nullopt;
  }

  if (doc["version"].GetInt() != 1) {
    LOG(ERROR) << "Unsupported config file version. Only version 1 is supportted now";
    return std::nullopt;
  }

  ///=================== CONFIG VERSION 1======================
  Configuration config = Configuration::defaults();

  return config;
}
