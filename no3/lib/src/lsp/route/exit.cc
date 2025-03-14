#include <lsp/route/RoutesList.hh>

using namespace no3::lsp;

void srv::DoExit(const NotificationMessage&) {
  /// FIXME: Ensure all threads are cleaned up

  std::exit(0);  // NOLINT(concurrency-mt-unsafe)
}
