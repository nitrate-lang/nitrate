#include <quix-lexer/Lexer.h>
#include <quix-parser/Parser.h>
#include <quix-qxir/QXIR.h>

#include <iostream>
#include <memory>
#include <optional>

class Lexer {
  qlex_t *m_ctx;

public:
  Lexer(FILE *file, std::string_view filename) { m_ctx = qlex_new(file, filename.data()); }
  ~Lexer() { qlex_free(m_ctx); }

  qlex_t &get() { return *m_ctx; }
};

class ParseConf {
  qparse_conf_t *m_ctx;

public:
  ParseConf(bool use_defaults = true) { m_ctx = qparse_conf_new(use_defaults); }
  ~ParseConf() { qparse_conf_free(m_ctx); }

  qparse_conf_t &get() { return *m_ctx; }

  void set(qparse_key_t key, qparse_val_t value) { qparse_conf_setopt(m_ctx, key, value); }
  bool has(qparse_key_t key) { return qparse_conf_getopt(m_ctx, key, nullptr); }
  std::optional<qparse_val_t> get(qparse_key_t key) {
    qparse_val_t value;
    if (qparse_conf_getopt(m_ctx, key, &value)) {
      return value;
    }
    return std::nullopt;
  }
};

class Parser {
  std::unique_ptr<Lexer> m_lexer;
  std::shared_ptr<ParseConf> m_conf;
  qcore_arena_t m_arena;
  qparse_t *m_ctx;

public:
  Parser(std::unique_ptr<Lexer> lexer, std::shared_ptr<ParseConf> conf) {
    m_ctx = qparse_new(&lexer->get(), &conf->get());
    m_lexer = std::move(lexer);
    m_conf = conf;

    qcore_arena_open(&m_arena);
  }
  ~Parser() {
    qparse_free(m_ctx);
    qcore_arena_close(&m_arena);
  }

  qparse_t &get() { return *m_ctx; }

  std::optional<qparse_node_t *> go() {
    qparse_node_t *node = nullptr;

    if (!qparse_do(m_ctx, &m_arena, &node)) {
      return std::nullopt;
    }

    return node;
  }

  void dump(std::ostream &os) {
    auto cb = [](const char *str, size_t len, uintptr_t data) {
      std::ostream &os = *std::bit_cast<std::ostream *>(data);
      os.write(str, len);
    };

    qparse_dumps(m_ctx, false, cb, std::bit_cast<uintptr_t>(&os));
  }
};

class QXIRConf {
  qxir_conf_t *m_ctx;

public:
  QXIRConf(bool use_defaults = true) { m_ctx = qxir_conf_new(use_defaults); }
  ~QXIRConf() { qxir_conf_free(m_ctx); }

  qxir_conf_t &get() { return *m_ctx; }

  void set(qxir_key_t key, qxir_val_t value) { qxir_conf_setopt(m_ctx, key, value); }
  bool has(qxir_key_t key) { return qxir_conf_getopt(m_ctx, key, nullptr); }
  std::optional<qxir_val_t> get(qxir_key_t key) {
    qxir_val_t value;
    if (qxir_conf_getopt(m_ctx, key, &value)) {
      return value;
    }
    return std::nullopt;
  }
};

class QXIR {
  qxir_t *m_ctx;
  qcore_arena_t m_arena;
  std::shared_ptr<QXIRConf> m_conf;

public:
  QXIR(qparse_node_t *node, std::shared_ptr<QXIRConf> conf) {
    m_ctx = qxir_new(node, &conf->get());
    m_conf = conf;

    qcore_arena_open(&m_arena);
  }
  ~QXIR() {
    qxir_free(m_ctx);
    qcore_arena_close(&m_arena);
  }

  qxir_t &get() { return *m_ctx; }

  std::optional<qxir_node_t *> go() {
    qxir_node_t *node = nullptr;

    if (!qxir_do(m_ctx, &m_arena, &node)) {
      return std::nullopt;
    }

    return node;
  }

  void dump(std::ostream &os) {
    auto cb = [](const char *str, size_t len, uintptr_t data) {
      std::ostream &os = *std::bit_cast<std::ostream *>(data);
      os.write(str, len);
    };

    qxir_dumps(m_ctx, false, cb, std::bit_cast<uintptr_t>(&os));
  }
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
    return 1;
  }

  qparse_node_t *pnode = nullptr;

  { /* Parse source */
    FILE *file = fopen(argv[1], "r");
    if (!file) {
      std::cerr << "Failed to open file" << std::endl;
      fclose(file);
      return 1;
    }

    auto parse_conf = std::make_shared<ParseConf>();
    auto parser = std::make_unique<Parser>(std::make_unique<Lexer>(file, argv[1]), parse_conf);

    auto node = parser->go();

    if (!node) {
      parser->dump(std::cout);
      fclose(file);
      return 2;
    }

    pnode = *node;

    fclose(file);
  }

  qxir_node_t *qnode = nullptr;

  { /* Lower parse tree to QIR */
    auto qxir_conf = std::make_shared<QXIRConf>();
    auto qxir = std::make_unique<QXIR>(pnode, qxir_conf);
    auto qxir_node = qxir->go();
    if (!qxir_node) {
      qxir->dump(std::cout);
      return 3;
    }

    qnode = *qxir_node;
  }

  std::cout << "Success" << std::endl;

  return 0;
}
