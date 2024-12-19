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

#include <argparse.h>
#include <nitrate-emit/Code.h>
#include <nitrate-emit/Lib.h>
#include <nitrate-ir/Lib.h>
#include <nitrate-seq/Lib.h>

#include <cstdint>
#include <memory>
#include <nitrate-core/Init.hh>
#include <nitrate-lexer/Init.hh>
#include <nitrate-parser/Init.hh>
#include <string>

using namespace argparse;

namespace no3::argparse_setup {
  static void setup_argparse_init(ArgumentParser &parser) {
    parser.add_argument("package-name")
        .help("name of package to initialize")
        .nargs(1);

    parser.add_argument("-o", "--output")
        .help("output directory")
        .default_value(std::string("."))
        .nargs(1);

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-f", "--force")
        .help("force overwrite of existing package")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-t", "--type")
        .help("type of package to initialize")
        .choices("program", "staticlib", "sharedlib")
        .default_value(std::string("program"))
        .nargs(1);

    parser.add_argument("-r", "--version")
        .help("version of package")
        .default_value(std::string("1.0.0"))
        .nargs(1);

    parser.add_argument("-l", "--license")
        .help("license to use for package")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("-a", "--author")
        .help("author of package")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("-e", "--email")
        .help("email of author")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("-u", "--url")
        .help("URL of package")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("-d", "--description")
        .help("description of package")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("-r", "--repository")
        .help("URL of repository")
        .default_value(std::string(""))
        .nargs(1);
  }

  static void setup_argparse_build(ArgumentParser &parser) {
    parser.add_argument("package-src")
        .help("path to package source")
        .nargs(1)
        .default_value(std::string("."));

    parser.add_argument("-o", "--output")
        .help("output directory")
        .default_value(std::string("."))
        .nargs(1);

    parser.add_argument("-N", "--no-cache")
        .help("do not use cached files")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-j", "--jobs")
        .help("number of jobs to run simultaneously. 0 for auto")
        .default_value(0)
        .nargs(1)
        .scan<'u', uint32_t>();

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    auto &optimization_group = parser.add_mutually_exclusive_group();
    optimization_group.add_argument("-O", "--optimize")
        .help(
            "request optimization from build pipeline. not all pipelines will "
            "support this, and it may be ignored")
        .default_value(false)
        .implicit_value(true);

    optimization_group.add_argument("-Os", "--optimize-size")
        .help(
            "request size optimization from build pipeline. not all pipelines "
            "will support this, and it may be ignored")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-g", "--debug")
        .help(
            "request that the pipeline generate and preserve debug "
            "information. "
            "not all pipelines will support this, and it may be ignored")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-C", "--certify")
        .help(
            "digitally sign the output with the specified PKCS#12 certificate")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--certify-password")
        .help("password for the PKCS#12 certificate")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--supply-chain-insecure")
        .help(
            "do not verify OR require dependencies to be validly signed by a "
            "trusted source")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--trustkey")
        .help(
            "add a trusted public key fingerprint that may be used to verify "
            "dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--trustkeys")
        .help(
            "add a file containing trusted public key fingerprints that may be "
            "used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);
  }

  static void setup_argparse_clean(ArgumentParser &parser) {
    parser.add_argument("package-src").help("path to package source").nargs(1);

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);
  }

