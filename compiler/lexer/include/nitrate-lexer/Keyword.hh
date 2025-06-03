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

    map[Keyword::Scope] = "scope";
    map[Keyword::Import] = "import";
    map[Keyword::Pub] = "pub";
    map[Keyword::Sec] = "sec";
    map[Keyword::Pro] = "pro";

    map[Keyword::Let] = "let";
    map[Keyword::Var] = "var";
    map[Keyword::Fn] = "fn";

    map[Keyword::Safe] = "safe";
    map[Keyword::Unsafe] = "unsafe";
    map[Keyword::Promise] = "promise";
    map[Keyword::Static] = "static";
    map[Keyword::Mut] = "mut";
    map[Keyword::Const] = "const";

    map[Keyword::If] = "if";
    map[Keyword::Else] = "else";
    map[Keyword::For] = "for";
    map[Keyword::While] = "while";
    map[Keyword::Do] = "do";
    map[Keyword::Switch] = "switch";
    map[Keyword::Break] = "break";
    map[Keyword::Continue] = "continue";
    map[Keyword::Return] = "ret";
    map[Keyword::Foreach] = "foreach";
    map[Keyword::Try] = "try";
    map[Keyword::Catch] = "catch";
    map[Keyword::Throw] = "throw";
    map[Keyword::Async] = "async";
    map[Keyword::Await] = "await";
    map[Keyword::Asm] = "asm";

    map[Keyword::Null] = "null";
    map[Keyword::True] = "true";
    map[Keyword::False] = "false";

    map[Keyword::Type] = "type";
    map[Keyword::Opaque] = "opaque";
    map[Keyword::Enum] = "enum";
    map[Keyword::Struct] = "struct";
    map[Keyword::Class] = "class";
    map[Keyword::Contract] = "interface";
    map[Keyword::Trait] = "trait";
    map[Keyword::Union] = "union";

    return mapping;
  }();
}  // namespace nitrate::compiler::lexer
