
#include <gtest/gtest.h>

#include <algorithm>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <optional>

static const char* LogContent = "Hello, World!";

using namespace ncc;

struct LogOutput {
  std::string m_text;
  Sev m_level;
};

#define TEST_RAW_LOG(__LEVEL)                                                          \
  TEST(Core, Log_Mono_##__LEVEL) {                                                     \
    if (auto lib_rc = CoreLibrary.GetRC()) {                                           \
      Log.SuspendAll();                                                                \
      LogOutput log_output;                                                            \
      auto subid = Log.Subscribe([&](const std::string& msg, Sev sev, const ECBase&) { \
        log_output.m_text = msg;                                                       \
        log_output.m_level = sev;                                                      \
      });                                                                              \
                                                                                       \
      Log << __LEVEL << LogContent;                                                    \
                                                                                       \
      Log.Unsubscribe(subid);                                                          \
      Log.ResumeAll();                                                                 \
                                                                                       \
      ASSERT_EQ(log_output.m_text, LogContent);                                        \
      ASSERT_EQ(log_output.m_level, __LEVEL);                                          \
    }                                                                                  \
  }

#define TEST_ANSI_LOG(__LEVEL, __INPUT_STRING, __OUTPUT_STRING)                                                        \
  TEST(Core, Log_Ansi_##__LEVEL) {                                                                                     \
    if (auto lib_rc = CoreLibrary.GetRC()) {                                                                           \
      Log.SuspendAll();                                                                                                \
      std::string log_output;                                                                                          \
      auto subid =                                                                                                     \
          Log.Subscribe([&](const std::string& msg, Sev sev, const ECBase& ec) { log_output = ec.Format(msg, sev); }); \
                                                                                                                       \
      Log << __LEVEL << __INPUT_STRING;                                                                                \
      Log.Unsubscribe(subid);                                                                                          \
      Log.ResumeAll();                                                                                                 \
                                                                                                                       \
      EXPECT_EQ(log_output, __OUTPUT_STRING);                                                                          \
    }                                                                                                                  \
  }

TEST_RAW_LOG(Trace)
TEST_RAW_LOG(Debug)
TEST_RAW_LOG(Info)
TEST_RAW_LOG(Notice)
TEST_RAW_LOG(Warning)
TEST_RAW_LOG(Error)
TEST_RAW_LOG(Critical)
TEST_RAW_LOG(Alert)
TEST_RAW_LOG(Emergency)

TEST_ANSI_LOG(Trace, "Hello, World!", "\x1b[1mtrace:\x1b[0m \x1b[37;1mHello, World!\x1b[0m")
TEST_ANSI_LOG(Debug, "Hello, World!", "\x1b[1mdebug:\x1b[0m \x1b[37;1mHello, World!\x1b[0m")
TEST_ANSI_LOG(Info, "Hello, World!", "\x1b[37;1minfo:\x1b[0m \x1b[37;1mHello, World!\x1b[0m")
TEST_ANSI_LOG(Notice, "Hello, World!", "\x1b[37;1mnotice:\x1b[0m \x1b[37;1mHello, World!\x1b[0m")
TEST_ANSI_LOG(Warning, "Hello, World!", "\x1b[35;1mwarning:\x1b[0m \x1b[37;1mHello, World!\x1b[0m")
TEST_ANSI_LOG(Error, "Hello, World!", "\x1b[31;1merror:\x1b[0m \x1b[37;1mHello, World!\x1b[0m")
TEST_ANSI_LOG(Critical, "Hello, World!", "\x1b[31;1;4mcritical:\x1b[0m \x1b[37;1mHello, World!\x1b[0m")
TEST_ANSI_LOG(Alert, "Hello, World!", "\x1b[31;1;4malert:\x1b[0m \x1b[37;1mHello, World!\x1b[0m")
TEST_ANSI_LOG(Emergency, "Hello, World!", "\x1b[31;1;4memergency:\x1b[0m \x1b[37;1mHello, World!\x1b[0m")

TEST(Core, Log_Unsubscribe_Okay) {
  if (auto lib_rc = CoreLibrary.GetRC()) {
    Log.SuspendAll();
    LogOutput log_output;
    auto subid = Log.Subscribe([&](const std::string& msg, Sev sev, const ECBase&) {
      log_output.m_text = msg;
      log_output.m_level = sev;
    });

    Log << Info << LogContent;

    Log.Unsubscribe(subid);
    Log.ResumeAll();

    ASSERT_EQ(log_output.m_text, LogContent);
    ASSERT_EQ(log_output.m_level, Info);
  }
}

