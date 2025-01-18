#include <lsp/core/SyncFS.hh>
#include <lsp/route/RoutesList.hh>

using namespace no3::lsp;

void srv::DoDidOpen(const NotificationMessage& notif) {
  if (!notif.GetJSON().contains("textDocument")) {
    LOG(ERROR) << "Missing textDocument member";
    return;
  }

  if (!notif.GetJSON()["textDocument"].is_object()) {
    LOG(ERROR) << "textDocument is not an object";
    return;
  }

  auto text_document = notif.GetJSON()["textDocument"];

  if (!text_document.contains("uri")) {
    LOG(ERROR) << "Missing uri member";
    return;
  }

  if (!text_document["uri"].is_string()) {
    LOG(ERROR) << "uri member is not a string";
    return;
  }

  if (!text_document.contains("languageId")) {
    LOG(ERROR) << "Missing languageId member";
    return;
  }

  if (!text_document["languageId"].is_string()) {
    LOG(ERROR) << "languageId member is not a string";
    return;
  }

  if (!text_document.contains("version")) {
    LOG(ERROR) << "Missing version member";
    return;
  }

  if (!text_document["version"].is_number_integer()) {
    LOG(ERROR) << "version member is not an integer";
    return;
  }

  if (!text_document.contains("text")) {
    LOG(ERROR) << "Missing text member";
    return;
  }

  if (!text_document["text"].is_string()) {
    LOG(ERROR) << "text member is not a string";
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
    LOG(ERROR) << "Failed to replace code";
    return;
  }
}
