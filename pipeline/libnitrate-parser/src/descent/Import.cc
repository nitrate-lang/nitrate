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
#include <nitrate-lexer/Scanner.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTExpr.hh>
#include <nitrate-parser/ASTFactory.hh>
#include <nitrate-parser/ASTStmt.hh>
#include <nitrate-parser/ASTType.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <nitrate-parser/Algorithm.hh>
#include <nitrate-parser/Package.hh>
#include <stack>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

namespace ncc::parse::import {
  class ImportSubgraphVisitor : public ASTVisitor {
    std::stack<Vis> m_vis_stack;
    ASTFactory &m_fac;
    const ImportName &m_package_importer;
    const ImportName &m_package_importee;

    void Visit(FlowPtr<NamedTy> n) override { n->Discard(); };
    void Visit(FlowPtr<InferTy> n) override { n->Discard(); };
    void Visit(FlowPtr<TemplateType> n) override { n->Discard(); };
    void Visit(FlowPtr<U1> n) override { n->Discard(); };
    void Visit(FlowPtr<U8> n) override { n->Discard(); };
    void Visit(FlowPtr<U16> n) override { n->Discard(); };
    void Visit(FlowPtr<U32> n) override { n->Discard(); };
    void Visit(FlowPtr<U64> n) override { n->Discard(); };
    void Visit(FlowPtr<U128> n) override { n->Discard(); };
    void Visit(FlowPtr<I8> n) override { n->Discard(); };
    void Visit(FlowPtr<I16> n) override { n->Discard(); };
    void Visit(FlowPtr<I32> n) override { n->Discard(); };
    void Visit(FlowPtr<I64> n) override { n->Discard(); };
    void Visit(FlowPtr<I128> n) override { n->Discard(); };
    void Visit(FlowPtr<F16> n) override { n->Discard(); };
    void Visit(FlowPtr<F32> n) override { n->Discard(); };
    void Visit(FlowPtr<F64> n) override { n->Discard(); };
    void Visit(FlowPtr<F128> n) override { n->Discard(); };
    void Visit(FlowPtr<VoidTy> n) override { n->Discard(); };
    void Visit(FlowPtr<PtrTy> n) override { n->Discard(); };
    void Visit(FlowPtr<OpaqueTy> n) override { n->Discard(); };
    void Visit(FlowPtr<TupleTy> n) override { n->Discard(); };
    void Visit(FlowPtr<ArrayTy> n) override { n->Discard(); };
    void Visit(FlowPtr<RefTy> n) override { n->Discard(); };
    void Visit(FlowPtr<FuncTy> n) override { n->Discard(); };
    void Visit(FlowPtr<Unary> n) override { n->Discard(); };
    void Visit(FlowPtr<Binary> n) override { n->Discard(); };
    void Visit(FlowPtr<Integer> n) override { n->Discard(); };
    void Visit(FlowPtr<Float> n) override { n->Discard(); };
    void Visit(FlowPtr<Boolean> n) override { n->Discard(); };
    void Visit(FlowPtr<String> n) override { n->Discard(); };
    void Visit(FlowPtr<Character> n) override { n->Discard(); };
    void Visit(FlowPtr<Null> n) override { n->Discard(); };
    void Visit(FlowPtr<Call> n) override { n->Discard(); };
    void Visit(FlowPtr<TemplateCall> n) override { n->Discard(); };
    void Visit(FlowPtr<List> n) override { n->Discard(); };
    void Visit(FlowPtr<Assoc> n) override { n->Discard(); };
    void Visit(FlowPtr<Index> n) override { n->Discard(); };
    void Visit(FlowPtr<Slice> n) override { n->Discard(); };
    void Visit(FlowPtr<FString> n) override { n->Discard(); };
    void Visit(FlowPtr<Identifier> n) override { n->Discard(); };
    void Visit(FlowPtr<Assembly> n) override { n->Discard(); };
    void Visit(FlowPtr<If> n) override { n->Discard(); };
    void Visit(FlowPtr<While> n) override { n->Discard(); };
    void Visit(FlowPtr<For> n) override { n->Discard(); };
    void Visit(FlowPtr<Foreach> n) override { n->Discard(); };
    void Visit(FlowPtr<Break> n) override { n->Discard(); };
    void Visit(FlowPtr<Continue> n) override { n->Discard(); };
    void Visit(FlowPtr<Return> n) override { n->Discard(); };
    void Visit(FlowPtr<Case> n) override { n->Discard(); };
    void Visit(FlowPtr<Switch> n) override { n->Discard(); };

