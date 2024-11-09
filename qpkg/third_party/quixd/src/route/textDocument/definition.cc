#include <rapidjson/document.h>

#include <core/server.hh>

using namespace rapidjson;

static void do_definition(const lsp::RequestMessage&, lsp::ResponseMessage& resp) {
  /// TODO: Implement definition request

  resp.error(lsp::ErrorCodes::RequestFailed, "Not implemented");
}

ADD_REQUEST_HANDLER("textDocument/definition", do_definition);
