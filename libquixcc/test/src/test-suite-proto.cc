#include <gtest/gtest.h>
#include <lexer-test/suite.h>
#include <openssl/sha.h>
#include <optimizer-test/suite.h>
#include <parser-test/suite.h>
#include <stdio.h>

#include <fstream>
#include <quixcc.hpp>

TEST(libquixcc, library_init) { ASSERT_TRUE(quixcc_init()); }

TEST(libquixcc, create_build_context) {
  quixcc_job_t *job = quixcc_new();

  ASSERT_TRUE(job != nullptr);

  ASSERT_TRUE(quixcc_dispose(job));
}

TEST(libquixcc_lexer, all_tests) {
  ASSERT_TRUE(libquixcc::test::lexer_test_suite());
}

TEST(libquixcc_parser, all_tests) {
  ASSERT_TRUE(libquixcc::test::parser_test_suite());
}

TEST(libquixcc_optimizer, all_tests) {
  ASSERT_TRUE(libquixcc::test::optimizer_test_suite());
}

static void sha1_file_ignstr(FILE *file, uint8_t sha1[20]) {
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  std::string file_contents;
  file_contents.resize(size);
  if (fread(&file_contents[0], 1, size, file) != size) {
    throw std::runtime_error("Failed to read file");
  }

  SHA_CTX ctx;
  SHA1_Init(&ctx);
  SHA1_Update(&ctx, file_contents.c_str(), file_contents.size());
  SHA1_Final(sha1, &ctx);
}

/* PYTHON_INSERT_POINT_SYNTHESIZE_TEST_SUITE */
