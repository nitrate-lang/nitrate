////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
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

#include <nitrate-core/Env.h>
#include <nitrate-core/Lib.h>
#include <nitrate-core/Macro.h>
#include <nitrate-emit/Lib.h>
#include <nitrate-ir/Lib.h>
#include <nitrate-lexer/Lib.h>
#include <nitrate-parser/Lib.h>
#include <nitrate-seq/Lib.h>

#include <cerrno>
#include <core/Stream.hh>
#include <core/Transformer.hh>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <nitrate/code.hh>
#include <sstream>
#include <streambuf>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

static const char *empty_options[] = {NULL};

static std::optional<std::vector<std::string>> parse_options(
    const char *const options[]) {
  constexpr size_t max_options = 100000;

  if (!options) {
    return std::nullopt;
  }

  std::vector<std::string> opts;

  for (size_t i = 0; options[i]; i++) {
    if (i >= max_options) {
      qcore_logf(QCORE_ERROR, "Too many options provided, max is %zu",
                 max_options);
      return std::nullopt;
    }

    opts.push_back(options[i]);
  }

  return opts;
}

static void diag_nop(const char *, void *) {}

C_EXPORT void nit_diag_stdout(const char *message, void *) {
  fprintf(stdout, "%s", message);
}

C_EXPORT void nit_diag_stderr(const char *message, void *) {
  fprintf(stderr, "%s", message);
}

///============================================================================///

bool nit::codegen(std::istream &source, std::ostream &output,
                  const std::unordered_set<std::string_view> &opts) {
  (void)source;
  (void)output;
  (void)opts;

  /// TODO: Implement codegen wrapper
  return false;
}

C_EXPORT void nit_fclose(nit_stream_t *f) { delete f; }

C_EXPORT nit_stream_t *nit_from(FILE *f, bool auto_close) {
  if (!f) {
    return nullptr;
  }

  return new nit_stream_t(f, auto_close);
}

C_EXPORT nit_stream_t *nit_join(bool auto_close, size_t num, ...) {
  va_list va;
  va_start(va, num);
  nit_stream_t *obj = nit_joinv(auto_close, num, va);
  va_end(va);

  return obj;
}

C_EXPORT nit_stream_t *nit_joinv(bool auto_close, size_t num, va_list va) {
  std::vector<FILE *> streams;
  streams.resize(num);

  for (size_t i = 0; i < num; i++) {
    streams[i] = va_arg(va, FILE *);
  }

  return new nit_stream_t(streams, auto_close);
}

C_EXPORT nit_stream_t *nit_njoin(bool auto_close, size_t num,
                                 FILE *const *files) {
  return new nit_stream_t(std::vector<FILE *>(files, files + num), auto_close);
}

///============================================================================///

static const std::unordered_map<std::string_view, nit::subsystem_func>
    dispatch_funcs = {{"lex", nit::lex},
                      {"seq", nit::seq},
                      {"parse", nit::parse},
                      {"ir", nit::nr},
                      {"codegen", nit::codegen},

                      /* Helper routes */
                      {"echo", nit::echo}};

extern bool nit_lib_init();
extern void nit_deinit();

class LibraryInitRAII {
  bool ok;

public:
  LibraryInitRAII() { ok = nit_lib_init(); }
  ~LibraryInitRAII() {
    if (ok) {
      nit_deinit();
    }
  }

  bool is_initialized() const { return ok; }
};

C_EXPORT bool nit_pipeline(nit_stream_t *in, nit_stream_t *out,
                           nit_diag_func diag_cb, void *opaque,
                           const char *const c_options[]) {
  errno = 0;

  if (!in) { /* No input stream */
    return false;
  }

  /***************************************************************************/
  /* Rectify input arguments                                                 */
  /***************************************************************************/

  std::stringstream null_ostream;
  std::streambuf *the_output =
      out ? static_cast<std::streambuf *>(out)
          : static_cast<std::streambuf *>(null_ostream.rdbuf());

  c_options = c_options ? c_options : empty_options;
  diag_cb = diag_cb ? diag_cb : diag_nop;

  /***************************************************************************/
  /* Auto initialization                                                     */
  /***************************************************************************/

  LibraryInitRAII init_manager;

  if (!init_manager.is_initialized()) {
    return false;
  }

  /***************************************************************************/
  /* Setup thread-local shared environment                                   */
  /***************************************************************************/

  qcore_env env; /* Don't remove me */

  struct LoggerCtx {
    nit_diag_func diag_cb;
    void *opaque;
  } logger_ctx = {diag_cb, opaque};

  qcore_bind_logger(
      [](qcore_log_t, const char *msg, size_t, void *opaque) {
        let logger_ctx = *reinterpret_cast<LoggerCtx *>(opaque);
        logger_ctx.diag_cb(msg, logger_ctx.opaque);
      },
      &logger_ctx);

  /***************************************************************************/
  /* Parse options */
  /***************************************************************************/

  let options_opt = parse_options(c_options);
  if (!options_opt.has_value()) { /* Options parse error */
    return false;
  }

  let options = options_opt.value();

  if (options.empty()) { /* Nothing to do */
    return true;
  }

  /***************************************************************************/
  /* Prepare input/output streams                                            */
  /***************************************************************************/

  std::unordered_set<std::string_view> opts_set(options.begin() + 1,
                                                options.end());

  let input_stream = std::make_shared<std::istream>(in);
  let output_stream = std::make_shared<std::ostream>(the_output);

  /***************************************************************************/
  /* Dynamic dispatch                                                        */
  /***************************************************************************/

  if (!dispatch_funcs.contains(options.at(0))) {
    qcore_logf(QCORE_ERROR, "Unknown subsystem name in options: %s",
               options[0].c_str());
    return false;
  }

  let subsystem_func = dispatch_funcs.at(options.at(0));
  let is_success = subsystem_func(*input_stream, *output_stream, opts_set);

  /***************************************************************************/
  /* Flush buffers                                                           */
  /***************************************************************************/

  output_stream->flush();

  return is_success;
}

