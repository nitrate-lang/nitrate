#include <gtest/gtest.h>

#include <cstdio>
#include <nitrate/code.hh>

class CFile {
public:
  CFile(FILE* file) : file_(file) {}
  CFile(CFile&& other) : file_(other.file_) { other.file_ = nullptr; }
  ~CFile() {
    if (file_) {
      fclose(file_);
    }
  }

  FILE* get() { return file_; }

  operator FILE*() { return file_; }

private:
  FILE* file_;
};

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
