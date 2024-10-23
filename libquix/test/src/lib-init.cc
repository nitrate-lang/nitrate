#include <gtest/gtest.h>

#define LIBQUIX_INTERNAL
#include <quix/code.h>

TEST(LibInit, Manual) {
  ASSERT_EQ(quix_lib_ready, false);
  ASSERT_EQ(quix_lib_init(), true);
  ASSERT_EQ(quix_lib_ready, true);
  quix_deinit();
  ASSERT_EQ(quix_lib_ready, false);
}

TEST(LibInit, Auto) {
  ASSERT_EQ(quix_lib_ready, false);
  ASSERT_EQ(quix_cc(stdin, stderr, nullptr, 0, nullptr), true);
  ASSERT_EQ(quix_lib_ready, true);
  quix_deinit();
  ASSERT_EQ(quix_lib_ready, false);
}

TEST(LibInit, RefCount) {
  ASSERT_EQ(quix_lib_ready, false);

  ASSERT_EQ(quix_cc(stdin, stderr, nullptr, 0, nullptr), true);
  ASSERT_EQ(quix_lib_ready, true);

  ASSERT_EQ(quix_lib_init(), true);
  ASSERT_EQ(quix_lib_ready, true);

  quix_deinit();
  ASSERT_EQ(quix_lib_ready, true);
  quix_deinit();
  ASSERT_EQ(quix_lib_ready, false);
}
