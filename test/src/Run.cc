#include <gtest/gtest.h>

#include <nitrate-core/Macro.hh>

#include "nitrate-core/Logger.hh"

namespace nitrate::testing {
  NCC_EXPORT bool RunTestSuite(const std::vector<std::string>& args) {
    if (getenv("NCC_CONF") == nullptr) {  // NOLINT(concurrency-mt-unsafe)
      ncc::Log << "NCC_CONF environment variable not set. Please set it to the path application data directory.";
      return false;
    }

    auto args_copy = args;
    std::vector<char*> argv(args_copy.size() + 1);
    argv[args_copy.size()] = nullptr;

    for (size_t i = 0; i < args_copy.size(); ++i) {
      argv[i] = args_copy[i].data();
    }

    int argc = args_copy.size();
    ::testing::InitGoogleTest(&argc, argv.data());
    return RUN_ALL_TESTS() == 0;
  }
}  // namespace nitrate::testing
