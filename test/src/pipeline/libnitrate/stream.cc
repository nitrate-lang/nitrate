#include <gtest/gtest.h>

#include <cstdlib>
#include <initializer_list>
#include <nitrate/code.hh>

using namespace nitrate;

TEST(Stream, Echo) {
  const std::string text = "This is some example content";
  std::string output;

  EXPECT_TRUE(Pipeline(text, output, {"echo"}).Get());

  ASSERT_EQ(text + "\n", output);
}

TEST(Stream, ChainTransforms) {
  const std::string text = "This is some example content";
  std::string output;

  EXPECT_TRUE(Chain(text, output, {{"echo"}}).Get());

  ASSERT_EQ(text + "\n", output);
}