  static void setup_argparse_update(ArgumentParser &parser) {
    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--supply-chain-insecure")
        .help(
            "do not verify OR require dependencies to be validly signed by a "
            "trusted source")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--trustkey")
        .help(
            "add a trusted public key fingerprint that may be used to verify "
            "dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--trustkeys")
        .help(
            "add a file containing trusted public key fingerprints that may be "
            "used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("package-name")
        .help("name of package to update")
        .required()
        .remaining();
  }

  static void setup_argparse_install(ArgumentParser &parser) {
    parser.add_argument("src").help("source of package to install").nargs(1);

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-O", "--override")
        .help("override existing package")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-g", "--global")
        .help("install package globally (requires higher permissions)")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-n", "--no-build")
        .help("do not build package after downloading")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--supply-chain-insecure")
        .help(
            "do not verify OR require dependencies to be validly signed by a "
            "trusted source")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--trustkey")
        .help(
            "add a trusted public key fingerprint that may be used to verify "
            "dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--trustkeys")
        .help(
            "add a file containing trusted public key fingerprints that may be "
            "used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);
  }

  static void setup_argparse_doc(ArgumentParser &parser) {
    parser.add_argument("package-src")
        .help("name of package to document")
        .nargs(1);

    parser.add_argument("--html")
        .help("generate HTML report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--plain")
        .help("generate plain text report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--pdf")
        .help("generate PDF report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--json")
        .help("generate JSON report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--xml")
        .help("generate XML report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--reactjs")
        .help("generate ReactJS report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-o", "--output")
        .help("output directory")
        .default_value(std::string("."))
        .nargs(1);

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-r", "--recursive")
        .help("document all dependencies")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-d", "--depth")
        .help("maximum depth of dependency tree to document")
        .default_value((size_t)1)
        .nargs(1);

    parser.add_argument("-C", "--certify")
        .help(
            "digitally sign the documentation with the specified PKCS#12 "
            "certificate")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--certify-password")
        .help("password for the PKCS#12 certificate")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--supply-chain-insecure")
        .help(
            "do not verify OR require dependencies to be validly signed by a "
            "trusted source")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--trustkey")
        .help(
            "add a trusted public key fingerprint that may be used to verify "
            "dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--trustkeys")
        .help(
            "add a file containing trusted public key fingerprints that may be "
            "used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);
  }

  static void setup_argparse_format(ArgumentParser &parser) {
    parser.add_argument("package-src").help("path to package source").nargs(1);

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-r", "--recursive")
        .help("reformat all dependencies")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-d", "--depth")
        .help("maximum depth of dependency tree to reformat")
        .default_value(1)
        .nargs(1);
  }

  static void setup_argparse_list(ArgumentParser &parser) {
    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-p", "--packages")
        .help("list all packages installed")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-x", "--executables")
        .help("list all executables installed")
        .default_value(false)
        .implicit_value(true);
  }

  static void setup_argparse_test(ArgumentParser &parser) {
    parser.add_argument("package-name")
        .help("name of package to test")
        .nargs(1);

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-o", "--output", "--report-dir")
        .help("output directory for reports")
        .default_value(std::string("."))
        .nargs(1);

    parser.add_argument("--html")
        .help("generate HTML report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--plain")
        .help("generate plain text report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--pdf")
        .help("generate PDF report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--json")
        .help("generate JSON report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--xml")
        .help("generate XML report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--reactjs")
        .help("generate ReactJS report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-c, --coverage")
        .help("generate code coverage report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-p, --profile")
        .help("generate profiling report")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-r", "--recursive")
        .help("test all dependencies")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-d", "--depth")
        .help("maximum depth of dependency tree to test")
        .default_value((size_t)1)
        .nargs(1);

    parser.add_argument("--supply-chain-insecure")
        .help(
            "do not verify OR require dependencies to be validly signed by a "
            "trusted source")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--trustkey")
        .help(
            "add a trusted public key fingerprint that may be used to verify "
            "dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--trustkeys")
        .help(
            "add a file containing trusted public key fingerprints that may be "
            "used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("-C", "--certify")
        .help(
            "digitally sign the test reports with the specified PKCS#12 "
            "certificate")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--certify-password")
        .help("password for the PKCS#12 certificate")
        .default_value(std::string(""))
        .nargs(1);
  }

  static void setup_argparse_lsp(ArgumentParser &parser) {
    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-o", "--log")
        .default_value(std::string("no3-lsp.log"))
        .help("Specify the log file");

    parser.add_argument("--config")
        .default_value(std::string(""))
        .help("Specify the configuration file");

    auto &group = parser.add_mutually_exclusive_group();

    group.add_argument("--pipe").help("Specify the pipe file to connect to");
    group.add_argument("--port").help("Specify the port to connect to");
    group.add_argument("--stdio")
        .default_value(false)
        .implicit_value(true)
        .help("Use standard I/O");
  }

  static void setup_argparse_dev(
      ArgumentParser &parser,
      std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>>
          &subparsers) {
    /*================= CONFIG BASIC =================*/
    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    /*================== OTHER STUFF =================*/
    parser.add_argument("--demangle")
        .help("demangle Nitrate symbol names")
        .nargs(1);

    /*================= BENCH SUBPARSER =================*/
    auto bench = std::make_unique<ArgumentParser>("bench", "1.0",
                                                  default_arguments::help);

    bench->add_argument("-n", "--name")
        .choices("lexer", "parser", "nitrate-ir", "delta-ir", "llvm-ir",
                 "llvm-codegen", "c11-codegen", "pipeline")
        .help("name of benchmark to run");

    bench->add_argument("--list")
        .help("list available benchmarks")
        .default_value(false)
        .implicit_value(true);
    bench->add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    subparsers["bench"] = std::move(bench);

    /*================= PARSE SUBPARSER =================*/
    auto parse = std::make_unique<ArgumentParser>("parse", "1.0",
                                                  default_arguments::help);

    parse->add_argument("source").help("source file to parse").nargs(1);
    parse->add_argument("-o", "--output")
        .help("output file for parse tree")
        .default_value(std::string(""))
        .nargs(1);
    parse->add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    subparsers["parse"] = std::move(parse);

    /*================= NR SUBPARSER =================*/
    auto nr =
        std::make_unique<ArgumentParser>("nr", "1.0", default_arguments::help);

    nr->add_argument("source").help("source file to lower into NR").nargs(1);
    nr->add_argument("-o", "--output")
        .help("output file for nr tree")
        .default_value(std::string(""))
        .nargs(1);
    nr->add_argument("-O", "--opts")
        .help("optimizations to apply to NR")
        .default_value(std::string(""))
        .nargs(1);
    nr->add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    subparsers["nr"] = std::move(nr);

    /*================= CODEGEN SUBPARSER =================*/
    auto codegen = std::make_unique<ArgumentParser>("codegen", "1.0",
                                                    default_arguments::help);

    codegen->add_argument("source")
        .help("source file to generate code for")
        .nargs(1);
    codegen->add_argument("-o", "--output")
        .help("output file for generated code")
        .default_value(std::string(""))
        .nargs(1);
    codegen->add_argument("-O", "--opts")
        .help("optimizations to apply to codegen")
        .default_value(std::string(""))
        .nargs(1);
    codegen->add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);
    codegen->add_argument("-t", "--target")
        .help("Target to generate code for")
        .default_value(std::string("native"))
        .nargs(1);

    subparsers["codegen"] = std::move(codegen);

    /*================= TEST SUBPARSER =================*/
    auto test = std::make_unique<ArgumentParser>("test", "1.0",
                                                 default_arguments::help);

    test->add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    subparsers["test"] = std::move(test);

    parser.add_subparser(*subparsers["bench"]);
    parser.add_subparser(*subparsers["test"]);
    parser.add_subparser(*subparsers["parse"]);
    parser.add_subparser(*subparsers["nr"]);
    parser.add_subparser(*subparsers["codegen"]);
  }

  static ArgumentParser &setup_argparse(
      ArgumentParser &parser, ArgumentParser &init_parser,
      ArgumentParser &build_parser, ArgumentParser &clean_parser,
      ArgumentParser &update_parser, ArgumentParser &install_parser,
      ArgumentParser &doc_parser, ArgumentParser &format_parser,
      ArgumentParser &list_parser, ArgumentParser &test_parser,
      ArgumentParser &lsp_parser, ArgumentParser &dev_parser,
      std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>>
          &dev_subparsers) {
    using namespace argparse;

    setup_argparse_init(init_parser);
    setup_argparse_build(build_parser);
    setup_argparse_clean(clean_parser);
    setup_argparse_update(update_parser);
    setup_argparse_install(install_parser);
    setup_argparse_doc(doc_parser);
    setup_argparse_format(format_parser);
    setup_argparse_list(list_parser);
    setup_argparse_test(test_parser);
    setup_argparse_lsp(lsp_parser);
    setup_argparse_dev(dev_parser, dev_subparsers);

    parser.add_subparser(init_parser);
    parser.add_subparser(build_parser);
    parser.add_subparser(clean_parser);
    parser.add_subparser(update_parser);
    parser.add_subparser(install_parser);
    parser.add_subparser(doc_parser);
    parser.add_subparser(format_parser);
    parser.add_subparser(list_parser);
    parser.add_subparser(test_parser);
    parser.add_subparser(lsp_parser);
    parser.add_subparser(dev_parser);

    parser.add_argument("--license")
        .help("show license information")
        .default_value(false)
        .implicit_value(true);

    return parser;
  }

  static std::string no3_deps_version_string() {
    /* FIXME: Automate setting of 'is stable build' flag */

#define NO3_STABLE false

    std::stringstream ss;

    std::array<std::string_view, 6> NO3_DEPS = {
        ncc::core::CoreLibrary::GetVersion(),
        ncc::lex::LexerLibrarySetup::GetVersionId(),
        qprep_lib_version(),
        ncc::parse::ParseLibrary::GetVersion(),
        nr_lib_version(),
        qcode_lib_version()};

    ss << "{\"ver\":\"" << __TARGET_VERSION
       << "\",\"stable\":" << (NO3_STABLE ? "true" : "false") << ",\"using\":[";
    for (size_t i = 0; i < NO3_DEPS.size(); i++) {
      ss << "\"" << NO3_DEPS[i] << "\"";
      if (i < NO3_DEPS.size() - 1) ss << ",";
    }
    ss << "]}";

    return ss.str();
  }

  ArgumentParser init_parser("init", "1.0", default_arguments::help);
  ArgumentParser build_parser("build", "1.0", default_arguments::help);
  ArgumentParser clean_parser("clean", "1.0", default_arguments::help);
  ArgumentParser update_parser("update", "1.0", default_arguments::help);
  ArgumentParser install_parser("install", "1.0", default_arguments::help);
  ArgumentParser doc_parser("doc", "1.0", default_arguments::help);
  ArgumentParser format_parser("format", "1.0", default_arguments::help);
  ArgumentParser list_parser("list", "1.0", default_arguments::help);
  ArgumentParser test_parser("test", "1.0", default_arguments::help);
  ArgumentParser lsp_parser("lsp", "1.0", default_arguments::help);
  ArgumentParser dev_parser("dev", "1.0", default_arguments::help);
  std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>>
      dev_subparsers;

  static ArgumentParser argument_parser("no3", no3_deps_version_string());

  // Setup parsing

  ArgumentParser &program = setup_argparse(
      argument_parser, init_parser, build_parser, clean_parser, update_parser,
      install_parser, doc_parser, format_parser, list_parser, test_parser,
      lsp_parser, dev_parser, dev_subparsers);
}  // namespace no3::argparse_setup
