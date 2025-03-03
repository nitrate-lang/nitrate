#include <lsp/route/RoutesList.hh>

using namespace nlohmann;
using namespace no3::lsp;

void srv::DoInitialize(const RequestMessage&, ResponseMessage& resp) {
  auto& json = resp.GetJSON();

  json["serverInfo"]["name"] = "nitrateLanguageServer";
  json["serverInfo"]["version"] = "0.0.1";

  json["capabilities"]["positionEncodings"] = "utf-8";
  json["capabilities"]["textDocumentSync"] = 1;  // Full sync
  json["capabilities"]["documentFormattingProvider"] = true;
}
