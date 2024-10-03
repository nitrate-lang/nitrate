#include <gtest/gtest.h>
#include <quix/code.h>

#include <string_view>

TEST(LexerRoute, VectorA) {
  std::string_view source_code = "";
  std::string_view expected_output = "";

  FILE* source = fmemopen((void*)source_code.data(), source_code.size(), "r");
  ASSERT_NE(source, nullptr);

  char* output_buf = nullptr;
  size_t output_size = 0;
  FILE* output = open_memstream(&output_buf, &output_size);
  if (output == nullptr) {
    fclose(source);
    FAIL() << "Failed to open memory stream.";
  }

  const char* options[] = {
      "lex",        /* Lexer route */
      "-fuse-json", /* Output as JSON */
      NULL,         /* End of options */
  };

  if (!quix_cc(source, output, nullptr, 0, options)) {
    fclose(source);
    fclose(output);
    free(output_buf);
    quix_deinit();

    FAIL() << "Failed to run quix_cc.";
  }

  fclose(source);
  fclose(output);

  std::string_view output_code(output_buf, output_size);

  EXPECT_EQ(output_code, expected_output);

  free(output_buf);
  quix_deinit();
}
