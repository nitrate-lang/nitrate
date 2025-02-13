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

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <core/EC.hh>
#include <core/PImpl.hh>
#include <iostream>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-seq/Sequencer.hh>

extern "C" {
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/lualib.h>
}

using namespace ncc::lex;
using namespace ncc::seq;
using namespace ncc::seq::ec;

static const std::array LUA_AUTOLOAD_WHITELIST = {
    luaL_Reg{LUA_GNAME, luaopen_base},        /* Core lua functions */
    luaL_Reg{LUA_TABLIBNAME, luaopen_table},  /* Table manipulation */
    luaL_Reg{LUA_STRLIBNAME, luaopen_string}, /* String manipulation */
    luaL_Reg{LUA_MATHLIBNAME, luaopen_math},  /* Math functions */
    luaL_Reg{LUA_UTF8LIBNAME, luaopen_utf8},  /* UTF-8 manipulation */
};

void Sequencer::BindMethod(Sequencer &self, const char *name, MethodType func) noexcept {
  /**
   * This function binds a C++ method to a LUA function.
   * The process is as follows:
   *
   * 1. Cast a pointer to `this` and to the `method` to `lua_Integer`.
   * 2. Push these values onto the LUA stack.
   * 3. Then, create a closure that closes over these values.
   * 4. Inside the closure, reinterpret the abovementioned values back to their
   *    original types and invoke them as a indirect method call.
   */

  auto s = self.m_shared;

  const auto &func_ref = s->m_captures.emplace_back(func);
  lua_pushinteger(s->m_L, reinterpret_cast<lua_Integer>(&self));
  lua_pushinteger(s->m_L, reinterpret_cast<lua_Integer>(&func_ref));

  lua_pushcclosure(
      s->m_L,
      [](auto lua) {
        const auto method_ptr_integer = lua_tointeger(lua, lua_upvalueindex(2));
        const auto self_ptr_integer = lua_tointeger(lua, lua_upvalueindex(1));
        const auto &method_ref = *reinterpret_cast<MethodType *>(method_ptr_integer);
        auto &self_ref = *reinterpret_cast<Sequencer *>(self_ptr_integer);

        // Indirect method call
        auto rc = (self_ref.*method_ref)();

        return rc;
      },
      2);

  lua_setfield(s->m_L, -2, name);
}

void Sequencer::AttachAPIFunctions(Sequencer &self) noexcept {
  /**
   * This function initializes the LUA environment with the following functions:
   *  | Name  | Description
   *  |-------|-----------------------------------------------------------------|
   *  | next  | Fetch the next token from the input stream                      |
   *  | peek  | Peek at the next token without consuming it                     |
   *  | emit  | Recursively apply preprocessing to a string                     |
   *  | debug | Log a debug message to the console                              |
   *  | info  | Log an informational message to the console                     |
   *  | warn  | Log a warning message to the console                            |
   *  | error | Log an error message to the console                             |
   *  | abort | Log an error message to the console and stop preprocessing      |
   *  | fatal | Log a fatal error message to the console and stop preprocessing |
   *  | get   | Retrieve a translation context variable                         |
   *  | set   | Set a translation context variable                              |
   *  | ctrl  | Vendor-specific control functions                               |
   *  | fetch | Fetch a module from the filesystem                              |
   *  | random| Generate a random number                                        |
   *  |-------|-----------------------------------------------------------------|
   *
   * The functions are stored in the `n` LUA table.
   */

  auto *lua = self.m_shared->m_L;

  lua_newtable(lua);

  {
    /* Lexical API */
    BindMethod(self, "next", &Sequencer::SysNext);
    BindMethod(self, "peek", &Sequencer::SysPeek);
    BindMethod(self, "emit", &Sequencer::SysEmit);

    /* Logging API */
    BindMethod(self, "debug", &Sequencer::SysDebug);
    BindMethod(self, "info", &Sequencer::SysInfo);
    BindMethod(self, "warn", &Sequencer::SysWarn);
    BindMethod(self, "error", &Sequencer::SysError);
    BindMethod(self, "abort", &Sequencer::SysAbort);
    BindMethod(self, "fatal", &Sequencer::SysFatal);

    /* Environment API */
    BindMethod(self, "get", &Sequencer::SysGet);
    BindMethod(self, "set", &Sequencer::SysSet);

    /* Vendor specific features */
    BindMethod(self, "ctrl", &Sequencer::SysCtrl);

    /* Resource API */
    BindMethod(self, "fetch", &Sequencer::SysFetch);

    /* Math and Logic API */
    BindMethod(self, "random", &Sequencer::SysRandom);
  }

  /* Store the functions inside the 'n' namespace */
  lua_setglobal(lua, "n");
}

