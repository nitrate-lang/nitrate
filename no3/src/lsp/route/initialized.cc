#include <lsp/route/RoutesList.hh>

using namespace no3::lsp;

void srv::DoInitialized(const NotificationMessage&) {
  LOG(INFO) << "Language server initialized";
}