TEST(Core, Log_Ubsubscribe_Invalid) {
  if (auto lib_rc = CoreLibrary.GetRC()) {
    Log.SuspendAll();
    Log.Unsubscribe(6969);  // Invalid filter id

    LogOutput log_output;
    auto subid = Log.Subscribe([&](const std::string& msg, Sev sev, const ECBase&) {
      log_output.m_text = msg;
      log_output.m_level = sev;
    });

    Log << Info << LogContent;

    Log.Unsubscribe(subid);
    Log.ResumeAll();

    ASSERT_EQ(log_output.m_text, LogContent);
    ASSERT_EQ(log_output.m_level, Info);
  }
}

TEST(Core, Log_Unsubscribe_all) {
  if (auto lib_rc = CoreLibrary.GetRC()) {
    std::vector<LogOutput> log_outputs;
    auto sub_func = [&](const std::string& msg, Sev sev, const ECBase&) { log_outputs.push_back({msg, sev}); };

    Log.Subscribe(sub_func);
    auto subid_2 = Log.Subscribe(sub_func);
    Log.Subscribe(sub_func);
    Log.Unsubscribe(subid_2);
    Log.Subscribe(sub_func);
    Log.Subscribe(sub_func);
    Log.UnsubscribeAll();
    Log.Subscribe(sub_func);
    Log.Subscribe(sub_func);
    Log.Subscribe(sub_func);

    Log << Info << LogContent;

    ASSERT_EQ(log_outputs.size(), 3);

    for (const auto& log_output : log_outputs) {
      EXPECT_EQ(log_output.m_text, LogContent);
      EXPECT_EQ(log_output.m_level, Info);
    }

    Log.UnsubscribeAll();
  }
}

TEST(Core, Log_AddFilter) {
  if (auto lib_rc = CoreLibrary.GetRC()) {
    Log.ClearFilters();

    auto filter_id = Log.AddFilter([](const std::string&, Sev level, const ECBase&) { return level != Debug; });

    std::vector<LogOutput> log_outputs;
    auto subid =
        Log.Subscribe([&](const std::string& msg, Sev sev, const ECBase&) { log_outputs.push_back({msg, sev}); });

    Log << Warning << LogContent;
    ASSERT_EQ(log_outputs.size(), 1);
    EXPECT_EQ(log_outputs.back().m_text, LogContent);
    EXPECT_EQ(log_outputs.back().m_level, Warning);

    Log << Error << LogContent;
    ASSERT_EQ(log_outputs.size(), 2);
    EXPECT_EQ(log_outputs.back().m_text, LogContent);
    EXPECT_EQ(log_outputs.back().m_level, Error);

    Log << Debug << LogContent;
    ASSERT_EQ(log_outputs.size(), 2);
    EXPECT_EQ(log_outputs.back().m_text, LogContent);
    EXPECT_EQ(log_outputs.back().m_level, Error);

    Log.Unsubscribe(subid);
    Log.RemoveFilter(filter_id);
    Log.ClearFilters();
  }
}

TEST(Core, Log_RemoveFilter) {
  if (auto lib_rc = CoreLibrary.GetRC()) {
    Log.ClearFilters();

    Log.RemoveFilter(1111);  // Invalid filter id

    auto filter_id = Log.AddFilter([](const std::string&, Sev level, const ECBase&) { return level != Debug; });

    std::vector<LogOutput> log_outputs;
    auto subid =
        Log.Subscribe([&](const std::string& msg, Sev sev, const ECBase&) { log_outputs.push_back({msg, sev}); });

    Log << Warning << LogContent;
    ASSERT_EQ(log_outputs.size(), 1);
    EXPECT_EQ(log_outputs.back().m_text, LogContent);
    EXPECT_EQ(log_outputs.back().m_level, Warning);

    Log.RemoveFilter(6969);  // Invalid filter id

    Log << Error << LogContent;
    ASSERT_EQ(log_outputs.size(), 2);
    EXPECT_EQ(log_outputs.back().m_text, LogContent);
    EXPECT_EQ(log_outputs.back().m_level, Error);

    Log.RemoveFilter(3434);  // Invalid filter id

    Log << Debug << LogContent;
    ASSERT_EQ(log_outputs.size(), 2);
    EXPECT_EQ(log_outputs.back().m_text, LogContent);
    EXPECT_EQ(log_outputs.back().m_level, Error);

    Log.Unsubscribe(subid);
    Log.RemoveFilter(filter_id);
    Log.ClearFilters();
  }
}

