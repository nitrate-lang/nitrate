#include <lsp/core/server.hh>
#include <lsp/route/RoutesList.hh>

void do_shutdown(const lsp::RequestMessage&, lsp::ResponseMessage& resp) {
  LOG(INFO) << "Shutdown requested";

  resp->SetObject();
}
