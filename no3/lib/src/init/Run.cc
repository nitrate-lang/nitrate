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

#include <core/InterpreterImpl.hh>
#include <core/argparse.hpp>
#include <nitrate-core/LogOStream.hh>

using namespace ncc;

static void SetPackageInitCategoryOptions(argparse::ArgumentParser& program) {
  auto& exclusion = program.AddMutuallyExclusiveGroup(true);

  exclusion.AddArgument("--lib")
      .Help("Initialize a new Nitrate library project.")
      .ImplicitValue(true)
      .DefaultValue(false);

  exclusion.AddArgument("--exe")
      .Help("Initialize a new Nitrate executable project.")
      .ImplicitValue(true)
      .DefaultValue(false);

  exclusion.AddArgument("--comptime")
      .Help("Initialize a new Nitrate comptime project.")
      .ImplicitValue(true)
      .DefaultValue(false);
}

static std::unique_ptr<argparse::ArgumentParser> CreateArgumentParser(bool& did_default) {
  auto program = std::make_unique<argparse::ArgumentParser>(ncc::clog, did_default, "impl", "1.0",
                                                            argparse::default_arguments::help);

  SetPackageInitCategoryOptions(*program);

  program->AddArgument("package-name").Help("The name of the package to initialize.").Nargs(1);

  return program;
}

bool no3::Interpreter::PImpl::CommandInit(ConstArguments, const MutArguments& argv) {
  bool did_default = false;
  auto program = CreateArgumentParser(did_default);

  try {
    program->ParseArgs(argv);
  } catch (const std::runtime_error& e) {
    if (did_default) {
      return true;
    }

    Log << e.what();
    return false;
  }

  if (did_default) {
    return true;
  }

  /// TODO: Implement package initialization

  Log << "Package initialization is not implemented yet.";

  return true;
}
