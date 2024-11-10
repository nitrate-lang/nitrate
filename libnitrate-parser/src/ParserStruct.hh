#pragma once

#include <nitrate-core/Env.h>
#include <nitrate-core/Memory.h>
#include <nitrate-lexer/Lexer.h>
#include <nitrate-parser/Config.h>
#include <nitrate-parser/Node.h>
#include <stdbool.h>

struct qparse_impl_t;
typedef struct qparse_impl_t qparse_impl_t;

struct qparse_t {
  qcore_env_t env;     /* The Environment */
  uint64_t id;         /* Process unique instance identifier. Never reused. Never 0. */
  qcore_arena arena;   /* The Main allocator */
  qparse_impl_t *impl; /* Parser implementation struct */
  qlex_t *lexer;       /* Polymporphic lexer */
  qparse_conf_t *conf; /* Parser configuration */
  bool failed;         /* Whether the parser failed (ie syntax errors) */
};
