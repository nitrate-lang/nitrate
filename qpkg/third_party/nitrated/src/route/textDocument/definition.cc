#include <rapidjson/document.h>

#include <core/server.hh>
#include <route/RoutesList.hh>

using namespace rapidjson;

void do_definition(const lsp::RequestMessage&, lsp::ResponseMessage& resp) {
  /// TODO: Implement definition request

  resp.error(lsp::ErrorCodes::RequestFailed, "Not implemented");
}