static void nit_diag_functor(const char *message, void *ctx) {
  const auto &callback = *reinterpret_cast<nitrate::DiagnosticFunc *>(ctx);

  callback(message);
}

CPP_EXPORT std::future<bool> nitrate::pipeline(
    std::shared_ptr<Stream> in, std::shared_ptr<Stream> out,
    const std::vector<std::string> &options,
    std::optional<DiagnosticFunc> diag) {
  return std::async(std::launch::async, [=]() {
    /* Convert options to C strings */
    std::vector<const char *> options_c_str(options.size() + 1);
    for (size_t i = 0; i < options.size(); i++) {
      options_c_str[i] = options[i].c_str();
    }
    options_c_str[options.size()] = nullptr;

    void *functor_ctx = nullptr;
    DiagnosticFunc callback = diag.value_or(nullptr);

    if (callback != nullptr) {
      functor_ctx = reinterpret_cast<void *>(&callback);
    }

    return nit_pipeline(in.get()->get(), out.get()->get(),
                        functor_ctx ? nit_diag_functor : nullptr, functor_ctx,
                        options_c_str.data());
  });
}

CPP_EXPORT std::future<bool> nitrate::pipeline(
    Stream in, Stream out, const std::vector<std::string> &options,
    std::optional<DiagnosticFunc> diag) {
  return std::async(
      std::launch::async, [Out = std::move(out), In = std::move(in), options,
                           Diag = std::move(diag)]() {
        /* Convert options to C strings */
        std::vector<const char *> options_c_str(options.size() + 1);
        for (size_t i = 0; i < options.size(); i++) {
          options_c_str[i] = options[i].c_str();
        }
        options_c_str[options.size()] = nullptr;

        void *functor_ctx = nullptr;
        DiagnosticFunc callback = Diag.value_or(nullptr);

        if (callback != nullptr) {
          functor_ctx = reinterpret_cast<void *>(&callback);
        }

        return nit_pipeline(In.get(), Out.get(),
                            functor_ctx ? nit_diag_functor : nullptr,
                            functor_ctx, options_c_str.data());
      });
}

CPP_EXPORT std::future<bool> nitrate::pipeline(
    Stream in, std::vector<uint8_t> &out,
    const std::vector<std::string> &options,
    std::optional<DiagnosticFunc> diag) {
  return std::async(std::launch::async, [&out, In = std::move(in), options,
                                         Diag = std::move(diag)]() {
    /* Convert options to C strings */
    std::vector<const char *> options_c_str(options.size() + 1);
    for (size_t i = 0; i < options.size(); i++) {
      options_c_str[i] = options[i].c_str();
    }
    options_c_str[options.size()] = nullptr;

    void *functor_ctx = nullptr;
    DiagnosticFunc callback = Diag.value_or(nullptr);

    if (callback != nullptr) {
      functor_ctx = reinterpret_cast<void *>(&callback);
    }

    FILE *out_file = tmpfile();
    if (!out_file) {
      return false;
    }

    bool status = nit_pipeline(In.get(), Stream(out_file).get(),
                               functor_ctx ? nit_diag_functor : nullptr,
                               functor_ctx, options_c_str.data());

    if (fseek(out_file, 0, SEEK_END) != 0) {
      fclose(out_file);
      return false;
    }

    auto out_size = ftell(out_file);
    if (out_size == -1) {
      fclose(out_file);
      return false;
    }

    rewind(out_file);

    out.resize(out_size);
    if (fread(out.data(), 1, out_size, out_file) != (size_t)out_size) {
      fclose(out_file);
      return false;
    }

    fclose(out_file);

    return status;
  });
}
