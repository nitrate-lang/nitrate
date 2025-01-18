#include <lsp/route/RoutesList.hh>

using namespace no3::lsp;

void srv::DoExit(const NotificationMessage&) {
  LOG(INFO) << "Exiting language server";

  std::exit(0);
}
