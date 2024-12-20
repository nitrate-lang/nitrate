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

#include <nitrate-emit/Lib.h>
#include <nitrate-ir/Lib.h>
#include <nitrate-seq/Lib.h>

#include <cerrno>
#include <core/Transform.hh>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-parser/Init.hh>
#include <nitrate/code.hh>
#include <sstream>
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
                                 const char *transform, let opts_set,
                                 std::shared_ptr<ncc::core::Environment> env) {
  if (!dispatch_funcs.contains(transform)) {
    qcore_logf(QCORE_ERROR, "Unknown transform name in options: %s", transform);
    return false;
  }

  let transform_func = dispatch_funcs.at(transform);
  let is_success = transform_func(in, out, opts_set, env);

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

  auto env = std::make_shared<ncc::core::Environment>();

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

  bool status = false;

  if (let options = parse_options(c_options)) {
    if (!options->empty()) {
      std::unordered_set opts_set(options->begin() + 1, options->end());
      let name = options->at(0).c_str();

      status = nit_dispatch_request(in, out, name, opts_set, env);
    } /* No options provided */
  } /* Failed to parse options */

  qcore_bind_logger(nullptr, nullptr);

  return status;
}

CPP_EXPORT nitrate::LazyResult<bool> nitrate::pipeline(
    std::istream &in, std::ostream &out, std::vector<std::string> options,
    std::optional<DiagnosticFunc> diag) {
  return nitrate::LazyResult<bool>(
      [&in, &out, Options = std::move(options), diag]() -> bool {
        /* Convert options to C strings */
        std::vector<const char *> options_c_str(Options.size() + 1);
        for (size_t i = 0; i < Options.size(); i++) {
          options_c_str[i] = Options[i].c_str();
        }
        options_c_str[Options.size()] = nullptr;

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

CPP_EXPORT nitrate::LazyResult<bool> nitrate::chain(
    std::istream &in, std::ostream &out, ChainOptions operations,
    std::optional<DiagnosticFunc> diag, bool) {
  return nitrate::LazyResult<bool>(
      [&in, &out, Operations = std::move(operations), diag]() -> bool {
        if (Operations.empty()) {
          return true;
        }

        if (Operations.size() == 1) {
          return nitrate::pipeline(in, out, Operations[0], diag).get();
        } else {
          std::stringstream s0, s1;
          if (!nitrate::pipeline(in, s0, Operations[0], diag).get()) {
            return false;
          }

          for (size_t i = 1; i < Operations.size() - 1; i++) {
            if (!nitrate::pipeline(s0, s1, Operations[i], diag).get()) {
              return false;
            }

            s0.str("");
            s0.clear();
            s0.swap(s1);
          }

          return nitrate::pipeline(s0, out, Operations.back(), diag).get();
        }
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
