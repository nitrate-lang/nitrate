#include <gtest/gtest.h>

#include <pipeline/libnitrate-parser/TestCase.hh>

///===================================================================================================
/// PARSE RETURN VOID STATEMENT
TEST_CASE(Stmt_Return, Void, 0, R"( ret; )", block{safety : None statements{return {}}});
TEST_CASE(Stmt_Return, Void, 1, R"( ret /* */ ;; )", block{safety : None statements{return {}}});
TEST_CASE(Stmt_Return, Void, 2, R"( ret void; )",
          block{safety : None statements{return {value{identifier{name : "void"}}}}});
TEST_CASE(Stmt_Return, Void, 3, R"( ret  )", nullptr);
TEST_CASE(Stmt_Return, Void, 4, R"( return; )", block{safety : None statements{identifier{name : "return"}}});
TEST_CASE(Stmt_Return, Void, 5, R"( return /* */ ;; )", block{safety : None statements{identifier{name : "return"}}});
TEST_CASE(Stmt_Return, Void, 6, R"( return void; )", nullptr);
TEST_CASE(Stmt_Return, Void, 7, R"( return  )", nullptr);

///===================================================================================================
/// PARSE RETURN EXPRESSION STATEMENT
TEST_CASE(Stmt_Return, Expr, 0, R"( ret 1; )", block{safety : None statements{return {value{integer{number : "1"}}}}});
TEST_CASE(Stmt_Return, Expr, 1, R"( ret foo; )",
          block{safety : None statements{return {value{identifier{name : "foo"}}}}});
TEST_CASE(Stmt_Return, Expr, 2, R"( ret 1,2; )", nullptr);
TEST_CASE(Stmt_Return, Expr, 3, R"( ret ret; )", nullptr);
TEST_CASE(Stmt_Return, Expr, 4, R"( return 1; )", nullptr);
TEST_CASE(Stmt_Return, Expr, 5, R"( return foo; )", nullptr);
TEST_CASE(Stmt_Return, Expr, 6, R"( return 1,2; )", nullptr);
TEST_CASE(Stmt_Return, Expr, 7, R"( return ret; )", nullptr);
