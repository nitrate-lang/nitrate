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

#include <boost/shared_ptr.hpp>
#include <memory>
#include <nitrate-core/EnvironmentFwd.hh>
#include <nitrate-core/FlowPtr.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/ScannerFwd.hh>
#include <nitrate-parser/ASTFwd.hh>
#include <sstream>

namespace ncc::parse {
  class NCC_EXPORT ASTRoot final {
    FlowPtr<Expr> m_base;
    bool m_failed;
    std::shared_ptr<void> m_allocator;

  public:
    constexpr ASTRoot(auto base, auto allocator, auto failed)
        : m_base(std::move(base)), m_failed(failed), m_allocator(std::move(allocator)) {}

    auto Get() -> FlowPtr<Expr> & { return m_base; }
    [[nodiscard]] auto Get() const -> FlowPtr<Expr> { return m_base; }

    [[nodiscard]] auto Check() const -> bool;
  };

  class NCC_EXPORT Parser final {
    class PImpl;
    std::unique_ptr<PImpl> m_impl;

    Parser(lex::IScanner &lexer, std::shared_ptr<IEnvironment> env, std::shared_ptr<void> lifetime);

  public:
    static auto Create(lex::IScanner &lexer, std::shared_ptr<IEnvironment> env,
                       std::shared_ptr<void> lifetime = nullptr) {
      return boost::shared_ptr<Parser>(new Parser(lexer, std::move(env), std::move(lifetime)));
    }

    ~Parser();

    auto Parse() -> ASTRoot;

    void SetFailBit();
    auto GetLexer() -> lex::IScanner &;

    template <typename Scanner>
    static auto FromString(std::string_view str, std::shared_ptr<IEnvironment> env) {
      auto state = std::make_shared<std::pair<std::stringstream, std::unique_ptr<Scanner>>>(
          std::stringstream(std::string(str)), nullptr);
      state->second = std::make_unique<Scanner>(state->first, env);

      return Create(*state->second, env, state);
    }

    template <typename Scanner>
    static auto FromStream(std::istream &stream, std::shared_ptr<IEnvironment> env) {
      auto lexer = std::make_shared<Scanner>(stream, env);
      return Create(lexer, env, lexer);
    }
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_PARSER_H__