    ///=========================================================================

    void Visit(FlowPtr<Import> n) override {
      n->GetSubtree()->Accept(*this);
      if (n->RecursiveChildCount() == 0) {
        n->Discard();
      }
    };

    void Visit(FlowPtr<Block> n) override {
      for (auto &stmt : n->GetStatements()) {
        switch (stmt->GetKind()) {
          case AST_sBLOCK:
          case AST_sSCOPE:
          case AST_sEXPORT: {
            stmt->Accept(*this);
            break;
          }

          case AST_sTYPEDEF:
          case AST_sSTRUCT:
          case AST_sENUM:
          case AST_sVAR:
          case AST_sFUNCTION:
          case AST_eIMPORT: {
            auto vis = m_vis_stack.top();

            switch (vis) {
              case Vis::Pub: {
                stmt->Accept(*this);
                break;
              }

              case Vis::Sec: {
                stmt->Discard();
                break;
              }

              case Vis::Pro: {
                if (m_package_importee == m_package_importer) {
                  stmt->Accept(*this);
                } else {
                  stmt->Discard();
                }

                break;
              }
            }

            break;
          }

          default: {
            stmt->Discard();
            break;
          }
        }
      }

      if (n->RecursiveChildCount() == 0) {
        n->Discard();
      }
    }

    void Visit(FlowPtr<Variable> n) override {
      // Prevent multiple definitions of the same global variable
      // by setting a linkage attribute on all imported variables to
      // extern. Also require all global variables to have explicit
      // type specifications.

      if (!n->GetType()) {
        Log << ParserSignal << "Global variable '" << n->GetName() << "' must have a type specification";
        return;
      }

      bool is_extern =
          std::find_if(n->GetAttributes().begin(), n->GetAttributes().end(), [](const FlowPtr<Expr> &attr) {
            if (!attr->Is(AST_eCALL)) {
              return false;
            }

            const auto &call = attr->As<Call>();

            if (!call->GetFunc()->Is(AST_eIDENT) || call->GetFunc()->As<Identifier>()->GetName() != "linkage") {
              return false;
            }

            if (call->GetArgs().size() != 1 || !call->GetArgs()[0].second->Is(AST_eSTRING)) {
              return false;
            }

            return call->GetArgs()[0].second->As<String>()->GetValue() == "extern";
          }) != n->GetAttributes().end();

      if (!is_extern) {
        auto extern_linkage = m_fac
                                  .CreateCall(m_fac.CreateIdentifier("linkage"),
                                              {
                                                  {0U, m_fac.CreateString("extern")},
                                              })
                                  .value();

        auto orig_attributes = n->GetAttributes();
        auto orig_attributes_size = orig_attributes.size();

        auto attributes = m_fac.AllocateArray<FlowPtr<Expr>>(orig_attributes_size + 1);
        std::copy(orig_attributes.begin(), orig_attributes.end(), attributes.begin());
        attributes[orig_attributes_size] = extern_linkage;

        n->SetAttributes(attributes);
      }

      n->SetInitializer(std::nullopt);
    }

    void Visit(FlowPtr<Typedef>) override {}

    void Visit(FlowPtr<Function> n) override {
      // Prevent multiple definitions of the same function
      // by removing the body of all imported functions, thereby
      // turning them into declarations.

      n->SetBody(std::nullopt);
      qcore_assert(n->IsDeclaration());
    }

    void Visit(FlowPtr<Struct> n) override {
      for (auto &method : n->GetMethods()) {
        method.m_func->SetBody(std::nullopt);
      }
    }

    void Visit(FlowPtr<Enum>) override {}

    void Visit(FlowPtr<Scope> n) override {
      n->GetBody()->Accept(*this);

      if (n->RecursiveChildCount() == 0) {
        n->Discard();
      }
    }

