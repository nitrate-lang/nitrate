#include <cmath>
#include <core/SyncFS.hh>
#include <core/server.hh>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>

struct SyncFS::Impl {
  class File {
    std::string m_content;
    std::string m_mime_type;
    std::vector<std::pair<size_t, size_t>> m_line_to_offset;

    /* Result includes the EOL */
    bool lsp_read_line(std::stringstream& ss, size_t& line_size, size_t& line_size_w_eol) {
      line_size = 0;
      line_size_w_eol = 0;

      char c;
      while (ss.get(c)) {
        if (c == '\r') {
          line_size_w_eol = ss.peek() == '\n' ? line_size + 2 : line_size + 1;
          return true;
        } else if (c == '\n') {
          line_size_w_eol = line_size + 1;
          return true;
        }
        line_size++;
      }

      line_size_w_eol = line_size;
      return line_size > 0;
    }

    void sync() {
      // m_line_to_offset.clear();

      // std::stringstream ss(m_content);
      // size_t offset = 0, line_size_w_eol, line_size;
      // while (lsp_read_line(ss, line_size, line_size_w_eol)) {
      //   m_line_to_offset.emplace_back(offset, line_size);
      //   offset += line_size_w_eol;
      // }

      // if (m_line_to_offset.empty()) {  // never empty
      //   m_line_to_offset.emplace_back(0, 0);
      // }

      std::fstream copy("/tmp/quixd-copy.txt", std::ios::out);
      copy << m_content;
    }

  public:
    File(const std::string& mime_type) : m_mime_type(mime_type) { sync(); }

    const std::string& content() const noexcept { return m_content; }

    void set_content(const std::string& content) {
      m_content = content;
      sync();
    }

    void replace(size_t offset, size_t length, std::string_view text) {
      m_content.replace(offset, length, text);
      sync();
    }

    size_t get_size() const noexcept { return m_content.size(); }

    const char* get_mime_type() const noexcept { return m_mime_type.c_str(); }
  };

  std::unordered_map<std::string, File> m_files;
  std::mutex m_mutex;
};

thread_local std::string SyncFS::m_current;

std::optional<size_t> SyncFS::compressed_size() const noexcept {
  // No compression yet
  return size();
}

SyncFS::SyncFS() {
  m_impl = std::make_unique<Impl>();
  LOG(INFO) << "Creating syncronized virtual file system";
}

SyncFS::~SyncFS() { LOG(INFO) << "Destroying syncronized virtual file system"; }

SyncFS& SyncFS::the() {
  static SyncFS instance;
  return instance;
}

///===========================================================================

void SyncFS::select(const std::string& path) noexcept { m_current = path; }

SyncFS::OpenCode SyncFS::open(const std::string& mime_type) noexcept {
  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
    return OpenCode::ALREADY_OPEN;
  }

  try {
    if (!std::filesystem::exists(m_current)) {
      return OpenCode::NOT_FOUND;
    }

    std::ifstream file(m_current);
    if (!file.is_open()) {
      return OpenCode::OPEN_FAILED;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    Impl::File f(mime_type);
    f.set_content(std::move(content));
    m_impl->m_files.emplace(m_current, std::move(f));

    return OpenCode::OK;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Failed to open file: " << e.what();
    return OpenCode::OPEN_FAILED;
  }
}

SyncFS::CloseCode SyncFS::close() noexcept {
  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
    m_impl->m_files.erase(it);
    return CloseCode::OK;
  }

  return CloseCode::NOT_OPEN;
}

SyncFS::ReplaceCode SyncFS::replace(size_t offset, size_t length, std::string_view text) noexcept {
  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
    try {
      it->second.replace(offset, length, text);
    } catch (const std::exception& e) {
      LOG(ERROR) << "Failed to replace text: " << e.what();
      return ReplaceCode::REPLACE_FAILED;
    }

    return ReplaceCode::OK;
  }

  return ReplaceCode::NOT_OPEN;
}

std::optional<const char*> SyncFS::mime_type() const noexcept {
  std::lock_guard lock(m_impl->m_mutex);

  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
    return it->second.get_mime_type();
  }
  return std::nullopt;
}

std::optional<size_t> SyncFS::size() const noexcept {
  std::lock_guard lock(m_impl->m_mutex);

  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
    return it->second.get_size();
  }
  return std::nullopt;
}

bool SyncFS::read_current(std::string& content) const noexcept {
  std::lock_guard lock(m_impl->m_mutex);

  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
    content = it->second.content();
    return true;
  }
  return false;
}
