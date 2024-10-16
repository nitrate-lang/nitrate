#include <core/server.hh>

static void do_exit(const lsp::NotificationMessage&) {
  LOG(INFO) << "Exiting language server";

  exit(0);
}

ADD_NOTIFICATION_HANDLER("exit", do_exit);
