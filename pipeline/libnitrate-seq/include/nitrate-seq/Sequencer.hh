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
#include <memory>
#include <nitrate-core/EnvironmentFwd.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-lexer/Lexer.hh>
#include <optional>
#include <string_view>
#include <vector>

struct lua_State;

namespace ncc::seq {
  using FetchModuleFunc = std::function<std::optional<std::string>(std::string_view)>;
  auto FileSystemFetchModule(std::string_view path) -> std::optional<std::string>;

  class Sequencer;
  class SequencerPImpl;
  using MethodType = int (seq::Sequencer::*)();

  extern const std::string_view SEQUENCER_DIALECT_CODE_PREFIX;

  class NCC_EXPORT Sequencer final : public lex::IScanner {
    lex::Tokenizer m_scanner;
    std::shared_ptr<SequencerPImpl> m_shared;

    ///=========================================================================
    /// Preprocessor API route handlers

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
    /// Helper functions to configure the LUA interpreter

    static void BindMethod(Sequencer& self, const char* name, MethodType func) noexcept;
    static void AttachAPIFunctions(Sequencer& self) noexcept;
    static void LoadSecureLibs(Sequencer& self) noexcept;

    ///=========================================================================
    /// Preprocessor helper functions

    static auto FetchModuleData(Sequencer& self, std::string_view module_name) -> std::optional<std::string>;
    static auto ExecuteLua(Sequencer& self, const char* code) -> std::optional<std::string>;
    static auto CreateChild(Sequencer& self, std::istream& file) -> std::unique_ptr<Sequencer>;
    static auto SequenceSource(Sequencer& self, std::string_view code) -> void;
    static auto HandleImportDirective(Sequencer& self) -> bool;
    static auto HandleMacroBlock(Sequencer& self, lex::Token macro) -> bool;
    static auto HandleMacroStatement(Sequencer& self, lex::Token macro) -> bool;

    ///=========================================================================
    /// IScanner overrides

    auto GetNext() -> lex::Token override;
    auto GetLocationFallback(lex::LocationID id) -> std::optional<lex::Location> override;

    Sequencer(std::istream& file, std::shared_ptr<SequencerPImpl> shared);

  public:
    Sequencer(std::istream& file, std::shared_ptr<IEnvironment> env);
    ~Sequencer() override;

    [[nodiscard]] auto HasError() const -> bool override;
    auto SetFailBit(bool fail = true) -> bool override;

    auto SetFetchFunc(FetchModuleFunc func) -> void;
    auto GetSourceWindow(Point start, Point end, char fillchar) -> std::optional<std::vector<std::string>> override;
  };

}  // namespace ncc::seq

#endif
