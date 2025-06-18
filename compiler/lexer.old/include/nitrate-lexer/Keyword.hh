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

namespace nitrate::compiler::lexer {
  enum class Keyword : uint8_t {
    /* Storage */
    Let,      /* 'let' */
    Var,      /* 'var' */
    Fn,       /* 'fn' */
    Enum,     /* 'enum' */
    Struct,   /* 'struct' */
    Class,    /* 'class' */
    Union,    /* 'union' */
    Contract, /* 'interface' */
    Trait,    /* 'trait' */
    Type,     /* 'type' */
    Opaque,   /* 'opaque' */
    Scope,    /* 'scope' */
    Import,   /* 'import' */
    UnitTest, /* 'unit_test' */

    /* Modifiers */
    Safe,    /* 'safe' */
    Unsafe,  /* 'unsafe' */
    Promise, /* 'promise' */
    Static,  /* 'static' */
    Mut,     /* 'mut' */
    Const,   /* 'const' */
    Pub,     /* 'pub' */
    Sec,     /* 'sec' */
    Pro,     /* 'pro' */

    /* Control Flow */
    If,       /* 'if' */
    Else,     /* 'else' */
    For,      /* 'for' */
    While,    /* 'while' */
    Do,       /* 'do' */
    Switch,   /* 'switch' */
    Break,    /* 'break' */
    Continue, /* 'continue' */
    Return,   /* 'ret' */
    Foreach,  /* 'foreach' */
    Try,      /* 'try' */
    Catch,    /* 'catch' */
    Throw,    /* 'throw' */
    Async,    /* 'async' */
    Await,    /* 'await' */
    Asm,      /* 'asm' */

    /* Literals */
    Null,  /* 'null' */
    True,  /* 'true' */
    False, /* 'false' */
  };

  static inline const boost::bimap<Keyword, std::string_view> KEYWORD_MAP = [] {
    boost::bimap<Keyword, std::string_view> mapping;
    auto& map = mapping.left;

    map.insert({Keyword::Let, "let"});
    map.insert({Keyword::Var, "var"});
    map.insert({Keyword::Fn, "fn"});
    map.insert({Keyword::Enum, "enum"});
    map.insert({Keyword::Struct, "struct"});
    map.insert({Keyword::Class, "class"});
    map.insert({Keyword::Union, "union"});
    map.insert({Keyword::Contract, "interface"});
    map.insert({Keyword::Trait, "trait"});
    map.insert({Keyword::Opaque, "opaque"});
    map.insert({Keyword::Type, "type"});
    map.insert({Keyword::Scope, "scope"});
    map.insert({Keyword::Import, "import"});
    map.insert({Keyword::UnitTest, "unit_test"});

    map.insert({Keyword::Safe, "safe"});
    map.insert({Keyword::Unsafe, "unsafe"});
    map.insert({Keyword::Promise, "promise"});
    map.insert({Keyword::Static, "static"});
    map.insert({Keyword::Mut, "mut"});
    map.insert({Keyword::Const, "const"});
    map.insert({Keyword::Pub, "pub"});
    map.insert({Keyword::Sec, "sec"});
    map.insert({Keyword::Pro, "pro"});

    map.insert({Keyword::If, "if"});
    map.insert({Keyword::Else, "else"});
    map.insert({Keyword::For, "for"});
    map.insert({Keyword::While, "while"});
    map.insert({Keyword::Do, "do"});
    map.insert({Keyword::Switch, "switch"});
    map.insert({Keyword::Break, "break"});
    map.insert({Keyword::Continue, "continue"});
    map.insert({Keyword::Return, "ret"});
    map.insert({Keyword::Foreach, "foreach"});
    map.insert({Keyword::Try, "try"});
    map.insert({Keyword::Catch, "catch"});
    map.insert({Keyword::Throw, "throw"});
    map.insert({Keyword::Async, "async"});
    map.insert({Keyword::Await, "await"});
    map.insert({Keyword::Asm, "asm"});

    map.insert({Keyword::Null, "null"});
    map.insert({Keyword::True, "true"});
    map.insert({Keyword::False, "false"});

    return mapping;
  }();

  [[nodiscard]] inline auto is_keyword(std::string_view str) -> bool {
    return KEYWORD_MAP.right.find(str) != KEYWORD_MAP.right.end();
  }

  [[nodiscard]] inline auto keyword_from_string(std::string_view str) -> std::optional<Keyword> {
    auto it = KEYWORD_MAP.right.find(str);
    if (it != KEYWORD_MAP.right.end()) {
      return it->second;
    }

    return std::nullopt;
  }

  [[nodiscard]] inline auto keyword_to_string(Keyword keyword) -> std::string_view {
    return KEYWORD_MAP.left.at(keyword);
  }
}  // namespace nitrate::compiler::lexer
