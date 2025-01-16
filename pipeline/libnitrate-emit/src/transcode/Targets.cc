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

#include <nitrate-emit/Code.h>

#include <core/Config.hh>
#include <functional>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <streambuf>
#include <transcode/Targets.hh>
#include <unordered_map>

using namespace ncc::ir;

#ifdef TRANSCODE_TARGET_ALL
#define TRANSCODE_TARGET_C11
#define TRANSCODE_TARGET_CXX11
#define TRANSCODE_TARGET_TYPESCRIPT
#define TRANSCODE_TARGET_RUST
#define TRANSCODE_TARGET_PYTHON
#define TRANSCODE_TARGET_CSHARP
#endif

static const std::unordered_map<
    QcodeLangT, std::function<bool(IRModule*, std::ostream&, std::ostream&)>>
    TRANSCODERS = {
#ifdef TRANSCODE_TARGET_C11
        {QCODE_C11, codegen::ForC11},
#endif
#ifdef TRANSCODE_TARGET_CXX11
        {QCODE_CXX11, codegen::ForCxx11},
#endif
#ifdef TRANSCODE_TARGET_TYPESCRIPT
        {QCODE_TS, codegen::ForTs},
#endif
#ifdef TRANSCODE_TARGET_RUST
        {QCODE_RUST, codegen::ForRust},
#endif
#ifdef TRANSCODE_TARGET_PYTHON
        {QCODE_PYTHON3, codegen::ForPython},
#endif
#ifdef TRANSCODE_TARGET_CSHARP
        {QCODE_CSHARP, codegen::ForCSharp},
#endif
};

static const std::unordered_map<int, std::string_view> TARGET_NAMES = {
    {QCODE_C11, "C11"},   {QCODE_CXX11, "C++11"},    {QCODE_TS, "TypeScript"},
    {QCODE_RUST, "Rust"}, {QCODE_PYTHON3, "Python"}, {QCODE_CSHARP, "C#"},
};

class OStreamWriter : public std::streambuf {
  FILE* m_file;

public:
  OStreamWriter(FILE* file) : m_file(file) {}

  virtual auto xsputn(const char* s, std::streamsize n) -> std::streamsize override {
    return fwrite(s, 1, n, m_file);
  }

  virtual auto overflow(int c) -> int override { return fputc(c, m_file); }
};

class OStreamDiscard : public std::streambuf {
public:
  virtual auto xsputn(const char*, std::streamsize n) -> std::streamsize override {
    return n;
  }
  virtual auto overflow(int c) -> int override { return c; }
};

NCC_EXPORT auto QcodeTranscode(IRModule* module, QCodegenConfig*,
                               QcodeLangT lang, QcodeStyleT, FILE* err,
                               FILE* out) -> bool {
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

  if (TRANSCODERS.contains(lang)) {
    std::ostream err_stream(err_stream_buf.get());
    std::ostream out_stream(out_stream_buf.get());

    /* Do the transcoding. */
    bool status = TRANSCODERS.at(lang)(module, err_stream, out_stream);

    /* Flush the outer and inner streams. */
    err_stream.flush();
    out_stream.flush();
    err&& fflush(err);
    out&& fflush(out);

    return status;
  } else {
    /* Panic if the transcoder is not available. */
    qcore_panicf(
        "The code generator was not built with transcoder support for target "
        "%s.",
        TARGET_NAMES.at(lang).data());
  }
}