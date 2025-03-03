#include <gtest/gtest.h>

#include <nitrate-core/Init.hh>

TEST(Core, Init_Init) {
  if (!ncc::CoreLibrary.IsInitialized()) {
    EXPECT_FALSE(ncc::CoreLibrary.IsInitialized());

    if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
      EXPECT_TRUE(ncc::CoreLibrary.IsInitialized());
    }

    EXPECT_FALSE(ncc::CoreLibrary.IsInitialized());
  }
}

TEST(Core, Init_Version) { EXPECT_FALSE(ncc::CoreLibrary.GetVersion().empty()); }
