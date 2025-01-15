#include <lsp/core/server.hh>
#include <lsp/route/RoutesList.hh>

void DoShutdown(const lsp::RequestMessage&, lsp::ResponseMessage& resp) {
  LOG(INFO) << "Shutdown requested";

  resp->SetObject();
}
