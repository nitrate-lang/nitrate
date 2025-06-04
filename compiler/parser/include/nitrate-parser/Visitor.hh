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
#define W_NITRATE_PARSER_VISITOR_DEFINE(name, ref_prefix) \
  class name {                                            \
  public:                                                 \
    virtual ~name() = default;                            \
                                                          \
    virtual void visit(ref_prefix Expr& expr) = 0;        \
    virtual void visit(ref_prefix Break& expr) = 0;       \
  };

  W_NITRATE_PARSER_VISITOR_DEFINE(Visitor, )
  W_NITRATE_PARSER_VISITOR_DEFINE(ConstVisitor, const)

#undef W_NITRATE_PARSER_VISITOR_DEFINE
}  // namespace nitrate::compiler::parser
