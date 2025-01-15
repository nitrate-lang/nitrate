#include <chrono>
#include <lsp/core/server.hh>
#include <lsp/core/thread-pool.hh>
#include <mutex>
#include <stop_token>

void ThreadPool::Start() {
  uint32_t num_threads =
      std::jthread::hardware_concurrency();  // Max # of threads the system
                                             // supports

  // We want at least 2 threads
  if (num_threads < 2) {
    num_threads = 2;
  }

  LOG(INFO) << "Starting thread pool with " << num_threads << " threads";

  for (uint32_t ii = 0; ii < num_threads; ++ii) {
    m_threads.emplace_back(
        std::jthread([this](std::stop_token st) { ThreadLoop(st); }));
  }
}

void ThreadPool::ThreadLoop(std::stop_token st) {
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

void ThreadPool::QueueJob(const std::function<void(std::stop_token st)>& job) {
  std::lock_guard<std::mutex> lock(m_queue_mutex);
  m_jobs.push(job);
}

bool ThreadPool::Busy() {
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

  while (Busy());

  m_threads.clear();
}
