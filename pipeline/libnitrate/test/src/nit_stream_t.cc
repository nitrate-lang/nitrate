#include <gtest/gtest.h>

#include <cstdio>
#include <nitrate/code.hh>

TEST(nit_stream_t, OpenClose) {
  std::string content = "This is some example content";
  FILE* content_file = fmemopen((void*)content.c_str(), content.size(), "r");

  nit_stream_t* ss = nit_from(content_file, false);
  if (ss == nullptr) { /* 'nit_from' returns nullptr on failure */
    if (content_file) fclose(content_file);
    SUCCEED();
  }

  nit_fclose(ss);
  fclose(content_file);
}
