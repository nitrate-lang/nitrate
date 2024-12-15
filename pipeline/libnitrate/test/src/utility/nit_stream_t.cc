#include <gtest/gtest.h>

#include <nitrate/code.hh>

TEST(nit_stream_t, EchoContent) {
  const std::string text = "This is some example content";
  std::string output;

  EXPECT_TRUE(nitrate::pipeline(text, output, {"echo"}).get());

  ASSERT_EQ(text, output);
}
