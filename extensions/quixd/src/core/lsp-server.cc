#include <glog/logging.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <core/server.hh>
#include <functional>
#include <memory>

/**
 * - Implement the LSP protocol textDocument/didOpen, textDocument/didClose methods
 * - Implement the LSP protocol textDocument/didChange method
 * - Implement the LSP protocol textDocument/completion method
 * - Implement the LSP protocol textDocument/hover method
 * - Implement the LSP protocol textDocument/definition method
 * - Implement the LSP protocol textDocument/references method
 * - Implement the LSP protocol textDocument/documentSymbol method
 * - Implement the LSP protocol textDocument/documentHighlight method
 * - Implement the LSP protocol textDocument/documentLink method
 * - Implement the LSP protocol textDocument/documentColor method
 * - Implement the LSP protocol textDocument/colorPresentation method
 * - Implement the LSP protocol textDocument/formatting method
 * - Implement the LSP protocol textDocument/rangeFormatting method
 * - Implement the LSP protocol textDocument/onTypeFormatting method
 */

ServerDispatcher& ServerDispatcher::the() {
  static ServerDispatcher instance;
  return instance;
}

std::optional<std::unique_ptr<lsp::Message>> next_request(std::iostream& io) {
  try {
    size_t content_length = 0;
    std::string body;

    while (true) { /* Parse the headers */
      std::string header;
      std::getline(io, header);

      if (header.empty() || std::isspace(header[0])) {
        break;
      } else if (header.starts_with("Content-Length: ")) {
        content_length = std::stoul(header.substr(16));
      } else if (header.starts_with("Content-Type: ")) {
        LOG(WARNING) << "Ignoring Content-Type header";
      }
    }

    { /* Read the body */
      body.resize(content_length);
      io.read(body.data(), content_length);
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
        LOG(ERROR) << "Unsupported jsonrpc version: " << doc["jsonrpc"].GetString();
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
          return std::make_unique<lsp::NotificationMessage>(
              doc["method"].GetString(), rapidjson::Value(rapidjson::kObjectType));
        }

        if (!doc["params"].IsObject() && !doc["params"].IsArray()) {
          LOG(ERROR) << "params is not an object or array";
          return std::nullopt;
        }

        return std::make_unique<lsp::NotificationMessage>(doc["method"].GetString(),
                                                          std::move(doc["params"]));
      } else { /* Request or error */
        if (!doc["id"].IsString() && !doc["id"].IsInt()) {
          LOG(ERROR) << "id is not a string or integer";
          return std::nullopt;
        }

        rapidjson::Value params;
        if (doc.HasMember("params")) {
          if (!doc["params"].IsObject() && !doc["params"].IsArray()) {
            LOG(ERROR) << "params is not an object or array";
            return std::nullopt;
          }

          params = std::move(doc["params"]);
        } else {
          params.SetObject();
        }

        if (doc["id"].IsString()) {
          return std::make_unique<lsp::RequestMessage>(
              doc["id"].GetString(), doc["method"].GetString(), std::move(params));
        } else {
          return std::make_unique<lsp::RequestMessage>(
              doc["id"].GetInt(), doc["method"].GetString(), std::move(params));
        }
      }
    }
  } catch (const std::exception& e) {
    LOG(ERROR) << "Failed to parse request: " << e.what();
    return std::nullopt;
  }
}

bool start_language_server(std::iostream& io, const Configuration& config) {
  (void)config;

  ServerDispatcher& dispatcher = ServerDispatcher::the();

  while (true) {
    auto request = next_request(io);
    if (!request) {
      break;
    }

    dispatcher.dispatch(request.value().get(), io);
  }

  return true;
}

void ServerDispatcher::dispatch(const lsp::Message* request, std::iostream& io) {
  if (m_callback) {
    m_callback(request);
  }

  if (request->type() == lsp::MessageType::Request) {
    const lsp::RequestMessage& req = *static_cast<const lsp::RequestMessage*>(request);
    auto response = lsp::ResponseMessage::from_request(req);

    auto it = m_request_handlers.find(req.method());
    if (it == m_request_handlers.end()) {
      LOG(WARNING) << "No request handler for method: " << req.method();
      return;
    }

    it->second(req, response);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    writer.StartObject();
    writer.Key("jsonrpc");
    writer.String("2.0");

    writer.Key("id");
    if (std::holds_alternative<std::string>(response.id())) {
      writer.String(std::get<std::string>(response.id()).c_str());
    } else {
      writer.Int(std::get<int64_t>(response.id()));
    }

    if (response.result().has_value()) {
      writer.Key("result");
      response.result()->Accept(writer);
    } else if (response.error().has_value()) {
      writer.Key("error");
      writer.StartObject();
      writer.Key("code");
      writer.Int(response.error()->m_code);
      writer.Key("message");
      writer.String(response.error()->m_message.c_str());
      if (response.error()->m_data.has_value()) {
        writer.Key("data");
        response.error()->m_data->Accept(writer);
      }
      writer.EndObject();
    } else {
      LOG(ERROR) << "Response has neither result nor error";
      return;
    }

    writer.EndObject();

    std::string response_body(buffer.GetString(), buffer.GetSize());
    response_body =
        "Content-Length: " + std::to_string(response_body.size()) + "\r\n\r\n" + response_body;
    io << response_body;
  } else {
    const lsp::NotificationMessage& notif = *static_cast<const lsp::NotificationMessage*>(request);

    auto it = m_notification_handlers.find(notif.method());
    if (it == m_notification_handlers.end()) {
      LOG(WARNING) << "No notify handler for method: " << notif.method();
      return;
    }

    it->second(notif);
  }
}
