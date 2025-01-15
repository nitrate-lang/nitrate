#include <rapidjson/document.h>

#include <lsp/core/server.hh>
#include <lsp/route/RoutesList.hh>

using namespace rapidjson;

void DoDefinition(const lsp::RequestMessage&, lsp::ResponseMessage& resp) {
  /// TODO: Implement definition request

  resp.Error(lsp::ErrorCodes::RequestFailed, "Not implemented");
}
