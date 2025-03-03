#include <chrono>
#include <functional>
#include <lsp/core/LSP.hh>
#include <lsp/core/Server.hh>
#include <lsp/route/RoutesList.hh>
#include <memory>
#include <mutex>
#include <nitrate-core/Logger.hh>
#include <shared_mutex>
#include <stop_token>
#include <utility>
#include <variant>

using namespace ncc;
using namespace no3::lsp::srv;

auto ServerContext::The() -> ServerContext& {
  static ServerContext instance;
  return instance;
}

void ServerContext::RequestQueueLoop(const std::stop_token& st) {
  while (!st.stop_requested()) {
    std::function<void()> job;
    {
      std::unique_lock<std::mutex> lock(m_request_queue_mutex);

      if (m_request_queue.empty()) {
        lock.unlock();

        // Climate change is real, lets do our part
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        continue;
      }

      job = m_request_queue.front();
      m_request_queue.pop();
    }

    job();
  }
}

auto ServerContext::NextMessage(std::istream& in) -> std::optional<MessageObject> {
  /**
   * We don't need to lock the std::istream because this is the only place where
   * we read from it in a single threaded context. The ostream is seperately
   * locked because it is written to from any number of threads.
   */

  size_t remaining_bytes = 0;
  std::string body;
  std::string header;

  while (true) { /* Parse the headers */
    header.clear();

    std::getline(in, header);

    if (header.ends_with("\r")) {
      header.pop_back();
    }

    if (header.empty()) {
      break;
    }

    if (header.starts_with("Content-Length: ")) {
      remaining_bytes = std::stoul(header.substr(16));
    } else if (header.starts_with("Content-Type: ")) {
      Log << Warning << "The Content-Type header is ignored";
    } else {
      Log << Warning << "Discarding unrecognized header: " << header;
    }
  }

  if (remaining_bytes == 0) {
    return std::nullopt;
  }

  { /* Read the body */
    body.resize(remaining_bytes);
    size_t bytes_read = 0;

    while (remaining_bytes > 0) {
      if (!in.read(body.data() + bytes_read, remaining_bytes)) {
        Log << "Failed to read message body";
        return std::nullopt;
      }

      bytes_read += in.gcount();
      remaining_bytes -= in.gcount();
    }
  }

  Log << Info << "Received message: " << body;

  nlohmann::json doc = nlohmann::json::parse(body, nullptr, false);
  body.clear();

  if (doc.is_discarded()) {
    Log << "Failed to parse JSON: " << doc;
    return std::nullopt;
  }

  if (!doc.contains("jsonrpc")) [[unlikely]] {
    Log << "Request object is missing key \"jsonrpc\"";
    return std::nullopt;
  }

  if (!doc["jsonrpc"].is_string()) [[unlikely]] {
    Log << "Request object key \"jsonrpc\" is not a string";
    return std::nullopt;
  }

  if (doc["jsonrpc"].get<std::string>() != std::string_view("2.0")) [[unlikely]] {
    Log << "Client is using unsupported LSP JSON-RPC version";
    Log << Info << "Client is using version: " << doc["jsonrpc"].get<std::string>();
    Log << Info << "Server only supports version: 2.0";

    return std::nullopt;
  }

  if (!doc.contains("method")) [[unlikely]] {
    Log << "Request object is missing key \"method\"";
    return std::nullopt;
  }

  if (!doc["method"].is_string()) [[unlikely]] {
    Log << "Request object key \"method\" is not a string";
    return std::nullopt;
  }

  nlohmann::json params = nlohmann::json::object();

  if (doc.contains("params")) {
    if (!doc["params"].is_object() && !doc["params"].is_array()) [[unlikely]] {
      Log << "Method parameters is not an object or array";
      return std::nullopt;
    }

    params = doc["params"];
  }

  std::optional<MessageObject> message;

  bool is_lsp_notification = !doc.contains("id");

  if (is_lsp_notification) {
    message = MessageObject(NotificationMessage(String(doc["method"].get<std::string>()), std::move(params)));
  } else {
    if (!doc["id"].is_string() && !doc["id"].is_number()) [[unlikely]] {
      Log << "Request object key \"id\" is not a string or int";
      return std::nullopt;
    }

    if (doc["id"].is_string()) {
      message = MessageObject(
          RequestMessage(String(doc["id"].get<std::string>()), doc["method"].get<std::string>(), std::move(params)));
    } else {
      message =
          MessageObject(RequestMessage(doc["id"].get<int64_t>(), doc["method"].get<std::string>(), std::move(params)));
    }
  }

  return message;
}

