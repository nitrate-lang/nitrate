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
#include <lsp/server/ThreadPool.hh>
#include <mutex>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/SmartLock.hh>
#include <stop_token>

using namespace ncc;

void ThreadPool::Start() {
  auto optimal_thread_count = std::max(std::jthread::hardware_concurrency(), 1U);
  Log << Debug << "Starting thread pool with " << optimal_thread_count << " threads";

  // Enable thread synchronization
  EnableSync = true;

  auto parent_thread_logger = Log;

  for (uint32_t i = 0; i < optimal_thread_count; ++i) {
    m_threads.emplace_back([this, parent_thread_logger](const std::stop_token& st) {
      // Use the logger from the parent thread
      Log = parent_thread_logger;
      ThreadLoop(st);
    });
  }
}

void ThreadPool::ThreadLoop(const std::stop_token& st) {
  Log << Trace << "ThreadPool: ThreadLoop(" << std::this_thread::get_id() << ") started";

  while (!st.stop_requested()) {
    Task job;

    {
      std::unique_lock lock(m_queue_mutex);
      if (m_jobs.empty()) {
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::microseconds(64));
        std::this_thread::yield();
        continue;
      }

      job = m_jobs.front();
      m_jobs.pop();
    }

    job(st);
  }

  Log << Trace << "ThreadPool: ThreadLoop(" << std::this_thread::get_id() << ") stopped";
}

void ThreadPool::Schedule(Task job) {
  std::lock_guard lock(m_queue_mutex);
  m_jobs.emplace(std::move(job));
}

void ThreadPool::WaitForAll() {
  while (!Empty()) {
    std::this_thread::yield();
  }
}

auto ThreadPool::Empty() -> bool {
  std::lock_guard lock(m_queue_mutex);
  return m_jobs.empty();
}

void ThreadPool::Stop() {
  { /* Gracefully request stop */
    std::lock_guard lock(m_queue_mutex);
    for (auto& active_thread : m_threads) {
      active_thread.request_stop();
    }
  }

  while (!Empty()) {
  }

  m_threads.clear();
}
