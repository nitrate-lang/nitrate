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

#pragma once

#include <core/termcolor.hh>
#include <list>
#include <memory>
#include <nitrate-core/Logger.hh>
#include <no3/Interpreter.hh>

class no3::Interpreter::PImpl {
  using ConstArguments = std::span<const std::string_view>;
  using MutArguments = std::list<std::string_view>;
  using CommandFunction = std::function<bool(ConstArguments full_argv, MutArguments argv)>;

  std::unique_ptr<detail::RCInitializationContext> m_init_rc = OpenLibrary();
  std::unordered_map<std::string_view, CommandFunction> m_commands;

  static bool CommandBuild(ConstArguments full_argv, MutArguments argv);
  static bool CommandClean(ConstArguments full_argv, MutArguments argv);
  static bool CommandImpl(ConstArguments full_argv, MutArguments argv);
  static bool CommandDoc(ConstArguments full_argv, MutArguments argv);
  static bool CommandFormat(ConstArguments full_argv, MutArguments argv);
  static bool CommandHelp(ConstArguments full_argv, const MutArguments& argv);
  static bool CommandInit(ConstArguments full_argv, MutArguments argv);
  static bool CommandInstall(ConstArguments full_argv, MutArguments argv);
  static bool CommandFind(ConstArguments full_argv, MutArguments argv);
  static bool CommandRemove(ConstArguments full_argv, MutArguments argv);
  static bool CommandLSP(ConstArguments full_argv, const MutArguments& argv);
  static bool CommandLicense(ConstArguments full_argv, const MutArguments& argv);
  static bool CommandTest(ConstArguments full_argv, MutArguments argv);
  static bool CommandVersion(ConstArguments full_argv, const MutArguments& argv);
  static bool CommandUpdate(ConstArguments full_argv, MutArguments argv);

  void SetupCommands();

public:
  PImpl() noexcept { SetupCommands(); }

  bool Perform(const std::vector<std::string_view>& command);
};
