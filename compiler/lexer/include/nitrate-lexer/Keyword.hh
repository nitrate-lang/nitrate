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
    /* Code Organization */
    Scope,  /* 'scope' */
    Import, /* 'import' */
    Pub,    /* 'pub' */
    Sec,    /* 'sec' */
    Pro,    /* 'pro' */

    /* Code Symbols */
    Let, /* 'let' */
    Var, /* 'var' */
    Fn,  /* 'fn' */

    /* Modifiers */
    Safe,    /* 'safe' */
    Unsafe,  /* 'unsafe' */
    Promise, /* 'promise' */
    Static,  /* 'static' */
    Mut,     /* 'mut' */
    Const,   /* 'const' */

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

    /* Data Types */
    Type,     /* 'type' */
    Opaque,   /* 'opaque' */
    Enum,     /* 'enum' */
    Struct,   /* 'struct' */
    Class,    /* 'class' */
    Contract, /* 'interface' */
    Trait,    /* 'trait' */
    Union,    /* 'union' */
  };

  static inline const boost::bimap<Keyword, std::string_view> KEYWORD_MAP = [] {
    boost::bimap<Keyword, std::string_view> mapping;
    auto& map = mapping.left;

    map.insert({Keyword::Scope, "scope"});
    map.insert({Keyword::Import, "import"});
    map.insert({Keyword::Pub, "pub"});
    map.insert({Keyword::Sec, "sec"});
    map.insert({Keyword::Pro, "pro"});

    map.insert({Keyword::Let, "let"});
    map.insert({Keyword::Var, "var"});
    map.insert({Keyword::Fn, "fn"});

    map.insert({Keyword::Safe, "safe"});
    map.insert({Keyword::Unsafe, "unsafe"});
    map.insert({Keyword::Promise, "promise"});
    map.insert({Keyword::Static, "static"});
    map.insert({Keyword::Mut, "mut"});
    map.insert({Keyword::Const, "const"});

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

    map.insert({Keyword::Type, "type"});
    map.insert({Keyword::Opaque, "opaque"});
    map.insert({Keyword::Enum, "enum"});
    map.insert({Keyword::Struct, "struct"});
    map.insert({Keyword::Class, "class"});
    map.insert({Keyword::Contract, "interface"});
    map.insert({Keyword::Trait, "trait"});
    map.insert({Keyword::Union, "union"});

    return mapping;
  }();
}  // namespace nitrate::compiler::lexer
