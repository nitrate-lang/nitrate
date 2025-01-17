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

#include <cstddef>
#include <fstream>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-seq/EC.hh>
#include <nitrate-seq/Sequencer.hh>
#include <ranges>
#include <sys/List.hh>

extern "C" {
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

static constexpr auto kMaxRecursionDepth = 10000;

using namespace ncc::lex;
using namespace ncc::seq;
using namespace ncc::seq::ec;

Sequencer::PImpl::PImpl(std::shared_ptr<Environment> env)
    : m_env(std::move(env)) {
  m_L = luaL_newstate();
  m_random.seed(0);
}

Sequencer::PImpl::~PImpl() { lua_close(m_L); }

static inline auto Ltrim(std::string_view s) -> std::string_view {
  s.remove_prefix(std::min(s.find_first_not_of(" \t\n\r"), s.size()));
  return s;
}

static inline auto Rtrim(std::string_view s) -> std::string_view {
  s.remove_suffix(
      std::min(s.size() - s.find_last_not_of(" \t\n\r") - 1, s.size()));
  return s;
}

auto Sequencer::ApplyDynamicTransforms(Token last) -> bool {
  /**
   * @brief We do it this way because the callback could potentially modify the
   * `m_defer` vector, which would invalidate the iterator.
   * The callback is able to instruct us to keep it around for the next token or
   * to remove it.
   *
   * The callback is able to consume and emit token sequences, which may result
   * in this `ApplyDynamicTransforms` function being called recursively.
   *
   * So like, if any of the callbacks says to emit a token, we should emit a
   * token. Otherwise, we should not emit a token.
   */

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
    if (op == EmitToken) {
      emit_token = true;
    }
  }

  m_core->m_defer = saved;

  return emit_token;
}

