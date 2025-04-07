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

#include <lsp/server/Scheduler.hh>
#include <lsp/server/Server.hh>
#include <lsp/server/ThreadPool.hh>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp::core;
using namespace no3::lsp::message;

class Server::PImpl {
public:
  State m_state = State::Suspended;
  std::mutex m_state_mutex;
  std::iostream& m_io;
  std::mutex m_is_mutex;
  std::mutex m_os_mutex;
  Scheduler m_request_scheduler;

  PImpl(std::iostream& io) : m_io(io), m_request_scheduler(io, m_os_mutex) {}
};

Server::Server(std::iostream& io) : m_pimpl(std::make_unique<PImpl>(io)) {}

Server::~Server() = default;

auto Server::Start() -> bool {
  qcore_assert(m_pimpl != nullptr);

  {
    std::lock_guard lock(m_pimpl->m_state_mutex);
    Log << Trace << "Server: Start(): State::Suspended -> State::Running";
    m_pimpl->m_state = State::Running;
  }

  constexpr size_t kMaxFailedRequestCount = 3;
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
        auto request = ReadRequest(m_pimpl->m_io, m_pimpl->m_is_mutex);
        if (!request.has_value()) [[unlikely]] {
          sucessive_failed_request_count++;
          Log << "Server: Start(): ReadRequest() failed";

          if (sucessive_failed_request_count > kMaxFailedRequestCount) {
            Log << "Server: Start(): Too many successive invalid requests (max: " << kMaxFailedRequestCount
                << "). Exiting.";
            Log << Trace << "Server: Start(): State::Running -> State::Exited";
            m_pimpl->m_state = State::Exited;
            break;
          }

          break;
        }

        sucessive_failed_request_count = 0;

        auto& scheduler = m_pimpl->m_request_scheduler;
        scheduler.Schedule(std::move(request.value()));

        if (scheduler.IsExitRequested()) [[unlikely]] {
          Log << Trace << "Server: Start(): Exit requested";
          Log << Trace << "Server: Start(): State::Running -> State::Exited";
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

auto Server::Suspend() -> bool {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  switch (auto current_state = m_pimpl->m_state) {
    case State::Suspended: {
      Log << Trace << "Server: Suspend(): State::Suspended -> State::Suspended";
      return true;
    }

    case State::Running: {
      Log << Trace << "Server: Suspend(): State::Running -> State::Suspended";
      m_pimpl->m_state = State::Suspended;
      return true;
    }

    case State::Exited: {
      Log << Trace << "Server: Suspend(): State::Exited -> State::Exited";
      // Already exited, can not suspend
      return false;
    }
  }
}

auto Server::Resume() -> bool {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  switch (auto current_state = m_pimpl->m_state) {
    case State::Suspended: {
      Log << Trace << "Server: Resume(): State::Suspended -> State::Running";
      m_pimpl->m_state = State::Running;
      return true;
    }

    case State::Running: {
      Log << Trace << "Server: Resume(): State::Running -> State::Running";
      return true;
    }

    case State::Exited: {
      Log << Trace << "Server: Resume(): State::Exited -> State::Exited";
      // Already exited, can not resume
      return false;
    }
  }
}

auto Server::Stop() -> bool {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  switch (auto current_state = m_pimpl->m_state) {
    case State::Suspended: {
      Log << Trace << "Server: Stop(): State::Suspended -> State::Exited";
      m_pimpl->m_state = State::Exited;
      return true;
    }

    case State::Running: {
      Log << Trace << "Server: Stop(): State::Running -> State::Exited";
      m_pimpl->m_state = State::Exited;
      return true;
    }

    case State::Exited: {
      Log << Trace << "Server: Stop(): State::Exited -> State::Exited";
      return true;
    }
  }
}

auto Server::GetState() const -> State {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  return m_pimpl->m_state;
}
