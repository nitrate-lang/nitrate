#pragma once

#include <nitrate-core/Env.h>
#include <nitrate-core/Memory.h>
#include <nitrate-lexer/Lexer.h>
#include <nitrate-parser/Node.h>
#include <stdbool.h>

#include <core/ParseReport.hh>

struct qparse_t {
  qcore_env_t env; /* The Environment */
  uint64_t id; /* Process unique instance identifier. Never reused. Never 0. */
  qcore_arena arena;              /* The Main allocator */
  qparse::DiagnosticManager diag; /* The Diagnostic Manager */
  qlex_t *lexer;                  /* Polymporphic lexer */
  bool failed; /* Whether the parser failed (ie syntax errors) */
};
