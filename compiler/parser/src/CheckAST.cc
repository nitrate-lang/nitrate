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

#include <boost/config.hpp>
#include <nitrate-parser/ParseTree.hh>
#include <nitrate-parser/Parser.hh>

using namespace nitrate::compiler::parser;

class CheckVisitor : public ConstVisitor {
  const SymbolTable& m_symbol_table;
  bool m_has_errors = false;

public:
  constexpr CheckVisitor(const SymbolTable& symbol_table) : m_symbol_table(symbol_table) {}

  [[nodiscard]] constexpr auto has_errors() const -> bool { return m_has_errors; }

  auto visit(const BinExpr& node) -> void override {
    // TODO: Check node
    (void)node;
    (void)m_symbol_table;  // Suppress unused variable warning
  }

  auto visit(const UnaryExpr& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Number& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const FString& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const String& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Char& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const List& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Ident& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Index& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Slice& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Call& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const TemplateCall& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const If& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Else& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const For& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const While& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Do& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Switch& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Break& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Continue& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Return& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Foreach& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Try& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Catch& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Throw& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Await& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Asm& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const InferTy& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const OpaqueTy& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const NamedTy& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const RefTy& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const PtrTy& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const ArrayTy& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const TupleTy& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const TemplateTy& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const LambdaTy& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Let& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Var& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Fn& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Enum& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Struct& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Union& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Contract& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Trait& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const TypeDef& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Scope& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const Import& node) -> void override {
    // TODO: Check node
    (void)node;
  }

  auto visit(const UnitTest& node) -> void override {
    // TODO: Check node
    (void)node;
  }
};

BOOST_SYMBOL_EXPORT auto Expr::check(const SymbolTable& symbol_table) const -> bool {
  auto visitor = CheckVisitor(symbol_table);
  accept(visitor);

  return visitor.has_errors();
}
