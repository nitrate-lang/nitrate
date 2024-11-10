#include <core/server.hh>
#include <route/RoutesList.hh>

void do_exit(const lsp::NotificationMessage&) {
  LOG(INFO) << "Exiting language server";

  exit(0);
}