void Sequencer::RecursiveExpand(std::string_view code) {
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

auto Sequencer::ExecuteLua(const char *code) -> bool {
  auto top = lua_gettop(m_core->m_L);
  auto rc = luaL_dostring(m_core->m_L, code);

  if (rc) {
    ncc::Log << SeqError << "Lua error: " << lua_tostring(m_core->m_L, -1);

    SetFailBit();
    return false;
  }

  if (lua_isstring(m_core->m_L, -1) != 0) {
    RecursiveExpand(lua_tostring(m_core->m_L, -1));
  } else if (lua_isnumber(m_core->m_L, -1) != 0) {
    RecursiveExpand(std::to_string(lua_tonumber(m_core->m_L, -1)));
  } else if (lua_isboolean(m_core->m_L, -1)) {
    RecursiveExpand((lua_toboolean(m_core->m_L, -1) != 0) ? "true" : "false");
  } else if (lua_gettop(m_core->m_L) != top && !lua_isnil(m_core->m_L, -1)) {
    return false;
  }

  return true;
}

class RecursiveGuard {
  size_t &m_depth;

public:
  RecursiveGuard(size_t &depth) : m_depth(depth) { m_depth++; }
  ~RecursiveGuard() { m_depth--; }

  [[nodiscard]] auto ShouldStop() const -> bool {
    return m_depth >= kMaxRecursionDepth;
  }
};

auto Sequencer::GetNext() -> Token {
func_entry:  // do tail call optimization manually

  Token x;

  try {
    RecursiveGuard guard(m_core->m_depth);
    if (guard.ShouldStop()) {
      ncc::Log << SeqError << "Maximum macro recursion depth reached, aborting";

      throw StopException();
    }

    if (!m_core->m_buffer.empty()) {
      x = m_core->m_buffer.front();
      m_core->m_buffer.pop_front();
    } else {
      x = m_scanner->Next();

      SetFailBit(HasError() || m_scanner->HasError());
    }

    switch (x.GetKind()) {
      case EofF:
        return x;
      case KeyW:
      case IntL:
      case Text:
      case Char:
      case NumL:
      case Oper:
      case Punc:
      case Note: {
        goto emit_token;
      }

      case Name: { /* Handle the expansion of defines */
        if (x.GetString() == "import") {
          auto import_name = m_scanner->Next().GetString();
          auto semicolon = m_scanner->Next();
          if (!semicolon.is<PuncSemi>()) {
            ncc::Log << SeqError << "Expected semicolon after import name";

            SetFailBit();
            x = Token::EndOfFile();
            SetFailBit();
            goto emit_token;
          }

          if (auto module_data = m_core->FetchModuleData(import_name.Get())) {
            RecursiveExpand(module_data.value());
          } else {
            SetFailBit();
          }

          goto func_entry;
        } else {
          goto emit_token;
        }
      }

      case MacB: {
        auto block = Ltrim(x.GetString());
        if (!block.starts_with("fn ")) {
          if (!ExecuteLua(std::string(block).c_str())) {
            ncc::Log << SeqError << "Failed to expand macro block: " << block;

            SetFailBit();
            x = Token::EndOfFile();

            goto emit_token;
          }
        } else {
          block = Ltrim(block.substr(3));
          auto pos = block.find_first_of('(');
          if (pos == std::string_view::npos) {
            ncc::Log << SeqError
                     << "Invalid macro function definition: " << block;

            x = Token::EndOfFile();
            SetFailBit();

            goto emit_token;
          }

          auto name = Rtrim(block.substr(0, pos));
          auto code =
              "function " + std::string(name) + std::string(block.substr(pos));

          { /* Remove the opening brace */
            pos = code.find_first_of("{");
            if (pos == std::string::npos) {
              ncc::Log << SeqError
                       << "Invalid macro function definition: " << block;

              x = Token::EndOfFile();
              SetFailBit();

              goto emit_token;
            }
            code.erase(pos, 1);
          }

          { /* Remove the closing brace */
            pos = code.find_last_of("}");
            if (pos == std::string::npos) {
              ncc::Log << SeqError
                       << "Invalid macro function definition: " << block;

              x = Token::EndOfFile();
              SetFailBit();

              goto emit_token;
            }
            code.erase(pos, 1);
            code.insert(pos, "end");
          }

          if (!ExecuteLua(code.c_str())) {
            ncc::Log << SeqError << "Failed to expand macro function: " << name;

            x = Token::EndOfFile();
            SetFailBit();

            goto emit_token;
          }
        }

        goto func_entry;
      }

      case Macr: {
        auto body = x.GetString().Get();
        auto pos = body.find_first_of('(');

        if (pos != std::string_view::npos) {
          if (!ExecuteLua(("return " + std::string(body)).c_str())) {
            ncc::Log << SeqError << "Failed to expand macro function: " << body;

            x = Token::EndOfFile();
            SetFailBit();

            goto emit_token;
          }

          goto func_entry;
        } else {
          if (!ExecuteLua(("return " + std::string(body) + "()").c_str())) {
            ncc::Log << SeqError << "Failed to expand macro function: " << body;

            x = Token::EndOfFile();
            SetFailBit();

            goto emit_token;
          }

          goto func_entry;
        }
      }
    }

  emit_token:
    if (!ApplyDynamicTransforms(x)) {
      goto func_entry;
    }
  } catch (StopException &) {
    x = Token::EndOfFile();
  }

  return x;
}

void Sequencer::LoadLuaLibs() {
  static constexpr std::array<luaL_Reg, 5> kTheLibs = {
      luaL_Reg{LUA_GNAME, luaopen_base},
      luaL_Reg{LUA_TABLIBNAME, luaopen_table},
      luaL_Reg{LUA_STRLIBNAME, luaopen_string},
      luaL_Reg{LUA_MATHLIBNAME, luaopen_math},
      luaL_Reg{LUA_UTF8LIBNAME, luaopen_utf8},
  };

  for (const auto &lib : kTheLibs) {
    luaL_requiref(m_core->m_L, lib.name, lib.func, 1);
    lua_pop(m_core->m_L, 1);
  }
}

void Sequencer::BindLuaAPI() {
  lua_newtable(m_core->m_L);

  for (auto func : SYS_FUNCTIONS) {
    lua_pushinteger(m_core->m_L, (lua_Integer)(uintptr_t)this);
    lua_pushcclosure(m_core->m_L, func.GetFunc(), 1);
    lua_setfield(m_core->m_L, -2, func.GetName().data());
  }

  lua_setglobal(m_core->m_L, "n");
}

Sequencer::Sequencer(std::istream &file, std::shared_ptr<ncc::Environment> env,
                     bool is_root)
    : ncc::lex::IScanner(std::move(env)),
      m_scanner(std::make_unique<Tokenizer>(file, m_env)) {
  if (is_root) {
    m_core = std::make_shared<PImpl>(m_env);
    SetFetchFunc(nullptr);

    LoadLuaLibs();
    BindLuaAPI();
    RecursiveExpand(CodePrefix);
  }
}

auto Sequencer::GetSourceWindow(Point start, Point end, char fillchar)
    -> std::optional<std::vector<std::string>> {
  return m_scanner->GetSourceWindow(start, end, fillchar);
}

static auto DynfetchGetMapkey(std::string_view module_name) -> std::string {
  return "map." + std::string(module_name);
}

static auto DynfetchGetUri(std::string_view module_name,
                           std::string_view jobid) -> std::string {
  return "file:///package/" + std::string(jobid) + "/" +
         std::string(module_name);
}

static auto CanonicalizeModuleName(std::string_view module_name)
    -> std::string {
  std::string text(module_name);

  for (size_t pos = 0; (pos = text.find("::", pos)) != std::string::npos;
       pos += 2) {
    text.replace(pos, 2, ".");
  }

  std::replace(text.begin(), text.end(), '/', '.');

  return text;
}

auto Sequencer::HasError() const -> bool {
  return IScanner::HasError() || m_scanner->HasError();
}

auto Sequencer::SetFailBit(bool fail) -> bool {
  auto old = HasError();

  IScanner::SetFailBit(fail);
  m_scanner->SetFailBit(fail);

  return old;
}

void Sequencer::SetFetchFunc(FetchModuleFunc func) {
  if (!func) {
    func = [](std::string_view) {
      ncc::Log << SeqError << Debug << "No module fetch function provided";
      return std::nullopt;
    };
  }

  m_core->m_fetch_module = func;
}

auto Sequencer::PImpl::FetchModuleData(std::string_view raw_module_name)
    -> std::optional<std::string> {
  auto module_name = CanonicalizeModuleName(raw_module_name);

  /* Dynamic translation of module names into their actual named */
  if (auto actual_name = m_env->Get(DynfetchGetMapkey(module_name))) {
    module_name = actual_name.value();
  }

  auto module_uri = DynfetchGetUri(module_name, m_env->Get("this.job").value());

  ncc::Log << SeqError << Debug << "Fetching module: '" << module_name << "'";

  if (!m_fetch_module) {
    qcore_panic(
        "Internal runtime: ncc::seq::Sequencer can't include modules "
        "because 'm_fetch_module' is nullptr");
  }

  auto module = m_fetch_module(module_uri);
  if (!module.has_value()) {
    ncc::Log << SeqError << "Import not found: '" << module_name << "'";

    return std::nullopt;
  }

  return module;
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

  ncc::Log << Debug << "Opening file '" << path << "' on behalf of job '"
           << job_uuid << "'";

  /// TODO: Get the base directory of the project

  std::fstream file(std::string(path), std::ios::in);
  if (!file.is_open()) {
    return std::nullopt;
  }

  return std::string((std::istreambuf_iterator<char>(file)),
                     (std::istreambuf_iterator<char>()));
}
