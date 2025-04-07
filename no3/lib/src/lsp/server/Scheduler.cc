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

#include <lsp/protocol/Base.hh>
#include <lsp/server/Scheduler.hh>
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

class Scheduler::PImpl {
public:
  std::optional<ThreadPool> m_thread_pool;
  std::atomic<bool> m_exit_requested = false;

  Context m_context;
  std::mutex m_fruition;

  PImpl(std::ostream& os, std::mutex& os_lock) : m_context(os, os_lock) {}

  static auto IsConcurrentRequest(const message::Message& message) -> bool {
    static const std::unordered_set<std::string_view> parallelizable_messages = {
        ///========================================================================
        /// BEGIN: LSP Lifecycle messages
        "$/setTrace",

        ///========================================================================
        /// BEGIN: LSP Document Synchronization messages

        ///========================================================================
        /// BEGIN: LSP Feature messages
        "textDocument/completion",
    };

    return parallelizable_messages.contains(message.GetMethod());
  }
};

void Scheduler::Schedule(std::unique_ptr<Message> request) {
  qcore_assert(m_pimpl != nullptr);

  auto& m = *m_pimpl;

  if (m.m_exit_requested) [[unlikely]] {
    Log << Trace << "Scheduler: Scheduler::Schedule(): Exit requested, ignoring request";
    return;
  }

  {  // Lazy initialization of the thread pool
    if (!m.m_thread_pool.has_value()) [[unlikely]] {
      Log << Trace << "Scheduler: Scheduler::Schedule(): Starting thread pool";

      m.m_thread_pool.emplace();
      m.m_thread_pool->Start();
    }
  }

  const auto method = request->GetMethod();

  if (PImpl::IsConcurrentRequest(*request)) {
    std::lock_guard lock(m.m_fruition);

    Log << Trace << "Scheduler: Scheduler::Schedule(\"" << method << "\"): Scheduling concurrent request";

    const auto sh = std::make_shared<std::unique_ptr<Message>>(std::move(request));
    m.m_thread_pool->Schedule([this, sh](const std::stop_token&) {
      auto& m = *m_pimpl;

      bool exit_requested = false;
      m.m_context.ExecuteRPC(**sh, exit_requested);
      m.m_exit_requested = exit_requested || m.m_exit_requested;
    });

    return;
  }

  Log << Trace << "Scheduler: Scheduler::Schedule(\"" << method << "\"): Concurrency disallowed";

  {  // Shall block the primary thread
    std::lock_guard lock(m.m_fruition);
    while (!m.m_thread_pool->Empty()) {
      std::this_thread::yield();
    }

    bool exit_requested = false;
    m.m_context.ExecuteRPC(*request, exit_requested);
    m.m_exit_requested = exit_requested || m.m_exit_requested;
  }
}

bool Scheduler::IsExitRequested() const {
  qcore_assert(m_pimpl != nullptr);
  return m_pimpl->m_exit_requested;
}

Scheduler::Scheduler(std::ostream& os, std::mutex& os_lock) : m_pimpl(std::make_unique<PImpl>(os, os_lock)) {}

Scheduler::~Scheduler() = default;
