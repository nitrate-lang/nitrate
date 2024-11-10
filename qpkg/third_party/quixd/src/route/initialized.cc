#include <core/server.hh>
#include <route/RoutesList.hh>

void do_initialized(const lsp::NotificationMessage&) { LOG(INFO) << "Language server initialized"; }
