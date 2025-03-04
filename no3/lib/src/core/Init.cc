////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <git2/global.h>
#include <nitrate-emit/Lib.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/Init.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-parser/Init.hh>
#include <nitrate-seq/Init.hh>
#include <no3/Interpreter.hh>
#include <unordered_map>

using namespace no3;
using namespace no3::detail;

static std::atomic<size_t> RCInitializationContextCounter = 0;
static std::mutex RCInitializationContextMutex;

static bool PerformInitialize(std::ostream& log);
static void PerformDeinitialize();

NCC_EXPORT RCInitializationContext::RCInitializationContext(LibraryDeinitializationCallback on_deinit) noexcept
    : m_on_deinit(std::move(on_deinit)), m_active(true) {}

NCC_EXPORT RCInitializationContext::RCInitializationContext(const RCInitializationContext& o) noexcept
    : m_on_deinit(o.m_on_deinit), m_active(o.m_active) {
  if (m_active) {  // Dont increment RC for moved-from instances
    ++RCInitializationContextCounter;
  }
}

NCC_EXPORT RCInitializationContext::RCInitializationContext(RCInitializationContext&& o) noexcept
    : m_on_deinit(std::move(o.m_on_deinit)), m_active(o.m_active) {
  // RC count is preserved
  o.m_active = false;
}

NCC_EXPORT RCInitializationContext& RCInitializationContext::operator=(const RCInitializationContext& o) noexcept {
  m_active = o.m_active;
  m_on_deinit = o.m_on_deinit;
  if (m_active) {  // Dont increment RC for moved-from instances
    ++RCInitializationContextCounter;
  }

  return *this;
}

NCC_EXPORT RCInitializationContext& RCInitializationContext::operator=(RCInitializationContext&& o) noexcept {
  // RC count is preserved
  m_active = o.m_active;
  m_on_deinit = o.m_on_deinit;
  o.m_active = false;

  return *this;
}

NCC_EXPORT RCInitializationContext::~RCInitializationContext() noexcept {
  if (m_active && (--RCInitializationContextCounter == 0)) {
    if (m_on_deinit) {
      m_on_deinit();
    }

    PerformDeinitialize();
  }

  m_active = false;
}

class detail::No3LibraryInitialization {
public:
  static std::unique_ptr<RCInitializationContext> GetInitializationContext(
      std::ostream& init_log, LibraryDeinitializationCallback on_deinit) noexcept {
    if (++RCInitializationContextCounter == 1) {
      bool init_ok = PerformInitialize(init_log);
      if (!init_ok) {
        RCInitializationContextCounter = 0;
        return nullptr;
      }
    }

    return std::unique_ptr<RCInitializationContext>(new RCInitializationContext(std::move(on_deinit)));
  }
};

NCC_EXPORT std::unique_ptr<detail::RCInitializationContext> no3::OpenLibrary(
    std::ostream& init_log, detail::LibraryDeinitializationCallback on_deinit) noexcept {
  return No3LibraryInitialization::GetInitializationContext(init_log, std::move(on_deinit));
}

///===================================================================================================

ncc::Sev GetMinimumLogLevel() {
  static const std::unordered_map<std::string, ncc::Sev> map = {
      {"TRACE", ncc::Trace},         {"DEBUG", ncc::Debug}, {"INFO", ncc::Info},         {"NOTICE", ncc::Notice},
      {"WARNING", ncc::Warning},     {"ERROR", ncc::Error}, {"CRITICAL", ncc::Critical}, {"ALERT", ncc::Alert},
      {"EMERGENCY", ncc::Emergency}, {"RAW", ncc::Raw},
  };

  constexpr auto kDefaultLevel = ncc::Info;
  const char* env_val = std::getenv("NCC_LOG_LEVEL");  // NOLINT(concurrency-mt-unsafe)
  if (env_val == nullptr) {
    return kDefaultLevel;
  }

  std::string level(env_val);
  std::transform(level.begin(), level.end(), level.begin(), ::toupper);

  if (auto it = map.find(level); it != map.end()) {
    return it->second;
  }

  return kDefaultLevel;
}

static bool PerformInitialize(std::ostream& log) {
  using namespace ncc;

  std::lock_guard<std::mutex> lock(RCInitializationContextMutex);

  /* Initialize compiler pipeline libraries */
  if (!CoreLibrary.InitRC()) {
    log << "Failed to initialize libnitrate-core library" << std::endl;
    return false;
  }

  auto log_subid = Log.Subscribe([](auto msg, auto sev, const auto& ec) {
    using namespace ncc;

    if (sev < GetMinimumLogLevel()) {
      return;
    }

    std::cerr << ec.Format(msg, sev) << std::endl;
  });

  std::shared_ptr<void> unsub(nullptr, [&](...) { Log.Unsubscribe(log_subid); });

  if (!lex::LexerLibrary.InitRC()) {
    log << "Failed to initialize libnitrate-lexer library" << std::endl;
    return false;
  }

  if (!seq::SeqLibrary.InitRC()) {
    log << "Failed to initialize libnitrate-seq library" << std::endl;
    return false;
  }

  if (!parse::ParseLibrary.InitRC()) {
    log << "Failed to initialize libnitrate-parse library" << std::endl;
    return false;
  }

  if (!ir::IRLibrary.InitRC()) {
    log << "Failed to initialize libnitrate-ir library" << std::endl;
    return false;
  }

  if (!QcodeLibInit()) {
    log << "Failed to initialize libnitrate-emit library" << std::endl;
    return false;
  }

  if (git_libgit2_init() <= 0) {
    log << "Failed to initialize libgit2" << std::endl;
    return false;
  }

  Log << Debug << "Initialized Nitrate Toolchain";

  return true;
}

static void PerformDeinitialize() {
  using namespace ncc;

  std::lock_guard<std::mutex> lock(RCInitializationContextMutex);

  auto log_subid = Log.Subscribe([](auto msg, auto sev, const auto& ec) {
    using namespace ncc;

    if (sev < GetMinimumLogLevel()) {
      return;
    }

    std::cerr << ec.Format(msg, sev) << std::endl;
  });

  std::shared_ptr<void> unsub(nullptr, [&](...) { Log.Unsubscribe(log_subid); });

  git_libgit2_shutdown();

  QcodeLibDeinit();
  ir::IRLibrary.DeinitRC();
  parse::ParseLibrary.DeinitRC();
  seq::SeqLibrary.DeinitRC();
  lex::LexerLibrary.DeinitRC();
  CoreLibrary.DeinitRC();
}
