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

#include <nitrate-lexer/Comment.hh>

using namespace nitrate::compiler::lexer;

BOOST_SYMBOL_EXPORT auto Comment::is_documentation() const -> bool {
  if (is_single_line()) {  // looks like "/// comment"
    return value().starts_with("///");
  }

  if (is_multi_line()) {  // looks like "/** comment */"
    return value().starts_with("/**") && value().ends_with("*/");
  }

  return false;
}

BOOST_SYMBOL_EXPORT auto Comment::is_tool_signal() const -> bool {
  if (is_single_line()) {  // looks like "#! comment"
    return value().starts_with("#!");
  }

  if (is_multi_line()) {  // looks like "/*! comment */"
    return value().starts_with("/*!") && value().ends_with("*/");
  }

  return false;
}