    void Visit(FlowPtr<Export> n) override {
      m_vis_stack.push(n->GetVis());
      n->GetBody()->Accept(*this);
      m_vis_stack.pop();

      if (n->RecursiveChildCount() == 0) {
        n->Discard();
      }
    }

  public:
    ImportSubgraphVisitor(ASTFactory &fac, const ImportName &package_importer, const ImportName &package_importee)
        : m_fac(fac), m_package_importer(package_importer), m_package_importee(package_importee) {
      m_vis_stack.push(Vis::Sec);
    }
  };
}  // namespace ncc::parse::import

static auto RecurseImportName(GeneralParser::Context &m) -> std::pair<string, ImportMode> {
  if (m.NextIf<PuncLPar>()) {
    auto arguments = m.RecurseCallArguments({Token(Punc, PuncRPar)}, false);

    if (!m.NextIf<PuncRPar>()) {
      Log << ParserSignal << m.Current() << "Expected ')' to close the import call";
    }

    string import_name;
    std::optional<ImportMode> import_mode;

    for (const auto &arg : arguments) {
      if (arg.first == "src" || arg.first == "0") {
        if (import_name) {
          Log << ParserSignal << m.Current() << "Duplicate argument: 'src' in call to import";
        }

        auto pvalue = arg.second;
        if (!pvalue->Is(AST_eSTRING)) {
          Log << ParserSignal << m.Current() << "Expected string literal for import source";
          continue;
        }

        import_name = pvalue->As<parse::String>()->GetValue();
        continue;
      }

      if (arg.first == "mode" || arg.first == "1") {
        if (import_mode) {
          Log << ParserSignal << m.Current() << "Duplicate argument: 'mode' in call to import";
        }

        auto pvalue = arg.second;
        if (!pvalue->Is(AST_eSTRING)) {
          Log << ParserSignal << m.Current() << "Expected string literal for import mode";
          continue;
        }

        auto mode = pvalue->As<parse::String>()->GetValue();
        if (mode == "code") {
          import_mode = ImportMode::Code;
        } else if (mode == "string") {
          import_mode = ImportMode::String;
        } else {
          Log << ParserSignal << m.Current() << "Invalid import mode: " << mode;
        }

        continue;
      }

      Log << ParserSignal << m.Current() << "Unexpected argument: " << arg.first;
    }

    if (!import_name) [[unlikely]] {
      Log << ParserSignal << m.Current() << "parameter 'src': missing positional argument 0 in call to import";
    }

    return {import_name, import_mode.value_or(ImportMode::Code)};
  }

  if (auto tok = m.NextIf<Text>()) {
    return {tok->GetString(), ImportMode::Code};
  }

  auto name = m.RecurseName();
  if (!name) [[unlikely]] {
    Log << ParserSignal << m.Current() << "Expected import name";
  }

  return {name, ImportMode::Code};
}

