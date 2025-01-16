#pragma once

#include <lsp/core/SyncFS.hh>
#include <memory>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/Context.hh>
#include <optional>
#include <unordered_map>

namespace lang {
  class ParseTreeWrapper {
    const ncc::parse::Base* m_root;

  public:
    ParseTreeWrapper();
    ~ParseTreeWrapper();

    [[nodiscard]] auto IsOkay() const -> bool { return m_root != nullptr; }
    auto FromSyncfs(const std::string& uri) -> bool;

    [[nodiscard]] auto Root() const -> const ncc::parse::Base* { return m_root; }
  };
  using ParseTree = std::shared_ptr<ParseTreeWrapper>;

  class ParseTreeCache {
    std::unordered_map<std::string, std::pair<SyncFSFile::Digest, ParseTree>>
        m_cache;
    size_t m_cache_size = kDefaultCacheLimit;

  public:
    constexpr static size_t kDefaultCacheLimit = 1024 * 1024 * 10;  // 10 MB

    static auto The() -> ParseTreeCache&;

    auto Get(std::string_view uri,
                                 bool permit_outdated = false) const -> std::optional<ParseTree>;

    void Clear();
    void SetCacheLimit(size_t max_bytes);
  };
}  // namespace lang