NCC_EC_GROUP(Test_Core);
NCC_EC_EX(Test_Core, TestError, Formatter, "$NCC_CONF/ec/core/TestError")

TEST(Core, Log_EC_ToJson) {
  if (std::getenv  // NOLINT(concurrency-mt-unsafe)
      ("NCC_CONF") == nullptr) {
    qcore_panic(
        "NCC_CONF environment variable not set. Set it prior to running "
        "the tests.");
  }

  if (auto lib_rc = CoreLibrary.GetRC()) {
    std::string json_output;
    auto subid = Log.Subscribe([&](const std::string&, Sev, const ECBase& ec) { json_output = ec.AsJson(); });

    Log << TestError << Info << LogContent;

    Log.Unsubscribe(subid);

    ASSERT_EQ(
        json_output,
        R"({"flagname":"-Wcore-internal","nice_name":"Core library internal compiler diagnostics","details":"Diagnostic messages produced for reasons primarily of interest to compiler developers. These diagnostics are generally independent of the code under translation and are often useful internal compiler optimization.","tags":["nitrate-core","pipeline"],"fixes":["This is not a fix, just a not so useful message...","another fix message"],"examples":["When you run the test suite this error is emitted","Another example of how this error can be triggered"],"dev_notes":["Devs are gods, they don't need notes","Another note for the devs"],"user_notes":["Users are mere mortals, but they won't bother to read this anyway","Another note for the users"]})");
  }
}

TEST(Core, Log_EC_ParseECJson_01) {
  std::unordered_map<std::string, std::optional<ECDetails>> test_vectors = {
      /* Not JSON */
      {R"(not valid)", std::nullopt},

      /* Not an object */
      {R"([])", std::nullopt},

      /* Missing "flagname" field */
      {R"({ 
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Wrong type for "flagname" field */
      {R"({ 
            "flagname": 123,
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Missing "nice_name" field */
      {R"({ 
            "flagname": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Wrong type for "nice_name" field */
      {R"({ 
            "flagname": "",
            "nice_name": 123,
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Missing "details" field */
      {R"({ 
            "flagname": "",
            "nice_name": "",
            "tags": [],
            "fixes": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Wrong type for "details" field */
      {R"({ 
            "flagname": "",
            "nice_name": "",
            "details": 123,
            "tags": [],
            "fixes": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Missing "tags" field */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "fixes": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Wrong type for "tags" field */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": 123,
            "fixes": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Wrong type for "tags" field item */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [123],
            "fixes": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Missing "fixes" field */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Wrong type for "fixes" field */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": 123,
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Wrong type for "fixes" field item */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [123],
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Missing "examples" field */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Wrong type for "examples" field */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": 123,
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Wrong type for "examples" field item */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": [123],
            "dev_notes": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Missing "dev_notes" field */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": [],
            "user_notes": []
          })",
       std::nullopt},

      /* Wrong type for "dev_notes" field */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": [],
            "dev_notes": 123,
            "user_notes": []
          })",
       std::nullopt},

      /* Wrong type for "dev_notes" field item */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": [],
            "dev_notes": [123],
            "user_notes": []
          })",
       std::nullopt},

      /* Missing "user_notes" field */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": [],
            "dev_notes": []
          })",
       std::nullopt},

      /* Wrong type for "user_notes" field */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": 123
          })",
       std::nullopt},

      /* Wrong type for "user_notes" field item */
      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": [123]
          })",
       std::nullopt},

      {R"({
            "flagname": "",
            "nice_name": "",
            "details": "",
            "tags": [],
            "fixes": [],
            "examples": [],
            "dev_notes": [],
            "user_notes": []
          })",
       ECDetails()},

      {R"({
            "flagname": "abc",
            "nice_name": "def",
            "details": "ghi",
            "tags": ["jkl"],
            "fixes": ["mno"],
            "examples": ["pqr"],
            "dev_notes": ["stu"],
            "user_notes": ["vwx"]
          })",
       ECDetails{
           {"jkl"},
           {"mno"},
           {"pqr"},
           {"stu"},
           {"vwx"},
           "abc",
           "def",
           "ghi",
       }},
  };

  bool passed = std::all_of(test_vectors.begin(), test_vectors.end(), [](const auto& test_vector) {
    auto ec = ECBase::ParseJsonECConfig(test_vector.first);
    return ec == test_vector.second;
  });

  ASSERT_TRUE(passed);
}
