#include <gtest/gtest.h>

#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-lexer/Lexer.hh>
#include <pipeline/libnitrate-lexer/LexicalCase.hh>

static const ncc::ECBase& LexerCaptureECImpl() {
  const ncc::ECBase* ec_ptr = nullptr;

  // we'll capture the error internal struct for later use
  auto subid = ncc::Log.Subscribe([&](const auto&, auto, const auto& ec) { ec_ptr = &ec; });

  LexString("0xffffffffffffffffffffffffffffffff_f");  // emit lexical error message
  ncc::Log.Unsubscribe(subid);

  if (ec_ptr == nullptr) {
    qcore_panic("Failed to capture error code");
  }

  // return reference to internal library object which has static lifetime

  return *ec_ptr;
}

#define TEST_ANSI_LOG(__LEVEL, __INPUT_STRING, __OUTPUT_STRING)                                                       \
  TEST(Lexer, Log_Ansi_##__LEVEL) {                                                                                   \
    using namespace ncc;                                                                                              \
    using namespace ncc::lex;                                                                                         \
                                                                                                                      \
    if (auto lib_rc = LexerLibrary.GetRC()) {                                                                         \
      std::string log_output;                                                                                         \
      auto subid =                                                                                                    \
          Log.Subscribe([&](const std::string& msg, auto sev, const auto& ec) { log_output = ec.Format(msg, sev); }); \
                                                                                                                      \
      Log << LexerCaptureECImpl() << __LEVEL << __INPUT_STRING;                                                       \
      Log.Unsubscribe(subid);                                                                                         \
                                                                                                                      \
      EXPECT_EQ(log_output, __OUTPUT_STRING);                                                                         \
    }                                                                                                                 \
  }

TEST_ANSI_LOG(Trace, "Message!", "\x1B[37;1m[\x1b[0m\x1B[34;1mLexer\x1b[0m\x1B[37;1m]: debug:\x1b[0m Message!")
TEST_ANSI_LOG(Debug, "Message!", "\x1B[37;1m[\x1b[0m\x1B[34;1mLexer\x1b[0m\x1B[37;1m]: debug:\x1b[0m Message!")
TEST_ANSI_LOG(Info, "Message!", "\x1B[37;1m[\x1b[0m\x1B[34;1mLexer\x1b[0m\x1B[37;1m]:\x1b[0m Message!")
TEST_ANSI_LOG(Notice, "Message!", "\x1B[37;1m[\x1b[0m\x1B[34;1mLexer\x1b[0m\x1B[37;1m]:\x1b[0m Message!")
TEST_ANSI_LOG(Warning, "Message!", "\x1B[37;1m[\x1b[0m\x1B[34;1mLexer\x1b[0m\x1B[37;1m]:\x1b[0m Message!")
TEST_ANSI_LOG(Error, "Message!", "\x1B[37;1m[\x1b[0m\x1B[34;1mLexer\x1b[0m\x1B[37;1m]:\x1b[0m Message!")
TEST_ANSI_LOG(Critical, "Message!", "\x1B[37;1m[\x1b[0m\x1B[34;1mLexer\x1b[0m\x1B[37;1m]:\x1b[0m Message!")
TEST_ANSI_LOG(Alert, "Message!", "\x1B[37;1m[\x1b[0m\x1B[34;1mLexer\x1b[0m\x1B[37;1m]:\x1b[0m Message!")
TEST_ANSI_LOG(Emergency, "Message!", "\x1B[37;1m[\x1b[0m\x1B[34;1mLexer\x1b[0m\x1B[37;1m]:\x1b[0m Message!")
TEST_ANSI_LOG(Raw, "Message!", "Message!")
