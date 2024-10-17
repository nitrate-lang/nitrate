#include <rapidjson/document.h>

#include <core/server.hh>

using namespace rapidjson;

static void do_completion(const lsp::RequestMessage&, lsp::ResponseMessage& resp) {
  /// TODO: Implement completion logic

  auto& alloc = resp->GetAllocator();

  std::vector<std::string> completions = {"foo", "bar", "baz"};

  resp->AddMember("isIncomplete", false, alloc);

  Value items(kArrayType);
  for (const auto& completion : completions) {
    Value item(kObjectType);
    item.AddMember("label", Value(completion.c_str(), alloc).Move(), alloc);
    item.AddMember("kind", 1, alloc);
    items.PushBack(item, alloc);
  }

  resp->AddMember("items", items, alloc);
}

ADD_REQUEST_HANDLER("textDocument/completion", do_completion);
