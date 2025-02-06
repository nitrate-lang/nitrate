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

#ifndef __NITRATE_SEQ_HH__
#define __NITRATE_SEQ_HH__

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-seq/EC.hh>
#include <optional>
#include <queue>
#include <random>
#include <string_view>
#include <vector>

struct lua_State;

namespace ncc::seq {
  using FetchModuleFunc = std::function<std::optional<std::string>(std::string_view)>;

  auto FileSystemFetchModule(std::string_view path) -> std::optional<std::string>;

  class NCC_EXPORT Sequencer final : public ncc::lex::IScanner {
    using MethodType = int (ncc::seq::Sequencer::*)();
    struct StopException {};

    struct SharedState {
      std::mt19937 m_random;
      std::queue<std::queue<lex::Token>> m_emission;
      FetchModuleFunc m_fetch_module;
      std::list<MethodType> m_captures;
      lua_State* m_L;
      size_t m_depth;

      SharedState();
      ~SharedState();
    };

    static std::string_view CodePrefix;

    ncc::lex::Tokenizer m_scanner;
    std::shared_ptr<SharedState> m_shared;

    ///=========================================================================

    [[nodiscard]] auto SysNext() -> int32_t;
    [[nodiscard]] auto SysPeek() -> int32_t;
    [[nodiscard]] auto SysEmit() -> int32_t;
    [[nodiscard]] auto SysDebug() -> int32_t;
    [[nodiscard]] auto SysInfo() -> int32_t;
    [[nodiscard]] auto SysWarn() -> int32_t;
    [[nodiscard]] auto SysError() -> int32_t;
    [[nodiscard]] auto SysAbort() -> int32_t;
    [[nodiscard]] auto SysFatal() -> int32_t;
    [[nodiscard]] auto SysGet() -> int32_t;
    [[nodiscard]] auto SysSet() -> int32_t;
    [[nodiscard]] auto SysCtrl() -> int32_t;
    [[nodiscard]] auto SysFetch() -> int32_t;
    [[nodiscard]] auto SysRandom() -> int32_t;

    ///=========================================================================

    auto BindMethod(const char* name, MethodType func) -> void;
    auto BindLuaAPI() -> void;
    auto ConfigureLUAEnvironment() -> void;
    auto ExecuteLua(const char* code) -> std::optional<std::string>;
    auto FetchModuleData(std::string_view module_name) -> std::optional<std::string>;

    auto SequenceSource(std::string_view code) -> void;
    auto GetNext() -> ncc::lex::Token override;
    auto GetLocationFallback(ncc::lex::LocationID id) -> std::optional<ncc::lex::Location> override;

  public:
    Sequencer(std::istream& file, std::shared_ptr<ncc::Environment> env, bool is_root = true);
    Sequencer(const Sequencer&) = delete;
    Sequencer(Sequencer&&) = delete;
    ~Sequencer() override;

    [[nodiscard]] auto HasError() const -> bool override;
    auto SetFailBit(bool fail = true) -> bool override;

    auto SetFetchFunc(FetchModuleFunc func) -> void;
    auto GetSourceWindow(Point start, Point end, char fillchar) -> std::optional<std::vector<std::string>> override;
  };

}  // namespace ncc::seq

#endif
