#pragma once

#include <lsp/core/SyncFS.hh>
#include <memory>
#include <nitrate-lexer/Classes.hh>
#include <nitrate-parser/Context.hh>
#include <optional>
#include <unordered_map>

namespace lang {
  class ParseTreeWrapper {
    qlex m_lexer;
    const ncc::parse::Base* m_root;

  public:
    ParseTreeWrapper();
    ~ParseTreeWrapper();

    bool is_okay() const { return m_root != nullptr; }
    bool from_syncfs(const std::string& uri);

    const ncc::parse::Base* root() const { return m_root; }
    NCCLexer* lexer() { return m_lexer.get(); }
  };
  using ParseTree = std::shared_ptr<ParseTreeWrapper>;

  class ParseTreeCache {
    std::unordered_map<std::string, std::pair<SyncFSFile::Digest, ParseTree>>
        m_cache;
    size_t m_cache_size = DEFAULT_CACHE_LIMIT;

  public:
    constexpr static size_t DEFAULT_CACHE_LIMIT = 1024 * 1024 * 10;  // 10 MB

    static ParseTreeCache& the();

    std::optional<ParseTree> get(std::string_view uri,
                                 bool permit_outdated = false) const;

    void clear();
    void set_cache_limit(size_t max_bytes);
  };
}  // namespace lang
