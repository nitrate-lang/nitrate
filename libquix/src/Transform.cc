////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define LIBQUIX_INTERNAL

#include <quix-codegen/Lib.h>
#include <quix-core/Lib.h>
#include <quix-lexer/Lib.h>
#include <quix-parser/Lib.h>
#include <quix-prep/Lib.h>
#include <quix-qxir/Lib.h>
#include <quix/code.h>

#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "LibMacro.h"
#include "quix-core/Env.h"

static const char *empty_options[] = {NULL};

static bool parse_options(const char *const options[], std::vector<std::string_view> &opts) {
  constexpr size_t max_options = 100000;

  if (!options) {
    return true;
  }

  for (size_t i = 0; options[i]; i++) {
    if (i >= max_options) {
      qcore_print(QCORE_ERROR, "Too many options provided, max is %zu", max_options);
      return false;
    }

    opts.push_back(options[i]);
  }

  return true;
}

static void diag_nop(const char *, const char *, uint64_t) {}

LIB_EXPORT void quix_diag_stdout(const char *message, const char *, uint64_t) {
  fprintf(stdout, "%s", message);
}

LIB_EXPORT void quix_diag_stderr(const char *message, const char *, uint64_t) {
  fprintf(stderr, "%s", message);
}

typedef bool (*quix_subsystem_impl)(FILE *source, FILE *output,
                                    std::function<void(const char *)> diag_cb,
                                    const std::unordered_set<std::string_view> &opts);

bool impl_subsys_basic_lexer(FILE *source, FILE *output, std::function<void(const char *)> diag_cb,
                             const std::unordered_set<std::string_view> &opts);

bool impl_subsys_meta(FILE *source, FILE *output, std::function<void(const char *)> diag_cb,
                      const std::unordered_set<std::string_view> &opts);

bool impl_subsys_parser(FILE *source, FILE *output, std::function<void(const char *)> diag_cb,
                        const std::unordered_set<std::string_view> &opts);

bool impl_subsys_qxir(FILE *source, FILE *output, std::function<void(const char *)> diag_cb,
                      const std::unordered_set<std::string_view> &opts);

static bool impl_subsys_codegen(FILE *source, FILE *output,
                                std::function<void(const char *)> diag_cb,
                                const std::unordered_set<std::string_view> &opts);

static const std::unordered_map<std::string_view, quix_subsystem_impl> dispatch_funcs = {
    {"lex", impl_subsys_basic_lexer}, {"meta", impl_subsys_meta},
    {"parse", impl_subsys_parser},    {"ir", impl_subsys_qxir},
    {"codegen", impl_subsys_codegen},
};

static bool check_out_stream_usable(FILE *stream, const char *name) {
  long pos = ftell(stream);
  if (pos == -1) {
    qcore_print(QCORE_DEBUG, "quix_cc: %s pipe is not seekable", name);
    return false;
  }

  if (fseek(stream, 0, SEEK_SET) == -1) {
    qcore_print(QCORE_DEBUG, "quix_cc: Failed to rewind %s pipe", name);
    return false;
  }

  if (fseek(stream, pos, SEEK_SET) == -1) {
    qcore_print(QCORE_DEBUG, "quix_cc: Failed to restore %s pipe position", name);
    return false;
  }

  return true;
}

LIB_EXPORT bool quix_cc(FILE *S, FILE *O, quix_diag_cb diag_cb, uint64_t userdata,
                        const char *const options[]) {
  /* This API will be used by mortals, protect them from themselves */
  if (!quix_detail_is_init && !quix_lib_init()) {
    return false;
  }

  { /* Argument validate and normalize */
    if (!S) qcore_panic("source pipe is NULL");
    if (!O) qcore_panic("output pipe is NULL");

    if (!options) {
      options = empty_options;
    }

    if (!diag_cb) {
      diag_cb = diag_nop;
    }
  }

  qcore_env env;

  std::vector<std::string_view> opts;
  if (!parse_options(options, opts)) {
    qcore_print(QCORE_ERROR, "Failed to parse options");
    return false;
  }

  if (opts.empty()) {
    return true;
  }

  const auto subsystem = dispatch_funcs.find(opts[0]);
  if (subsystem == dispatch_funcs.end()) {
    qcore_print(QCORE_ERROR, "Unknown subsystem name in options: %s", opts[0].data());
    return false;
  }

  std::unordered_set<std::string_view> opts_set(opts.begin() + 1, opts.end());

  bool is_output_usable = check_out_stream_usable(O, "output");
  errno = 0;

  FILE *out_alias = nullptr;
  char *out_alias_buf = nullptr;
  size_t out_alias_size = 0;

  if (!is_output_usable) {
    out_alias = open_memstream(&out_alias_buf, &out_alias_size);
    if (!out_alias) {
      qcore_print(QCORE_ERROR, "Failed to open temporary output stream");
      return false;
    }
  } else {
    out_alias = O;
  }

  bool ok = subsystem->second(
      S, out_alias,
      [&](const char *msg) {
        /* string_views's in opts are null terminated */
        diag_cb(msg, opts[0].data(), userdata);
      },
      opts_set);

  if (!is_output_usable) {
    fclose(out_alias);
    ok &= fwrite(out_alias_buf, 1, out_alias_size, O) == out_alias_size;
    free(out_alias_buf);
  }

  fflush(O);

  return ok;
}

///============================================================================///

static bool impl_subsys_codegen(FILE *source, FILE *output,
                                std::function<void(const char *)> diag_cb,
                                const std::unordered_set<std::string_view> &opts) {
  (void)source;
  (void)output;
  (void)diag_cb;
  (void)opts;

  /// TODO: Implement codegen wrapper
  return false;
}
