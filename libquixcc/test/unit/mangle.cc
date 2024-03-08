#include <gtest/gtest.h>
#include <mangle/Symbol.h>
#include <parse/nodes/variable.h>
#include <parse/nodes/integer.h>

using namespace libquixcc;

TEST(Mangle, Serialize) {
    auto varDecl = std::make_shared<VarDeclNode>("myVar", std::make_shared<U8TypeNode>(), nullptr);

    std::string str = Symbol::mangle(varDecl);

    EXPECT_EQ(str, "_ZJ0v5myVar2u00");

    auto varDecl2 = std::static_pointer_cast<VarDeclNode>(Symbol::demangle(str));
    ASSERT_NE(varDecl2, nullptr);

    EXPECT_EQ(*varDecl, *varDecl2);
}