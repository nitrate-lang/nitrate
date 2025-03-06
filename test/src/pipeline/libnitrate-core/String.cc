#include <gtest/gtest.h>

#include <nitrate-core/Init.hh>
#include <nitrate-core/SmartLock.hh>
#include <nitrate-core/String.hh>

// Some not-empty string content.
static const char* StringContent = "Hello, World!";

TEST(Core, String_Construct_From_Raw_CString) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    ncc::string str_a(StringContent);
    EXPECT_EQ(str_a.Get(), std::string_view(StringContent));

    ncc::string str_b("");
    EXPECT_TRUE(str_b.Get().empty());
  }
}

TEST(Core, String_Construct_From_StdStringView) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    ncc::string str_a = std::string_view(StringContent);
    EXPECT_EQ(str_a.Get(), std::string_view(StringContent));

    ncc::string str_b = std::string_view();
    EXPECT_TRUE(str_b.Get().empty());
  }
}

TEST(Core, String_Construct_From_StdString) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    ncc::string str_a = std::string(StringContent);
    EXPECT_EQ(str_a.Get(), std::string_view(StringContent));

    ncc::string str_b = std::string();
    EXPECT_TRUE(str_b.Get().empty());
  }
}

TEST(Core, String_Construct_Moved_StdString) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto input_str = std::string(StringContent);
    ncc::string str_a(std::move(input_str));
    EXPECT_EQ(str_a.Get(), std::string_view(StringContent));

    input_str.clear();
    ncc::string str_b = std::move(input_str);
    EXPECT_TRUE(str_b.Get().empty());
  }
}

TEST(Core, String_Eq) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    EXPECT_EQ(ncc::string(StringContent), ncc::string(StringContent));
  }
}

TEST(Core, String_Ne) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    EXPECT_NE(ncc::string(StringContent), ncc::string(""));
  }
}

TEST(Core, String_Lt) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    EXPECT_LT(ncc::string(""), ncc::string(StringContent));
  }
}

TEST(Core, String_Le) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    EXPECT_LE(ncc::string(""), ncc::string(StringContent));
    EXPECT_LE(ncc::string(StringContent), ncc::string(StringContent));
  }
}

TEST(Core, String_Gt) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    EXPECT_GT(ncc::string(StringContent), ncc::string(""));
  }
}

TEST(Core, String_Ge) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    EXPECT_GE(ncc::string(StringContent), ncc::string(""));
    EXPECT_GE(ncc::string(StringContent), ncc::string(StringContent));
  }
}

static std::string HexEncode(uint64_t id) {
  std::array<char, 17> buffer;
  snprintf(buffer.data(), buffer.size(), "%016lX", id);
  return buffer.data();
}

TEST(Core, String_NoSync_Compare) {
  bool old = ncc::EnableSync;
  ncc::EnableSync = false;

  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    const auto count = 10'000;

    for (auto i = 0; i < count; ++i) {
      ncc::string a = HexEncode(i);
      ncc::string b = HexEncode(i + 1);

      EXPECT_EQ(a, a);
      EXPECT_EQ(b, b);
      EXPECT_NE(a, b);
      EXPECT_LT(a, b);
      EXPECT_LE(a, b);
      EXPECT_GT(b, a);
      EXPECT_GE(b, a);
    }
  }

  ncc::EnableSync = old;
}

TEST(Core, String_Sync_Compare) {
  bool old = ncc::EnableSync;
  ncc::EnableSync = true;

  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    const auto count = 10'000;

    for (auto i = 0; i < count; ++i) {
      ncc::string a = HexEncode(i);
      ncc::string b = HexEncode(i + 1);

      EXPECT_EQ(a, a);
      EXPECT_EQ(b, b);
      EXPECT_NE(a, b);
      EXPECT_LT(a, b);
      EXPECT_LE(a, b);
      EXPECT_GT(b, a);
      EXPECT_GE(b, a);
    }
  }

  ncc::EnableSync = old;
}
