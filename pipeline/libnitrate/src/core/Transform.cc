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
#include <core/Transform.hh>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <nitrate/code.hh>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

static const std::unordered_map<std::string_view, nit::transform_func>
    dispatch_funcs = {{"echo", nit::echo},
                      {"lex", nit::lex},
                      {"seq", nit::seq},
                      {"parse", nit::parse},
                      {"ir", nit::nr}};

///============================================================================///

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

static bool nit_dispatch_request(std::istream &in, std::ostream &out,
                                 const char *transform, let opts_set) {
  if (!dispatch_funcs.contains(transform)) {
    qcore_logf(QCORE_ERROR, "Unknown transform name in options: %s", transform);
    return false;
  }

  let transform_func = dispatch_funcs.at(transform);
  let is_success = transform_func(in, out, opts_set);

  out.flush();

  return is_success;
}

static bool nit_pipeline_stream(std::istream &in, std::ostream &out,
                                nit_diag_func diag_cb, void *opaque,
                                const char *const c_options[]) {
  errno = 0;

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
  /* Transform                                                               */
  /***************************************************************************/

  if (let options = parse_options(c_options)) {
    if (!options->empty()) {
      std::unordered_set opts_set(options->begin() + 1, options->end());
      let name = options->at(0).c_str();

      return nit_dispatch_request(in, out, name, opts_set);
    } else { /* No options provided */
      return true;
    }
  } else { /* Failed to parse options */
    return false;
  }
}

CPP_EXPORT std::future<bool> nitrate::pipeline(
    std::istream &in, std::ostream &out,
    const std::vector<std::string> &options,
    std::optional<DiagnosticFunc> diag) {
  return std::async(std::launch::async, [&in, &out, options, diag]() {
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

    static let diag_nop = [](const char *, void *) {};
    static let nit_diag_functor = [](const char *message, void *ctx) {
      let callback = *reinterpret_cast<nitrate::DiagnosticFunc *>(ctx);
      callback(message);
    };

    return nit_pipeline_stream(in, out,
                               functor_ctx ? nit_diag_functor : diag_nop,
                               functor_ctx, options_c_str.data());
  });
}

///============================================================================///

C_EXPORT bool nit_pipeline(FILE *in, FILE *out, nit_diag_func diag_cb,
                           void *opaque, const char *const c_options[]) {
  class FileStreamBuf : public std::streambuf {
    FILE *m_file;
    char c = 0;

  public:
    FileStreamBuf(FILE *file) : m_file(file) { errno = 0; }
    ~FileStreamBuf() = default;

    virtual int_type overflow(int_type ch) override {
      if (ch != EOF) {
        char temp = ch;
        if (fwrite(&temp, 1, 1, m_file) != 1) {
          int saved_errno = errno;
          qcore_logf(QCORE_ERROR, "Failed to write to stream: %s",
                     strerror(saved_errno));
          return traits_type::eof();
        }
      }

      if (ferror(m_file)) {
        int saved_errno = errno;
        qcore_logf(QCORE_ERROR, "File stream error: %s", strerror(saved_errno));
        return traits_type::eof();
      }

      return ch;
    }

    virtual std::streamsize xsputn(const char *s,
                                   std::streamsize count) override {
      std::streamsize written = 0;
      while (written < count) {
        size_t n = fwrite(s + written, 1, count - written, m_file);
        if (n == 0) {
          int saved_errno = errno;
          qcore_logf(QCORE_ERROR, "Failed to write to stream: %s",
                     strerror(saved_errno));
          break;
        }
        written += n;

        if (ferror(m_file)) {
          int saved_errno = errno;
          qcore_logf(QCORE_ERROR, "File stream error: %s",
                     strerror(saved_errno));
          break;
        }
      }

      return written;
    }

    virtual int_type underflow() override {
      if (gptr() == nullptr || gptr() >= egptr()) {
        size_t res = fread(&c, 1, 1, m_file);
        if (res == 0) {
          if (ferror(m_file)) {
            int saved_errno = errno;
            qcore_logf(QCORE_ERROR, "File stream error: %s",
                       strerror(saved_errno));
          }
          setg(nullptr, nullptr, nullptr);
          return traits_type::eof();
        }
        setg(&c, &c, &c + 1);
      }

      return traits_type::to_int_type(*gptr());
    }

    virtual std::streamsize xsgetn(char *s, std::streamsize count) override {
      std::streamsize bytes_read = 0;
      while (bytes_read < count) {
        size_t n = fread(s + bytes_read, 1, count - bytes_read, m_file);
        if (n == 0) {
          if (ferror(m_file)) {
            int saved_errno = errno;
            qcore_logf(QCORE_ERROR, "File stream error: %s",
                       strerror(saved_errno));
          }
          break;
        }
        bytes_read += n;
      }

      return bytes_read;
    }
  };

  if (!in || !out) {
    return false;
  }

  diag_cb = diag_cb ? diag_cb : [](const char *, void *) {};

  FileStreamBuf in_buf(in), out_buf(out);

  std::istream in_stream(&in_buf);
  std::ostream out_stream(&out_buf);

  return nit_pipeline_stream(in_stream, out_stream, diag_cb, opaque, c_options);
}
