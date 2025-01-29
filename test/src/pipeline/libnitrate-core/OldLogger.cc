#include <gtest/gtest.h>

#include <nitrate-core/Init.hh>
#include <nitrate-core/OldLogger.hh>

static const char* LogContent = "Hello, World!";

TEST(Core, OldLogger_Debug) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    qcore_logf(QCORE_DEBUG, LogContent);
  }
}

TEST(Core, OldLogger_Info) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    qcore_logf(QCORE_INFO, LogContent);
  }
}

TEST(Core, OldLogger_Warn) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    qcore_logf(QCORE_WARN, LogContent);
  }
}

TEST(Core, OldLogger_Error) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    qcore_logf(QCORE_ERROR, LogContent);
  }
}

TEST(Core, OldLogger_Fatal) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    qcore_logf(QCORE_FATAL, LogContent);
  }
}

TEST(Core, OldLogger_RemoveNewLine) {
  if (auto lib_rc = ncc::CoreLibrary.GetRC()) {
    qcore_logf(QCORE_INFO, "Hello, World!\n\n");
  }
}
