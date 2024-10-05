#pragma once

#include <quix-core/Arena.h>
#include <quix-core/Env.h>
#include <quix-lexer/Lexer.h>
#include <quix-parser/Config.h>
#include <quix-parser/Node.h>
#include <stdbool.h>

struct qparse_impl_t;
typedef struct qparse_impl_t qparse_impl_t;

typedef struct qparse_t {
  qcore_env_t env;     /* The Environment */
  qcore_arena arena;   /* The Main allocator */
  qparse_impl_t *impl; /* Parser implementation struct */
  qlex_t *lexer;       /* Polymporphic lexer */
  qparse_conf_t *conf; /* Parser configuration */
  bool failed;         /* Whether the parser failed (ie syntax errors) */
} qparse_t;
