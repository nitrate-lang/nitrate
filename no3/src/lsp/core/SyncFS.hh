#pragma once

#include <glog/logging.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

class SyncFSFile {
  std::shared_ptr<std::string> m_content;
  std::mutex m_mutex;

public:
  SyncFSFile() = default;
  using Digest = std::array<uint8_t, 20>;

  bool replace(size_t offset, int64_t length, std::string_view text) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (length < 0) {  // negative length starts from the end
      length = m_content->size() - offset + length + 1;
    }

    if (offset + length > m_content->size()) {
      LOG(ERROR) << "Invalid replace operation: offset=" << offset
                 << ", length=" << length << ", size=" << m_content->size();
      return false;
    }

    m_content->replace(offset, length, text);

    return true;
  }

  std::shared_ptr<const std::string> content() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_content;
  }

  size_t size() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_content->size();
  };

  void SetContent(std::shared_ptr<std::string> content) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_content = content;
  }
};

class SyncFS {
  struct Impl;
  std::unordered_map<std::string, std::shared_ptr<SyncFSFile>> m_files;
  std::mutex m_mutex;

  SyncFS();
  ~SyncFS();

  SyncFS(const SyncFS&) = delete;
  SyncFS& operator=(const SyncFS&) = delete;
  SyncFS(SyncFS&&) = delete;

public:
  static SyncFS& the();

  std::optional<std::shared_ptr<SyncFSFile>> open(std::string name);

  void close(const std::string& name);
};
