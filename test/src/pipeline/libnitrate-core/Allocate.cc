#include <gtest/gtest.h>

#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Init.hh>

TEST(Core, Arena_Create) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto arena = std::make_unique<ncc::DynamicArena>();
  }
}

TEST(Core, Arena_Allocate_DefaultAlignment) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto arena = std::make_unique<ncc::DynamicArena>();

    for (auto i = 0; i < 100'000; i++) {
      auto ptr = reinterpret_cast<uintptr_t>(arena->Alloc(i));
      ASSERT_NE(ptr, 0);
      EXPECT_EQ(ptr % 16, 0);
      memset(reinterpret_cast<void*>(ptr), 'A', i);
    }
  }
}

TEST(Core, Arena_Allocate_CustomAlignment) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto arena = std::make_unique<ncc::DynamicArena>();

    for (auto alignment = 1; alignment < 100; alignment++) {
      for (auto size = 0; size < 1000; size++) {
        auto ptr = reinterpret_cast<uintptr_t>(arena->Alloc(size, alignment));
        ASSERT_NE(ptr, 0);
        EXPECT_EQ(ptr % alignment, 0);
        memset(reinterpret_cast<void*>(ptr), 'A', size);
      }
    }
  }
}

TEST(Core, Arena_Allocate_Align_Zero) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto arena = std::make_unique<ncc::DynamicArena>();

    EXPECT_EQ(arena->Alloc(10, 0), nullptr);
  }
}

TEST(Core, Arena_Allocate_Large) {
  constexpr auto kSize = 10'000'000;

  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto arena = std::make_unique<ncc::DynamicArena>();

    auto ptr = reinterpret_cast<uintptr_t>(arena->Alloc(kSize));
    ASSERT_NE(ptr, 0);
    memset(reinterpret_cast<void*>(ptr), 'A', kSize);
  }
}

TEST(Core, Arena_Wierd_Alignment) {
  constexpr auto kSize = 2;
  constexpr auto kAlignment = 10'000'000;

  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto arena = std::make_unique<ncc::DynamicArena>();

    auto ptr = reinterpret_cast<uintptr_t>(arena->Alloc(kSize, kAlignment));
    ASSERT_NE(ptr, 0);
    EXPECT_EQ(ptr % kAlignment, 0);
    memset(reinterpret_cast<void*>(ptr), 'A', kSize);
  }
}
