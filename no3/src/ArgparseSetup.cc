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

#include <nitrate-emit/Code.h>
#include <nitrate-emit/Lib.h>

#include <argparse.hpp>
#include <cstdint>
#include <memory>
#include <nitrate-core/Init.hh>
#include <nitrate-ir/Init.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-parser/Init.hh>
#include <nitrate-seq/Init.hh>
#include <string>

using namespace argparse;

namespace no3::argparse_setup {
  static void SetupArgparseInit(ArgumentParser &parser) {
    parser.AddArgument("package-name")
        .Help("name of package to initialize")
        .Nargs(1);

    parser.AddArgument("-o", "--output")
        .Help("output directory")
        .DefaultValue(std::string("."))
        .Nargs(1);

    parser.AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-f", "--force")
        .Help("force overwrite of existing package")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-t", "--type")
        .Help("type of package to initialize")
        .Choices("program", "staticlib", "sharedlib")
        .DefaultValue(std::string("program"))
        .Nargs(1);

    parser.AddArgument("-r", "--version")
        .Help("version of package")
        .DefaultValue(std::string("1.0.0"))
        .Nargs(1);

    parser.AddArgument("-l", "--license")
        .Help("license to use for package")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("-a", "--author")
        .Help("author of package")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("-e", "--email")
        .Help("email of author")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("-u", "--url")
        .Help("URL of package")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("-d", "--description")
        .Help("description of package")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("-r", "--repository")
        .Help("URL of repository")
        .DefaultValue(std::string(""))
        .Nargs(1);
  }

  static void SetupArgparseBuild(ArgumentParser &parser) {
    parser.AddArgument("package-src")
        .Help("path to package source")
        .Nargs(1)
        .DefaultValue(std::string("."));

    parser.AddArgument("-o", "--output")
        .Help("output directory")
        .DefaultValue(std::string("."))
        .Nargs(1);

    parser.AddArgument("-N", "--no-cache")
        .Help("do not use cached files")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-j", "--jobs")
        .Help("number of jobs to run simultaneously. 0 for auto")
        .DefaultValue(0)
        .Nargs(1)
        .Scan<'u', uint32_t>();

    parser.AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    auto &optimization_group = parser.AddMutuallyExclusiveGroup();
    optimization_group.AddArgument("-O", "--optimize")
        .Help(
            "request optimization from build pipeline. not all pipelines will "
            "support this, and it may be ignored")
        .DefaultValue(false)
        .ImplicitValue(true);

    optimization_group.AddArgument("-Os", "--optimize-size")
        .Help(
            "request size optimization from build pipeline. not all pipelines "
            "will support this, and it may be ignored")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-g", "--debug")
        .Help(
            "request that the pipeline generate and preserve debug "
            "information. "
            "not all pipelines will support this, and it may be ignored")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-C", "--certify")
        .Help(
            "digitally sign the output with the specified PKCS#12 certificate")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("--certify-password")
        .Help("password for the PKCS#12 certificate")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("--supply-chain-insecure")
        .Help(
            "do not verify OR require dependencies to be validly signed by a "
            "trusted source")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--trustkey")
        .Help(
            "add a trusted public key fingerprint that may be used to verify "
            "dependencies (only applies to this build)")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("--trustkeys")
        .Help(
            "add a file containing trusted public key fingerprints that may be "
            "used to verify dependencies (only applies to this build)")
        .DefaultValue(std::string(""))
        .Nargs(1);
  }

