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
  enum class Punctor : uint8_t {
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

  static inline const boost::bimap<Punctor, std::string_view> PUNCTOR_MAP = [] {
    boost::bimap<Punctor, std::string_view> mapping;
    auto& map = mapping.left;

    map.insert({Punctor::LeftParenthesis, "("});
    map.insert({Punctor::RightParenthesis, ")"});
    map.insert({Punctor::LeftBracket, "["});
    map.insert({Punctor::RightBracket, "]"});
    map.insert({Punctor::LeftBrace, "{"});
    map.insert({Punctor::RightBrace, "}"});
    map.insert({Punctor::Comma, ","});
    map.insert({Punctor::Semicolon, ";"});
    map.insert({Punctor::Colon, ":"});
    map.insert({Punctor::AtSign, "@"});

    return mapping;
  }();
}  // namespace nitrate::compiler::lexer
