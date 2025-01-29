#include <gtest/gtest.h>

#include <nitrate-core/Cache.hh>
#include <nitrate-core/Init.hh>

static const ncc::ResourceKey KEY_A = {0x76, 0x03, 0x33, 0x49, 0x35, 0x3b, 0xdc,
                                       0xe0, 0x9e, 0xd7, 0x3e, 0xb7, 0x33, 0x41,
                                       0x74, 0x74, 0x48, 0xff, 0x7f, 0x3d};
static const ncc::ResourceKey KEY_B = {0xcb, 0xcb, 0xd4, 0x1e, 0x37, 0xaa, 0xae,
                                       0x73, 0x5d, 0x83, 0x22, 0x50, 0xc0, 0xb8,
                                       0x26, 0x42, 0x32, 0x21, 0x39, 0xb8};

static const std::string VALUE_A = "Hello, World!";
static const std::string VALUE_B = "Hello, Earth!";

static std::map<ncc::ResourceKey, std::string> Cache;

static bool HasImpl(const ncc::ResourceKey& key) { return Cache.contains(key); }

static bool ReadImpl(const ncc::ResourceKey& key, std::string& value) {
  if (auto it = Cache.find(key); it != Cache.end()) {
    value = it->second;
    return true;
  }

  return false;
}

static bool WriteImpl(const ncc::ResourceKey& key, const std::string& value) {
  Cache[key] = value;
  return true;
}

TEST(Core, Cache_Create) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    Cache.clear();
    ncc::GetCache().Bind(HasImpl, ReadImpl, WriteImpl);
  }
}

TEST(Core, Cache_Write) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    Cache.clear();
    ncc::GetCache().Bind(HasImpl, ReadImpl, WriteImpl);

    EXPECT_FALSE(Cache.contains(KEY_A));
    ncc::GetCache().Write(KEY_A, VALUE_A);
    EXPECT_TRUE(Cache.contains(KEY_A));

    EXPECT_FALSE(Cache.contains(KEY_B));
    ncc::GetCache().Write(KEY_B, VALUE_B);
    EXPECT_TRUE(Cache.contains(KEY_B));
  }
}

TEST(Core, Cache_Read) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    Cache.clear();
    ncc::GetCache().Bind(HasImpl, ReadImpl, WriteImpl);

    EXPECT_FALSE(Cache.contains(KEY_A));
    ncc::GetCache().Write(KEY_A, VALUE_A);
    EXPECT_TRUE(Cache.contains(KEY_A));

    EXPECT_FALSE(Cache.contains(KEY_B));
    ncc::GetCache().Write(KEY_B, VALUE_B);
    EXPECT_TRUE(Cache.contains(KEY_B));

    std::string value;
    EXPECT_TRUE(ncc::GetCache().Read(KEY_A, value));
    EXPECT_EQ(value, VALUE_A);

    EXPECT_TRUE(ncc::GetCache().Read(KEY_B, value));
    EXPECT_EQ(value, VALUE_B);
  }
}
