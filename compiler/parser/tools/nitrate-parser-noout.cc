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
  std::vector<std::string> args(argv, argv + argc);
  if (args.size() < 2) {
    std::cerr << "Usage: " << args[0] << " <input_file>\n";
    return 1;
  }

  std::string input_file = args[1];
  auto is = std::ifstream(input_file, std::ios::in | std::ios::binary);
  if (!is.is_open()) {
    std::cerr << "Failed to open input file: " << input_file << "\n";
    return 1;
  }

  auto lexer = Lexer(is, boost::flyweight<std::string>(input_file));

  // TODO: Parse using the lexer

  return 0;
}
