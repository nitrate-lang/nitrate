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
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define __QUIX_IMPL__

#include <quix-core/Env.h>
#include <quix-lexer/Token.h>

#include <memory>
#include <optional>
#include <quix-lexer/Base.hh>
#include <string_view>

#define get_engine() ((qprep_impl_t *)(uintptr_t)luaL_checkinteger(L, lua_upvalueindex(1)))

struct lua_State;

enum class DeferOp {
  EmitToken,
  SkipToken,
  UninstallHandler,
};

struct qprep_impl_t;

typedef std::function<DeferOp(qprep_impl_t *obj, qlex_tok_t last)> DeferCallback;

extern std::string_view quix_code_prefix;

struct qprep_impl_t final : public qlex_t {
  struct Core {
    lua_State *L;
    std::vector<DeferCallback> defer_callbacks;
    std::deque<qlex_tok_t> buffer;

    ~Core();
  };

  std::shared_ptr<Core> m_core;
  bool m_do_expanse;
  size_t m_depth;

  virtual qlex_tok_t next_impl() override;

  bool run_defer_callbacks(qlex_tok_t last);

  std::optional<std::string> run_lua_code(const std::string &s);
  bool run_and_expand(const std::string &code);
  void expand_raw(std::string_view code);
  void install_lua_api();
  qlex_t *weak_clone(FILE *file, const char *filename);

public:
  qprep_impl_t(FILE *file, const char *filename, qcore_env_t env);
  qprep_impl_t(FILE *file, const char *filename, bool is_owned, qcore_env_t env);
  virtual ~qprep_impl_t() override;
};

class StopException {};
