//! Tests for keyword lexing.

use crate::tests::eq;
use nitrate_token::Token;

macro_rules! keyword_tests {
    ($($name:ident => $str:expr => $tok:ident),* $(,)?) => {
        $(
            #[test]
            fn $name() { eq($str, Token::$tok); }
        )*
    };
}

keyword_tests! {
    test_keyword_let       => "let"       => Let,
    test_keyword_var       => "var"       => Var,
    test_keyword_fn        => "fn"        => Fn,
    test_keyword_enum      => "enum"      => Enum,
    test_keyword_struct    => "struct"    => Struct,
    test_keyword_class     => "class"     => Class,
    test_keyword_union     => "union"     => Union,
    test_keyword_contract  => "contract"  => Contract,
    test_keyword_trait     => "trait"     => Trait,
    test_keyword_impl      => "impl"      => Impl,
    test_keyword_type      => "type"      => Type,
    test_keyword_scope     => "scope"     => Scope,
    test_keyword_use       => "use"       => Use,
    test_keyword_mod       => "mod"       => Mod,
    test_keyword_safe      => "safe"      => Safe,
    test_keyword_unsafe    => "unsafe"    => Unsafe,
    test_keyword_promise   => "promise"   => Promise,
    test_keyword_static    => "static"    => Static,
    test_keyword_mut       => "mut"       => Mut,
    test_keyword_const     => "const"     => Const,
    test_keyword_poly      => "poly"      => Poly,
    test_keyword_iso       => "iso"       => Iso,
    test_keyword_pub       => "pub"       => Pub,
    test_keyword_sec       => "sec"       => Sec,
    test_keyword_pro       => "pro"       => Pro,
    test_keyword_if        => "if"        => If,
    test_keyword_else      => "else"      => Else,
    test_keyword_for       => "for"       => For,
    test_keyword_in        => "in"        => In,
    test_keyword_while     => "while"     => While,
    test_keyword_do        => "do"        => Do,
    test_keyword_match     => "match"     => Match,
    test_keyword_break     => "break"     => Break,
    test_keyword_continue  => "continue"  => Continue,
    test_keyword_ret       => "ret"       => Ret,
    test_keyword_async     => "async"     => Async,
    test_keyword_await     => "await"     => Await,
    test_keyword_asm       => "asm"       => Asm,
    test_keyword_null      => "null"      => Null,
    test_keyword_true      => "true"      => True,
    test_keyword_false     => "false"     => False,
    test_keyword_bool      => "bool"      => Bool,
    test_keyword_u8        => "u8"        => U8,
    test_keyword_u16       => "u16"       => U16,
    test_keyword_u32       => "u32"       => U32,
    test_keyword_u64       => "u64"       => U64,
    test_keyword_u128      => "u128"      => U128,
    test_keyword_usize     => "usize"     => USize,
    test_keyword_i8        => "i8"        => I8,
    test_keyword_i16       => "i16"       => I16,
    test_keyword_i32       => "i32"       => I32,
    test_keyword_i64       => "i64"       => I64,
    test_keyword_i128      => "i128"      => I128,
    test_keyword_f8        => "f8"        => F8,
    test_keyword_f16       => "f16"       => F16,
    test_keyword_f32       => "f32"       => F32,
    test_keyword_f64       => "f64"       => F64,
    test_keyword_f128      => "f128"      => F128,
    test_keyword_opaque    => "opaque"    => Opaque,
    test_keyword_as        => "as"        => As,
    test_keyword_typeof    => "typeof"    => Typeof,
}

#[test]
fn test_keyword_not_matched_when_part_of_identifier() {
    use crate::tests::tokens_skipping_trivia;
    let toks = tokens_skipping_trivia("letfoo");
    assert_eq!(toks.len(), 1);
    assert_eq!(toks[0].token, Token::Name("letfoo".into()));
}

#[test]
fn test_keyword_with_trailing_underscore_is_identifier() {
    use crate::tests::tokens_skipping_trivia;
    let toks = tokens_skipping_trivia("let_");
    assert_eq!(toks.len(), 1);
    assert_eq!(toks[0].token, Token::Name("let_".into()));
}
