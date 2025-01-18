#pragma once

#include <glog/logging.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

class SyncFSFile {
  std::shared_ptr<std::string> m_content;
  std::mutex m_mutex;

public:
  SyncFSFile() = default;
  using Digest = std::array<uint8_t, 20>;

  auto Replace(size_t offset, int64_t length, std::string_view text) -> bool {
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

  auto Content() -> std::shared_ptr<const std::string> {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_content;
  }

  auto Size() -> size_t {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_content->size();
  };

  void SetContent(std::shared_ptr<std::string> content) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_content = std::move(content);
  }
};

class SyncFS {
  struct Impl;
  std::unordered_map<std::string, std::shared_ptr<SyncFSFile>> m_files;
  std::mutex m_mutex;

  SyncFS();
  ~SyncFS();

public:
  SyncFS(const SyncFS&) = delete;
  auto operator=(const SyncFS&) -> SyncFS& = delete;
  SyncFS(SyncFS&&) = delete;

  static auto The() -> SyncFS&;

  auto Open(std::string path) -> std::optional<std::shared_ptr<SyncFSFile>>;

  void Close(const std::string& name);
};
