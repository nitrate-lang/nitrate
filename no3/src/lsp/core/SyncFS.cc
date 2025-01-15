#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <lsp/core/SyncFS.hh>
#include <lsp/core/server.hh>

SyncFS::SyncFS() { LOG(INFO) << "Creating mirrored file system abstraction"; }

SyncFS::~SyncFS() {
  LOG(INFO) << "Destroying mirrored file system abstraction";
}

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

std::optional<std::shared_ptr<SyncFSFile>> SyncFS::open(std::string path) {
  path = url_decode(path);
  if (path.starts_with("file://")) {
    path = path.substr(7);
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_files.find(path);
  if (it != m_files.end()) [[likely]] {
    LOG(INFO) << "SyncFS: File already open: " << path;
    return it->second;
  }

  if (!std::filesystem::exists(path)) {
    LOG(ERROR) << "SyncFS: File not found: " << path;
    return std::nullopt;
  }

  LOG(INFO) << "SyncFS: Opening file: " << path;

  std::ifstream file(path);
  if (!file.is_open()) {
    LOG(ERROR) << "SyncFS: Failed to open file: " << path;
    return std::nullopt;
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  auto ptr = std::make_shared<SyncFSFile>();
  ptr->set_content(std::make_shared<std::string>(std::move(content)));

  m_files[path] = ptr;

  return ptr;
}

void SyncFS::close(const std::string& name) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_files.erase(name);
}
