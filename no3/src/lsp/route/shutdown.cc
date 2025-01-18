#include <lsp/route/RoutesList.hh>

using namespace no3::lsp;

void srv::DoShutdown(const RequestMessage&, ResponseMessage& resp) {
  LOG(INFO) << "Shutdown requested";

  resp->SetObject();
}
