#include <fstream>
#include <lsp/core/Server.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp;

auto srv::ParseConfig(const std::filesystem::path& path) -> std::optional<srv::Configuration> {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    Log << "Failed to open file: " << path;
    return std::nullopt;
  }

  nlohmann::json doc = nlohmann::json::parse(ifs, nullptr, false);

  if (doc.is_discarded()) {
    Log << "Failed to parse JSON config file: " << path;
    return std::nullopt;
  }

  if (!doc.is_object()) {
    Log << "Expected object at JSON root, got " << doc.type_name();
    return std::nullopt;
  }

  if (!doc.contains("version")) {
    Log << "Expected 'version' field in the config file, not found";
    return std::nullopt;
  }

  if (!doc["version"].is_number()) {
    Log << "Expected 'version' field in the config file to be an integer";
    return std::nullopt;
  }

  if (doc["version"].get<int>() != 1) {
    Log << "Unsupported config file version. Only version 1 is supportted now";
    return std::nullopt;
  }

  ///=================== CONFIG VERSION 1 ======================
  Configuration config = Configuration::Defaults();

  return config;
}