void ServerContext::RegisterHandlers() {
  auto& ctx = ServerContext::The();

  ctx.RegisterRequestHandler("initialize", DoInitialize);
  ctx.RegisterNotificationHandler("initialized", DoInitialized);
  ctx.RegisterRequestHandler("shutdown", DoShutdown);
  ctx.RegisterNotificationHandler("exit", DoExit);

  ctx.RegisterNotificationHandler("$/setTrace", SetTrace);

  ctx.RegisterNotificationHandler("textDocument/didChange", DoDidChange);
  ctx.RegisterNotificationHandler("textDocument/didClose", DoDidClose);
  ctx.RegisterNotificationHandler("textDocument/didOpen", DoDidOpen);
  ctx.RegisterNotificationHandler("textDocument/didSave", DoDidSave);

  ctx.RegisterRequestHandler("textDocument/formatting", DoFormatting);
}

[[noreturn]] void ServerContext::StartServer(Connection& io) {
  RegisterHandlers();

  m_thread_pool.Start();
  m_thread_pool.QueueJob([this](auto st) { RequestQueueLoop(st); });

  while (true) {
    auto message = NextMessage(*io.first);
    if (!message.has_value()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }

    Dispatch(message.value(), *io.second);
  }
}

void ServerContext::DoRequest(const RequestMessage& req, std::ostream& out) {
  Log << Info << "Handling request: \"" << req.GetMethod() << "\"";

  auto sub_response = ResponseMessage::FromRequest(req);

  auto it = m_request_handlers.find(req.GetMethod());
  if (it == m_request_handlers.end()) {
    if (req.GetMethod()->starts_with("$/")) {
      Log << Info << "Ignoring request: " << req.GetMethod();
      return;
    }
    Log << Warning << "No request handler for method: " << req.GetMethod();
    return;
  }

  it->second(req, sub_response);

  nlohmann::json response;
  response["jsonrpc"] = "2.0";

  if (req.GetMID().GetKind() == MessageIdKind::String) {
    response["id"] = req.GetMID().GetString();
  } else {
    response["id"] = req.GetMID().GetInt();
  }

  if (sub_response.Error().has_value()) {
    response["error"] = nlohmann::json::object();
    response["error"]["code"] = (int64_t)sub_response.Error()->m_code;
    response["error"]["message"] = sub_response.Error()->m_message;
    response["error"]["data"] = sub_response.Error()->m_data.value_or(nlohmann::json::object());
  } else {
    response["result"] = sub_response.GetJSON();
  }

  {
    /**
     * We must guard the ostream because it is written to from any number of
     * threads. The language server protocol dicates the use of message id to
     * distinguish between different transactions
     * (RequestMessage/ResponseMessage pairs).
     */

    auto buffer = response.dump(0);

    { /* Write the response */
      std::lock_guard<std::mutex> lock(m_io_mutex);

      out << "Content-Length: " << std::to_string(buffer.size()) << "\r\n\r\n" << buffer;

      out.flush();
    }
  }
}

void ServerContext::DoNotification(const NotificationMessage& notif) {
  Log << Info << "Handling notification: \"" << notif.Method() << "\"";

  auto it = m_notification_handlers.find(notif.Method());
  if (it == m_notification_handlers.end()) {
    if (notif.Method()->starts_with("$/")) {
      Log << Info << "Ignoring notification: " << notif.Method();
      return;
    }

    Log << Warning << "No notify handler for method: " << notif.Method();
    return;
  }

  it->second(notif);
}

void ServerContext::Dispatch(MessageObject message, std::ostream& out) {
  if (std::holds_alternative<RequestMessage>(message.get())) {
    std::lock_guard<std::mutex> lock(m_request_queue_mutex);

    m_request_queue.emplace([this, message, &out]() {
      std::shared_lock<std::shared_mutex> lock(m_task_mutex);

      DoRequest(std::get<RequestMessage>(message.get()), out);
    });
  } else {
    std::unique_lock<std::shared_mutex> lock(m_task_mutex);

    DoNotification(std::get<NotificationMessage>(message.get()));
  }
}
