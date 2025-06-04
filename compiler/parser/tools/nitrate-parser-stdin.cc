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

#define FMT_HEADER_ONLY
#include <spdlog/spdlog.h>

#include <fstream>
#include <iostream>
#include <nitrate-lexer/Lexer.hh>

using namespace nitrate::compiler::lexer;

auto main(int argc, char* argv[]) -> int {
  spdlog::enable_backtrace(32);

  std::vector<std::string> args(argv, argv + argc);
  if (args.size() < 2) {
    std::cerr << "Usage: " << args[0] << " <output_file>\n";
    return 1;
  }

  std::string output_file = args[1];
  auto os = std::fstream(output_file, std::ios::out | std::ios::trunc | std::ios::binary);
  if (!os.is_open()) {
    std::cerr << "Failed to open output file: " << output_file << "\n";
    return 1;
  }

  {
    auto lexer = Lexer(std::cin, boost::flyweight<std::string>("stdin"));

    // TODO: Parse using the lexer
  }

  return 0;
}