static auto RecurseImportRegularFile(GeneralParser::Context &m, ImportedFilesSet &imported_files,
                                     ImportConfig &import_config, const std::filesystem::path &import_file,
                                     ImportMode import_mode) -> FlowPtr<Expr> {
  { /* Try to prevent infinite import recursion */
    if (imported_files->contains(import_file)) {
      Log << ParserSignal << Debug << m.Current() << "Detected circular import: " << import_file << " (skipping)";
      return m.CreateImport(import_file.string(), import_mode, m.CreateBlock());
    }

    if (import_config.GetFilesToNotImport().contains(import_file)) {
      Log << ParserSignal << Debug << m.Current() << "Skipping file: " << import_file;
      return m.CreateImport(import_file.string(), import_mode, m.CreateBlock());
    }

    imported_files->insert(import_file);
  }

  auto abs_import_path = OMNI_CATCH(std::filesystem::absolute(import_file)).value_or(import_file).lexically_normal();

  Log << Trace << "RecurseImport: Importing regular file: " << abs_import_path;

  const auto exists = OMNI_CATCH(std::filesystem::exists(abs_import_path));
  if (!exists) {
    Log << ParserSignal << m.Current() << "Could not check if file exists: " << abs_import_path;
    return &m.CreateImport(import_file.string(), import_mode, m.CreateBlock())->SetMock();
  }

  if (!*exists) {
    Log << ParserSignal << m.Current() << "File not found: " << abs_import_path;
    return &m.CreateImport(import_file.string(), import_mode, m.CreateBlock())->SetMock();
  }

  auto is_regular_file = OMNI_CATCH(std::filesystem::is_regular_file(abs_import_path));
  if (!is_regular_file) {
    Log << ParserSignal << m.Current() << "Could not check if file is regular: " << abs_import_path;
    return &m.CreateImport(import_file.string(), import_mode, m.CreateBlock())->SetMock();
  }

  if (!*is_regular_file) {
    Log << ParserSignal << m.Current() << "File is not regular: " << abs_import_path;
    return &m.CreateImport(import_file.string(), import_mode, m.CreateBlock())->SetMock();
  }

  Log << Trace << "RecurseImport: Reading regular file: " << abs_import_path;

  std::ifstream file(abs_import_path, std::ios::binary);
  if (!file.is_open()) {
    Log << ParserSignal << m.Current() << "Failed to open file: " << abs_import_path;
    return &m.CreateImport(import_file.string(), import_mode, m.CreateBlock())->SetMock();
  }

  auto content = OMNI_CATCH(std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()));
  if (!content.has_value()) {
    Log << ParserSignal << m.Current() << "Failed to read file: " << abs_import_path;
    return &m.CreateImport(import_file.string(), import_mode, m.CreateBlock())->SetMock();
  }

  if (content->empty()) {
    Log << ParserSignal << Warning << m.Current() << "File is empty: " << abs_import_path;
  }

  Log << Trace << "RecurseImport: Got file (" << content.value().size() << " bytes): " << abs_import_path;

  switch (import_mode) {
    case ImportMode::Code: {
      Log << Trace << "RecurseImport: Import as code: " << abs_import_path;

      auto in_src = boost::iostreams::stream<boost::iostreams::array_source>(content->data(), content->size());
      auto scanner = Tokenizer(in_src, m.GetEnvironment());
      scanner.SetCurrentFilename(abs_import_path.string());
      scanner.SkipCommentsState(true);

      lex::IScanner *scanner_ptr = &scanner;
      ParserSwapScanner(scanner_ptr);

      Log << Trace << "RecurseImport: Creating subparser for: " << abs_import_path;

      auto subparser = m.CreateSubParser(scanner);
      auto subtree = subparser.m_impl->RecurseBlock(false, false, BlockMode::Unknown);

      ImportName importee_name;
      import::ImportSubgraphVisitor subgraph_visitor(m, import_config.GetThisImportName(), importee_name);
      subtree->Accept(subgraph_visitor);

      ParserSwapScanner(scanner_ptr);

      auto import_node = m.CreateImport(import_file.string(), import_mode, std::move(subtree));
      return import_node;
    }

    case ImportMode::String: {
      Log << Trace << "RecurseImport: Import as string literal: " << abs_import_path;

      auto literal = m.CreateString(content.value());
      auto import_node = m.CreateImport(import_file.string(), import_mode, std::move(literal));

      return import_node;
    }
  }
}

