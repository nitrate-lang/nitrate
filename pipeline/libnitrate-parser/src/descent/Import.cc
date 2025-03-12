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
#include <nitrate-parser/ASTFactory.hh>
#include <nitrate-parser/ASTVisitor.hh>
#include <nitrate-parser/Algorithm.hh>
#include <nitrate-parser/Package.hh>
#include <stack>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

/// TODO: FIXME: Implement scanner elsewhere; Remove this line
static const thread_local auto PKGS = FindPackages({"/tmp/test"});

namespace ncc::parse::import {
  class ImportSubgraphVisitor : public ASTVisitor {
    std::stack<Vis> m_vis_stack;
    ASTFactory &m_fac;
    const std::vector<std::string> &m_package_importer;
    const std::vector<std::string> &m_package_importee;

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
    void Visit(FlowPtr<Ternary> n) override { n->Discard(); };
    void Visit(FlowPtr<Integer> n) override { n->Discard(); };
    void Visit(FlowPtr<Float> n) override { n->Discard(); };
    void Visit(FlowPtr<Boolean> n) override { n->Discard(); };
    void Visit(FlowPtr<String> n) override { n->Discard(); };
    void Visit(FlowPtr<Character> n) override { n->Discard(); };
    void Visit(FlowPtr<Null> n) override { n->Discard(); };
    void Visit(FlowPtr<Undefined> n) override { n->Discard(); };
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
    void Visit(FlowPtr<ReturnIf> n) override { n->Discard(); };
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
          case QAST_BLOCK:
          case QAST_SCOPE:
          case QAST_EXPORT: {
            stmt->Accept(*this);
            break;
          }

          case QAST_TYPEDEF:
          case QAST_STRUCT:
          case QAST_ENUM:
          case QAST_VAR:
          case QAST_FUNCTION:
          case QAST_IMPORT: {
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
                /// TODO: Implement protected visibility

                (void)m_package_importee;
                (void)m_package_importer;
                Log << ParserSignal << "Protected visibility not implemented";
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

      for (auto &method : n->GetStaticMethods()) {
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
    ImportSubgraphVisitor(ASTFactory &fac, const std::vector<std::string> &package_importer,
                          const std::vector<std::string> &package_importee)
        : m_fac(fac), m_package_importer(package_importer), m_package_importee(package_importee) {
      m_vis_stack.push(Vis::Sec);
    }
  };
}  // namespace ncc::parse::import

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

auto GeneralParser::PImpl::RecurseImportRegularFile(string import_file, ImportMode import_mode) -> FlowPtr<Expr> {
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

      // The destination file has no package name
      std::vector<std::string> import_dst;

      auto in_src = boost::iostreams::stream<boost::iostreams::array_source>(content->data(), content->size());
      auto scanner = Tokenizer(in_src, m_env);
      scanner.SetCurrentFilename(abs_import_path.string());
      scanner.SkipCommentsState(true);

      lex::IScanner *scanner_ptr = &scanner;
      ParserSwapScanner(scanner_ptr);

      Log << Trace << "RecurseImport: Creating subparser for: " << abs_import_path;

      auto subparser = GeneralParser(scanner, {}, m_env, m_pool);
      subparser.m_impl->m_recursion_depth = m_recursion_depth;
      auto subtree = subparser.m_impl->RecurseBlock(false, false, BlockMode::Unknown);

      import::ImportSubgraphVisitor subgraph_visitor(m_fac, PackageNameChunks(), import_dst);
      subtree->Accept(subgraph_visitor);

      ParserSwapScanner(scanner_ptr);

      auto import_node = m_fac.CreateImport(import_file, import_mode, std::move(subtree));

      return import_node;
    }

    case ImportMode::String: {
      Log << Trace << "RecurseImport: Import as string literal: " << abs_import_path;

      auto literal = m_fac.CreateString(content.value());
      auto import_node = m_fac.CreateImport(import_file, import_mode, std::move(literal));

      return import_node;
    }
  }
}

[[nodiscard]] auto GeneralParser::PImpl::RecurseImportPackage(string import_name) -> FlowPtr<Expr> {
  Log << Trace << "RecurseImport: Importing package: " << import_name;

  // Find the package by import name
  auto pkg_it =
      std::find_if(PKGS.begin(), PKGS.end(), [&](const auto &pkg) { return pkg.PackageName() == import_name; });

  // If the package is not found, return a mock import node
  if (pkg_it == PKGS.end()) [[unlikely]] {
    Log << ParserSignal << Current() << "Package not found: " << import_name;
    return m_fac.CreateMockInstance<Import>();
  }

  // Lazy load the package content with caching
  const auto &files = pkg_it->Read();
  if (!files.has_value()) [[unlikely]] {
    Log << ParserSignal << Current() << "Failed to read package: " << import_name;
    return m_fac.CreateMockInstance<Import>();
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

  std::string import_dst;  /// FIXME: Get our current package name
  const auto qualified_import_name = SplitPackageName(import_name);

  for (const auto &name : sorted_keys) {
    const auto &content_getter = files->at(name);
    const auto &file_content = content_getter.Get();
    if (!file_content.has_value()) [[unlikely]] {
      Log << "RecurseImport: Failed to read package chunk: " << name;
      return m_fac.CreateMockInstance<Import>();
    }

    Log << Trace << "RecurseImport: Putting package chunk (" << file_content->size() << " bytes): " << name;

    // Construct a readonly stream from the file content string reference
    auto in_src = boost::iostreams::stream<boost::iostreams::array_source>(file_content->data(), file_content->size());

    // Create a sub-lexer for this package file
    auto scanner = Tokenizer(in_src, m_env);
    scanner.SetCurrentFilename(name.string());

    // Update the thread_local content for the diagnostics subsystem
    lex::IScanner *scanner_ptr = &scanner;
    ParserSwapScanner(scanner_ptr);

    Log << Trace << "RecurseImport: Creating subparser for: " << name;
    auto subparser = GeneralParser(scanner, qualified_import_name, m_env, m_pool);

    // Preserve the recursion depth for the subparser
    subparser.m_impl->m_recursion_depth = m_recursion_depth;

    // Perform the parsing of the package file
    auto subtree = subparser.m_impl->RecurseBlock(false, false, BlockMode::Unknown);

    // Prepare the subgraph by stripping out unnecessary nodes and
    // transforming definitions into declarations as needed
    // to create the external interface of the package
    import::ImportSubgraphVisitor subgraph_visitor(m_fac, PackageNameChunks(), SplitPackageName(import_name));
    subtree->Accept(subgraph_visitor);

    // Restore the thread_local content for the diagnostics subsystem
    ParserSwapScanner(scanner_ptr);

    // Create block for each package file
    blocks.push_back(std::move(subtree));
  }

  // The returned block is a block of blocks
  // The order the nodes appear is unspecified but deterministic
  auto block = m_fac.CreateBlock(blocks);
  auto import_node = m_fac.CreateImport(import_name, ImportMode::Code, std::move(block));

  return import_node;
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

  if (!is_regular_file && import_mode == ImportMode::Code) {
    return RecurseImportPackage(import_name);
  }

  Log << ParserSignal << Current() << "Can not import package content as a string literal";

  return m_fac.CreateMockInstance<Import>();
}
