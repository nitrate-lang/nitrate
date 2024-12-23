#include <gtest/gtest.h>

#include <cstdlib>
#include <initializer_list>
#include <nitrate/code.hh>

TEST(Stream, Echo) {
  const std::string text = "This is some example content";
  std::string output;

  EXPECT_TRUE(nitrate::pipeline(text, output, {"echo"}).get());

  ASSERT_EQ(text + "\n", output);
}

TEST(Stream, ChainTransforms) {
  const std::string text = "This is some example content";
  std::string output;

  EXPECT_TRUE(nitrate::chain(text, output, {{"echo"}}).get());

  ASSERT_EQ(text + "\n", output);
}

TEST(Stream, ChainLexParse) {
  const std::string text = "let x = 0;";
  std::string output;

  EXPECT_TRUE(nitrate::chain(text, output, {{"lex"}, {"parse"}}).get());

  ASSERT_EQ(
      output,
      R"({"kind":"Block","loc":null,"safe":null,"body":[{"kind":"Let","loc":null,"mode":"let","name":"x","type":null,"value":{"kind":"Int","loc":null,"value":"0"},"attributes":[]}]})");
}

TEST(Stream, ChainOperations) {
  const std::string text = "let x = 0;";
  std::string output;

  EXPECT_TRUE(nitrate::chain(
                  text, output,
                  {{"lex"}, {"echo"}, {"echo"}, {"parse"}, {"echo"}, {"echo"}})
                  .get());

  ASSERT_EQ(
      output,
      R"({"kind":"Block","loc":null,"safe":null,"body":[{"kind":"Let","loc":null,"mode":"let","name":"x","type":null,"value":{"kind":"Int","loc":null,"value":"0"},"attributes":[]}]}

)");
}

TEST(Stream, ChainOperations2) {
  const std::string text = "let x = 0;";
  std::string output;

  EXPECT_TRUE(nitrate::chain(text, output,
                             {{"lex"}, {"echo"}, {"parse"}, {"echo"}, {"echo"}})
                  .get());

  ASSERT_EQ(
      output,
      R"({"kind":"Block","loc":null,"safe":null,"body":[{"kind":"Let","loc":null,"mode":"let","name":"x","type":null,"value":{"kind":"Int","loc":null,"value":"0"},"attributes":[]}]}

)");
}