  static void SetupArgparseClean(ArgumentParser &parser) {
    parser.AddArgument("package-src").Help("path to package source").Nargs(1);

    parser.AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);
  }

  static void SetupArgparseUpdate(ArgumentParser &parser) {
    parser.AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--supply-chain-insecure")
        .Help(
            "do not verify OR require dependencies to be validly signed by a "
            "trusted source")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--trustkey")
        .Help(
            "add a trusted public key fingerprint that may be used to verify "
            "dependencies (only applies to this build)")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("--trustkeys")
        .Help(
            "add a file containing trusted public key fingerprints that may be "
            "used to verify dependencies (only applies to this build)")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("package-name")
        .Help("name of package to update")
        .Required()
        .Remaining();
  }

  static void SetupArgparseInstall(ArgumentParser &parser) {
    parser.AddArgument("src").Help("source of package to install").Nargs(1);

    parser.AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-O", "--override")
        .Help("override existing package")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-g", "--global")
        .Help("install package globally (requires higher permissions)")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-n", "--no-build")
        .Help("do not build package after downloading")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--supply-chain-insecure")
        .Help(
            "do not verify OR require dependencies to be validly signed by a "
            "trusted source")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--trustkey")
        .Help(
            "add a trusted public key fingerprint that may be used to verify "
            "dependencies (only applies to this build)")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("--trustkeys")
        .Help(
            "add a file containing trusted public key fingerprints that may be "
            "used to verify dependencies (only applies to this build)")
        .DefaultValue(std::string(""))
        .Nargs(1);
  }

  static void SetupArgparseDoc(ArgumentParser &parser) {
    parser.AddArgument("package-src")
        .Help("name of package to document")
        .Nargs(1);

    parser.AddArgument("--html")
        .Help("generate HTML report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--plain")
        .Help("generate plain text report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--pdf")
        .Help("generate PDF report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--json")
        .Help("generate JSON report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--xml")
        .Help("generate XML report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--reactjs")
        .Help("generate ReactJS report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-o", "--output")
        .Help("output directory")
        .DefaultValue(std::string("."))
        .Nargs(1);

    parser.AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-r", "--recursive")
        .Help("document all dependencies")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-d", "--depth")
        .Help("maximum depth of dependency tree to document")
        .DefaultValue((size_t)1)
        .Nargs(1);

    parser.AddArgument("-C", "--certify")
        .Help(
            "digitally sign the documentation with the specified PKCS#12 "
            "certificate")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("--certify-password")
        .Help("password for the PKCS#12 certificate")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("--supply-chain-insecure")
        .Help(
            "do not verify OR require dependencies to be validly signed by a "
            "trusted source")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--trustkey")
        .Help(
            "add a trusted public key fingerprint that may be used to verify "
            "dependencies (only applies to this build)")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("--trustkeys")
        .Help(
            "add a file containing trusted public key fingerprints that may be "
            "used to verify dependencies (only applies to this build)")
        .DefaultValue(std::string(""))
        .Nargs(1);
  }

  static void SetupArgparseFormat(ArgumentParser &parser) {
    parser.AddArgument("package-src").Help("path to package source").Nargs(1);

    parser.AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);
  }

  static void SetupArgparseList(ArgumentParser &parser) {
    parser.AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-p", "--packages")
        .Help("list all packages installed")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-x", "--executables")
        .Help("list all executables installed")
        .DefaultValue(false)
        .ImplicitValue(true);
  }

  static void SetupArgparseTest(ArgumentParser &parser) {
    parser.AddArgument("package-name").Help("name of package to test").Nargs(1);

    parser.AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-o", "--output", "--report-dir")
        .Help("output directory for reports")
        .DefaultValue(std::string("."))
        .Nargs(1);

    parser.AddArgument("--html")
        .Help("generate HTML report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--plain")
        .Help("generate plain text report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--pdf")
        .Help("generate PDF report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--json")
        .Help("generate JSON report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--xml")
        .Help("generate XML report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--reactjs")
        .Help("generate ReactJS report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-c, --coverage")
        .Help("generate code coverage report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-p, --profile")
        .Help("generate profiling report")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-r", "--recursive")
        .Help("test all dependencies")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-d", "--depth")
        .Help("maximum depth of dependency tree to test")
        .DefaultValue((size_t)1)
        .Nargs(1);

    parser.AddArgument("--supply-chain-insecure")
        .Help(
            "do not verify OR require dependencies to be validly signed by a "
            "trusted source")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("--trustkey")
        .Help(
            "add a trusted public key fingerprint that may be used to verify "
            "dependencies (only applies to this build)")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("--trustkeys")
        .Help(
            "add a file containing trusted public key fingerprints that may be "
            "used to verify dependencies (only applies to this build)")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("-C", "--certify")
        .Help(
            "digitally sign the test reports with the specified PKCS#12 "
            "certificate")
        .DefaultValue(std::string(""))
        .Nargs(1);

    parser.AddArgument("--certify-password")
        .Help("password for the PKCS#12 certificate")
        .DefaultValue(std::string(""))
        .Nargs(1);
  }

  static void SetupArgparseLsp(ArgumentParser &parser) {
    parser.AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    parser.AddArgument("-o", "--log")
        .DefaultValue(std::string("no3-lsp.log"))
        .Help("Specify the log file");

    parser.AddArgument("--config")
        .DefaultValue(std::string(""))
        .Help("Specify the configuration file");

    auto &group = parser.AddMutuallyExclusiveGroup();

    group.AddArgument("--pipe").Help("Specify the pipe file to connect to");
    group.AddArgument("--port").Help("Specify the port to connect to");
    group.AddArgument("--stdio").DefaultValue(false).ImplicitValue(true).Help(
        "Use standard I/O");
  }

  static void SetupArgparseDev(
      ArgumentParser &parser,
      std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>>
          &subparsers) {
    /*================= CONFIG BASIC =================*/
    parser.AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    /*================== OTHER STUFF =================*/
    parser.AddArgument("--demangle")
        .Help("demangle Nitrate symbol names")
        .Nargs(1);

    /*================= BENCH SUBPARSER =================*/
    auto bench = std::make_unique<ArgumentParser>("bench", "1.0",
                                                  default_arguments::help);

    bench->AddArgument("-n", "--name")
        .Choices("lexer", "sequencer", "parser", "nitrate-ir", "llvm-ir",
                 "llvm-codegen", "c11-codegen", "pipeline")
        .Help("name of benchmark to run");

    bench->AddArgument("--list")
        .Help("list available benchmarks")
        .DefaultValue(false)
        .ImplicitValue(true);
    bench->AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    subparsers["bench"] = std::move(bench);

    /*================= PARSE SUBPARSER =================*/
    auto parse = std::make_unique<ArgumentParser>("parse", "1.0",
                                                  default_arguments::help);

    parse->AddArgument("source").Help("source file to parse").Nargs(1);
    parse->AddArgument("-o", "--output")
        .Help("output file for parse tree")
        .DefaultValue(std::string(""))
        .Nargs(1);
    parse->AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    subparsers["parse"] = std::move(parse);

    /*================= NR SUBPARSER =================*/
    auto nr =
        std::make_unique<ArgumentParser>("nr", "1.0", default_arguments::help);

    nr->AddArgument("source").Help("source file to lower into NR").Nargs(1);
    nr->AddArgument("-o", "--output")
        .Help("output file for nr tree")
        .DefaultValue(std::string(""))
        .Nargs(1);
    nr->AddArgument("-O", "--opts")
        .Help("optimizations to apply to NR")
        .DefaultValue(std::string(""))
        .Nargs(1);
    nr->AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    subparsers["nr"] = std::move(nr);

    /*================= CODEGEN SUBPARSER =================*/
    auto codegen = std::make_unique<ArgumentParser>("codegen", "1.0",
                                                    default_arguments::help);

    codegen->AddArgument("source")
        .Help("source file to generate code for")
        .Nargs(1);
    codegen->AddArgument("-o", "--output")
        .Help("output file for generated code")
        .DefaultValue(std::string(""))
        .Nargs(1);
    codegen->AddArgument("-O", "--opts")
        .Help("optimizations to apply to codegen")
        .DefaultValue(std::string(""))
        .Nargs(1);
    codegen->AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);
    codegen->AddArgument("-t", "--target")
        .Help("Target to generate code for")
        .DefaultValue(std::string("native"))
        .Nargs(1);

    subparsers["codegen"] = std::move(codegen);

    /*================= TEST SUBPARSER =================*/
    auto test = std::make_unique<ArgumentParser>("test", "1.0",
                                                 default_arguments::help);

    test->AddArgument("-v", "--verbose")
        .Help("print verbose output")
        .DefaultValue(false)
        .ImplicitValue(true);

    subparsers["test"] = std::move(test);

    parser.AddSubparser(*subparsers["bench"]);
    parser.AddSubparser(*subparsers["test"]);
    parser.AddSubparser(*subparsers["parse"]);
    parser.AddSubparser(*subparsers["nr"]);
    parser.AddSubparser(*subparsers["codegen"]);
  }

  static auto SetupArgparse(
      ArgumentParser &parser, ArgumentParser &init_parser,
      ArgumentParser &build_parser, ArgumentParser &clean_parser,
      ArgumentParser &update_parser, ArgumentParser &install_parser,
      ArgumentParser &doc_parser, ArgumentParser &format_parser,
      ArgumentParser &list_parser, ArgumentParser &test_parser,
      ArgumentParser &lsp_parser, ArgumentParser &dev_parser,
      std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>>
          &dev_subparsers) -> ArgumentParser & {
    using namespace argparse;

    SetupArgparseInit(init_parser);
    SetupArgparseBuild(build_parser);
    SetupArgparseClean(clean_parser);
    SetupArgparseUpdate(update_parser);
    SetupArgparseInstall(install_parser);
    SetupArgparseDoc(doc_parser);
    SetupArgparseFormat(format_parser);
    SetupArgparseList(list_parser);
    SetupArgparseTest(test_parser);
    SetupArgparseLsp(lsp_parser);
    SetupArgparseDev(dev_parser, dev_subparsers);

    parser.AddSubparser(init_parser);
    parser.AddSubparser(build_parser);
    parser.AddSubparser(clean_parser);
    parser.AddSubparser(update_parser);
    parser.AddSubparser(install_parser);
    parser.AddSubparser(doc_parser);
    parser.AddSubparser(format_parser);
    parser.AddSubparser(list_parser);
    parser.AddSubparser(test_parser);
    parser.AddSubparser(lsp_parser);
    parser.AddSubparser(dev_parser);

    parser.AddArgument("--license")
        .Help("show license information")
        .DefaultValue(false)
        .ImplicitValue(true);

    return parser;
  }

  static auto No3DepsVersionString() -> std::string {
    /* FIXME: Automate setting of 'is stable build' flag */

#define NO3_STABLE false

    std::stringstream ss;

    std::array<std::string_view, 6> no3_deps = {
        ncc::CoreLibrary.GetVersion(),
        ncc::lex::LexerLibrary.GetVersion(),
        ncc::seq::SeqLibrary.GetVersion(),
        ncc::parse::ParseLibrary.GetVersion(),
        ncc::ir::IRLibrary.GetVersion(),
        QcodeLibVersion()};

    ss << R"({"ver":")" << __TARGET_VERSION << R"(","stable":)"
       << (NO3_STABLE ? "true" : "false") << ",\"using\":[";
    for (size_t i = 0; i < no3_deps.size(); i++) {
      ss << "\"" << no3_deps[i] << "\"";
      if (i < no3_deps.size() - 1) {
        ss << ",";
      }
    }
    ss << "]}";

    return ss.str();
  }

  ArgumentParser InitParser("init", "1.0", default_arguments::help);
  ArgumentParser BuildParser("build", "1.0", default_arguments::help);
  ArgumentParser CleanParser("clean", "1.0", default_arguments::help);
  ArgumentParser UpdateParser("update", "1.0", default_arguments::help);
  ArgumentParser InstallParser("install", "1.0", default_arguments::help);
  ArgumentParser DocParser("doc", "1.0", default_arguments::help);
  ArgumentParser FormatParser("format", "1.0", default_arguments::help);
  ArgumentParser ListParser("list", "1.0", default_arguments::help);
  ArgumentParser TestParser("test", "1.0", default_arguments::help);
  ArgumentParser LspParser("lsp", "1.0", default_arguments::help);
  ArgumentParser DevParser("dev", "1.0", default_arguments::help);
  std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>>
      DevSubparsers;

  static ArgumentParser ArgumentParser("no3", No3DepsVersionString());

  // Setup parsing

  class ArgumentParser &Program = SetupArgparse(
      ArgumentParser, InitParser, BuildParser, CleanParser, UpdateParser,
      InstallParser, DocParser, FormatParser, ListParser, TestParser, LspParser,
      DevParser, DevSubparsers);
}  // namespace no3::argparse_setup
