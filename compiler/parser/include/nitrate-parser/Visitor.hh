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

#include <nitrate-parser/ParseTreeFwd.hh>

namespace nitrate::compiler::parser {
#define W_NITRATE_PARSER_VISITOR_DEFINE(name, ref_prefix)  \
  class name {                                             \
  public:                                                  \
    virtual ~name() = default;                             \
                                                           \
    virtual void visit(ref_prefix BinExpr& node) = 0;      \
    virtual void visit(ref_prefix UnaryExpr& node) = 0;    \
    virtual void visit(ref_prefix Number& node) = 0;       \
    virtual void visit(ref_prefix FString& node) = 0;      \
    virtual void visit(ref_prefix String& node) = 0;       \
    virtual void visit(ref_prefix Char& node) = 0;         \
    virtual void visit(ref_prefix List& node) = 0;         \
    virtual void visit(ref_prefix Ident& node) = 0;        \
    virtual void visit(ref_prefix Index& node) = 0;        \
    virtual void visit(ref_prefix Slice& node) = 0;        \
    virtual void visit(ref_prefix Call& node) = 0;         \
    virtual void visit(ref_prefix TemplateCall& node) = 0; \
    virtual void visit(ref_prefix If& node) = 0;           \
    virtual void visit(ref_prefix Else& node) = 0;         \
    virtual void visit(ref_prefix For& node) = 0;          \
    virtual void visit(ref_prefix While& node) = 0;        \
    virtual void visit(ref_prefix Do& node) = 0;           \
    virtual void visit(ref_prefix Switch& node) = 0;       \
    virtual void visit(ref_prefix Break& node) = 0;        \
    virtual void visit(ref_prefix Continue& node) = 0;     \
    virtual void visit(ref_prefix Return& node) = 0;       \
    virtual void visit(ref_prefix Foreach& node) = 0;      \
    virtual void visit(ref_prefix Try& node) = 0;          \
    virtual void visit(ref_prefix Catch& node) = 0;        \
    virtual void visit(ref_prefix Throw& node) = 0;        \
    virtual void visit(ref_prefix Await& node) = 0;        \
    virtual void visit(ref_prefix Asm& node) = 0;          \
                                                           \
    virtual void visit(ref_prefix InferTy& node) = 0;      \
    virtual void visit(ref_prefix OpaqueTy& node) = 0;     \
    virtual void visit(ref_prefix NamedTy& node) = 0;      \
    virtual void visit(ref_prefix RefTy& node) = 0;        \
    virtual void visit(ref_prefix PtrTy& node) = 0;        \
    virtual void visit(ref_prefix ArrayTy& node) = 0;      \
    virtual void visit(ref_prefix TupleTy& node) = 0;      \
    virtual void visit(ref_prefix TemplateTy& node) = 0;   \
    virtual void visit(ref_prefix LambdaTy& node) = 0;     \
                                                           \
    virtual void visit(ref_prefix Let& node) = 0;          \
    virtual void visit(ref_prefix Var& node) = 0;          \
    virtual void visit(ref_prefix Fn& node) = 0;           \
    virtual void visit(ref_prefix Enum& node) = 0;         \
    virtual void visit(ref_prefix Struct& node) = 0;       \
    virtual void visit(ref_prefix Union& node) = 0;        \
    virtual void visit(ref_prefix Contract& node) = 0;     \
    virtual void visit(ref_prefix Trait& node) = 0;        \
    virtual void visit(ref_prefix TypeDef& node) = 0;      \
    virtual void visit(ref_prefix Scope& node) = 0;        \
    virtual void visit(ref_prefix Import& node) = 0;       \
    virtual void visit(ref_prefix UnitTest& node) = 0;     \
  };

  W_NITRATE_PARSER_VISITOR_DEFINE(Visitor, )
  W_NITRATE_PARSER_VISITOR_DEFINE(ConstVisitor, const)

#undef W_NITRATE_PARSER_VISITOR_DEFINE
}  // namespace nitrate::compiler::parser
