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

#include <descent/Recurse.hh>
#include <fstream>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-parser/Package.hh>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

auto GeneralParser::PImpl::RecurseImportName() -> std::pair<string, ImportMode> {
  if (NextIf<PuncLPar>()) {
    auto arguments = RecurseCallArguments({Token(Punc, PuncRPar)}, false);

    if (!NextIf<PuncRPar>()) {
      Log << SyntaxError << Current() << "Expected ')' to close the import call";
    }

    string import_name;
    std::optional<ImportMode> import_mode;

    for (const auto &arg : arguments) {
      if (arg.first == "src" || arg.first == "0") {
        if (import_name) {
          Log << SyntaxError << Current() << "Duplicate argument: 'src' in call to import";
        }

        auto pvalue = arg.second;
        if (!pvalue->Is(QAST_STRING)) {
          Log << SyntaxError << Current() << "Expected string literal for import source";
          continue;
        }

        import_name = pvalue->As<String>()->GetValue();
        continue;
      }

      if (arg.first == "mode" || arg.first == "1") {
        if (import_mode) {
          Log << SyntaxError << Current() << "Duplicate argument: 'mode' in call to import";
        }

        auto pvalue = arg.second;
        if (!pvalue->Is(QAST_STRING)) {
          Log << SyntaxError << Current() << "Expected string literal for import mode";
          continue;
        }

        auto mode = pvalue->As<String>()->GetValue();
        if (mode == "code") {
          import_mode = ImportMode::Code;
        } else if (mode == "string") {
          import_mode = ImportMode::String;
        } else if (mode == "raw") {
          import_mode = ImportMode::Raw;
        } else {
          Log << SyntaxError << Current() << "Invalid import mode: " << mode;
        }

        continue;
      }

      Log << SyntaxError << Current() << "Unexpected argument: " << arg.first;
    }

    if (!import_name) [[unlikely]] {
      Log << SyntaxError << Current() << "parameter 'src': missing positional argument 0 in call to import";
    }

    return {import_name, import_mode.value_or(ImportMode::Code)};
  }

  if (auto tok = NextIf<Text>()) {
    return {tok->GetString(), ImportMode::Code};
  }

  auto name = RecurseName();
  if (!name) [[unlikely]] {
    Log << SyntaxError << Current() << "Expected import name";
  }

  return {name, ImportMode::Code};
}

[[nodiscard]] auto GeneralParser::PImpl::RecurseImportRegularFile(string import_file,
                                                                  ImportMode import_mode) -> FlowPtr<Expr> {
  const auto exists = OMNI_CATCH(std::filesystem::exists(*import_file));
  if (!exists) {
    Log << SyntaxError << Current() << "Could not check if file exists: " << *import_file;
    return m_fac.CreateMockInstance<Import>();
  }

  if (!*exists) {
    Log << SyntaxError << Current() << "File not found: " << *import_file;
    return m_fac.CreateMockInstance<Import>();
  }

  std::ifstream file(*import_file, std::ios::binary);
  if (!file.is_open()) {
    Log << SyntaxError << Current() << "Failed to open file: " << *import_file;
    return m_fac.CreateMockInstance<Import>();
  }

  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  if (content.empty()) {
    Log << SyntaxError << Warning << Current() << "File is empty: " << *import_file;
    return m_fac.CreateMockInstance<Import>();
  }

  /// TODO: Implement regular file import
  (void)import_file;

  switch (import_mode) {
    case ImportMode::Code: {
      /// TODO: Implement code import
      break;
    }

    case ImportMode::String: {
      /// TODO: Implement string import
      break;
    }

    case ImportMode::Raw: {
      /// TODO: Implement raw import
      break;
    }
  }

  return m_fac.CreateMockInstance<Import>();
}

[[nodiscard]] auto GeneralParser::PImpl::RecurseImportPackage(string import_name,
                                                              ImportMode import_mode) -> FlowPtr<Expr> {
  /// TODO: FIXME: Implement scanner elsewhere; Remove this line
  static const thread_local auto pkgs = FindPackages({"/tmp/test"});

  auto pkg_it =
      std::find_if(pkgs.begin(), pkgs.end(), [&](const auto &pkg) { return pkg.PackageName() == import_name; });

  if (pkg_it == pkgs.end()) [[unlikely]] {
    Log << SyntaxError << Current() << "Package not found: " << import_name;
    return m_fac.CreateMockInstance<Import>();
  }

  const auto &files = pkg_it->Read();
  if (!files.has_value()) [[unlikely]] {
    Log << SyntaxError << Current() << "Failed to read package: " << import_name;
    return m_fac.CreateMockInstance<Import>();
  }

  Log << Debug << "RecurseImport: Got package: " << import_name;

  for (const auto &[path, content_getter] : files.value()) {
    const auto &content = content_getter.Get();
    if (!content.has_value()) [[unlikely]] {
      Log << Error << "RecurseImport: Failed to read package chunk: " << path;
      return m_fac.CreateMockInstance<Import>();
    }

    Log << Trace << "RecurseImport: Got package chunk (" << content.value().size() << " bytes): " << path;
  }

  switch (import_mode) {
    case ImportMode::Code: {
      /// TODO: Implement code import
      break;
    }

    case ImportMode::String: {
      /// TODO: Implement string import
      break;
    }

    case ImportMode::Raw: {
      /// TODO: Implement raw import
      break;
    }
  }

  return m_fac.CreateMockInstance<Import>();
}

auto GeneralParser::PImpl::RecurseImport() -> FlowPtr<Expr> {
  const auto [import_name, import_mode] = RecurseImportName();

  const bool is_regular_file =
      import_name->find("/") == std::string::npos || import_name->find("\\") == std::string::npos;

  if (is_regular_file) {
    return RecurseImportRegularFile(import_name, import_mode);
  }

  return RecurseImportPackage(import_name, import_mode);
}
