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
#include <nitrate-ir/Init.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-parser/Init.hh>
#include <nitrate-seq/Init.hh>
#include <nitrate/code.hh>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

static const std::unordered_map<std::string_view, nit::TransformFunc>
    DISPATCH_FUNCS = {{"echo", nit::echo},
                      {"lex", nit::lex},
                      {"seq", nit::seq},
                      {"parse", nit::parse},
                      {"ir", nit::nr}};

///============================================================================///

extern bool NitLibInit();
extern void NitDeinit();

class LibraryInitRAII {
  bool m_ok;

public:
  LibraryInitRAII() { m_ok = NitLibInit(); }
  ~LibraryInitRAII() {
    if (m_ok) {
      NitDeinit();
    }
  }

  bool IsInitialized() const { return m_ok; }
};

static std::optional<std::vector<std::string>> ParseOptions(
    const char *const options[]) {
  constexpr size_t kMaxOptions = 100000;

  if (!options) {
    return std::nullopt;
  }

  std::vector<std::string> opts;

  for (size_t i = 0; options[i]; i++) {
    if (i >= kMaxOptions) {
      qcore_logf(QCORE_ERROR, "Too many options provided, max is %zu",
                 kMaxOptions);
      return std::nullopt;
    }

    opts.push_back(options[i]);
  }

  return opts;
}

static bool NitDispatchRequest(std::istream &in, std::ostream &out,
                               const char *transform, let opts_set,
                               std::shared_ptr<ncc::Environment> env) {
  if (!DISPATCH_FUNCS.contains(transform)) {
    qcore_logf(QCORE_ERROR, "Unknown transform name in options: %s", transform);
    return false;
  }

  let transform_func = DISPATCH_FUNCS.at(transform);
  let is_success = transform_func(in, out, opts_set, env);

  out.flush();

  return is_success;
}

static bool NitPipelineStream(std::istream &in, std::ostream &out,
                              NitDiagFunc diag_cb, void *opaque,
                              const char *const c_options[]) {
  errno = 0;

  /***************************************************************************/
  /* Auto initialization                                                     */
  /***************************************************************************/

  LibraryInitRAII init_manager;

  if (!init_manager.IsInitialized()) {
    return false;
  }

  /***************************************************************************/
  /* Setup thread-local shared environment                                   */
  /***************************************************************************/

  auto subid = ncc::Log.Subscribe([&](auto msg, auto sev, const auto &ec) {
    diag_cb(ec.Format(msg, sev).c_str(), opaque);
  });

  auto env = std::make_shared<ncc::Environment>();

  /***************************************************************************/
  /* Transform                                                               */
  /***************************************************************************/

  bool status = false;

  if (let options = ParseOptions(c_options)) {
    if (!options->empty()) {
      std::unordered_set opts_set(options->begin() + 1, options->end());
      let name = options->at(0).c_str();

      status = NitDispatchRequest(in, out, name, opts_set, env);
    } /* No options provided */
  } /* Failed to parse options */

  ncc::Log.Unsubscribe(subid);

  return status;
}

NCC_EXPORT nitrate::LazyResult<bool> nitrate::Pipeline(
    std::istream &in, std::ostream &out, std::vector<std::string> options,
    std::optional<DiagnosticFunc> diag) {
  return nitrate::LazyResult<bool>([&in, &out, options = std::move(options),
                                    diag]() -> bool {
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

    return NitPipelineStream(in, out, functor_ctx ? nit_diag_functor : diag_nop,
                             functor_ctx, options_c_str.data());
  });
}

NCC_EXPORT nitrate::LazyResult<bool> nitrate::Chain(
    std::istream &in, std::ostream &out, ChainOptions operations,
    std::optional<DiagnosticFunc> diag, bool) {
  return nitrate::LazyResult<bool>(
      [&in, &out, operations = std::move(operations), diag]() -> bool {
        if (operations.empty()) {
          return true;
        }

        if (operations.size() == 1) {
          return nitrate::Pipeline(in, out, operations[0], diag).Get();
        } else {
          std::stringstream s0, s1;
          if (!nitrate::Pipeline(in, s0, operations[0], diag).Get()) {
            return false;
          }

          for (size_t i = 1; i < operations.size() - 1; i++) {
            if (!nitrate::Pipeline(s0, s1, operations[i], diag).Get()) {
              return false;
            }

            s0.str("");
            s0.clear();
            s0.swap(s1);
          }

          return nitrate::Pipeline(s0, out, operations.back(), diag).Get();
        }
      });
}

///============================================================================///

extern "C" NCC_EXPORT bool NitPipeline(FILE *in, FILE *out, NitDiagFunc diag_cb,
                                       void *opaque,
                                       const char *const c_options[]) {
  class FileStreamBuf : public std::streambuf {
    FILE *m_file;
    char m_c = 0;

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
        size_t res = fread(&m_c, 1, 1, m_file);
        if (res == 0) {
          if (ferror(m_file)) {
            int saved_errno = errno;
            qcore_logf(QCORE_ERROR, "File stream error: %s",
                       strerror(saved_errno));
          }
          setg(nullptr, nullptr, nullptr);
          return traits_type::eof();
        }
        setg(&m_c, &m_c, &m_c + 1);
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

  return NitPipelineStream(in_stream, out_stream, diag_cb, opaque, c_options);
}
