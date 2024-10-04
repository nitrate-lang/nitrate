#include <gtest/gtest.h>

#define LIBQUIX_INTERNAL
#include <quix/code.h>

TEST(LibInit, Manual) {
  ASSERT_EQ(quix_detail_is_init, false);
  ASSERT_EQ(quix_lib_init(), true);
  ASSERT_EQ(quix_detail_is_init, true);
  quix_deinit();
  ASSERT_EQ(quix_detail_is_init, false);
}

TEST(LibInit, Auto) {
  ASSERT_EQ(quix_detail_is_init, false);
  ASSERT_EQ(quix_cc(stdin, stderr, nullptr, 0, nullptr), true);
  ASSERT_EQ(quix_detail_is_init, true);
  quix_deinit();
  ASSERT_EQ(quix_detail_is_init, false);
}

TEST(LibInit, RefCount) {
  ASSERT_EQ(quix_detail_is_init, false);

  ASSERT_EQ(quix_cc(stdin, stderr, nullptr, 0, nullptr), true);
  ASSERT_EQ(quix_detail_is_init, true);

  ASSERT_EQ(quix_lib_init(), true);
  ASSERT_EQ(quix_detail_is_init, true);

  quix_deinit();
  ASSERT_EQ(quix_detail_is_init, true);
  quix_deinit();
  ASSERT_EQ(quix_detail_is_init, false);
}
