#include <gtest/gtest.h>

#include <pipeline/libnitrate-parser/TestCase.hh>

///===================================================================================================
/// PARSE RETURN-IF MISING RETURN VALUE
TEST_CASE(Stmt_ReturnIf, NoValue, 0, R"( retif 10; )", block{statements{return_if{condition{integer{number : "10"}}}}});
TEST_CASE(Stmt_ReturnIf, NoValue, 1, R"( retif 10, ; )", nullptr);
TEST_CASE(Stmt_ReturnIf, NoValue, 2, R"( retif 10 )", nullptr);
TEST_CASE(Stmt_ReturnIf, NoValue, 3, R"( retif 10, )", nullptr);

///===================================================================================================
/// PARSE RETURN-IF MISING RETURN CONDITION
TEST_CASE(Stmt_ReturnIf, NoCond, 0, R"( retif ; )", nullptr);
TEST_CASE(Stmt_ReturnIf, NoCond, 1, R"( retif , ; )", nullptr);

///===================================================================================================
/// PARSE RETURN-IF EXPRESSION FORGOT COMMA
TEST_CASE(Stmt_ReturnIf, NoComma, 0, R"( retif 10 20; )", nullptr);
TEST_CASE(Stmt_ReturnIf, NoComma, 1, R"( retif 10 20, ; )", nullptr);
TEST_CASE(Stmt_ReturnIf, NoComma, 2, R"( retif 10 20 )", nullptr);
TEST_CASE(Stmt_ReturnIf, NoComma, 3, R"( retif 10 20, )", nullptr);

///===================================================================================================
/// PARSE RETURN-IF EXPRESSION FORGOT SEMICOLON
TEST_CASE(Stmt_ReturnIf, NoSemi, 0, R"( retif 10, 20 )", nullptr);
TEST_CASE(Stmt_ReturnIf, NoSemi, 1, R"( retif 10, 20, )", nullptr);

///===================================================================================================
/// PARSE RETURN-IF EXPRESSION STATEMENT
TEST_CASE(Stmt_ReturnIf, Expr, 0, R"( retif 10, 20; )",
          block{statements{return_if{condition{integer{number : "10"}} value{integer{number : "20"}}}}});
TEST_CASE(Stmt_ReturnIf, Expr, 1, R"( retif 1+1, 2+2; )", block{statements{return_if{condition{binary{operator:Op_Plusleft{integer{number:"1"}}right{integer{number:"1"}}}}value{binary{operator:Op_Plusleft{integer{number:"2"}}right{integer{number:"2"}}}}}}});
