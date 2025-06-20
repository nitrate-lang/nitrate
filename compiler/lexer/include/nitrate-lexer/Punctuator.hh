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

#include <boost/bimap.hpp>
#include <cstdint>
#include <string_view>

namespace nitrate::compiler::lexer {
  enum class Punctuator : uint8_t {
    LeftParenthesis,   // (
    RightParenthesis,  // )
    LeftBracket,       // [
    RightBracket,      // ]
    LeftBrace,         // {
    RightBrace,        // }
    Comma,             // ,
    Semicolon,         // ;
    Colon,             // :
    AtSign,            // @
  };

  static inline const boost::bimap<Punctuator, uint8_t> PUNCTOR_MAP = [] {
    boost::bimap<Punctuator, uint8_t> mapping;
    auto& map = mapping.left;

    map.insert({Punctuator::LeftParenthesis, '('});
    map.insert({Punctuator::RightParenthesis, ')'});
    map.insert({Punctuator::LeftBracket, '['});
    map.insert({Punctuator::RightBracket, ']'});
    map.insert({Punctuator::LeftBrace, '{'});
    map.insert({Punctuator::RightBrace, '}'});
    map.insert({Punctuator::Comma, ','});
    map.insert({Punctuator::Semicolon, ';'});
    map.insert({Punctuator::Colon, ':'});
    map.insert({Punctuator::AtSign, '@'});

    return mapping;
  }();

  [[nodiscard]] inline auto is_punctor(uint8_t str) -> bool {
    return PUNCTOR_MAP.right.find(str) != PUNCTOR_MAP.right.end();
  }

  [[nodiscard]] inline auto punctor_from_byte(uint8_t str) -> std::optional<Punctuator> {
    auto it = PUNCTOR_MAP.right.find(str);
    if (it != PUNCTOR_MAP.right.end()) {
      return it->second;
    }

    return std::nullopt;
  }

  [[nodiscard]] inline auto punctor_to_string(Punctuator punctor) -> std::string_view {
    const uint8_t& ch = PUNCTOR_MAP.left.at(punctor);
    const char* ch_ptr = reinterpret_cast<const char*>(&ch);
    return {ch_ptr, 1};
  }
}  // namespace nitrate::compiler::lexer
