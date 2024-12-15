#include <gtest/gtest.h>

#include <nitrate/code.hh>

TEST(nit_stream_t, EchoContent) {
  const std::string text = "This is some example content";
  std::string output;

  EXPECT_TRUE(nitrate::pipeline(text, output, {"echo"}).get());

  ASSERT_EQ(text, output);
}

TEST(nit_stream_t, ConcatStream) {
  const std::vector<std::string_view> texts = {"TextA", "TextB", "TextC",
                                               "TextD", "TextE"};
  std::string expected_text = "TextATextBTextCTextDTextE";

  std::string output;
  EXPECT_TRUE(nitrate::pipeline(std::span(texts), output, {"echo"}).get());

  ASSERT_EQ(expected_text, output);
}
