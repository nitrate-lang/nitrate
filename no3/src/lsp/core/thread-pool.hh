#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <thread>

/// FROM: https://stackoverflow.com/questions/15752659/thread-pooling-in-c11

// If a stop is requested, any jobs not currently running will be discarded
// The current jobs [in context] will be cleanly stopped
// It the the callbacks responsibility to check the stop token and return early

class ThreadPool {
public:
  ThreadPool() = default;
  ~ThreadPool() { Stop(); }

  void Start();
  void QueueJob(const std::function<void(std::stop_token)>& job);
  void Stop();
  bool Busy();

private:
  void ThreadLoop(std::stop_token);

  std::mutex m_queue_mutex;
  std::vector<std::jthread> m_threads;
  std::queue<std::function<void(std::stop_token)>> m_jobs;
};
