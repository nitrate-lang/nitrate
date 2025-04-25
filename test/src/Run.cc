#include <gtest/gtest.h>

#include <nitrate-core/Environment.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>

namespace nitrate::testing {
  NCC_EXPORT bool RunTestSuite(const std::vector<std::string>& args) {
    if (!ncc::Environment().Contains("NCC_CONF")) {
      ncc::Log << ncc::Warning << "NCC_CONF environment variable is not set. Some tests may not run.";
      ncc::Log << ncc::Notice << "Set the NCC_CONF environment variable to the path of the application data directory.";
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
