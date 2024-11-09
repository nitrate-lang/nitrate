#include <core/server.hh>

static void do_shutdown(const lsp::RequestMessage&, lsp::ResponseMessage& resp) {
  LOG(INFO) << "Shutdown requested";

  resp->SetObject();
}

ADD_REQUEST_HANDLER("shutdown", do_shutdown);
