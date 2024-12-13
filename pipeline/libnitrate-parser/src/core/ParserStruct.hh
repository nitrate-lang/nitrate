#pragma once

#include <nitrate-core/Env.h>
#include <nitrate-core/Memory.h>
#include <nitrate-lexer/Lexer.h>
#include <stdbool.h>

#include <core/ParseReport.hh>
#include <nitrate-parser/AST.hh>

struct npar_t {
  qcore_env_t env; /* The Environment */
  uint64_t id; /* Process unique instance identifier. Never reused. Never 0. */
  qcore_arena arena;            /* The Main allocator */
  npar::DiagnosticManager diag; /* The Diagnostic Manager */
  qlex_t *lexer;                /* Polymporphic lexer */
  bool failed; /* Whether the parser failed (ie syntax errors) */
};
