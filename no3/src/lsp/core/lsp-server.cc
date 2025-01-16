#include <glog/logging.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <chrono>
#include <functional>
#include <lsp/core/server.hh>
#include <lsp/route/RoutesList.hh>
#include <memory>
#include <stop_token>

using namespace lsp;

auto ServerContext::The() -> ServerContext& {
  static ServerContext instance;
  return instance;
}

void ServerContext::RequestQueueLoop(std::stop_token st) {
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

auto ServerContext::NextMessage(
    std::istream& io) -> std::optional<std::unique_ptr<Message>> {
  /**
   * We don't need to lock the std::istream because this is the only place where
   * we read from it in a single threaded context. The ostream is seperately
   * locked because it is written to from any number of threads.
   */

  size_t remaining_bytes = 0;
  std::string body;

  while (true) { /* Parse the headers */
    std::string header;
    std::getline(io, header);

    if (header.ends_with("\r")) {
      header.pop_back();
    }

    if (header.empty()) {
      break;
    } else if (header.starts_with("Content-Length: ")) {
      remaining_bytes = std::stoul(header.substr(16));
    } else if (header.starts_with("Content-Type: ")) {
      LOG(WARNING) << "Ignoring Content-Type header";
    } else {
      LOG(WARNING) << "Ignoring unknown header: " << header;
    }
  }

  if (remaining_bytes == 0) {
    return std::nullopt;
  }

  { /* Read the body */
    body.resize(remaining_bytes);
    size_t bytes_read = 0;

    while (remaining_bytes > 0) {
      if (!io.read(body.data() + bytes_read, remaining_bytes)) {
        LOG(ERROR) << "Failed to read message body";
        return std::nullopt;
      }

      bytes_read += io.gcount();
      remaining_bytes -= io.gcount();
    }
  }

  { /* Parse the body */
    rapidjson::Document doc;
    doc.Parse(body.c_str(), body.size());

    if (doc.HasParseError()) {
      LOG(ERROR) << "Failed to parse JSON: " << doc.GetParseError();
      return std::nullopt;
    }

    if (!doc.HasMember("jsonrpc")) {
      LOG(ERROR) << "Request is missing jsonrpc";
      return std::nullopt;
    }

    if (!doc["jsonrpc"].IsString()) {
      LOG(ERROR) << "jsonrpc is not a string";
      return std::nullopt;
    }

    if (doc["jsonrpc"].GetString() != std::string_view("2.0")) {
      LOG(ERROR) << "Unsupported jsonrpc version: "
                 << doc["jsonrpc"].GetString();
      return std::nullopt;
    }

    if (!doc.HasMember("method")) {
      LOG(ERROR) << "Request is missing method";
      return std::nullopt;
    }

    if (!doc["method"].IsString()) {
      LOG(ERROR) << "method is not a string";
      return std::nullopt;
    }

    if (!doc.HasMember("id")) { /* Notification */
      if (!doc.HasMember("params")) {
        return std::make_unique<NotificationMessage>(
            doc["method"].GetString(),
            rapidjson::Document(rapidjson::kObjectType));
      }

      if (!doc["params"].IsObject() && !doc["params"].IsArray()) {
        LOG(ERROR) << "params is not an object or array";
        return std::nullopt;
      }

      rapidjson::Document params;
      params.CopyFrom(doc["params"], params.GetAllocator());
      return std::make_unique<NotificationMessage>(doc["method"].GetString(),
                                                   std::move(params));
    } else { /* Request or error */
      if (!doc["id"].IsString() && !doc["id"].IsInt()) {
        LOG(ERROR) << "id is not a string or integer";
        return std::nullopt;
      }

      rapidjson::Document params;
      if (doc.HasMember("params")) {
        if (!doc["params"].IsObject() && !doc["params"].IsArray()) {
          LOG(ERROR) << "params is not an object or array";
          return std::nullopt;
        }

        params.CopyFrom(doc["params"], params.GetAllocator());
      } else {
        params.SetObject();
      }

      if (doc["id"].IsString()) {
        return std::make_unique<RequestMessage>(doc["id"].GetString(),
                                                doc["method"].GetString(),
                                                std::move(params));
      } else {
        return std::make_unique<RequestMessage>(
            doc["id"].GetInt(), doc["method"].GetString(), std::move(params));
      }
    }
  }
}

void ServerContext::RegisterHandlers() {
  auto& ctx = ServerContext::The();

  ctx.RegisterRequestHandler("initialize", DoInitialize);
  ctx.RegisterNotificationHandler("initialized", DoInitialized);
  ctx.RegisterRequestHandler("shutdown", DoShutdown);
  ctx.RegisterNotificationHandler("exit", DoExit);

  ctx.RegisterRequestHandler("textDocument/completion", DoCompletion);
  ctx.RegisterRequestHandler("textDocument/declaration", DoDeclaration);
  ctx.RegisterRequestHandler("textDocument/definition", DoDefinition);
  ctx.RegisterNotificationHandler("textDocument/didChange", DoDidChange);
  ctx.RegisterNotificationHandler("textDocument/didClose", DoDidClose);
  ctx.RegisterNotificationHandler("textDocument/didOpen", DoDidOpen);
  ctx.RegisterNotificationHandler("textDocument/didSave", DoDidSave);
  ctx.RegisterRequestHandler("textDocument/documentColor", DoDocumentColor);
  ctx.RegisterRequestHandler("textDocument/formatting", DoFormatting);
}

[[noreturn]] void ServerContext::StartServer(Connection& io) {
  RegisterHandlers();

  m_thread_pool.Start();
  m_thread_pool.QueueJob([this](std::stop_token st) { RequestQueueLoop(st); });

  while (true) {
    auto message = NextMessage(*io.first);
    if (!message.has_value()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }

    std::shared_ptr<Message> message_ptr = std::move(*message);
    Dispatch(message_ptr, *io.second);
  }
}

void ServerContext::HandleRequest(const RequestMessage& req, std::ostream& io) {
  if (m_callback) {
    m_callback(&req);
  }

  auto response = ResponseMessage::FromRequest(req);

  auto it = m_request_handlers.find(req.Method());
  if (it == m_request_handlers.end()) {
    if (req.Method().starts_with("$/")) {
      LOG(INFO) << "Ignoring request: " << req.Method();
      return;
    }
    LOG(WARNING) << "No request handler for method: " << req.Method();
    return;
  }

  it->second(req, response);

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  writer.StartObject();
  writer.Key("jsonrpc");
  writer.String("2.0");

  writer.Key("id");
  if (std::holds_alternative<std::string>(response.Id())) {
    writer.String(std::get<std::string>(response.Id()).c_str());
  } else {
    writer.Int(std::get<int64_t>(response.Id()));
  }

  if (response.Error().has_value()) {
    writer.Key("error");
    writer.StartObject();
    writer.Key("code");
    writer.Int((int)response.Error()->m_code);
    writer.Key("message");
    writer.String(response.Error()->m_message.c_str());
    if (response.Error()->m_data.has_value()) {
      writer.Key("data");
      response.Error()->m_data->Accept(writer);
    }
    writer.EndObject();
  } else {
    if (response.Result().has_value()) {
      writer.Key("result");
      response.Result()->Accept(writer);
    } else {
      writer.Key("result");
      writer.Null();
    }
  }

  writer.EndObject();

  {
    /**
     * We must guard the ostream because it is written to from any number of
     * threads. The language server protocol dicates the use of message id to
     * distinguish between different transactions
     * (RequestMessage/ResponseMessage pairs).
     */

    std::lock_guard<std::mutex> lock(m_io_mutex);
    io << "Content-Length: " << std::to_string(buffer.GetSize()) << "\r\n\r\n"
       << std::string_view(buffer.GetString(), buffer.GetSize());
  }
}

void ServerContext::HandleNotification(const NotificationMessage& notif) {
  if (m_callback) {
    m_callback(&notif);
  }

  auto it = m_notification_handlers.find(notif.Method());
  if (it == m_notification_handlers.end()) {
    if (notif.Method().starts_with("$/")) {
      LOG(INFO) << "Ignoring notification: " << notif.Method();
      return;
    }

    LOG(WARNING) << "No notify handler for method: " << notif.Method();
    return;
  }

  it->second(notif);
}

void ServerContext::Dispatch(const std::shared_ptr<Message> message,
                             std::ostream& io) {
  if (message->Type() == MessageType::Request) {
    std::lock_guard<std::mutex> lock(m_request_queue_mutex);
    m_request_queue.push([this, message, &io]() {
      HandleRequest(*std::static_pointer_cast<RequestMessage>(message), io);
    });
  } else if (message->Type() == MessageType::Notification) {
    m_thread_pool.QueueJob([this, message](std::stop_token) {
      HandleNotification(
          *std::static_pointer_cast<NotificationMessage>(message));
    });
  } else {
    LOG(ERROR) << "Unsupported message type";
  }
}
