#include <lsp/core/server.hh>
#include <lsp/route/RoutesList.hh>

void DoExit(const lsp::NotificationMessage&) {
  LOG(INFO) << "Exiting language server";

  exit(0);
}
