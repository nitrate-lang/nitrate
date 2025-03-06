#include <gtest/gtest.h>

#include <pipeline/libnitrate-parser/TestCase.hh>

///===================================================================================================
/// PARSE BINARY EXPRESSION
TEST_CASE(Expr_Bin, Basic, 0, R"( 1+1; )", block { safety: None statements { binary { operator: Op_Plus left { integer { number: "1" } } right { integer { number: "1" } } } }});
TEST_CASE(Expr_Bin, Basic, 1, R"( 2+2*3; )", block { safety: None statements { binary { operator: Op_Plus left { integer { number: "2" } } right { binary { operator: Op_Times left { integer { number: "2" } } right { integer { number: "3" } } } } } }});
