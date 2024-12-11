#pragma once

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
    length = length >= 0 ? length : m_content->size() - length + 1;

    if (offset + length > m_content->size()) {
      return false;
    }

    m_content->replace(offset, length, text);

    return true;
  }

  Digest thumbprint();

  std::shared_ptr<const std::string> content() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_content;
  }

  size_t size() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_content->size();
  };

  void set_content(std::shared_ptr<std::string> content) {
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
