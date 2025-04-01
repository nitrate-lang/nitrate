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

#include <chrono>
#include <lsp/core/Server.hh>
#include <lsp/core/ThreadPool.hh>
#include <mutex>
#include <nitrate-core/Logger.hh>
#include <stop_token>

using namespace ncc;

void ThreadPool::Start() {
  // We want at least 2 threads
  auto optimal_thread_count = std::max(std::jthread::hardware_concurrency(), 2U);
  Log << Info << "Starting thread pool with " << optimal_thread_count << " threads";

  for (uint32_t i = 0; i < optimal_thread_count; ++i) {
    m_threads.emplace_back([this](const std::stop_token& st) { ThreadLoop(st); });
  }
}

void ThreadPool::ThreadLoop(const std::stop_token& st) {
  while (!st.stop_requested()) {
    std::function<void(std::stop_token)> job;

    {
      std::unique_lock lock(m_queue_mutex);
      if (m_jobs.empty()) {
        lock.unlock();

        std::this_thread::yield();
        continue;
      }

      job = m_jobs.front();
      m_jobs.pop();
    }

    job(st);
  }
}

void ThreadPool::Schedule(const std::function<void(std::stop_token st)>& job) {
  std::lock_guard lock(m_queue_mutex);
  m_jobs.push(job);
}

void ThreadPool::WaitForAll() {
  while (Busy()) {
    std::this_thread::yield();
  }
}

auto ThreadPool::Busy() -> bool {
  std::lock_guard lock(m_queue_mutex);
  return !m_jobs.empty();
}

void ThreadPool::Stop() {
  { /* Gracefully request stop */
    std::lock_guard lock(m_queue_mutex);
    for (std::jthread& active_thread : m_threads) {
      active_thread.request_stop();
    }
  }

  while (Busy()) {
  }

  m_threads.clear();
}
