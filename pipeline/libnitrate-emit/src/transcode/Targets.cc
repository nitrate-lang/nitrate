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

#include <core/LibMacro.h>
#include <nitrate-core/Error.h>
#include <nitrate-emit/Code.h>

#include <core/Config.hh>
#include <functional>
#include <memory>
#include <streambuf>
#include <transcode/Targets.hh>
#include <unordered_map>

#ifdef TRANSCODE_TARGET_ALL
#define TRANSCODE_TARGET_C11
#define TRANSCODE_TARGET_CXX11
#define TRANSCODE_TARGET_TYPESCRIPT
#define TRANSCODE_TARGET_RUST
#define TRANSCODE_TARGET_PYTHON
#define TRANSCODE_TARGET_CSHARP
#endif

static const std::unordered_map<qcode_lang_t,
                                std::function<bool(qmodule_t*, std::ostream&, std::ostream&)>>
    transcoders = {
#ifdef TRANSCODE_TARGET_C11
        {QCODE_C11, codegen::for_c11},
#endif
#ifdef TRANSCODE_TARGET_CXX11
        {QCODE_CXX11, codegen::for_cxx11},
#endif
#ifdef TRANSCODE_TARGET_TYPESCRIPT
        {QCODE_TS, codegen::for_ts},
#endif
#ifdef TRANSCODE_TARGET_RUST
        {QCODE_RUST, codegen::for_rust},
#endif
#ifdef TRANSCODE_TARGET_PYTHON
        {QCODE_PYTHON3, codegen::for_python},
#endif
#ifdef TRANSCODE_TARGET_CSHARP
        {QCODE_CSHARP, codegen::for_csharp},
#endif
};

static const std::unordered_map<qcode_lang_t, std::string_view> target_names = {
    {QCODE_C11, "C11"},   {QCODE_CXX11, "C++11"},    {QCODE_TS, "TypeScript"},
    {QCODE_RUST, "Rust"}, {QCODE_PYTHON3, "Python"}, {QCODE_CSHARP, "C#"},
};

class OStreamWriter : public std::streambuf {
  FILE* m_file;

public:
  OStreamWriter(FILE* file) : m_file(file) {}

  virtual std::streamsize xsputn(const char* s, std::streamsize n) override {
    return fwrite(s, 1, n, m_file);
  }

  virtual int overflow(int c) override { return fputc(c, m_file); }
};

class OStreamDiscard : public std::streambuf {
public:
  virtual std::streamsize xsputn(const char*, std::streamsize n) override { return n; }
  virtual int overflow(int c) override { return c; }
};

LIB_EXPORT bool qcode_transcode(qmodule_t* module, qcode_conf_t*, qcode_lang_t lang, qcode_style_t,
                                FILE* err, FILE* out) {
  std::unique_ptr<std::streambuf> err_stream_buf, out_stream_buf;

  /* If the error stream is provided, use it. Otherwise, discard the output. */
  if (err) {
    err_stream_buf = std::make_unique<OStreamWriter>(err);
  } else {
    err_stream_buf = std::make_unique<OStreamDiscard>();
  }

  /* If the output stream is provided, use it. Otherwise, discard the output. */
  if (out) {
    out_stream_buf = std::make_unique<OStreamWriter>(out);
  } else {
    out_stream_buf = std::make_unique<OStreamDiscard>();
  }

  if (transcoders.contains(lang)) {
    std::ostream err_stream(err_stream_buf.get());
    std::ostream out_stream(out_stream_buf.get());

    /* Do the transcoding. */
    bool status = transcoders.at(lang)(module, err_stream, out_stream);

    /* Flush the outer and inner streams. */
    err_stream.flush();
    out_stream.flush();
    err&& fflush(err);
    out&& fflush(out);

    return status;
  } else {
    /* Panic if the transcoder is not available. */
    qcore_panicf("The code generator was not built with transcoder support for target %s.",
                 target_names.at(lang).data());
  }
}