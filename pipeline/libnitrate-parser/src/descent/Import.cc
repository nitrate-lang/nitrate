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
#include <memory>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-lexer/Lexer.hh>
#include <nitrate-parser/Package.hh>

#include "nitrate-lexer/Scanner.hh"

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

auto GeneralParser::PImpl::RecurseImportName() -> std::pair<string, ImportMode> {
  if (NextIf<PuncLPar>()) {
    auto arguments = RecurseCallArguments({Token(Punc, PuncRPar)}, false);

    if (!NextIf<PuncRPar>()) {
      Log << ParserSignal << Current() << "Expected ')' to close the import call";
    }

    string import_name;
    std::optional<ImportMode> import_mode;

    for (const auto &arg : arguments) {
      if (arg.first == "src" || arg.first == "0") {
        if (import_name) {
          Log << ParserSignal << Current() << "Duplicate argument: 'src' in call to import";
        }

        auto pvalue = arg.second;
        if (!pvalue->Is(QAST_STRING)) {
          Log << ParserSignal << Current() << "Expected string literal for import source";
          continue;
        }

        import_name = pvalue->As<String>()->GetValue();
        continue;
      }

      if (arg.first == "mode" || arg.first == "1") {
        if (import_mode) {
          Log << ParserSignal << Current() << "Duplicate argument: 'mode' in call to import";
        }

        auto pvalue = arg.second;
        if (!pvalue->Is(QAST_STRING)) {
          Log << ParserSignal << Current() << "Expected string literal for import mode";
          continue;
        }

        auto mode = pvalue->As<String>()->GetValue();
        if (mode == "code") {
          import_mode = ImportMode::Code;
        } else if (mode == "string") {
          import_mode = ImportMode::String;
        } else {
          Log << ParserSignal << Current() << "Invalid import mode: " << mode;
        }

        continue;
      }

      Log << ParserSignal << Current() << "Unexpected argument: " << arg.first;
    }

    if (!import_name) [[unlikely]] {
      Log << ParserSignal << Current() << "parameter 'src': missing positional argument 0 in call to import";
    }

    return {import_name, import_mode.value_or(ImportMode::Code)};
  }

  if (auto tok = NextIf<Text>()) {
    return {tok->GetString(), ImportMode::Code};
  }

  auto name = RecurseName();
  if (!name) [[unlikely]] {
    Log << ParserSignal << Current() << "Expected import name";
  }

  return {name, ImportMode::Code};
}

