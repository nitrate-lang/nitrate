#include <gtest/gtest.h>
#include <quixcc.hpp>
#include <stdio.h>
#include <openssl/sha.h>
#include <fstream>

TEST(libquixcc, library_init)
{
  ASSERT_TRUE(quixcc_init());
}

TEST(libquixcc, create_build_context)
{
  quixcc_job_t *job = quixcc_new();

  ASSERT_TRUE(job != nullptr);

  ASSERT_TRUE(quixcc_dispose(job));
}

static void sha1_file_ignstr(FILE *file, uint8_t sha1[20])
{
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  std::string file_contents;
  file_contents.resize(size);
  if (fread(&file_contents[0], 1, size, file) != size)
  {
    throw std::runtime_error("Failed to read file");
  }

  SHA_CTX ctx;
  SHA1_Init(&ctx);
  SHA1_Update(&ctx, file_contents.c_str(), file_contents.size());
  SHA1_Final(sha1, &ctx);
}

/* PYTHON_INSERT_POINT_SYNTHESIZE_TEST_SUITE */
