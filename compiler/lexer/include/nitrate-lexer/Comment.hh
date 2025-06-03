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
#include <cstdint>

namespace nitrate::compiler::lexer {
  enum class CommentType : uint8_t {
    SingleLine,
    MultiLine,
  };

  class Comment {
    boost::flyweight<std::string> m_content;
    CommentType m_type;

  public:
    Comment(CommentType type, boost::flyweight<std::string> content) : m_content(std::move(content)), m_type(type) {}

    [[nodiscard]] constexpr auto type() const -> CommentType { return m_type; }
    [[nodiscard]] constexpr auto content() const -> const boost::flyweight<std::string>& { return m_content; }
    [[nodiscard]] constexpr auto is_single_line() const -> bool { return m_type == CommentType::SingleLine; }
    [[nodiscard]] constexpr auto is_multi_line() const -> bool { return m_type == CommentType::MultiLine; }
    [[nodiscard]] auto is_documentation() const -> bool;
    [[nodiscard]] auto is_tool_signal() const -> bool;
  };
}  // namespace nitrate::compiler::lexer
