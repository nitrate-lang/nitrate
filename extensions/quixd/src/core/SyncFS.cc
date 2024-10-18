#include <openssl/sha.h>

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
      m_line_to_offset.clear();
      m_line_to_offset.shrink_to_fit();

      std::stringstream ss(m_content);
      size_t offset = 0, line_size_w_eol, line_size;
      while (lsp_read_line(ss, line_size, line_size_w_eol)) {
        m_line_to_offset.emplace_back(offset, line_size);
        offset += line_size_w_eol;
      }

      if (m_line_to_offset.empty()) {  // never empty
        m_line_to_offset.emplace_back(0, 0);
      }

      m_line_to_offset.shrink_to_fit();
    }

  public:
    File(std::string_view mime_type) : m_mime_type(mime_type) { sync(); }

    const std::string& content() const { return m_content; }

    void set_content(std::string_view content) {
      m_content = content;
      sync();
    }

    void replace(size_t offset, size_t length, std::string_view text) {
      m_content.replace(offset, length, text);
      sync();
    }

    size_t get_size() const { return m_content.size(); }

    const char* get_mime_type() const { return m_mime_type.c_str(); }
  };

  std::unordered_map<std::string, File> m_files;
  std::mutex m_mutex;
};

thread_local std::string SyncFS::m_current;

std::optional<size_t> SyncFS::compressed_size() const {
  // No compression yet
  return size();
}

SyncFS::SyncFS() {
  m_impl = std::make_unique<Impl>();
  LOG(INFO) << "Creating mirrored file system abstraction";
}

SyncFS::~SyncFS() { LOG(INFO) << "Destroying mirrored file system abstraction"; }

SyncFS& SyncFS::the() {
  static SyncFS instance;
  return instance;
}

///===========================================================================

static std::string url_decode(std::string_view str) {
  std::string result;
  result.reserve(str.size());

  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] == '%' && i + 2 < str.size()) {
      char c = 0;
      for (size_t j = 1; j <= 2; j++) {
        c <<= 4;
        if (str[i + j] >= '0' && str[i + j] <= '9') {
          c |= str[i + j] - '0';
        } else if (str[i + j] >= 'A' && str[i + j] <= 'F') {
          c |= str[i + j] - 'A' + 10;
        } else if (str[i + j] >= 'a' && str[i + j] <= 'f') {
          c |= str[i + j] - 'a' + 10;
        } else {
          return {};
        }
      }
      result.push_back(c);
      i += 2;
    } else {
      result.push_back(str[i]);
    }
  }

  return result;
}
void SyncFS::select_uri(std::string_view uri) {
  if (!uri.starts_with("file://")) {
    LOG(ERROR) << "URI scheme not supported: " << uri;
    return;
  }
  uri.remove_prefix(7);  // remove "file://"
  m_current = url_decode(uri);
}

SyncFS::OpenCode SyncFS::open(std::string_view mime_type) {
  std::lock_guard lock(m_impl->m_mutex);

  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
    return OpenCode::ALREADY_OPEN;
  }

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
}

SyncFS::CloseCode SyncFS::close() {
  std::lock_guard lock(m_impl->m_mutex);

  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
    m_impl->m_files.erase(it);
    return CloseCode::OK;
  }

  return CloseCode::NOT_OPEN;
}

SyncFS::ReplaceCode SyncFS::replace(size_t offset, size_t length, std::string_view text) {
  std::lock_guard lock(m_impl->m_mutex);

  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
    it->second.replace(offset, length, text);

    return ReplaceCode::OK;
  }

  return ReplaceCode::NOT_OPEN;
}

std::optional<const char*> SyncFS::mime_type() const {
  std::lock_guard lock(m_impl->m_mutex);

  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
    return it->second.get_mime_type();
  }
  return std::nullopt;
}

std::optional<size_t> SyncFS::size() const {
  std::lock_guard lock(m_impl->m_mutex);

  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
    return it->second.get_size();
  }
  return std::nullopt;
}

bool SyncFS::read_current(std::string& content) const {
  std::lock_guard lock(m_impl->m_mutex);

  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
    content = it->second.content();
    return true;
  }
  return false;
}

std::optional<std::array<uint8_t, 20>> SyncFS::thumbprint() const {
  std::lock_guard lock(m_impl->m_mutex);

  auto it = m_impl->m_files.find(m_current);
  if (it != m_impl->m_files.end()) [[likely]] {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, it->second.content().data(), it->second.content().size());

    std::array<uint8_t, 20> digest;
    SHA1_Final(digest.data(), &ctx);
#pragma GCC diagnostic pop

    return digest;
  }

  return std::nullopt;
}
