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
  auto in_file = CFile(fmemopen((void*)text.c_str(), text.size(), "r"));
  ASSERT_NE(in_file.get(), nullptr);

  auto out_file = CFile(tmpfile());

  EXPECT_TRUE(nitrate::pipeline(in_file.get(), out_file.get(), {"echo"}).get());
  rewind(out_file);

  char buffer[sizeof(text)];
  auto n = fread(buffer, 1, sizeof(buffer), out_file);
  ASSERT_EQ(n, text.size());
  ASSERT_EQ(text, std::string(buffer, n));
}

TEST(nit_stream_t, ConcatStream) {
  const std::vector<std::string> texts = {"TextA", "TextB", "TextC", "TextD",
                                          "TextE"};
  std::string expected_text;

  std::vector<CFile> in_files;
  std::vector<FILE*> in_files_ptrs;
  for (const auto& text : texts) {
    auto file = fmemopen((void*)text.c_str(), text.size(), "r");
    ASSERT_NE(file, nullptr);
    in_files.push_back(CFile(file));
    in_files_ptrs.push_back(file);

    expected_text += text;
  }

  auto out_file = CFile(tmpfile());
  EXPECT_TRUE(nitrate::pipeline(in_files_ptrs, out_file.get(), {"echo"}).get());

  rewind(out_file);

  char buffer[1024];
  auto n = fread(buffer, 1, sizeof(buffer), out_file);
  ASSERT_EQ(n, expected_text.size());
  ASSERT_EQ(expected_text, std::string(buffer, n));
}
