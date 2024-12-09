#include <gtest/gtest.h>

#define LIBNITRATE_INTERNAL
#include <nitrate/code.h>

TEST(LibInit, Manual) {
  ASSERT_EQ(nit_lib_ready, false);
  ASSERT_EQ(nit_lib_init(), true);
  ASSERT_EQ(nit_lib_ready, true);
  nit_deinit();
  ASSERT_EQ(nit_lib_ready, false);
}

TEST(LibInit, Auto) {
  ASSERT_EQ(nit_lib_ready, false);
  nit_stream_t *s = nit_from(stdin, false), *o = nit_from(stderr, false);
  ASSERT_EQ(nit_cc(s, o, nullptr, 0, nullptr), true);
  nit_fclose(s);
  ASSERT_EQ(nit_lib_ready, true);
  nit_deinit();
  ASSERT_EQ(nit_lib_ready, false);
}

TEST(LibInit, RefCount) {
  ASSERT_EQ(nit_lib_ready, false);
  nit_stream_t *s = nit_from(stdin, false), *o = nit_from(stderr, false);
  ASSERT_EQ(nit_cc(s, o, nullptr, 0, nullptr), true);
  nit_fclose(s);
  ASSERT_EQ(nit_lib_ready, true);

  ASSERT_EQ(nit_lib_init(), true);
  ASSERT_EQ(nit_lib_ready, true);

  nit_deinit();
  ASSERT_EQ(nit_lib_ready, true);
  nit_deinit();
  ASSERT_EQ(nit_lib_ready, false);
}
