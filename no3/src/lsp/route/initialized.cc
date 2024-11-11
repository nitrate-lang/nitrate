#include <lsp/core/server.hh>
#include <lsp/route/RoutesList.hh>

void do_initialized(const lsp::NotificationMessage&) { LOG(INFO) << "Language server initialized"; }
