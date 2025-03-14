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
#include <nitrate-seq/Init.hh>
#include <nitrate/code.hh>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace ncc;

static const std::unordered_map<std::string_view, nit::TransformFunc> DISPATCH_FUNCS = {
    {"echo", nit::echo}, {"lex", nit::lex}, {"seq", nit::seq}, {"parse", nit::parse}};

///============================================================================///

extern auto NitLibInit() -> bool;
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

  [[nodiscard]] auto IsInitialized() const -> bool { return m_ok; }
};

static auto ParseOptions(const char *const *options) -> std::optional<std::vector<std::string>> {
  constexpr size_t kMaxOptions = 100000;

  if (options == nullptr) {
    return std::nullopt;
  }

  std::vector<std::string> opts;

  for (size_t i = 0; options[i] != nullptr; i++) {
    if (i >= kMaxOptions) {
      Log << "Too many options provided, max is " << kMaxOptions;
      return std::nullopt;
    }

    opts.emplace_back(options[i]);
  }

  return opts;
}

static auto NitDispatchRequest(std::istream &in, std::ostream &out, const char *transform, const auto &opts_set,
                               const std::shared_ptr<ncc::Environment> &env) -> bool {
  if (!DISPATCH_FUNCS.contains(transform)) {
    Log << "Unknown transform name in options: " << transform;
    return false;
  }

  auto transform_func = DISPATCH_FUNCS.at(transform);
  auto is_success = transform_func(in, out, opts_set, env);

  out.flush();

  return is_success;
}

static auto NitPipelineStream(std::istream &in, std::ostream &out, const char *const *const c_options) -> bool {
  errno = 0;

  /***************************************************************************/
  /* Auto initialization                                                     */
  /***************************************************************************/

  LibraryInitRAII init_manager;

  if (!init_manager.IsInitialized()) {
    return false;
  }

  auto env = std::make_shared<ncc::Environment>();

  /***************************************************************************/
  /* Transform                                                               */
  /***************************************************************************/

  bool status = false;

  if (const auto &options = ParseOptions(c_options)) {
    if (!options->empty()) {
      std::unordered_set opts_set(options->begin() + 1, options->end());

      status = NitDispatchRequest(in, out, options->at(0).c_str(), opts_set, env);
    } /* No options provided */
  } /* Failed to parse options */

  return status;
}

NCC_EXPORT auto nitrate::Pipeline(std::istream &in, std::ostream &out,
                                  std::vector<std::string> options) -> nitrate::LazyResult<bool> {
  return {[&in, &out, options = std::move(options)]() -> bool {
    /* Convert options to C strings */
    std::vector<const char *> options_c_str(options.size() + 1);
    for (size_t i = 0; i < options.size(); i++) {
      options_c_str[i] = options[i].c_str();
    }
    options_c_str[options.size()] = nullptr;

    return NitPipelineStream(in, out, options_c_str.data());
  }};
}

NCC_EXPORT auto nitrate::Chain(std::istream &in, std::ostream &out, ChainOptions operations,
                               bool) -> nitrate::LazyResult<bool> {
  return {[&in, &out, operations = std::move(operations)]() -> bool {
    if (operations.empty()) {
      return true;
    }

    if (operations.size() == 1) {
      return nitrate::Pipeline(in, out, operations[0]).Get();
    }

    std::stringstream s0;
    std::stringstream s1;
    if (!nitrate::Pipeline(in, s0, operations[0]).Get()) {
      return false;
    }

    for (size_t i = 1; i < operations.size() - 1; i++) {
      if (!nitrate::Pipeline(s0, s1, operations[i]).Get()) {
        return false;
      }

      s0.str("");
      s0.clear();
      s0.swap(s1);
    }

    return nitrate::Pipeline(s0, out, operations.back()).Get();
  }};
}

///============================================================================///

extern "C" NCC_EXPORT auto NitPipeline(FILE *in, FILE *out, const char *const c_options[]) -> bool {
  class FileStreamBuf : public std::streambuf {
    FILE *m_file;
    char m_c = 0;

  public:
    FileStreamBuf(FILE *file) : m_file(file) { errno = 0; }
    ~FileStreamBuf() override = default;

    auto overflow(int_type ch) -> int_type override {
      if (ch != EOF) {
        char temp = ch;
        if (fwrite(&temp, 1, 1, m_file) != 1) {
          Log << "Failed to write to stream: " << GetStrerror();
          return traits_type::eof();
        }
      }

      if (ferror(m_file) != 0) {
        Log << "File stream error: " << GetStrerror();
        return traits_type::eof();
      }

      return ch;
    }

    auto xsputn(const char *s, std::streamsize count) -> std::streamsize override {
      std::streamsize written = 0;
      while (written < count) {
        size_t n = fwrite(s + written, 1, count - written, m_file);
        if (n == 0) {
          Log << "Failed to write to stream: " << GetStrerror();
          break;
        }
        written += n;

        if (ferror(m_file) != 0) {
          Log << "File stream error: " << GetStrerror();
          break;
        }
      }

      return written;
    }

    auto underflow() -> int_type override {
      if (gptr() == nullptr || gptr() >= egptr()) {
        size_t res = fread(&m_c, 1, 1, m_file);
        if (res == 0) {
          if (ferror(m_file) != 0) {
            Log << "File stream error: " << GetStrerror();
          }
          setg(nullptr, nullptr, nullptr);
          return traits_type::eof();
        }
        setg(&m_c, &m_c, &m_c + 1);
      }

      return traits_type::to_int_type(*gptr());
    }

    auto xsgetn(char *s, std::streamsize count) -> std::streamsize override {
      std::streamsize bytes_read = 0;
      while (bytes_read < count) {
        size_t n = fread(s + bytes_read, 1, count - bytes_read, m_file);
        if (n == 0) {
          if (ferror(m_file) != 0) {
            Log << "File stream error: " << GetStrerror();
          }
          break;
        }
        bytes_read += n;
      }

      return bytes_read;
    }
  };

  if ((in == nullptr) || (out == nullptr)) {
    return false;
  }

  FileStreamBuf in_buf(in);
  FileStreamBuf out_buf(out);

  std::istream in_stream(&in_buf);
  std::ostream out_stream(&out_buf);

  return NitPipelineStream(in_stream, out_stream, c_options);
}
