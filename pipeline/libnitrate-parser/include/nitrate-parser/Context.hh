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
#include <nitrate-core/Environment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/AST.hh>
#include <sstream>

namespace ncc::parse {
  class NCC_EXPORT ASTRoot final {
    FlowPtr<Base> m_base;
    bool m_failed;
    std::shared_ptr<void> m_allocator;

  public:
    constexpr ASTRoot(auto base, auto allocator, auto failed)
        : m_base(std::move(base)),
          m_failed(failed),
          m_allocator(std::move(allocator)) {}

    FlowPtr<Base> &Get() { return m_base; }
    [[nodiscard]] FlowPtr<Base> Get() const { return m_base; }

    [[nodiscard]] bool Check() const;
  };

  class NCC_EXPORT Parser final {
    class PImpl;
    std::unique_ptr<PImpl> m_impl;

    Parser(lex::IScanner &lexer, std::shared_ptr<Environment> env,
           std::shared_ptr<void> lifetime);

  public:
    static auto Create(lex::IScanner &lexer, std::shared_ptr<Environment> env,
                       std::shared_ptr<void> lifetime = nullptr) {
      return boost::shared_ptr<Parser>(
          new Parser(lexer, std::move(env), std::move(lifetime)));
    }

    ~Parser();

    ASTRoot Parse();

    void SetFailBit();
    lex::IScanner &GetLexer();

    template <typename Scanner = lex::Tokenizer>
    static auto FromString(std::string_view str,
                           std::shared_ptr<Environment> env) {
      auto state = std::make_shared<
          std::pair<std::stringstream, std::unique_ptr<Scanner>>>(
          std::stringstream(std::string(str)), nullptr);
      state->second = std::make_unique<Scanner>(state->first, env);

      return Create(*state->second, env, state);
    }

    template <typename Scanner = lex::Tokenizer>
    static auto FromStream(std::istream &stream,
                           std::shared_ptr<Environment> env) {
      auto lexer = std::make_shared<Scanner>(stream, env);
      return Create(lexer, env, lexer);
    }
  };
}  // namespace ncc::parse

#endif  // __NITRATE_AST_PARSER_H__
