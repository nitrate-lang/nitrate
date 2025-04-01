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
  uint32_t num_threads = std::jthread::hardware_concurrency();  // Max # of threads the system
                                                                // supports

  // We want at least 2 threads
  if (num_threads < 2) {
    num_threads = 2;
  }

  Log << Info << "Starting thread pool with " << num_threads << " threads";

  for (uint32_t ii = 0; ii < num_threads; ++ii) {
    m_threads.emplace_back([this](const std::stop_token& st) { ThreadLoop(st); });
  }
}

void ThreadPool::ThreadLoop(const std::stop_token& st) {
  bool has_any_yet = false;

  while (!st.stop_requested()) {
    std::function<void(std::stop_token)> job;
    {
      std::unique_lock<std::mutex> lock(m_queue_mutex);
      if (m_jobs.empty()) {
        lock.unlock();

        // Climate change is real, lets do our part
        if (!has_any_yet) {
          std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } else {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        continue;
      }

      has_any_yet = true;
      job = m_jobs.front();
      m_jobs.pop();
    }

    job(st);
  }
}

void ThreadPool::Schedule(const std::function<void(std::stop_token st)>& job) {
  std::lock_guard<std::mutex> lock(m_queue_mutex);
  m_jobs.push(job);
}

auto ThreadPool::Busy() -> bool {
  bool poolbusy;
  {
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    poolbusy = !m_jobs.empty();
  }
  return poolbusy;
}

void ThreadPool::Stop() {
  { /* Gracefully request stop */
    std::lock_guard<std::mutex> lock(m_queue_mutex);
    for (std::jthread& active_thread : m_threads) {
      active_thread.request_stop();
    }
  }

  while (Busy()) {
  }

  m_threads.clear();
}
