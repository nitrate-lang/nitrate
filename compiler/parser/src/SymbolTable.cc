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

#include <nitrate-parser/SymbolTable.hh>

using namespace nitrate::compiler::parser;

BOOST_SYMBOL_EXPORT SymbolName::SymbolName(boost::flyweight<std::string> unqualified_name,
                                           boost::flyweight<std::string> scope)
    : m_unqualified_name(std::move(unqualified_name)), m_scope(std::move(scope)) {
  if (m_unqualified_name->starts_with("::")) {
    // If the unqualified name starts with '::', it is already fully qualified
    // according to scope rules, so we can set the qualified name directly.

    m_qualified_name = m_unqualified_name;
  } else {
    // Otherwise, prepend the scope to the unqualified name to form the qualified name
    const auto separate = !m_scope->empty() && !m_unqualified_name->empty();

    m_qualified_name = *m_scope + (separate ? "::" : "") + *m_unqualified_name;
  }
}

BOOST_SYMBOL_EXPORT auto SymbolTable::define(SymbolName name, Expr& symbol) -> bool {
  auto result = m_symbols.emplace(std::move(name), &symbol);
  return result.second;  // Returns true if the insertion took place
}

BOOST_SYMBOL_EXPORT auto SymbolTable::undefine(const SymbolName& name) -> bool {
  return m_symbols.erase(name) > 0;  // Returns true if an element was removed
}

BOOST_SYMBOL_EXPORT auto SymbolTable::is_defined(const SymbolName& name) const -> bool {
  return m_symbols.contains(name);
}

BOOST_SYMBOL_EXPORT auto SymbolTable::resolve(const SymbolName& name) const -> std::optional<Expr*> {
  auto it = m_symbols.find(name);
  if (it != m_symbols.end()) {
    return it->second;
  }
  return std::nullopt;
}
