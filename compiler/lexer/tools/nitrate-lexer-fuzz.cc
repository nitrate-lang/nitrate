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
////////////////////////////////////////////////////////////////////////////////

#include <boost/flyweight/flyweight_fwd.hpp>
#define FMT_HEADER_ONLY
#include <spdlog/spdlog.h>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <nitrate-lexer/Lexer.hh>

using namespace nitrate::compiler::lexer;

static const auto LEXER_FILENAME = boost::flyweight<std::string>("stdin");

extern "C" auto LLVMFuzzerInitialize(int *, char ***) -> int {  // NOLINT(readability-identifier-naming)
  // This function is called once at the start of the fuzzer.
  // It can be used to initialize global state, set up logging, etc.
  // Here we disable logging for the fuzzer.

  spdlog::set_level(spdlog::level::off);

  return 0;  // Return 0 to indicate successful initialization.
}

extern "C" auto LLVMFuzzerTestOneInput(const uint8_t *data,  // NOLINT(readability-identifier-naming)
                                       size_t size) -> int {
  auto input = boost::iostreams::stream<boost::iostreams::array_source>(reinterpret_cast<const char *>(data), size);
  auto lexer = Lexer(input, LEXER_FILENAME);

  while (lexer.next_token()) {
  }

  return 0;  // Values other than 0 and -1 are reserved for future use.
}
