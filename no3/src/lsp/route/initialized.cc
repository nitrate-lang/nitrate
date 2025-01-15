#include <lsp/core/server.hh>
#include <lsp/route/RoutesList.hh>

void DoInitialized(const lsp::NotificationMessage&) {
  LOG(INFO) << "Language server initialized";
}
