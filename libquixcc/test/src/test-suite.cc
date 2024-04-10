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
  assert(fread(&file_contents[0], 1, size, file) == size);

  SHA_CTX ctx;
  SHA1_Init(&ctx);
  SHA1_Update(&ctx, file_contents.c_str(), file_contents.size());
  SHA1_Final(sha1, &ctx);
}

/* PYTHON_INSERT_POINT_SYNTHESIZE_TEST_SUITE */

TEST(libquixcc, Test_1) {
    ASSERT_TRUE(quixcc_init());
    const char* source_file = "/home/wesjo/Documents/Projects/quixcc/libquixcc/test/data/inputs/empty.q";
    const char* output_file = "/tmp/libquixcc_Test_1.tmp.out";
    uint8_t sha1[20];
    static const uint8_t expected[20] = {0xcd,0xb7,0x5f,0xd7,0xc8,0xb2,0xa3,0xc0,0xc1,0x45,0x27,0x96,0xd9,0xdf,0x02,0x65,0x98,0x72,0x73,0x32};
    FILE *output;
    if ((output = fopen(output_file, "w+")) == NULL) { FAIL() << "Could not open file: " << output_file; }
    ASSERT_TRUE(quixcc::CompilerBuilder().add_source(source_file).set_output(output).opt("-S").build().run().ok());
    sha1_file_ignstr(output,sha1);
    ASSERT_TRUE(memcmp(sha1, expected, 20) == 0);
    fclose(output);
    remove(output_file);
}
