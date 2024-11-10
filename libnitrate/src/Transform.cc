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
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define LIBNITRATE_INTERNAL
#include <nitrate-core/Env.h>
#include <nitrate-core/Lib.h>
#include <nitrate-emit/Lib.h>
#include <nitrate-ir/Lib.h>
#include <nitrate-lexer/Lib.h>
#include <nitrate-parser/Lib.h>
#include <nitrate-seq/Lib.h>
#include <nitrate/code.h>

#include <Stream.hh>
#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "LibMacro.h"

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

LIB_EXPORT void nit_diag_stdout(const char *message, const char *, uint64_t) {
  fprintf(stdout, "%s", message);
}

LIB_EXPORT void nit_diag_stderr(const char *message, const char *, uint64_t) {
  fprintf(stderr, "%s", message);
}

typedef bool (*nit_subsystem_impl)(std::shared_ptr<std::istream> source, FILE *output,
                                   std::function<void(const char *)> diag_cb,
                                   const std::unordered_set<std::string_view> &opts);

bool impl_subsys_basic_lexer(std::shared_ptr<std::istream> source, FILE *output,
                             std::function<void(const char *)> diag_cb,
                             const std::unordered_set<std::string_view> &opts);

bool impl_subsys_meta(std::shared_ptr<std::istream> source, FILE *output,
                      std::function<void(const char *)> diag_cb,
                      const std::unordered_set<std::string_view> &opts);

bool impl_subsys_parser(std::shared_ptr<std::istream> source, FILE *output,
                        std::function<void(const char *)> diag_cb,
                        const std::unordered_set<std::string_view> &opts);

bool impl_subsys_qxir(std::shared_ptr<std::istream> source, FILE *output,
                      std::function<void(const char *)> diag_cb,
                      const std::unordered_set<std::string_view> &opts);

static bool impl_subsys_codegen(std::shared_ptr<std::istream> source, FILE *output,
                                std::function<void(const char *)> diag_cb,
                                const std::unordered_set<std::string_view> &opts);

static const std::unordered_map<std::string_view, nit_subsystem_impl> dispatch_funcs = {
    {"lex", impl_subsys_basic_lexer}, {"meta", impl_subsys_meta},
    {"parse", impl_subsys_parser},    {"ir", impl_subsys_qxir},
    {"codegen", impl_subsys_codegen},
};

static bool check_out_stream_usable(FILE *stream, const char *name) {
  long pos = ftell(stream);
  if (pos == -1) {
    qcore_print(QCORE_DEBUG, "nit_cc: %s pipe is not seekable", name);
    return false;
  }

  if (fseek(stream, 0, SEEK_SET) == -1) {
    qcore_print(QCORE_DEBUG, "nit_cc: Failed to rewind %s pipe", name);
    return false;
  }

  if (fseek(stream, pos, SEEK_SET) == -1) {
    qcore_print(QCORE_DEBUG, "nit_cc: Failed to restore %s pipe position", name);
    return false;
  }

  return true;
}

LIB_EXPORT bool nit_cc(nit_stream_t *S, FILE *O, nit_diag_cb diag_cb, uint64_t userdata,
                       const char *const options[]) {
  /* This API will be used by mortals, protect them from themselves */
  if (!nit_lib_ready && !nit_lib_init()) {
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

  auto input_stream = std::make_shared<std::istream>(S);

  bool ok = subsystem->second(
      input_stream, out_alias,
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

static bool impl_subsys_codegen(std::shared_ptr<std::istream> source, FILE *output,
                                std::function<void(const char *)> diag_cb,
                                const std::unordered_set<std::string_view> &opts) {
  (void)source;
  (void)output;
  (void)diag_cb;
  (void)opts;

  /// TODO: Implement codegen wrapper
  return false;
}

LIB_EXPORT void nit_fclose(nit_stream_t *f) { delete f; }

LIB_EXPORT nit_stream_t *nit_from(FILE *f, bool auto_close) {
  if (!f) {
    return nullptr;
  }

  return new nit_stream_t(f, auto_close);
}

LIB_EXPORT nit_stream_t *nit_join(size_t num, ...) {
  va_list va;
  va_start(va, num);
  nit_stream_t *obj = nit_joinv(num, va);
  va_end(va);

  return obj;
}

LIB_EXPORT nit_stream_t *nit_joinv(size_t num, va_list va) {
  std::vector<FILE *> streams;
  streams.resize(num);

  for (size_t i = 0; i < num; i++) {
    streams[i] = va_arg(va, FILE *);
  }

  return new nit_stream_t(streams);
}

LIB_EXPORT nit_stream_t *nit_njoin(size_t num, FILE **files) {
  return new nit_stream_t(std::vector<FILE *>(files, files + num));
}
