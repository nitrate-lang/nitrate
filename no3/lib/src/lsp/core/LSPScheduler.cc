////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <lsp/core/LSPScheduler.hh>
#include <lsp/core/protocol/Base.hh>
#include <memory>
#include <mutex>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>
#include <nlohmann/json.hpp>
#include <unordered_set>

using namespace ncc;
using namespace no3::lsp::core;
using namespace no3::lsp::message;
using namespace no3::lsp::protocol;

LSPScheduler::LSPScheduler(std::iostream& io, std::mutex& io_lock) : m_io(io), m_io_lock(io_lock) {}

auto LSPScheduler::IsConcurrentRequest(const message::Message& message) -> bool {
  static const std::unordered_set<std::string_view> parallelizable_messages = {
      ///========================================================================
      /// BEGIN: LSP Lifecycle messages

      ///========================================================================
      /// BEGIN: LSP Document Synchronization messages
  };

  return parallelizable_messages.contains(message.GetMethod());
}

static void WriteRPCResponse(ResponseMessage response, std::ostream& os, std::mutex& os_lock) {
  const auto json_response = nlohmann::to_string(*response.Finalize());

  {  // Critical section
    std::lock_guard lock(os_lock);
    os << "Content-Length: " << json_response.size() << "\r\n";
    os << "Content-Type: application/vscode-jsonrpc; charset=utf-8\r\n\r\n";
    os << json_response;
    os.flush();

    Log << Trace << "WriteRPCResponse(): Wrote response: " << json_response;
  }
}

void LSPScheduler::Schedule(std::unique_ptr<Message> request) {
  if (m_exit_requested) [[unlikely]] {
    Log << Trace << "LSPScheduler: LSPScheduler::Schedule(): Exit requested, ignoring request";
    return;
  }

  {  // Lazy initialization of the thread pool
    if (!m_thread_pool.has_value()) [[unlikely]] {
      Log << Trace << "LSPScheduler: LSPScheduler::Schedule(): Starting thread pool";

      m_thread_pool.emplace();
      m_thread_pool->Start();
    }
  }

  const auto method = request->GetMethod();

  if (IsConcurrentRequest(*request)) {
    std::lock_guard lock(m_fruition);

    Log << Trace << "LSPScheduler: LSPScheduler::Schedule(\"" << method << "\"): Scheduling concurrent request";

    const auto sh = std::make_shared<std::unique_ptr<Message>>(std::move(request));
    m_thread_pool->Schedule([this, sh](const std::stop_token&) {
      bool exit_requested = false;
      if (auto response_opt = m_context.ExecuteRPC(**sh, exit_requested)) {
        WriteRPCResponse(std::move(*response_opt), m_io, m_io_lock);
      }
      m_exit_requested = exit_requested || m_exit_requested;
    });

    return;
  }

  Log << Trace << "LSPScheduler: LSPScheduler::Schedule(\"" << method << "\"): Concurrency disallowed";

  {  // Shall block the primary thread
    std::lock_guard lock(m_fruition);
    while (!m_thread_pool->Empty()) {
      std::this_thread::yield();
    }

    bool exit_requested = false;
    if (auto response_opt = m_context.ExecuteRPC(*request, exit_requested)) {
      WriteRPCResponse(std::move(*response_opt), m_io, m_io_lock);
    }
    m_exit_requested = exit_requested || m_exit_requested;
  }
}