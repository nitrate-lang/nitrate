#include <core/server.hh>

static void do_initialized(const lsp::NotificationMessage&) {
  LOG(INFO) << "Language server initialized";
}

ADD_NOTIFICATION_HANDLER("initialized", do_initialized);
