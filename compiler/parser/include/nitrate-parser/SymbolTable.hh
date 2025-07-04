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
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/flyweight.hpp>
#include <boost/unordered_map.hpp>
#include <nitrate-parser/ParseTree.hh>
#include <optional>

namespace nitrate::compiler::parser {
  class SymbolName {
  public:
    SymbolName(std::string_view unqualified_name, std::string_view scope = "")
        : SymbolName(boost::flyweight<std::string>(unqualified_name), boost::flyweight<std::string>(scope)) {}

    SymbolName(boost::flyweight<std::string> unqualified_name, boost::flyweight<std::string> scope);

    [[nodiscard]] auto operator==(const SymbolName& o) const -> bool { return qualified_name() == o.qualified_name(); }
    [[nodiscard]] auto operator<=>(const SymbolName& o) const -> std::strong_ordering {
      return qualified_name() <=> o.qualified_name();
    }

    [[nodiscard]] auto unqualified_name() const -> const std::string& { return m_unqualified_name.get(); }
    [[nodiscard]] auto qualified_name() const -> const std::string& { return m_qualified_name.get(); }
    [[nodiscard]] auto scope() const -> const std::string& { return m_scope.get(); }

  private:
    boost::flyweight<std::string> m_unqualified_name, m_qualified_name, m_scope;
  };

  inline auto hash_value(const SymbolName& name) -> std::size_t { return boost::hash_value(name.qualified_name()); }

  class SymbolTable {
  public:
    SymbolTable() = default;
    SymbolTable(const SymbolTable&) = delete;
    SymbolTable(SymbolTable&&) = delete;
    auto operator=(const SymbolTable&) -> SymbolTable& = delete;
    auto operator=(SymbolTable&&) -> SymbolTable& = delete;
    ~SymbolTable() = default;

    [[nodiscard]] auto define(SymbolName name, Expr& symbol) -> bool;
    [[nodiscard]] auto undefine(const SymbolName& name) -> bool;
    [[nodiscard]] auto is_defined(const SymbolName& name) const -> bool;
    [[nodiscard]] auto resolve(const SymbolName& name) const -> std::optional<Expr*>;

  private:
    boost::unordered_map<SymbolName, Expr*> m_symbols;
  };
}  // namespace nitrate::compiler::parser
