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

#ifndef __NITRATE_AST_PARSER_H__
#define __NITRATE_AST_PARSER_H__

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <memory>
#include <memory_resource>
#include <nitrate-core/EnvironmentFwd.hh>
#include <nitrate-core/FlowPtr.hh>
#include <nitrate-lexer/ScannerFwd.hh>
#include <nitrate-parser/ASTFwd.hh>
#include <nitrate-parser/ImportConfig.hh>
#include <nitrate-parser/Package.hh>

namespace ncc::parse {
  class NCC_EXPORT ASTRoot final {
    FlowPtr<Expr> m_base;
    bool m_success;

  public:
    constexpr ASTRoot(auto base, auto success) : m_base(std::move(base)), m_success(success) {}

    [[nodiscard]] auto Get() -> FlowPtr<Expr> & { return m_base; }
    [[nodiscard]] auto Get() const -> FlowPtr<Expr> { return m_base; }
    [[nodiscard]] auto Check() const -> bool;
  };

  class NCC_EXPORT GeneralParser final {
    class PImpl;
    std::unique_ptr<PImpl> m_impl;

  public:
    GeneralParser(lex::IScanner &lexer, std::shared_ptr<IEnvironment> env, std::pmr::memory_resource &pool,
                  const std::optional<ImportConfig> &import_config = std::nullopt);
    GeneralParser(const GeneralParser &) = delete;
    GeneralParser(GeneralParser &&o) noexcept;
    ~GeneralParser();

    auto operator=(const GeneralParser &) -> GeneralParser & = delete;
    auto operator=(GeneralParser &&o) noexcept -> GeneralParser &;

    [[nodiscard]] auto Parse() -> ASTRoot;
    [[nodiscard]] auto GetLexer() -> lex::IScanner &;

    template <typename Scanner>
    static auto ParseString(std::string_view source, std::shared_ptr<IEnvironment> env, std::pmr::memory_resource &pool,
                            const std::optional<ImportConfig> &import_config = std::nullopt) {
      auto in_src = boost::iostreams::stream<boost::iostreams::array_source>(source.data(), source.size());
      auto scanner = Scanner(in_src, env);
      return GeneralParser(scanner, env, pool, import_config).Parse();
    }
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_PARSER_H__
