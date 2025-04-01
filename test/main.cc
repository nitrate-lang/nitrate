#include <nitrate-core/Logger.hh>
#include <string>
#include <vector>

namespace nitrate::testing {
  bool RunTestSuite(const std::vector<std::string>& args);
}

int main(int argc, char** argv) {
  std::vector<std::string> args(argv, argv + argc);

  ncc::Log->Subscribe([](const ncc::LogMessage& msg) { std::cout << msg.m_by.Format(msg.m_message, msg.m_sev); });
  return nitrate::testing::RunTestSuite(args) ? 0 : 1;
}
