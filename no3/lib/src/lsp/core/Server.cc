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

#include <lsp/core/RPC.hh>
#include <lsp/core/Server.hh>
#include <lsp/core/ThreadPool.hh>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp::core;
using namespace no3::lsp::message;

class LSPServer::PImpl {
public:
  State m_state = State::Suspended;
  std::mutex m_state_mutex;
  std::iostream& m_io;
  std::mutex m_io_mutex;
  LSPScheduler m_request_scheduler;

  PImpl(std::iostream& io) : m_io(io), m_request_scheduler(io, m_io_mutex) {}
};

LSPServer::LSPServer(std::iostream& io) : m_pimpl(std::make_unique<PImpl>(io)) {}

LSPServer::~LSPServer() = default;

auto LSPServer::Start() -> bool {
  qcore_assert(m_pimpl != nullptr);

  {
    std::lock_guard lock(m_pimpl->m_state_mutex);
    Log << Trace << "LSPServer: Start(): State::Suspended -> State::Running";
    m_pimpl->m_state = State::Running;
  }

  constexpr size_t kMaxFailedRequestCount = 10;
  size_t sucessive_failed_request_count = 0;

  while (true) {
    std::lock_guard lock(m_pimpl->m_state_mutex);

    switch (auto current_state = m_pimpl->m_state) {
      case State::Suspended: {
        // Minimize CPU usage while waiting for the server to be resumed
        std::this_thread::sleep_for(std::chrono::milliseconds(32));
        break;
      }

      case State::Running: {
        auto request = LSPReadRequest(m_pimpl->m_io, m_pimpl->m_io_mutex);
        if (!request.has_value()) [[unlikely]] {
          sucessive_failed_request_count++;
          Log << "LSPServer: Start(): ReadRequest() failed";

          if (sucessive_failed_request_count > kMaxFailedRequestCount) {
            Log << "LSPServer: Start(): Too many successive invalid requests (max: " << kMaxFailedRequestCount
                << "). Exiting.";
            Log << Trace << "LSPServer: Start(): State::Running -> State::Exited";
            m_pimpl->m_state = State::Exited;
            break;
          }

          break;
        }

        sucessive_failed_request_count = 0;

        auto& scheduler = m_pimpl->m_request_scheduler;
        scheduler.Schedule(std::move(request.value()));

        if (scheduler.IsExitRequested()) [[unlikely]] {
          Log << Trace << "LSPServer: Start(): Exit requested";
          Log << Trace << "LSPServer: Start(): State::Running -> State::Exited";
          m_pimpl->m_state = State::Exited;
          break;
        }

        break;
      }

      case State::Exited: {
        return true;
      }
    }
  }
}

auto LSPServer::Suspend() -> bool {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  switch (auto current_state = m_pimpl->m_state) {
    case State::Suspended: {
      Log << Trace << "LSPServer: Suspend(): State::Suspended -> State::Suspended";
      return true;
    }

    case State::Running: {
      Log << Trace << "LSPServer: Suspend(): State::Running -> State::Suspended";
      m_pimpl->m_state = State::Suspended;
      return true;
    }

    case State::Exited: {
      Log << Trace << "LSPServer: Suspend(): State::Exited -> State::Exited";
      // Already exited, can not suspend
      return false;
    }
  }
}

auto LSPServer::Resume() -> bool {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  switch (auto current_state = m_pimpl->m_state) {
    case State::Suspended: {
      Log << Trace << "LSPServer: Resume(): State::Suspended -> State::Running";
      m_pimpl->m_state = State::Running;
      return true;
    }

    case State::Running: {
      Log << Trace << "LSPServer: Resume(): State::Running -> State::Running";
      return true;
    }

    case State::Exited: {
      Log << Trace << "LSPServer: Resume(): State::Exited -> State::Exited";
      // Already exited, can not resume
      return false;
    }
  }
}

auto LSPServer::Stop() -> bool {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  switch (auto current_state = m_pimpl->m_state) {
    case State::Suspended: {
      Log << Trace << "LSPServer: Stop(): State::Suspended -> State::Exited";
      m_pimpl->m_state = State::Exited;
      return true;
    }

    case State::Running: {
      Log << Trace << "LSPServer: Stop(): State::Running -> State::Exited";
      m_pimpl->m_state = State::Exited;
      return true;
    }

    case State::Exited: {
      Log << Trace << "LSPServer: Stop(): State::Exited -> State::Exited";
      return true;
    }
  }
}

auto LSPServer::GetState() const -> State {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  return m_pimpl->m_state;
}
