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

#include <core/Sequencer.hh>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-seq/EC.hh>
#include <nitrate-seq/Sequencer.hh>
#include <ranges>

extern "C" {
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

using namespace ncc::lex;
using namespace ncc::seq;
using namespace ncc::seq::ec;

class RecursiveLimitGuard {
  static constexpr auto kMaxRecursionDepth = 10000;

  size_t &m_depth;

public:
  RecursiveLimitGuard(size_t &depth) : m_depth(depth) { m_depth++; }
  ~RecursiveLimitGuard() { m_depth--; }

  [[nodiscard]] auto ShouldStop() const -> bool {
    return m_depth >= kMaxRecursionDepth;
  }
};

Sequencer::PImpl::PImpl(std::shared_ptr<Environment> env, std::istream &scanner)
    : m_env(std::move(env)),
      m_scanner(std::make_unique<lex::Tokenizer>(scanner, m_env)) {
  m_L = luaL_newstate();
  m_random.seed(0);

  LoadLuaLibs();
  BindLuaAPI();

  /// TODO: Finish this function
}

Sequencer::PImpl::~PImpl() {
  lua_close(m_L);

  /// TODO: Finish this function
}

Sequencer::Sequencer(std::istream &file, std::shared_ptr<ncc::Environment> env,
                     bool is_root)
    : ncc::lex::IScanner(std::move(env)) {
  if (is_root) {
    m_core = std::make_shared<PImpl>(m_env, file);

    SetFetchFunc(nullptr);
    SequenceSource(CodePrefix);
  }
}

Sequencer::~Sequencer() = default;

void Sequencer::SequenceSource(std::string_view code) {
  std::istringstream ss(std::string(code.data(), code.size()));
  std::vector<Token> tokens;

  {
    Sequencer clone(ss, m_env, false);
    clone.m_core = m_core;
    clone.m_core->m_depth = m_core->m_depth + 1;

    Token tok;
    while ((tok = (clone.Next())).GetKind() != EofF) {
      tokens.push_back(tok);
    }
  }

  for (auto &token : std::ranges::reverse_view(tokens)) {
    m_core->m_buffer.push_front(token);
  }
}

auto Sequencer::GetNext() -> Token {
  try {
    RecursiveLimitGuard guard(m_core->m_depth);
    if (guard.ShouldStop()) {
      Log << SeqError << "Maximum macro recursion depth reached, aborting";

      throw StopException();
    }

    while (true) {
      Token x;

      if (!m_core->m_buffer.empty()) {
        x = m_core->m_buffer.front();
        m_core->m_buffer.pop_front();
      } else {
        x = m_core->m_scanner->Next();

        SetFailBit(HasError() || m_core->m_scanner->HasError());
      }

      switch (x.GetKind()) {
        case EofF:
        case KeyW:
        case IntL:
        case Text:
        case Char:
        case NumL:
        case Oper:
        case Punc:
        case Note: {
          break;
        }

        case Name: {
          if (x.GetString() == "import") {
            auto import_name = m_core->m_scanner->Next().GetString();

            if (m_core->m_scanner->Next().is<PuncSemi>()) {
              if (auto content = m_core->FetchModuleData(import_name.Get())) {
                SequenceSource(content.value());
                continue;
              }

              SetFailBit();
              continue;
            }

            Log << SeqError << "Expected a semicolon after import name";

            x = Token::EndOfFile();
            SetFailBit();
          }

          break;
        }

        case MacB: {
          if (auto result_data = m_core->ExecuteLua(x.GetString().Get())) {
            SequenceSource(result_data.value());
            continue;
          }

          Log << SeqError << "Error executing Lua code block";

          x = Token::EndOfFile();
          SetFailBit();

          break;
        }

        case Macr: {
          std::cout << x.GetString();

          /// TODO: Implement macro calls
          qcore_implement();
        }
      }

      if (x.is(EofF)) {
        return x;
      }

      if (!ApplyDynamicTransforms(x)) {
        continue;
      }

      return x;
    }
  } catch (StopException &) {
    return Token::EndOfFile();
  }
}

auto Sequencer::GetLocationFallback(ncc::lex::LocationID id)
    -> std::optional<ncc::lex::Location> {
  return m_core->m_scanner->GetLocation(id);
}

auto Sequencer::ApplyDynamicTransforms(Token last) -> bool {
  std::vector<DeferCallback> saved;
  saved.reserve(m_core->m_defer.size());
  bool emit_token = m_core->m_defer.empty();

  while (!m_core->m_defer.empty()) {
    auto cb = m_core->m_defer.back();
    m_core->m_defer.pop_back();

    auto op = cb(this, last);
    if (op != UninstallHandler) {
      saved.push_back(cb);
    }
    if (op == DeferOp::EmitToken) {
      emit_token = true;
    }
  }

  m_core->m_defer = saved;

  return emit_token;
}

auto Sequencer::HasError() const -> bool {
  return IScanner::HasError() || m_core->m_scanner->HasError();
}

auto Sequencer::SetFailBit(bool fail) -> bool {
  auto old = HasError();

  IScanner::SetFailBit(fail);
  m_core->m_scanner->SetFailBit(fail);

  return old;
}

auto Sequencer::SetFetchFunc(FetchModuleFunc func) -> void {
  if (!func) {
    func = [](std::string_view) {
      Log << SeqError << Debug << "No module fetch function provided";
      return std::nullopt;
    };
  }

  m_core->m_fetch_module = func;
}

auto Sequencer::GetSourceWindow(Point start, Point end, char fillchar)
    -> std::optional<std::vector<std::string>> {
  return m_core->m_scanner->GetSourceWindow(start, end, fillchar);
}

static auto GetFetchURI(const std::string &module_name,
                        const std::string &jobid) -> std::string {
  return "file:///package/" + jobid + "/" + module_name;
}

static auto CanonicalizeModuleName(std::string module_name) -> std::string {
  size_t pos = 0;
  while ((pos = module_name.find("::", pos)) != std::string::npos) {
    module_name.replace(pos, 2, ".");
  }

  std::replace(module_name.begin(), module_name.end(), '/', '.');

  return module_name;
}

auto Sequencer::PImpl::FetchModuleData(std::string_view raw_module_name)
    -> std::optional<std::string> {
  auto module_name = CanonicalizeModuleName(std::string(raw_module_name));

  /* Translate module names into their actual names */
  if (auto actual_name = m_env->Get("map." + module_name)) {
    module_name = actual_name.value();
  }

  auto jobid = std::string(m_env->Get("this.job").value());
  auto module_uri = GetFetchURI(module_name, jobid);

  Log << SeqError << Debug << "Fetching module: '" << module_name << "'";

  qcore_assert(m_fetch_module);

  if (auto module_content = m_fetch_module(module_uri)) {
    return module_content;
  }

  Log << SeqError << "Import not found: '" << module_name << "'";

  return std::nullopt;
}

NCC_EXPORT auto ncc::seq::FileSystemFetchModule(std::string_view path)
    -> std::optional<std::string> {
  if (!path.starts_with("file:///package/")) {
    return std::nullopt;
  }

  path.remove_prefix(16);

  if (path.size() < 37) {
    return std::nullopt;
  }
  auto job_uuid = path.substr(0, 36);
  path.remove_prefix(37);

  Log << Debug << "Opening file '" << path << "' on behalf of job '" << job_uuid
      << "'";

  /// TODO: Get the base directory of the project

  std::fstream file(std::string(path), std::ios::in);
  if (!file.is_open()) {
    return std::nullopt;
  }

  return std::string((std::istreambuf_iterator<char>(file)),
                     (std::istreambuf_iterator<char>()));
}
