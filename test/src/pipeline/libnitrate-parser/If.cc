#include <gtest/gtest.h>

#include <pipeline/libnitrate-parser/TestCase.hh>

///===================================================================================================
/// PARSE IF ILL-FORMED CONDITION
TEST_CASE(Stmt_If, IllCond, 0, R"( if ; )", nullptr);
TEST_CASE(Stmt_If, IllCond, 1, R"( if , ; )", nullptr);
TEST_CASE(Stmt_If, IllCond, 2, R"( if )", nullptr);
TEST_CASE(Stmt_If, IllCond, 3, R"( if , )", nullptr);
TEST_CASE(Stmt_If, IllCond, 4, R"( if 10 type )", nullptr);

///===================================================================================================
/// PARSE IF ILL-FORMED THEN BLOCK
TEST_CASE(Stmt_If, IllThen, 0, R"( if 10; )", nullptr);
TEST_CASE(Stmt_If, IllThen, 1, R"( if 10 )", nullptr);
TEST_CASE(Stmt_If, IllThen, 2, R"( if 10 { )", nullptr);
TEST_CASE(Stmt_If, IllThen, 3, R"( if 10 => )", nullptr);
TEST_CASE(Stmt_If, IllThen, 4, R"( if 10 => { )", nullptr);

///===================================================================================================
/// PARSE IF ILL-FORMED ELSE BLOCK
TEST_CASE(Stmt_If, IllElse, 0, R"( if 10 {} { )", nullptr);
TEST_CASE(Stmt_If, IllElse, 1, R"( if 10 {} => )", nullptr);
TEST_CASE(Stmt_If, IllElse, 2, R"( if 10 {} => { )", nullptr);

///===================================================================================================
/// PARSE IF MISSING ELSE BLOCK
TEST_CASE(Stmt_If, NoElse, 0, R"( if 10 {} )",
          block{safety : None statements{if {condition{integer{number : "10"}} true_branch{block{safety : None}}}}});
TEST_CASE(Stmt_If, NoElse, 1, R"( if 10 {} ; )",
          block{safety : None statements{if {condition{integer{number : "10"}} true_branch{block{safety : None}}}}});

///===================================================================================================
/// PARSE IF STATEMENT CHAIN
TEST_CASE(Stmt_If, WithElse, 0, R"( if 10 {} else {} )", block{
  safety : Nonestatements{
      if {condition{integer{number : "10"}} true_branch{block{safety : None}} false_branch{block{safety : None}}}}
});

TEST_CASE(Stmt_If, WithElse, 1, R"( if 10 {} else {1+1;} )", block{safety:None statements{if{condition{integer{number:"10"}}true_branch{block{safety:None}}false_branch{block{safety:None statements{binary{operator: Op_Plus left{integer{number:"1"}}right{integer{number:"1"}}}}}}}}});

TEST_CASE(Stmt_If, WithElse, 2, R"( if 10 {} else if 20 {} )", block{
  safety : Nonestatements{if {condition{integer{number : "10"}} true_branch{
      block{safety : None}} false_branch{if {condition{integer{number : "20"}} true_branch{block{safety : None}}}}}}
});

TEST_CASE(Stmt_If, WithElse, 3, R"( if 10 {} else if 20 {} else {} )", block{
  safety : None statements{if {condition{integer{number : "10"}} true_branch{block{safety : None}} false_branch{
      if {condition{integer{number : "20"}} true_branch{block{safety : None}} false_branch{block{safety : None}}}}}}
});

TEST_CASE(Stmt_If, WithElse, 4, R"( if 10 {} else if 20 {} else if 30 {} )", block{
  safety : None statements{if {condition{integer{number : "10"}} true_branch{
      block{safety : None}} false_branch{if {condition{integer{number : "20"}} true_branch{
      block{safety : None}} false_branch{if {condition{integer{number : "30"}} true_branch{block{safety : None}}}}}}}}
});
