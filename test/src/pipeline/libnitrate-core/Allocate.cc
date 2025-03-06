#include <gtest/gtest.h>

#include <nitrate-core/Allocate.hh>
#include <nitrate-core/Init.hh>

#pragma GCC diagnostic ignored "-Wnon-power-of-two-alignment"

TEST(Core, Arena_Create) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto arena = std::make_unique<ncc::DynamicArena>();
  }
}

TEST(Core, Arena_Allocate_DefaultAlignment) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto arena = std::make_unique<ncc::DynamicArena>();

    for (auto i = 0; i < 100'000; i++) {
      auto ptr = reinterpret_cast<uintptr_t>(arena->allocate(i));
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
        auto ptr = reinterpret_cast<uintptr_t>(arena->allocate(size, alignment));
        ASSERT_NE(ptr, 0);
        EXPECT_EQ(ptr % alignment, 0);
        memset(reinterpret_cast<void*>(ptr), 'A', size);
      }
    }
  }
}

TEST(Core, Arena_Allocate_Large) {
  constexpr auto kSize = 10'000'000;

  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto arena = std::make_unique<ncc::DynamicArena>();

    auto ptr = reinterpret_cast<uintptr_t>(arena->allocate(kSize));
    ASSERT_NE(ptr, 0);
    memset(reinterpret_cast<void*>(ptr), 'A', kSize);
  }
}

TEST(Core, Arena_Wierd_Alignment) {
  constexpr auto kSize = 2;
  constexpr auto kAlignment = 10'000'000;

  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto arena = std::make_unique<ncc::DynamicArena>();

    auto ptr = reinterpret_cast<uintptr_t>(arena->allocate(kSize, kAlignment));
    ASSERT_NE(ptr, 0);
    EXPECT_EQ(ptr % kAlignment, 0);
    memset(reinterpret_cast<void*>(ptr), 'A', kSize);
  }
}

TEST(Core, Arena_MemoryUsed) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    auto arena = std::make_unique<ncc::DynamicArena>();

    for (auto i = 0; i < 10000; i++) {
      auto ptr = reinterpret_cast<uintptr_t>(arena->allocate(4, 1));
      ASSERT_NE(ptr, 0);
      memset(reinterpret_cast<void*>(ptr), i & 0xff, 4);
    }

    EXPECT_EQ(arena->GetSpaceUsed(), 40000);
    auto all_used = arena->GetSpaceManaged();

    arena->Reset();

    EXPECT_EQ(arena->GetSpaceUsed(), 0);
    EXPECT_LT(arena->GetSpaceManaged(), all_used);
  }
}
