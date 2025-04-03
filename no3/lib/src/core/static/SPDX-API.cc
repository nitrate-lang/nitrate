////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <core/static/SPDX.hh>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/Logger.hh>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

using namespace ncc;

static auto GetAPIEndpoint(const std::string& spdx_id) -> std::string {
  return "https://scancode-licensedb.aboutcode.org/" + spdx_id + ".json";
}

auto no3::constants::GetSPDXLicenseText(const std::string& query) -> std::optional<std::string> {
  std::string name = query;
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);

  Log << Trace << "Preparing to retrieve SPDX license text for SPDX license identifier: " << name;
  Log << Trace << "Checking if SPDX license identifier is an exact match: " << name;

  if (!IsExactSPDXLicenseMatch(name)) {
    Log << Trace << "Failed to retrieve SPDX license text because identifier is not an exact match: " << name;
    return std::nullopt;
  }

  const auto fallible_request = OMNI_CATCH([&] {
    curlpp::Easy request;
    std::string response;

    request.setOpt(curlpp::options::Url(GetAPIEndpoint(name)));
    request.setOpt(curlpp::options::HttpHeader({"User-Agent: nitrate:init/1.0"}));
    request.setOpt(curlpp::options::WriteFunction([&response](const char* data, size_t size, size_t nmemb) {
      response.append(data, size * nmemb);
      return size * nmemb;
    }));

    request.perform();

    return response;
  }());

  if (!fallible_request) {
    Log << "The LICENSE file content couldn't be fetched because an API call to \"" << GetAPIEndpoint(name)
        << "\" failed do to a network error.";
    return std::nullopt;
  }

  Log << Trace << "Parsing response from API for SPDX license text: " << name;

  nlohmann::json json = nlohmann::json::parse(*fallible_request, nullptr, false);
  if (json.is_discarded()) {
    Log << "Failed to parse JSON response from scancode-licensedb.aboutcode.org for SPDX license text: " << name;
    return std::nullopt;
  }

  Log << Trace
      << "Successfully parsed JSON response from scancode-licensedb.aboutcode.org for SPDX license text: " << name;

  if (!json.contains("text")) {
    Log << Trace << "API response did not contain JSON key 'text'";
    Log << Trace << "API response JSON: " << json.dump();
    return std::nullopt;
  }

  if (!json["text"].is_string()) {
    Log << Trace << "API response JSON key 'text'";
    Log << Trace << "API response JSON: " << json.dump();
    return std::nullopt;
  }

  Log << Trace << "Successfully retrieved SPDX license text for SPDX license identifier: " << name;

  return json["text"].get<std::string>() + "\n";
}
