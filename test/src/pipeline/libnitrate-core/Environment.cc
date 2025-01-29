#include <gtest/gtest.h>

#include <nitrate-core/Environment.hh>
#include <nitrate-core/Init.hh>

static const char* Key = "Hello, World!";
static const char* Value = "Hello, World!";

TEST(Core, Environment_Create) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto env = std::make_shared<ncc::Environment>();

    EXPECT_TRUE(env->Contains("this.keys"));
    EXPECT_TRUE(env->Contains("this.job"));
    EXPECT_TRUE(env->Contains("this.created_at"));
  }
}

TEST(Core, Environment_Set) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto env = std::make_shared<ncc::Environment>();

    EXPECT_FALSE(env->Contains(Key));
    env->Set(Key, Value);
    EXPECT_TRUE(env->Contains(Key));
  }
}

TEST(Core, Environment_Set_Special) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto env = std::make_shared<ncc::Environment>();

    EXPECT_TRUE(env->Contains("this.keys"));
    env->Set("this.keys", Value);
    EXPECT_TRUE(env->Contains("this.keys"));
  }
}

TEST(Core, Environment_Unset_Present) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto env = std::make_shared<ncc::Environment>();

    EXPECT_FALSE(env->Contains(Key));
    env->Set(Key, Value);
    EXPECT_TRUE(env->Contains(Key));
    env->Set(Key, std::nullopt);
    EXPECT_FALSE(env->Contains(Key));
  }
}

TEST(Core, Environment_Unset_NotPresent) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto env = std::make_shared<ncc::Environment>();

    EXPECT_FALSE(env->Contains(Key));
    env->Set(Key, std::nullopt);
    EXPECT_FALSE(env->Contains(Key));
  }
}

TEST(Core, Environment_Unset_Special) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto env = std::make_shared<ncc::Environment>();

    EXPECT_TRUE(env->Contains("this.keys"));
    env->Set("this.keys", std::nullopt);
    EXPECT_TRUE(env->Contains("this.keys"));
  }
}

TEST(Core, Environment_Contains_Present) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto env = std::make_shared<ncc::Environment>();

    EXPECT_FALSE(env->Contains(Key));
    env->Set(Key, Value);
    EXPECT_TRUE(env->Contains(Key));
  }
}

TEST(Core, Environment_Contains_NotPresent) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto env = std::make_shared<ncc::Environment>();

    EXPECT_FALSE(env->Contains(Key));
  }
}

TEST(Core, Environment_Get_Present) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto env = std::make_shared<ncc::Environment>();

    EXPECT_FALSE(env->Contains(Key));
    env->Set(Key, Value);
    EXPECT_TRUE(env->Contains(Key));
    EXPECT_EQ(env->Get(Key).value(), Value);
  }
}

TEST(Core, Environment_Get_NotPresent) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto env = std::make_shared<ncc::Environment>();

    EXPECT_FALSE(env->Contains(Key));
    EXPECT_EQ(env->Get(Key), std::nullopt);
  }
}

TEST(Core, Environment_Get_Special) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto env = std::make_shared<ncc::Environment>();

    EXPECT_TRUE(env->Contains("this.keys"));
    EXPECT_EQ(env->Get("this.keys").value(), "15 this.created_at8 this.job");
  }
}
