#include <gtest/gtest.h>

#include <pipeline/libnitrate-lexer/LexicalCase.hh>

using namespace ncc::lex;

///=============================================================================
/// VALID FLOATS
TEST_CASE(Float, Valid, 0, "3e14", {Token(NumL, "300000000000000.0")});
TEST_CASE(Float, Valid, 1, "34.7917892", {Token(NumL, "34.7917892")});
TEST_CASE(
    Float, Valid, 2, "41.5e+20.4",
    {Token(
        NumL,
        "10424328690764757461002."
        "88308136720868575785011824727302902153694676920947805277943008348168684285738124765991339201120379802772")});
TEST_CASE(Float, Valid, 3, "34.891891e-80.3",
          {Token(NumL,
                 "0."
                 "00000000000000000000000000000000000000000000000000000000000000000000000000000017487370326314319195595"
                 "1635193831572800772700778")});
TEST_CASE(Float, Valid, 4, "45.4444444444441", {Token(NumL, "45.4444444444441")});
TEST_CASE(Float, Valid, 5, "4e90",
          {Token(NumL,
                 "4000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000.0")});
TEST_CASE(Float, Valid, 6, "1.23e4", {Token(NumL, "12300.0")});
TEST_CASE(Float, Valid, 7, "9.8765e-2", {Token(NumL, "0.098765")});
TEST_CASE(Float, Valid, 8, "0.0001", {Token(NumL, "0.0001")});
TEST_CASE(Float, Valid, 9, "1e-9", {Token(NumL, "0.000000001")});
TEST_CASE(Float, Valid, 10, "123.456e7", {Token(NumL, "1234560000.0")});
TEST_CASE(Float, Valid, 11, "0.999999", {Token(NumL, "0.999999")});
TEST_CASE(Float, Valid, 12, "56.78e+3", {Token(NumL, "56780.0")});
TEST_CASE(Float, Valid, 13, "3.1415926535", {Token(NumL, "3.1415926535")});
TEST_CASE(Float, Valid, 14, "7.2e-5", {Token(NumL, "0.000072")});
TEST_CASE(Float, Valid, 15, "99.99", {Token(NumL, "99.99")});
TEST_CASE(Float, Valid, 16, "2.71828e2", {Token(NumL, "271.828")});
TEST_CASE(Float, Valid, 17, "6.022e23", {Token(NumL, "602200000000000000000000.0")});
TEST_CASE(Float, Valid, 18, "0.333e-2", {Token(NumL, "0.00333")});
TEST_CASE(Float, Valid, 19, "8.765e4", {Token(NumL, "87650.0")});
TEST_CASE(Float, Valid, 20, "4.5e+6", {Token(NumL, "4500000.0")});
TEST_CASE(Float, Valid, 21, "1.01e-3", {Token(NumL, "0.00101")});
TEST_CASE(Float, Valid, 22, "5.4321e-7", {Token(NumL, "0.00000054321")});
TEST_CASE(
    Float, Valid, 23, "99.9999e99",
    {Token(NumL,
           "99999900000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000.0")});
TEST_CASE(Float, Valid, 24, "7e-2", {Token(NumL, "0.07")});
TEST_CASE(Float, Valid, 25, "3.0", {Token(NumL, "3.0")});
TEST_CASE(Float, Valid, 26, "2.3e-4", {Token(NumL, "0.00023")});
TEST_CASE(Float, Valid, 27, "4.5678e8", {Token(NumL, "456780000.0")});
TEST_CASE(Float, Valid, 28, "1.9999e-10", {Token(NumL, "0.00000000019999")});
TEST_CASE(Float, Valid, 29, "2.345e+12", {Token(NumL, "2345000000000.0")});
TEST_CASE(Float, Valid, 30, "5e0", {Token(NumL, "5.0")});
TEST_CASE(Float, Valid, 31, "7.77", {Token(NumL, "7.77")});
TEST_CASE(Float, Valid, 32, "8.88e+1", {Token(NumL, "88.8")});
TEST_CASE(Float, Valid, 33, "0.1e1", {Token(NumL, "1.0")});
TEST_CASE(Float, Valid, 34, "123.456", {Token(NumL, "123.456")});
TEST_CASE(Float, Valid, 35, "3.5e-8", {Token(NumL, "0.000000035")});
TEST_CASE(Float, Valid, 36, "2e10", {Token(NumL, "20000000000.0")});
TEST_CASE(Float, Valid, 37, "6.283e+3", {Token(NumL, "6283.0")});
TEST_CASE(Float, Valid, 38, "4.321e-6", {Token(NumL, "0.000004321")});
TEST_CASE(
    Float, Valid, 39, "9e99",
    {Token(NumL,
           "9000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000.0")});
TEST_CASE(Float, Valid, 40, "5.5e-5", {Token(NumL, "0.000055")});
TEST_CASE(Float, Valid, 41, "1.234e4.53.",
          {Token(NumL,
                 "41813."
                 "36886757759430575696106796085125386386957287149695585909212064143149828937551992348582163146963908369"
                 "9737879276"),
           OpDot});

///=============================================================================
/// INVALID FLOATS
TEST_CASE(Float, Invalid, 0, "3e", {Token(IntL, "3"), Token(Name, "e")});
TEST_CASE(Float, Invalid, 1, "3e-", {Token(IntL, "3"), Token(Name, "e"), OpMinus});
TEST_CASE(Float, Invalid, 2, "3e+", {Token(IntL, "3"), Token(Name, "e"), OpPlus});
TEST_CASE(Float, Invalid, 3, "3e.hello", {Token(IntL, "3"), Token(Name, "e"), OpDot, Token(Name, "hello")});
TEST_CASE(Float, Invalid, 4, "3.14f5", {});
TEST_CASE(Float, Invalid, 5, "3.14f5e1", {});
TEST_CASE(Float, Invalid, 6, "3.14e5.3f2", {});
