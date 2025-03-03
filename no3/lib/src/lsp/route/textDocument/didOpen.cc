#include <lsp/core/SyncFS.hh>
#include <lsp/route/RoutesList.hh>

using namespace no3::lsp;

void srv::DoDidOpen(const NotificationMessage& notif) {
  if (!notif.GetJSON().contains("textDocument")) {
    Log << "Missing textDocument member";
    return;
  }

  if (!notif.GetJSON()["textDocument"].is_object()) {
    Log << "textDocument is not an object";
    return;
  }

  auto text_document = notif.GetJSON()["textDocument"];

  if (!text_document.contains("uri")) {
    Log << "Missing uri member";
    return;
  }

  if (!text_document["uri"].is_string()) {
    Log << "uri member is not a string";
    return;
  }

  if (!text_document.contains("languageId")) {
    Log << "Missing languageId member";
    return;
  }

  if (!text_document["languageId"].is_string()) {
    Log << "languageId member is not a string";
    return;
  }

  if (!text_document.contains("version")) {
    Log << "Missing version member";
    return;
  }

  if (!text_document["version"].is_number_integer()) {
    Log << "version member is not an integer";
    return;
  }

  if (!text_document.contains("text")) {
    Log << "Missing text member";
    return;
  }

  if (!text_document["text"].is_string()) {
    Log << "text member is not a string";
    return;
  }

  auto uri = text_document["uri"].get<std::string>();
  auto language_id = text_document["languageId"].get<std::string>();
  auto text = text_document["text"].get<std::string>();

  auto file_opt = SyncFS::The().Open(uri);
  if (!file_opt.has_value()) {
    return;
  }

  auto file = file_opt.value();

  if (!file->Replace(0, -1, text)) {
    Log << "Failed to replace code";
    return;
  }
}
