#include <rapidjson/document.h>

#include <core/server.hh>

using namespace rapidjson;

class ParseTree {};

class ParseTreeSet {
public:
  static ParseTreeSet& the();

  std::optional<std::shared_ptr<ParseTree>> get(std::string_view uri) const;

};

static void do_completion(const lsp::RequestMessage&, lsp::ResponseMessage& resp) {
  /// TODO: Implement completion logic

  /**
   * 1. select the uri for SyncFS
   * 2. Get parse tree
   * 3. Iterate over the parse tree to build a set of identifiers
   * 4. Cache the above steps
   * 5. Complete on that set
   */

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