void Sequencer::LoadSecureLibs(Sequencer &self) noexcept {
  /**
   * This function loads a secure subset of the LUA core libraries
   * and disables some functions for security and to ensure fully
   * deterministic behavior.
   */

  /* Load a secure subset of the LUA core libraries */
  for (const auto &lib : LUA_AUTOLOAD_WHITELIST) {
    luaL_requiref(self.m_shared->m_L, lib.name, lib.func, 1);
    lua_pop(self.m_shared->m_L, 1);
  }

  /* Disable some lua functions for security and determinism */
  SequenceSource(self,
                 R"(@(
  -- From lbaselib.c
  dofile = nil;
  loadfile = nil;

  print = function(...)
    local args = {...};
    local res = '';
    for i = 1, #args do
      res = res .. tostring(args[i]);
      if i < #args then
        res = res .. '\t';
      end
    end
    n.info(res);
  end

  -- From lmathlib.c
  math.random = n.random;
  math.randomseed = function() n.warn("math.randomseed is a no-op") end;
))");
}

auto Sequencer::ExecuteLua(Sequencer &self, const char *code) -> std::optional<std::string> {
  auto *lua = self.m_shared->m_L;
  const auto old_stack_size = lua_gettop(lua);

  if (luaL_dostring(lua, code)) [[unlikely]] {
    ncc::Log << ec::SeqError << "Lua error: " << lua_tostring(lua, -1);
    self.SetFailBit();

    return std::nullopt;
  }

  const auto new_stack_size = lua_gettop(lua);
  const auto any_return = new_stack_size != old_stack_size;

  return any_return ? std::optional<std::string>(lua_tostring(lua, -1)) : "";
}

auto Sequencer::CreateChild(Sequencer &self, std::istream &file) -> std::unique_ptr<Sequencer> {
  static constexpr auto kMaxRecursionDepth = 10000;

  auto clone = std::unique_ptr<Sequencer>(new Sequencer(file, self.m_shared));
  clone->m_shared->m_depth = self.m_shared->m_depth + 1;

  if (clone->m_shared->m_depth > kMaxRecursionDepth) [[unlikely]] {
    Log << SeqError << "Maximum macro recursion depth reached, aborting";
    return nullptr;
  }

  return clone;
}

void Sequencer::SequenceSource(Sequencer &self, std::string_view code) {
  /**
   * Preprocess the given source code and push the tokens onto the emission queue.
   */

  auto ss = boost::iostreams::stream<boost::iostreams::array_source>(code.data(), code.size());
  auto clone = CreateChild(self, ss);
  if (!clone) [[unlikely]] {
    self.SetFailBit();

    /* Push an end-of-file token to the emission queue */
    self.m_shared->m_emission.push(std::queue<Token>({Token::EndOfFile()}));
    return;
  }

  std::queue<Token> q;
  while (Token tok = clone->Next()) {
    q.push(tok);
  }

  if (!q.empty()) {
    self.m_shared->m_emission.push(std::move(q));
  }
}

auto Sequencer::HandleImportDirective(Sequencer &self) -> bool {
  /// TODO: Refactor me

  auto import_name = self.m_scanner.Next().GetString();

  if (!self.m_scanner.Next().Is<PuncSemi>()) [[unlikely]] {
    Log << SeqError << "Expected a semicolon after import name";
    self.SetFailBit();

    return false;
  }

  if (auto content = FetchModuleData(self, import_name.Get())) {
    SequenceSource(self, content.value());
  } else {
    self.SetFailBit();
  }

  return true;
}