[[nodiscard]] auto GeneralParser::PImpl::RecurseImportRegularFile(string import_file,
                                                                  ImportMode import_mode) -> FlowPtr<Expr> {
  auto abs_import_path = OMNI_CATCH(std::filesystem::absolute(*import_file)).value_or(*import_file).lexically_normal();

  Log << Trace << "RecurseImport: Importing regular file: " << abs_import_path;

  const auto exists = OMNI_CATCH(std::filesystem::exists(abs_import_path));
  if (!exists) {
    Log << ParserSignal << Current() << "Could not check if file exists: " << abs_import_path;
    return m_fac.CreateMockInstance<Import>();
  }

  if (!*exists) {
    Log << ParserSignal << Current() << "File not found: " << abs_import_path;
    return m_fac.CreateMockInstance<Import>();
  }

  auto is_regular_file = OMNI_CATCH(std::filesystem::is_regular_file(abs_import_path));
  if (!is_regular_file) {
    Log << ParserSignal << Current() << "Could not check if file is regular: " << abs_import_path;
    return m_fac.CreateMockInstance<Import>();
  }

  if (!*is_regular_file) {
    Log << ParserSignal << Current() << "File is not regular: " << abs_import_path;
    return m_fac.CreateMockInstance<Import>();
  }

  Log << Trace << "RecurseImport: Reading regular file: " << abs_import_path;

  std::ifstream file(abs_import_path, std::ios::binary);
  if (!file.is_open()) {
    Log << ParserSignal << Current() << "Failed to open file: " << abs_import_path;
    return m_fac.CreateMockInstance<Import>();
  }

  auto content = OMNI_CATCH(std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()));
  if (!content.has_value()) {
    Log << ParserSignal << Error << Current() << "Failed to read file: " << abs_import_path;
    return m_fac.CreateMockInstance<Import>();
  }

  if (content->empty()) {
    Log << ParserSignal << Warning << Current() << "File is empty: " << abs_import_path;
  }

  Log << Trace << "RecurseImport: Got file (" << content.value().size() << " bytes): " << abs_import_path;

  switch (import_mode) {
    case ImportMode::Code: {
      Log << Trace << "RecurseImport: Import as code: " << abs_import_path;

      if (m_rd.Current().GetStart().Get(m_rd).GetFilename() == abs_import_path.string()) {
        Log << ParserSignal << Current() << "Detected circular import: " << *import_file;
        return m_fac.CreateMockInstance<Import>();
      }

      auto in_src = boost::iostreams::stream<boost::iostreams::array_source>(content->data(), content->size());
      auto scanner = Tokenizer(in_src, m_env);
      scanner.SetCurrentFilename(abs_import_path.string());
      scanner.SkipCommentsState(true);

      lex::IScanner *scanner_ptr = &scanner;
      ParserSwapScanner(scanner_ptr);

      Log << Trace << "RecurseImport: Creating subparser for: " << abs_import_path;
      auto subparser = GeneralParser(scanner, m_env, m_pool);
      subparser.m_impl->m_recursion_depth = m_recursion_depth;
      auto subtree = subparser.m_impl->RecurseBlock(false, false, BlockMode::Unknown);

      ParserSwapScanner(scanner_ptr);

      return m_fac.CreateImport(import_file, import_mode, std::move(subtree));
    }

    case ImportMode::String: {
      Log << Trace << "RecurseImport: Import as string literal: " << abs_import_path;
      auto literal = m_fac.CreateString(content.value());
      return m_fac.CreateImport(import_file, import_mode, std::move(literal));
    }
  }
}

[[nodiscard]] auto GeneralParser::PImpl::RecurseImportPackage(string import_name,
                                                              ImportMode import_mode) -> FlowPtr<Expr> {
  /// TODO: FIXME: Implement scanner elsewhere; Remove this line
  static const thread_local auto pkgs = FindPackages({"/tmp/test"});

  Log << Trace << "RecurseImport: Importing package: " << import_name;

  auto pkg_it =
      std::find_if(pkgs.begin(), pkgs.end(), [&](const auto &pkg) { return pkg.PackageName() == import_name; });

  if (pkg_it == pkgs.end()) [[unlikely]] {
    Log << ParserSignal << Current() << "Package not found: " << import_name;
    return m_fac.CreateMockInstance<Import>();
  }

  const auto &files = pkg_it->Read();
  if (!files.has_value()) [[unlikely]] {
    Log << ParserSignal << Current() << "Failed to read package: " << import_name;
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
      Log << Trace << "RecurseImport: Import as code: " << import_name;
      /// TODO: Implement code import
      return m_fac.CreateMockInstance<Import>();
    }

    case ImportMode::String: {
      Log << Trace << "RecurseImport: Import as string literal: " << import_name;
      /// TODO: Implement string import
      return m_fac.CreateMockInstance<Import>();
    }
  }
}

auto GeneralParser::PImpl::RecurseImport() -> FlowPtr<Expr> {
  constexpr size_t kMaxRecursionDepth = 256;

  if (m_recursion_depth++ > kMaxRecursionDepth) {
    Log << ParserSignal << "Maximum import recursion depth reached: " << kMaxRecursionDepth;
    return m_fac.CreateMockInstance<Import>();
  }
  auto deferred_auto_dec = std::shared_ptr<void>(nullptr, [&](auto) { --m_recursion_depth; });

  const auto [import_name, import_mode] = RecurseImportName();

  const bool is_regular_file =
      import_name->find("/") != std::string::npos || import_name->find("\\") != std::string::npos;

  if (is_regular_file) {
    return RecurseImportRegularFile(import_name, import_mode);
  }

  return RecurseImportPackage(import_name, import_mode);
}
