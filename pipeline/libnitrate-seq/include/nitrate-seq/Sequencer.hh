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

#include <memory>
#include <nitrate-core/Environment.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-lexer/Token.hh>
#include <optional>
#include <string_view>

struct lua_State;

namespace ncc::seq {
  using FetchModuleFunc =
      std::function<std::optional<std::string>(std::string_view)>;

  auto FileSystemFetchModule(std::string_view path)
      -> std::optional<std::string>;

  class NCC_EXPORT Sequencer final : public ncc::lex::IScanner {
    static std::string_view CodePrefix;
    std::unique_ptr<ncc::lex::Tokenizer> m_scanner;

  public:
    enum DeferOp : uint8_t {
      EmitToken,
      SkipToken,
      UninstallHandler,
    };

    using DeferCallback =
        std::function<DeferOp(Sequencer *obj, ncc::lex::Token last)>;
    class StopException {};
    class PImpl;
    std::shared_ptr<PImpl> m_core;
    void RecursiveExpand(std::string_view code);

  private:
    auto GetNext() -> ncc::lex::Token override;
    auto GetLocationFallback(ncc::lex::LocationID id)
        -> std::optional<ncc::lex::Location> override {
      return m_scanner->GetLocation(id);
    }

    auto ApplyDynamicTransforms(ncc::lex::Token last) -> bool;

    auto ExecuteLua(const char *code) -> bool;
    void LoadLuaLibs();
    void BindLuaAPI();

  public:
    Sequencer(std::istream &file, std::shared_ptr<ncc::Environment> env,
              bool is_root = true);
    ~Sequencer() override = default;

    auto GetSourceWindow(Point start, Point end, char fillchar)
        -> std::optional<std::vector<std::string>> override;

    void SetFetchFunc(FetchModuleFunc func);
  };
}  // namespace ncc::seq

#endif
