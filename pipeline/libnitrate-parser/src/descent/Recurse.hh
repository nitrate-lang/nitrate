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

#ifndef __NITRATE_AST_PIMPL_H__
#define __NITRATE_AST_PIMPL_H__

#include <boost/shared_ptr.hpp>
#include <core/SyntaxDiagnostics.hh>
#include <memory>
#include <memory_resource>
#include <nitrate-core/IEnvironment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTFactory.hh>
#include <nitrate-parser/Context.hh>
#include <nitrate-parser/Package.hh>
#include <set>

namespace ncc::parse {
  using namespace ec;

  static inline auto BindComments(auto node, auto comments) {
    std::vector<string> comments_strs(comments.size());
    std::transform(comments.begin(), comments.end(), comments_strs.begin(),
                   [](const auto &c) { return c.GetString(); });
    node->SetComments(std::move(comments_strs));
    return node;
  }

  using ImportedFilesSet = std::shared_ptr<std::unordered_set<std::filesystem::path>>;

  class GeneralParser::Context final : public ASTFactory {
    friend class GeneralParser;

    ImportedFilesSet m_imported_files;
    ImportConfig m_import_config;
    std::shared_ptr<IEnvironment> m_env;
    std::pmr::memory_resource &m_pool;
    lex::IScanner &m_rd;
    size_t m_recursion_depth = 0;
    bool m_failed = false;
    GeneralParser::Context &m = *this;  // NOLINT(readability-identifier-naming)

  public:
    Context(lex::IScanner &lexer, ImportConfig import_config, std::shared_ptr<IEnvironment> env,
            std::pmr::memory_resource &pool)
        : ASTFactory(pool),
          m_imported_files(std::make_shared<std::unordered_set<std::filesystem::path>>()),
          m_import_config(std::move(import_config)),
          m_env(std::move(env)),
          m_pool(pool),
          m_rd(lexer) {}
    Context(const Context &) = delete;
    Context(Context &&o) noexcept = delete;
    ~Context() = default;

    auto CreateSubParser(lex::IScanner &scanner) -> GeneralParser {
      auto sub_parser = GeneralParser(scanner, m_env, m_pool, m_import_config);
      sub_parser.m_impl->m_recursion_depth = m_recursion_depth;
      sub_parser.m_impl->m_imported_files = m_imported_files;
      return sub_parser;
    }

    lex::Token Next() { return m_rd.Next(); }
    lex::Token Peek() { return m_rd.Peek(); }
    lex::Token Current() { return m_rd.Current(); }

    template <auto tok>
    auto NextIf(string value) -> std::optional<ncc::lex::Token> {
      static_assert(std::is_same_v<decltype(tok), ncc::lex::TokenType>);
      auto t = m_rd.Peek();
      if (t.Is<tok>(value)) {
        m_rd.Next();
        return t;
      }

      return std::nullopt;
    }

    template <auto tok>
    auto NextIf() -> std::optional<ncc::lex::Token> {
      auto t = m_rd.Peek();
      if constexpr (std::is_same_v<decltype(tok), ncc::lex::TokenType>) {
        if (t.Is(tok)) {
          m_rd.Next();
          return t;
        }

        return std::nullopt;
      } else {
        if (t.Is<tok>()) {
          m_rd.Next();
          return t;
        }

        return std::nullopt;
      }
    }

    template <auto tok>
    auto IsNext() -> bool {
      auto t = m_rd.Peek();
      if constexpr (std::is_same_v<decltype(tok), ncc::lex::TokenType>) {
        return t.Is(tok);
      } else {
        return t.Is<tok>();
      }
    }

    /****************************************************************************
     * @brief
     *  Language parsing functions
     ****************************************************************************/

    [[nodiscard]] auto IsEof() const -> bool { return m_rd.IsEof(); }
    [[nodiscard]] auto GetScanner() const -> lex::IScanner & { return m_rd; }
    [[nodiscard]] auto GetEnvironment() const -> std::shared_ptr<IEnvironment> { return m_env; }
    [[nodiscard]] auto RecurseAttributes(string kind) -> std::vector<FlowPtr<Expr>>;
    [[nodiscard]] auto RecurseTemplateParameters() -> std::optional<std::vector<TemplateParameter>>;
    [[nodiscard]] auto RecurseCallArguments(const std::set<lex::Token> &end,
                                            bool type_by_default) -> std::vector<CallArg>;

    [[nodiscard]] auto RecurseName() -> string;
    [[nodiscard]] auto RecurseFString() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseExport(Vis vis) -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseVariable(VariableType type) -> std::vector<FlowPtr<Expr>>;
    [[nodiscard]] auto RecurseEnum() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseStruct(CompositeType type) -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseScope() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseFunction(bool parse_declaration_only) -> FlowPtr<Function>;
    [[nodiscard]] auto RecurseType() -> FlowPtr<Type>;
    [[nodiscard]] auto RecurseTypedef() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseReturn() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseIf() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseWhile() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseFor() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseForeach() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseSwitch() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseAssembly() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseTry() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseThrow() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseAwait() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseImport() -> FlowPtr<Expr>;
    [[nodiscard]] auto RecurseBlock(bool braces = true, bool single = false,
                                    BlockMode safety = BlockMode::Unknown) -> FlowPtr<Block>;
    [[nodiscard]] auto RecurseExpr(const std::set<lex::Token> &end) -> FlowPtr<Expr>;
    auto RecurseEscapeBlock() -> void;
    [[nodiscard]] auto RecurseUnitAssert() -> FlowPtr<Expr>;
  };

  void ParserSwapScanner(lex::IScanner *&value);
}  // namespace ncc::parse

#endif  // __NITRATE_AST_PIMPL_H__
