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

#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-lexer/Token.hh>
#include <optional>
#include <random>
#include <string_view>

#define get_engine() \
  ((qprep_impl_t *)(uintptr_t)luaL_checkinteger(L, lua_upvalueindex(1)))

struct lua_State;

enum class DeferOp {
  EmitToken,
  SkipToken,
  UninstallHandler,
};

struct qprep_impl_t;

typedef std::function<DeferOp(qprep_impl_t *obj, ncc::lex::Token last)>
    DeferCallback;

extern std::string_view nit_code_prefix;

struct qprep_impl_t final : public ncc::lex::IScanner {
  struct Core {
    lua_State *L = nullptr;
    std::vector<DeferCallback> defer_callbacks;
    std::deque<ncc::lex::Token> buffer;
    std::mt19937 m_qsys_random_engine;
    bool m_do_expanse = true;
    size_t m_depth = 0;

    ~Core();
  };

  std::shared_ptr<Core> m_core;
  std::unique_ptr<ncc::lex::Tokenizer> m_scanner;

  virtual ncc::lex::Token GetNext() override;
  virtual std::optional<ncc::lex::Location> GetLocationFallback(
      ncc::lex::LocationID id) override {
    return m_scanner->GetLocation(id);
  }

  bool run_defer_callbacks(ncc::lex::Token last);

  std::optional<std::string> run_lua_code(const std::string &s);
  bool run_and_expand(const std::string &code);
  void expand_raw(std::string_view code);
  void install_lua_api();

public:
  qprep_impl_t(std::istream &file, std::shared_ptr<ncc::Environment> env,
               const char *filename, bool is_root = true);
  virtual ~qprep_impl_t() override;

  void SkipCommentsState(bool skip) override {
    IScanner::SkipCommentsState(skip);
    m_scanner->SkipCommentsState(skip);
  }

  virtual std::optional<std::vector<std::string>> GetSourceWindow(
      Point start, Point end, char fillchar) override;
};

class StopException {};
