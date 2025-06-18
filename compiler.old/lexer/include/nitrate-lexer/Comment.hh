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

#include <cstdint>
#include <nitrate-lexer/StringData.hh>

namespace nitrate::compiler::lexer {
  enum class CommentType : uint8_t {
    SingleLine,
    MultiLine,
  };

  class Comment {
    StringData m_value;
    CommentType m_type;

  public:
    Comment(StringData value, CommentType type) : m_value(std::move(value)), m_type(type) {}

    [[nodiscard]] constexpr auto type() const -> CommentType { return m_type; }
    [[nodiscard]] constexpr auto value() const -> const std::string& { return m_value.get(); }

    [[nodiscard]] constexpr auto is_single_line() const -> bool { return m_type == CommentType::SingleLine; }
    [[nodiscard]] constexpr auto is_multi_line() const -> bool { return m_type == CommentType::MultiLine; }
    [[nodiscard]] auto is_documentation() const -> bool;
    [[nodiscard]] auto is_tool_signal() const -> bool;

    [[nodiscard]] constexpr auto begin() const { return m_value->cbegin(); }
    [[nodiscard]] constexpr auto end() const { return m_value->cend(); }
  };
}  // namespace nitrate::compiler::lexer
