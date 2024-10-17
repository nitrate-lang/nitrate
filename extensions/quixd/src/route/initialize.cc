#include <core/server.hh>

using namespace rapidjson;

static void do_initialize(const lsp::RequestMessage&, lsp::ResponseMessage& resp) {
  LOG(INFO) << "Initializing language server";

  auto& alloc = resp->GetAllocator();

  resp->AddMember("capabilities", Value(kObjectType), alloc);
  resp->AddMember("serverInfo", Value(kObjectType), alloc);

  auto& server_info = (*resp)["serverInfo"];
  server_info.AddMember("name", "quixLanguageServer", alloc);
  server_info.AddMember("version", "0.0.1", alloc);

  auto& capabilities = (*resp)["capabilities"];
  capabilities.AddMember("completionProvider", Value(kObjectType), alloc);
}

ADD_REQUEST_HANDLER("initialize", do_initialize);