static auto RecurseImportPackage(GeneralParser::Context &m, ImportedFilesSet &imported_files,
                                 ImportConfig &import_config, const ImportName &import_name) -> FlowPtr<Expr> {
  Log << Trace << "RecurseImport: Importing package: " << import_name;

  // Find the package by import name
  const auto &pkgs = import_config.GetPackages();
  auto pkg_it =
      std::find_if(pkgs.begin(), pkgs.end(), [&](const auto &pkg) { return pkg.PackageName() == import_name; });

  // If the package is not found, return a mock import node
  if (pkg_it == pkgs.end()) [[unlikely]] {
    Log << ParserSignal << m.Current() << "Package not found: " << import_name;
    return &m.CreateImport(*import_name, ImportMode::Code, m.CreateBlock())->SetMock();
  }

  // Lazy load the package content with caching
  const auto &files = pkg_it->Read();
  if (!files.has_value()) [[unlikely]] {
    Log << ParserSignal << m.Current() << "Failed to read package: " << import_name;
    return &m.CreateImport(*import_name, ImportMode::Code, m.CreateBlock())->SetMock();
  }

  Log << Debug << "RecurseImport: Got package: " << import_name;

  // Produce a sorted deterministic order of the files in the package
  const auto sorted_keys = [&]() {
    std::vector<std::filesystem::path> keys;
    for (const auto &[path, _] : files.value()) {
      keys.push_back(path);
    }
    std::sort(keys.begin(), keys.end());
    return keys;
  }();

  Log << Trace << "RecurseImport: Import as code: " << import_name;

  std::vector<FlowPtr<Expr>> blocks;

  for (const auto &file_name : sorted_keys) {
    { /* Try to prevent infinite import recursion */
      if (imported_files->contains(file_name)) {
        Log << ParserSignal << Debug << m.Current() << "Detected circular import: " << file_name << " (skipping)";
        continue;
      }

      if (import_config.GetFilesToNotImport().contains(file_name)) {
        Log << ParserSignal << Debug << m.Current() << "Skipping file: " << file_name;
        continue;
      }

      imported_files->insert(file_name);
    }

    const auto &content_getter = files->at(file_name);
    const auto &file_content = content_getter.Get();
    if (!file_content.has_value()) [[unlikely]] {
      Log << "RecurseImport: Failed to read package chunk: " << file_name;
      return &m.CreateImport(*import_name, ImportMode::Code, m.CreateBlock())->SetMock();
    }

    Log << Trace << "RecurseImport: Putting package chunk (" << file_content->size() << " bytes): " << file_name;

    // Construct a readonly stream from the file content string reference
    auto in_src = boost::iostreams::stream<boost::iostreams::array_source>(file_content->data(), file_content->size());

    // Create a sub-lexer for this package file
    auto scanner = Tokenizer(in_src, m.GetEnvironment());
    scanner.SetCurrentFilename(file_name.string());
    scanner.SkipCommentsState(true);

    // Update the thread_local content for the diagnostics subsystem
    lex::IScanner *scanner_ptr = &scanner;
    ParserSwapScanner(scanner_ptr);

    Log << Trace << "RecurseImport: Creating subparser for: " << file_name;
    auto subparser = m.CreateSubParser(scanner);
    auto subtree = subparser.m_impl->RecurseBlock(false, false, BlockMode::Unknown);

    // Prepare the subgraph by stripping out unnecessary nodes and
    // transforming definitions into declarations as needed
    // to create the external interface of the package
    import::ImportSubgraphVisitor subgraph_visitor(m, import_config.GetThisImportName(), import_name);
    subtree->Accept(subgraph_visitor);

    // Restore the thread_local content for the diagnostics subsystem
    ParserSwapScanner(scanner_ptr);

    // Create block for each package file
    blocks.push_back(std::move(subtree));
  }

  // The returned block is a block of blocks
  // The order the nodes appear is unspecified but deterministic
  auto block = m.CreateBlock(blocks);
  auto import_node = m.CreateImport(*import_name, ImportMode::Code, std::move(block));

  return import_node;
}

auto GeneralParser::Context::RecurseImport() -> FlowPtr<Expr> {
  constexpr size_t kMaxRecursionDepth = 256;

  if (m_recursion_depth++ > kMaxRecursionDepth) {
    Log << ParserSignal << "Maximum import recursion depth reached: " << kMaxRecursionDepth;
    return CreateMockInstance<Import>();
  }

  auto deferred_auto_dec = std::shared_ptr<void>(nullptr, [&](auto) { --m_recursion_depth; });

  const auto [import_name_precheck, import_mode] = RecurseImportName(*this);
  const bool is_regular_file =
      import_name_precheck->find("/") != std::string::npos || import_name_precheck->find("\\") != std::string::npos;

  if (is_regular_file) {
    return RecurseImportRegularFile(*this, m_imported_files, m_import_config, *import_name_precheck, import_mode);
  }

  if (!is_regular_file && import_mode == ImportMode::Code) {
    ImportName checked_name(import_name_precheck);
    return RecurseImportPackage(*this, m_imported_files, m_import_config, checked_name);
  }

  Log << ParserSignal << Current() << "Can not import package content as a string literal";

  return CreateMockInstance<Import>();
}
