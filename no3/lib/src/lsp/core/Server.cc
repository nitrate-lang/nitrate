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

#include <lsp/core/LSP.hh>
#include <lsp/core/Server.hh>
#include <lsp/route/RoutesList.hh>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Logger.hh>

using namespace ncc;
using namespace no3::lsp::core;
using namespace no3::lsp::message;

class LSPServer::PImpl {
public:
  using RequestHandler = std::function<void(const message::RequestMessage&, message::ResponseMessage&)>;
  using NotificationHandler = std::function<void(const message::NotificationMessage&)>;

  State m_state = State::Suspended;
  std::mutex m_state_mutex;

  std::iostream& m_io;
  std::mutex m_io_mutex;

  PImpl(std::iostream& io) : m_io(io) {}
};

LSPServer::LSPServer(std::iostream& io) : m_pimpl(std::make_unique<PImpl>(io)) {
  /// TODO: Implement
}

LSPServer::~LSPServer() = default;

auto LSPServer::Start() -> bool {
  qcore_assert(m_pimpl != nullptr);

  while (true) {
    std::lock_guard lock(m_pimpl->m_state_mutex);

    switch (auto current_state = m_pimpl->m_state) {
      case State::Suspended: {
        /// TODO:
        break;
      }

      case State::Running: {
        /// TODO: Implement server logic
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
      return true;
    }

    case State::Running: {
      m_pimpl->m_state = State::Suspended;
      return true;
    }

    case State::Exited: {
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
      m_pimpl->m_state = State::Running;
      return true;
    }

    case State::Running: {
      return true;
    }

    case State::Exited: {
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
      m_pimpl->m_state = State::Exited;
      return true;
    }

    case State::Running: {
      m_pimpl->m_state = State::Exited;
      return true;
    }

    case State::Exited: {
      return true;
    }
  }
}

auto LSPServer::GetState() const -> State {
  qcore_assert(m_pimpl != nullptr);
  std::lock_guard lock(m_pimpl->m_state_mutex);

  return m_pimpl->m_state;
}