auto Sequencer::HandleMacroBlock(Sequencer &self, Token macro) -> bool {
  if (auto result_data = ExecuteLua(self, macro.GetString().c_str())) [[likely]] {
    SequenceSource(self, result_data.value());
    return true;
  }

  self.SetFailBit();

  return false;
}

auto Sequencer::HandleMacroStatement(Sequencer &self, Token macro) -> bool {
  if (auto result_code = ExecuteLua(self, (std::string(macro.GetString()) + "()").c_str())) [[likely]] {
    SequenceSource(self, result_code.value());
    return true;
  }

  self.SetFailBit();

  return false;
}

auto Sequencer::GetNext() -> Token {
  Token tok;

  while (true) {
    if (!m_shared->m_emission.empty()) [[unlikely]] {
      auto &emission = m_shared->m_emission.front();

      tok = emission.front();
      emission.pop();

      if (emission.empty()) {
        m_shared->m_emission.pop();
      }
    } else {
      switch (tok = m_scanner.Next(); tok.GetKind()) {
        case EofF:
        case IntL:
        case Text:
        case Char:
        case NumL:
        case Oper:
        case Punc:
        case Name:
        case Note:
          [[likely]] { break; }

        case KeyW: {
          if (tok.GetKeyword() == Import && HandleImportDirective(*this)) [[unlikely]] {
            continue;
          }

          break;
        }

        case MacB: {
          if (HandleMacroBlock(*this, tok)) [[likely]] {
            continue;
          } else {
            tok = Token::EndOfFile();
            break;
          }
        }

        case Macr: {
          if (HandleMacroStatement(*this, tok)) [[likely]] {
            continue;
          } else {
            tok = Token::EndOfFile();
            break;
          }
        }
      }
    }

    return tok;
  }
}

auto Sequencer::GetLocationFallback(ncc::lex::LocationID id) -> std::optional<ncc::lex::Location> {
  return m_scanner.GetLocation(id);
}

SequencerPImpl::SequencerPImpl(std::shared_ptr<ncc::Environment> env)
    : m_random(0),
      m_fetch_module(FileSystemFetchModule),
      m_captures({}),
      m_env(std::move(env)),
      m_L(luaL_newstate()),
      m_depth(0) {
  if (m_L == nullptr) {
    Log << Emergency << SeqError << "Failed to create Lua state";
    qcore_panic("Failed to create LUA context");
  }
}

SequencerPImpl::~SequencerPImpl() { lua_close(m_L); }

Sequencer::Sequencer(std::istream &file, std::shared_ptr<SequencerPImpl> shared)
    : ncc::lex::IScanner(shared->m_env), m_scanner(file, m_env), m_shared(std::move(shared)) {}

Sequencer::Sequencer(std::istream &file, std::shared_ptr<ncc::Environment> env)
    : ncc::lex::IScanner(std::move(env)), m_scanner(file, m_env), m_shared(std::make_shared<SequencerPImpl>(m_env)) {
  AttachAPIFunctions(*this);
  LoadSecureLibs(*this);
  SequenceSource(*this, SEQUENCER_DIALECT_CODE_PREFIX);
}

Sequencer::~Sequencer() = default;

auto Sequencer::HasError() const -> bool { return IScanner::HasError() || m_scanner.HasError(); }

auto Sequencer::SetFailBit(bool fail) -> bool {
  auto old = HasError();

  IScanner::SetFailBit(fail);
  m_scanner.SetFailBit(fail);

  return old;
}

auto Sequencer::SetFetchFunc(FetchModuleFunc func) -> void {
  m_shared->m_fetch_module = func ? std::move(func) : FileSystemFetchModule;
}

auto Sequencer::GetSourceWindow(Point start, Point end, char fillchar) -> std::optional<std::vector<std::string>> {
  return m_scanner.GetSourceWindow(start, end, fillchar);
}
