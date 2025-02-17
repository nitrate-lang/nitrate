#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

///=============================================================================
/// DOUBLE SLASH LINE COMMENTS
TEST_CASE(Comment, DoubleSlash, 0, "// hello", {Token(Note, " hello")})
TEST_CASE(Comment, DoubleSlash, 1, "// hello\n// world12 \\\\", {Token(Note, " hello"), Token(Note, " world12 \\\\")})
TEST_CASE(Comment, DoubleSlash, 2, "////////", {Token(Note, "//////")})
TEST_CASE(Comment, DoubleSlash, 3, "//", {Token(Note, "")})
TEST_CASE(Comment, DoubleSlash, 4, "//\n//", {Token(Note, ""), Token(Note, "")})
TEST_CASE(Comment, DoubleSlash, 5, "//hello\rworld\n//foo", {Token(Note, "hello\rworld"), Token(Note, "foo")})
TEST_CASE(Comment, DoubleSlash, 6, "// hehe 🔥🍉", {Token(Note, " hehe 🔥🍉")})
TEST_CASE(Comment, DoubleSlash, 7, "// \xed\xa0\x80\xed", {Token(Note, " \xed\xa0\x80\xed")})

///=============================================================================
/// HASHTAG LINE COMMENTS
TEST_CASE(Comment, Hashtag, 0, "# hello", {Token(Note, " hello")})
TEST_CASE(Comment, Hashtag, 1, "# hello\n# world12 \\\\", {Token(Note, " hello"), Token(Note, " world12 \\\\")})
TEST_CASE(Comment, Hashtag, 2, "####", {Token(Note, "###")})
TEST_CASE(Comment, Hashtag, 3, "#", {Token(Note, "")})
TEST_CASE(Comment, Hashtag, 4, "#\n#", {Token(Note, ""), Token(Note, "")})
TEST_CASE(Comment, Hashtag, 5, "#hello\rworld\n#foo", {Token(Note, "hello\rworld"), Token(Note, "foo")})
TEST_CASE(Comment, Hashtag, 6, "# hehe 🔥🍉", {Token(Note, " hehe 🔥🍉")})
TEST_CASE(Comment, Hashtag, 7, "# \xed\xa0\x80\xed", {Token(Note, " \xed\xa0\x80\xed")})

///=============================================================================
/// TILDE LINE COMMENTS
TEST_CASE(Comment, Tilde, 0, "~> hello", {Token(Note, " hello")})
TEST_CASE(Comment, Tilde, 1, "~> hello\n~> world12 \\\\", {Token(Note, " hello"), Token(Note, " world12 \\\\")})
TEST_CASE(Comment, Tilde, 2, "~>~>~>~>", {Token(Note, "~>~>~>")})
TEST_CASE(Comment, Tilde, 3, "~>", {Token(Note, "")})
TEST_CASE(Comment, Tilde, 4, "~>\n~>", {Token(Note, ""), Token(Note, "")})
TEST_CASE(Comment, Tilde, 5, "~>hello\rworld\n~>foo", {Token(Note, "hello\rworld"), Token(Note, "foo")})
TEST_CASE(Comment, Tilde, 6, "~> hehe 🔥🍉", {Token(Note, " hehe 🔥🍉")})
TEST_CASE(Comment, Tilde, 7, "~> \xed\xa0\x80\xed", {Token(Note, " \xed\xa0\x80\xed")})

///=============================================================================
/// MULTI-LINE COMMENTS
TEST_CASE(Comment, MultiLine, 0, "/* hello */", {Token(Note, " hello ")})
TEST_CASE(Comment, MultiLine, 1, "/* hello\nworld12 \\\\ */", {Token(Note, " hello\nworld12 \\\\ ")})
TEST_CASE(Comment, MultiLine, 2, "/*\n*/", {Token(Note, "\n")})
TEST_CASE(Comment, MultiLine, 3, "/* foo /* bar /* baz */ */ car */", {Token(Note, " foo /* bar /* baz */ */ car ")})
TEST_CASE(Comment, MultiLine, 4, "/*/*/*/*/**/*//**//**/*/*/*/", {Token(Note, "/*/*/*/**/*//**//**/*/*/")})
TEST_CASE(Comment, MultiLine, 5, "/* hehe 🔥🍉*/", {Token(Note, " hehe 🔥🍉")})
TEST_CASE(Comment, MultiLine, 6, "/*\xed\xa0\x80\xed*/", {Token(Note, "\xed\xa0\x80\xed")})
TEST_CASE(Comment, MultiLine, 7, "/* //foo// * */", {Token(Note, " //foo// * ")})
