#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

TEST_CASE(String, Sym, 0, R"("abc")", {Token(Text, "abc")});
TEST_CASE(String, Sym, 1, R"('abc')", {Token(Text, "abc")});
TEST_CASE(String, Sym, 2, R"("a")", {Token(Text, "a")});
TEST_CASE(String, Sym, 3, R"('a')", {Token(Char, "a")});
TEST_CASE(String, Sym, 4, R"('foo' 'bar')", {Token(Text, "foobar")});
TEST_CASE(String, Sym, 5, R"("foo" "bar")", {Token(Text, "foobar")});
TEST_CASE(String, Sym, 6, R"("foo" 'bar')", {Token(Text, "foo"), Token(Text, "bar")});
TEST_CASE(String, Sym, 7, R"('foo' "bar")", {Token(Text, "foo"), Token(Text, "bar")});
TEST_CASE(String, Sym, 8, R"("foo" \
                             "foo" \
                             "baz"  "hehe"
                             "dodo")",
          {Token(Text, "foofoobazhehedodo")});
TEST_CASE(String, Sym, 9, R"('🔥 🍉')", {Token(Text, "🔥 🍉")});
TEST_CASE(String, Sym, 10, "'\x4e\xe0\x10\x1b\x19\xa4\x36\x47\x2c\x07'",
          {Token(Text, "\x4e\xe0\x10\x1b\x19\xa4\x36\x47\x2c\x07")});
TEST_CASE(String, Sym, 11, "\"abc\x45\x89\xd0\"", {Token(Text, "abc\x45\x89\xd0")});
