////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///           ░▒▓██████▓▒░░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░                  ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓███████▓▒░░▒▓███████▓▒░░▒▓█▓▒▒▓███▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///           ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///             ░▒▓█▓▒░                                                      ///
///              ░▒▓██▓▒░                                                    ///
///                                                                          ///
///   * QUIX PACKAGE MANAGER - The official tool for the Quix language.      ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <argparse.h>
#include <glog/logging.h>
#include <quix-codegen/Code.h>
#include <quix-codegen/Lib.h>
#include <quix-core/Lib.h>
#include <quix-ir/Lib.h>
#include <quix-lexer/Lib.h>
#include <quix-parser/Lib.h>
#include <quix-prep/Lib.h>
#include <quixd/quixd.h>

#include <clean/Cleanup.hh>
#include <core/Config.hh>
#include <core/Logger.hh>
#include <fstream>
#include <init/Package.hh>
#include <ios>
#include <mutex>
#include <quix-codegen/Classes.hh>
#include <quix-core/Classes.hh>
#include <quix-ir/Classes.hh>
#include <quix-ir/Format.hh>
#include <quix-parser/Classes.hh>
#include <quix-prep/Classes.hh>
#include <string_view>
#include <unordered_map>

// #include <dev/bench/bench.hh>
// #include <dev/test/test.hh>
// #include <run/RunScript.hh>

#include <install/Install.hh>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

struct QPKGMode {
  bool use_color;
};

thread_local std::ostream &qout = std::cout;
thread_local std::ostream &qerr = std::cerr;

static qpkg::core::MyLogSink g_custom_log_sink;

static std::optional<std::string> quixcc_cc_demangle(std::string_view mangled_name) {
  if (mangled_name.starts_with("@")) {
    mangled_name.remove_prefix(1);
  }

  qxir::SymbolEncoding codec;
  return codec.demangle_name(mangled_name);
}

static std::string qpkg_deps_version_string() {
#define QPKG_STABLE false /* FIXME: Automate setting of 'is stable build' flag */

  std::stringstream ss;

  std::array<std::string_view, 6> QPKG_DEPS = {qcore_lib_version(), qlex_lib_version(),
                                               qprep_lib_version(), qparse_lib_version(),
                                               qxir_lib_version(),  qcode_lib_version()};

  ss << "{\"ver\":\"" << __TARGET_VERSION << "\",\"stable\":" << (QPKG_STABLE ? "true" : "false")
     << ",\"using\":[";
  for (size_t i = 0; i < QPKG_DEPS.size(); i++) {
    ss << "\"" << QPKG_DEPS[i] << "\"";
    if (i < QPKG_DEPS.size() - 1) ss << ",";
  }
  ss << "]}";

  return ss.str();
}

constexpr const char *FULL_LICENSE =
    R"(This file is part of QUIX Compiler Suite.
Copyright (C) 2024 Wesley C. Jones

The QUIX Compiler Suite is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.)";

using namespace argparse;

namespace argparse_setup {
  void setup_argparse_init(ArgumentParser &parser) {
    parser.add_argument("package-name").help("name of package to initialize").nargs(1);

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

  void setup_argparse_build(ArgumentParser &parser) {
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
            "request that the pipeline generate and preserve debug information. "
            "not all pipelines will support this, and it may be ignored")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-C", "--certify")
        .help("digitally sign the output with the specified PKCS#12 certificate")
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

  void setup_argparse_clean(ArgumentParser &parser) {
    parser.add_argument("package-src").help("path to package source").nargs(1);

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);
  }

  void setup_argparse_update(ArgumentParser &parser) {
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

    parser.add_argument("package-name").help("name of package to update").required().remaining();
  }

  void setup_argparse_install(ArgumentParser &parser) {
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

  void setup_argparse_doc(ArgumentParser &parser) {
    parser.add_argument("package-src").help("name of package to document").nargs(1);

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

  void setup_argparse_format(ArgumentParser &parser) {
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

  void setup_argparse_list(ArgumentParser &parser) {
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

  void setup_argparse_test(ArgumentParser &parser) {
    parser.add_argument("package-name").help("name of package to test").nargs(1);

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

  void setup_argparse_lsp(ArgumentParser &parser) {
    parser.add_argument("--license")
        .default_value(false)
        .implicit_value(true)
        .help("Print the LSP software license");

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-o", "--log")
        .default_value(std::string("quixd-lsp.log"))
        .help("Specify the log file");

    parser.add_argument("--config")
        .default_value(std::string(""))
        .help("Specify the configuration file");

    auto &group = parser.add_mutually_exclusive_group();

    group.add_argument("--pipe").help("Specify the pipe file to connect to");
    group.add_argument("--port").help("Specify the port to connect to");
    group.add_argument("--stdio").default_value(false).implicit_value(true).help(
        "Use standard I/O");
  }

  void setup_argparse_dev(
      ArgumentParser &parser,
      std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>> &subparsers) {
    /*================= CONFIG BASIC =================*/
    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    /*================== OTHER STUFF =================*/
    parser.add_argument("--demangle").help("demangle QUIX symbol names").nargs(1);

    /*================= BENCH SUBPARSER =================*/
    auto bench = std::make_unique<ArgumentParser>("bench", "1.0", default_arguments::help);

    bench->add_argument("-n", "--name")
        .choices("lexer", "parser", "quix-ir", "delta-ir", "llvm-ir", "llvm-codegen", "c11-codegen",
                 "pipeline")
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

    /*================= QPARSE SUBPARSER =================*/
    auto parse = std::make_unique<ArgumentParser>("parse", "1.0", default_arguments::help);

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

    /*================= QXIR SUBPARSER =================*/
    auto qxir = std::make_unique<ArgumentParser>("qxir", "1.0", default_arguments::help);

    qxir->add_argument("source").help("source file to lower into QXIR").nargs(1);
    qxir->add_argument("-o", "--output")
        .help("output file for qxir tree")
        .default_value(std::string(""))
        .nargs(1);
    qxir->add_argument("-O", "--opts")
        .help("optimizations to apply to QXIR")
        .default_value(std::string(""))
        .nargs(1);
    qxir->add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    subparsers["qxir"] = std::move(qxir);

    /*================= CODEGEN SUBPARSER =================*/
    auto codegen = std::make_unique<ArgumentParser>("codegen", "1.0", default_arguments::help);

    codegen->add_argument("source").help("source file to generate code for").nargs(1);
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
    auto test = std::make_unique<ArgumentParser>("test", "1.0", default_arguments::help);

    test->add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    subparsers["test"] = std::move(test);

    parser.add_subparser(*subparsers["bench"]);
    parser.add_subparser(*subparsers["test"]);
    parser.add_subparser(*subparsers["parse"]);
    parser.add_subparser(*subparsers["qxir"]);
    parser.add_subparser(*subparsers["codegen"]);
  }

  void setup_argparse(
      ArgumentParser &parser, ArgumentParser &init_parser, ArgumentParser &build_parser,
      ArgumentParser &clean_parser, ArgumentParser &update_parser, ArgumentParser &install_parser,
      ArgumentParser &doc_parser, ArgumentParser &format_parser, ArgumentParser &list_parser,
      ArgumentParser &test_parser, ArgumentParser &lsp_parser, ArgumentParser &dev_parser,
      std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>> &dev_subparsers) {
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
  }

}  // namespace argparse_setup

namespace qpkg::router {
  int run_init_mode(const ArgumentParser &parser, const QPKGMode &) {
    using namespace init;

    core::SetDebugMode(parser["--verbose"] == true);

    PackageBuilder builder = PackageBuilder()
                                 .output(parser.get<std::string>("--output"))
                                 .name(parser.get<std::string>("package-name"))
                                 .license(parser.get<std::string>("--license"))
                                 .author(parser.get<std::string>("--author"))
                                 .email(parser.get<std::string>("--email"))
                                 .url(parser.get<std::string>("--url"))
                                 .version(parser.get<std::string>("--version"))
                                 .description(parser.get<std::string>("--description"))
                                 .verbose(parser["--verbose"] == true)
                                 .force(parser["--force"] == true);

    if (parser.get<std::string>("--type") == "program")
      builder.type(PackageType::PROGRAM);
    else if (parser.get<std::string>("--type") == "staticlib")
      builder.type(PackageType::STATICLIB);
    else if (parser.get<std::string>("--type") == "sharedlib")
      builder.type(PackageType::SHAREDLIB);

    return builder.build().create() ? 0 : -1;
  }

  int run_build_mode(const ArgumentParser &parser, const QPKGMode &) {
    (void)parser;

    qerr << "build not implemented yet" << std::endl;
    return 1;

    // qpkg::core::SetDebugMode(parser["--verbose"] == true);

    // qpkg::build::EngineBuilder builder;

    // builder.set_package_src(parser.get<std::string>("package-src"));

    // if (parser.is_used("--output")) builder.set_output(parser.get<std::string>("--output"));

    // if (parser["--no-cache"] == true) builder.disable_cache();

    // if (parser.is_used("--jobs")) builder.jobs(parser.get<uint32_t>("--jobs"));

    // if (parser["--verbose"] == true) builder.verbose();

    // if (parser["--optimize"] == true) builder.optimize();

    // if (parser["--optimize-size"] == true) builder.optimize_size();

    // if (parser["--debug"] == true) builder.debug();

    // if (parser.is_used("--certify")) builder.certify(parser.get<std::string>("--certify"));

    // if (parser.is_used("--certify-password"))
    //   builder.certify_password(parser.get<std::string>("--certify-password"));

    // if (parser["--supply-chain-insecure"] == true) builder.disable_sigcheck();

    // if (parser.is_used("--trustkey")) builder.trustkey(parser.get<std::string>("--trustkey"));

    // if (parser.is_used("--trustkeys")) builder.trustkeys(parser.get<std::string>("--trustkeys"));

    // auto engine = builder.build();
    // if (!engine) {
    //   qerr << "Failed to construct engine" << std::endl;
    //   return -1;
    // }

    // return engine->run() ? 0 : -1;
  }

  int run_clean_mode(const ArgumentParser &parser, const QPKGMode &) {
    core::SetDebugMode(parser["--verbose"] == true);

    if (clean::CleanPackageSource(parser.get<std::string>("package-src"),
                                  parser["--verbose"] == true)) {
      return 0;
    } else {
      return -1;
    }
  }

  int run_update_mode(const ArgumentParser &parser, const QPKGMode &) {
    core::SetDebugMode(parser["--verbose"] == true);

    (void)parser;
    qerr << "update not implemented yet" << std::endl;
    return 1;
  }

  int run_install_mode(const ArgumentParser &parser, const QPKGMode &) {
    (void)parser;

    qerr << "install not implemented yet" << std::endl;
    return 1;

    // core::SetDebugMode(parser["--verbose"] == true);

    // std::string url = parser.get<std::string>("src");
    // std::string dest, package_name;
    // bool overwrite = parser["--override"] == true;
    // bool global = parser["--global"] == true;

    // if (global) {
    //   /// TODO: fix platform-specific global install
    //   try {
    //     std::filesystem::create_directories(QPKG_GLOBAL_PACKAGE_DIR);
    //   } catch (std::filesystem::filesystem_error &e) {
    //     qerr << e.what() << std::endl;
    //     qerr << "Try running with higher permissions" << std::endl;
    //     return -1;
    //   }
    //   dest = QPKG_GLOBAL_PACKAGE_DIR;
    // } else {
    //   dest = ".";
    // }

    // if (!install::install_from_url(url, dest, package_name, overwrite)) {
    //   return -1;
    // }

    // if (parser["--no-build"] == true) return 0;

    // build::EngineBuilder builder;
    // std::filesystem::path dest_path = dest + "/" + package_name;
    // builder.set_package_src(dest_path.string());

    // if (global) {
    //   try {
    //     std::filesystem::path app_file = QPKG_GLOBAL_BINARY_DIR;
    //     app_file /= package_name;

    //     std::filesystem::create_directories(QPKG_GLOBAL_BINARY_DIR);

    //     builder.set_output(app_file.string());
    //   } catch (std::filesystem::filesystem_error &e) {
    //     qerr << e.what() << std::endl;
    //     qerr << "Try running with higher permissions" << std::endl;
    //     return -1;
    //   }
    // }

    // auto engine = builder.build();
    // if (!engine) {
    //   qerr << "Failed to construct engine" << std::endl;
    //   return -1;
    // }

    // if (!engine->run()) {
    //   qerr << "Failed to build package" << std::endl;
    //   return -1;
    // }

    // if (!global) {
    //   qout << "Package installed to: " << dest_path << std::endl;
    //   return 0;
    // }

    // return 0;
  }

  int run_doc_mode(const ArgumentParser &parser, const QPKGMode &) {
    enum class OFormat { Html, Plain, Pdf, Json, Xml, ReactJS } oformat;

    core::SetDebugMode(parser["--verbose"] == true);

    bool html = parser["--html"] == true;
    bool plain = parser["--plain"] == true;
    bool pdf = parser["--pdf"] == true;
    bool json = parser["--json"] == true;
    bool xml = parser["--xml"] == true;
    bool reactjs = parser["--reactjs"] == true;

    std::string output = parser.get<std::string>("--output");
    bool verbose = parser["--verbose"] == true;
    bool recursive = parser["--recursive"] == true;
    size_t depth = parser.get<size_t>("--depth");
    std::string package_src = parser.get<std::string>("package-src");

    if ((html + plain + pdf + json + xml + reactjs) != 1) {
      qerr << "Exactly one output format must be specified" << std::endl;
      return -1;
    }

    (void)output;
    (void)verbose;
    (void)recursive;
    (void)depth;
    (void)package_src;

    if (html) oformat = OFormat::Html;
    if (plain) oformat = OFormat::Plain;
    if (pdf) oformat = OFormat::Pdf;
    if (json) oformat = OFormat::Json;
    if (xml) oformat = OFormat::Xml;
    if (reactjs) oformat = OFormat::ReactJS;

    switch (oformat) {
      case OFormat::Html:
        qerr << "HTML not implemented yet" << std::endl;
        return 1;
      case OFormat::Plain:
        qerr << "Plain not implemented yet" << std::endl;
        return 1;
      case OFormat::Pdf:
        qerr << "PDF not implemented yet" << std::endl;
        return 1;
      case OFormat::Json:
        qerr << "JSON not implemented yet" << std::endl;
        return 1;
      case OFormat::Xml:
        qerr << "XML not implemented yet" << std::endl;
        return 1;
      case OFormat::ReactJS:
        qerr << "ReactJS not implemented yet" << std::endl;
        return 1;

      default:
        qerr << "Unknown output format" << std::endl;
        return -1;
    }
  }

  int run_format_mode(const ArgumentParser &parser, const QPKGMode &) {
    core::SetDebugMode(parser["--verbose"] == true);

    (void)parser;
    qerr << "format not implemented yet" << std::endl;
    return 1;
  }

  int run_list_mode(const ArgumentParser &parser, const QPKGMode &) {
    core::SetDebugMode(parser["--verbose"] == true);

    (void)parser;
    qerr << "list not implemented yet" << std::endl;
    return 1;
  }

  int run_run_mode(const std::vector<std::string> &args, const QPKGMode &) {
    (void)args;

    qerr << "run not implemented yet" << std::endl;
    return 1;

    // core::FormatAdapter::PluginAndInit(false, mode.use_color);

    // if (args.size() < 1) {
    //   qerr << "No script specified" << std::endl;
    //   return -1;
    // }

    // run::RunScript script(args[0]);
    // if (!script.is_okay()) return -1;

    // return script.run(args);
  }

  int run_test_mode(const ArgumentParser &parser, const QPKGMode &) {
    core::SetDebugMode(parser["--verbose"] == true);

    (void)parser;
    qerr << "test not implemented yet" << std::endl;
    return 1;
  }

  int run_lsp_mode(const ArgumentParser &parser, const QPKGMode &) {
    core::SetDebugMode(parser["--verbose"] == true);

    std::vector<std::string> args;
    args.push_back("quixd");

    if (parser["--license"] == true) {
      args.push_back("--license");
    }

    if (parser.is_used("--config")) {
      args.push_back("--config");
      args.push_back(parser.get<std::string>("--config"));
    }

    if (parser.is_used("--log")) {
      args.push_back("--log");
      args.push_back(parser.get<std::string>("--log"));
    }

    if (parser.is_used("--pipe")) {
      args.push_back("--pipe");
      args.push_back(parser.get<std::string>("--pipe"));
    } else if (parser.is_used("--port")) {
      args.push_back("--port");
      args.push_back(parser.get<std::string>("--port"));
    } else if (parser["--stdio"] == true) {
      args.push_back("--stdio");
    }

    std::vector<char *> c_args;
    c_args.reserve(args.size());

    std::string inner_command;
    for (size_t i = 0; i < args.size(); i++) {
      c_args.push_back(args[i].data());
      inner_command += args[i];

      if (i != args.size() - 1) {
        inner_command += " ";
      }
    }
    LOG(INFO) << "Invoking LSP: \"" << inner_command << "\"";

    google::RemoveLogSink(&g_custom_log_sink);
    int ret = quixd_main(args.size(), c_args.data());
    google::AddLogSink(&g_custom_log_sink);
    return ret;
  }

  namespace dev::bench {
    int run_benchmark_lexer() {
      /// TODO: implement
      qerr << "benchmark lexer not implemented yet" << std::endl;
      return 1;
    }

    int run_benchmark_parser() {
      /// TODO: implement
      qerr << "benchmark parser not implemented yet" << std::endl;
      return 1;
    }

    int run_benchmark_quix_ir() {
      /// TODO: implement
      qerr << "benchmark quix-ir not implemented yet" << std::endl;
      return 1;
    }

    int run_benchmark_delta_ir() {
      /// TODO: implement
      qerr << "benchmark delta-ir not implemented yet" << std::endl;
      return 1;
    }

    int run_benchmark_llvm_ir() {
      /// TODO: implement
      qerr << "benchmark llvm-ir not implemented yet" << std::endl;
      return 1;
    }

    int run_benchmark_llvm_codegen() {
      /// TODO: implement
      qerr << "benchmark llvm-codegen not implemented yet" << std::endl;
      return 1;
    }

    int run_benchmark_c11_codegen() {
      /// TODO: implement
      qerr << "benchmark c11-codegen not implemented yet" << std::endl;
      return 1;
    }

    int run_benchmark_pipeline() {
      /// TODO: implement
      qerr << "benchmark pipeline not implemented yet" << std::endl;
      return 1;
    }
  }  // namespace dev::bench

  namespace dev::test {
    int run_tests() {
      /// TODO: implement tests
      qerr << "test not implemented yet" << std::endl;
      return 1;
    }
  }  // namespace dev::test

  int run_dev_mode(
      const ArgumentParser &parser,
      const std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>> &subparsers,
      const QPKGMode &mode) {
    if (parser.is_subcommand_used("bench")) {
      enum class Benchmark {
        LEXER,
        PARSER,
        Q_IR,
        DELTA_IR,
        LLVM_IR,
        LLVM_CODEGEN,
        C11_CODEGEN,
        PIPELINE
      };

      auto &bench_parser = *subparsers.at("bench");

      core::SetColorMode(mode.use_color);
      core::SetDebugMode(bench_parser["--verbose"] == true);

      if (bench_parser["--list"] == true) {
        qout << "Available benchmarks:" << std::endl;
        qout << "  lexer" << std::endl;
        qout << "  parser" << std::endl;
        qout << "  quix-ir" << std::endl;
        qout << "  delta-ir" << std::endl;
        qout << "  llvm-ir" << std::endl;
        qout << "  llvm-codegen" << std::endl;
        qout << "  c11-codegen" << std::endl;
        qout << "  pipeline" << std::endl;
        return 0;
      }

      if (!bench_parser.is_used("--name")) {
        qerr << "No benchmark specified" << std::endl;
        qerr << bench_parser;
        return 1;
      }

      Benchmark bench_type;

      std::string bench_name = bench_parser.get<std::string>("--name");

      if (bench_name == "lexer")
        bench_type = Benchmark::LEXER;
      else if (bench_name == "parser")
        bench_type = Benchmark::PARSER;
      else if (bench_name == "quix-ir")
        bench_type = Benchmark::Q_IR;
      else if (bench_name == "delta-ir")
        bench_type = Benchmark::DELTA_IR;
      else if (bench_name == "llvm-ir")
        bench_type = Benchmark::LLVM_IR;
      else if (bench_name == "llvm-codegen")
        bench_type = Benchmark::LLVM_CODEGEN;
      else if (bench_name == "c11-codegen")
        bench_type = Benchmark::C11_CODEGEN;
      else if (bench_name == "pipeline")
        bench_type = Benchmark::PIPELINE;
      else {
        qerr << "Unknown benchmark specified" << std::endl;
        qerr << bench_parser;
        return 1;
      }

      switch (bench_type) {
        case Benchmark::LEXER:
          return dev::bench::run_benchmark_lexer();
        case Benchmark::PARSER:
          return dev::bench::run_benchmark_parser();
        case Benchmark::Q_IR:
          return dev::bench::run_benchmark_quix_ir();
        case Benchmark::DELTA_IR:
          return dev::bench::run_benchmark_delta_ir();
        case Benchmark::LLVM_IR:
          return dev::bench::run_benchmark_llvm_ir();
        case Benchmark::LLVM_CODEGEN:
          return dev::bench::run_benchmark_llvm_codegen();
        case Benchmark::C11_CODEGEN:
          return dev::bench::run_benchmark_c11_codegen();
        case Benchmark::PIPELINE:
          return dev::bench::run_benchmark_pipeline();
        default:
          qerr << "Unknown benchmark name: " << bench_name << std::endl;
          return 1;
      }

      return 0;
    } else if (parser.is_subcommand_used("test")) {
      auto &test_parser = *subparsers.at("test");
      core::SetColorMode(mode.use_color);
      core::SetDebugMode(test_parser["--verbose"] == true);

      return dev::test::run_tests();
    } else if (parser.is_subcommand_used("parse")) {
      auto &parse_parser = *subparsers.at("parse");
      core::SetColorMode(mode.use_color);
      core::SetDebugMode(parse_parser["--verbose"] == true);

      std::string source = parse_parser.get<std::string>("source");
      std::string output = parse_parser.get<std::string>("--output");

      auto fp = std::make_shared<std::ifstream>(source, std::ios_base::in | std::ios_base::binary);
      if (!fp->is_open()) {
        qerr << "Failed to open source file" << std::endl;
        return 1;
      }

      qcore_env env;

      qprep lexer(fp, source.c_str(), env.get());

      qparse_conf pconf;
      qparser ctx(lexer.get(), pconf.get(), env.get());

      qparse_node_t *root = nullptr;
      if (!qparse_do(ctx.get(), &root)) {
        auto cb = [](const char *msg, size_t size, uintptr_t data) {
          (void)size;
          (void)data;
          qerr << msg << std::endl;
        };

        qparse_dumps(ctx.get(), false, cb, 0);
        qerr << "Failed to parse source" << std::endl;
        return 1;
      }

      size_t out_len = 0;
      char *out_str = qparse_repr(root, false, 2, &out_len);
      if (!out_str) {
        qerr << "Failed to generate parse tree" << std::endl;
        return 1;
      }

      FILE *out_fp = nullptr;
      if (!output.empty()) {
        out_fp = fopen(output.c_str(), "w");
        if (!out_fp) {
          qerr << "Failed to open output file" << std::endl;
          return 1;
        }
      } else {
        out_fp = stdout;
      }

      fwrite(out_str, 1, out_len, out_fp);
      free(out_str);

      if (!output.empty()) fclose(out_fp);

      return 0;
    } else if (parser.is_subcommand_used("qxir")) {
      auto &qxir_parser = *subparsers.at("qxir");

      core::SetColorMode(mode.use_color);
      core::SetDebugMode(qxir_parser["--verbose"] == true);

      std::string source = qxir_parser.get<std::string>("source");
      std::string output = qxir_parser.get<std::string>("--output");
      std::string opts = qxir_parser.get<std::string>("--opts");
      bool verbose = qxir_parser["--verbose"] == true;

      auto fp = std::make_shared<std::ifstream>(source, std::ios_base::in | std::ios_base::binary);
      if (!fp->is_open()) {
        qerr << "Failed to open source file" << std::endl;
        return 1;
      }

      qcore_env env;

      qprep lexer(fp, source.c_str(), env.get());
      qparse_conf pconf;
      qparser ctx(lexer.get(), pconf.get(), env.get());

      qparse_node_t *root = nullptr;
      if (!qparse_do(ctx.get(), &root)) {
        auto cb = [](const char *msg, size_t size, uintptr_t data) {
          (void)size;
          (void)data;
          qerr << msg << std::endl;
        };

        qparse_dumps(ctx.get(), false, cb, 0);
        qerr << "Failed to parse source" << std::endl;
        return 1;
      }

      qxir_conf conf;
      qmodule qmod(lexer.get(), conf.get(), source.c_str());

      auto cb = [](const uint8_t *msg, size_t size, qxir_level_t lvl, uintptr_t data) {
        if (!data && lvl < QXIR_LEVEL_INFO) {
          return;
        }
        qerr << std::string_view((const char *)msg, size) << std::endl;
      };

      if (!qxir_lower(qmod.get(), root, true)) {
        qxir_diag_read(qmod.get(), QXIR_AUDIT_ALL,
                       mode.use_color ? QXIR_DIAG_COLOR : QXIR_DIAG_NOCOLOR, cb, verbose);
        return 1;
      }

      qxir_diag_read(qmod.get(), QXIR_AUDIT_ALL,
                     mode.use_color ? QXIR_DIAG_COLOR : QXIR_DIAG_NOCOLOR, cb, verbose);

      FILE *out_fp = nullptr;
      if (!output.empty()) {
        out_fp = fopen(output.c_str(), "w");
        if (!out_fp) {
          qerr << "Failed to open output file" << std::endl;
          return 1;
        }
      } else {
        out_fp = stdout;
      }

      if (!qxir_write(qxir_base(qmod.get()), QXIR_SERIAL_CODE, out_fp, nullptr, 0)) {
        if (!output.empty()) fclose(out_fp);
        qerr << "Failed to generate QXIR tree" << std::endl;
        return 1;
      }

      if (!output.empty()) fclose(out_fp);

      return 0;
    } else if (parser.is_subcommand_used("codegen")) {
      auto &qxir_parser = *subparsers.at("codegen");

      core::SetColorMode(mode.use_color);
      core::SetDebugMode(qxir_parser["--verbose"] == true);

      std::string source = qxir_parser.get<std::string>("source");
      std::string output = qxir_parser.get<std::string>("--output");
      std::string opts = qxir_parser.get<std::string>("--opts");
      bool verbose = qxir_parser["--verbose"] == true;
      std::string target = qxir_parser.get<std::string>("--target");

      auto fp = std::make_shared<std::ifstream>(source, std::ios_base::in | std::ios_base::binary);
      if (!fp->is_open()) {
        qerr << "Failed to open source file" << std::endl;
        return 1;
      }

      qcore_env env;

      qprep lexer(fp, source.c_str(), env.get());
      qparse_conf pconf;
      qparser ctx(lexer.get(), pconf.get(), env.get());

      qcore_arena arena;
      qparse_node_t *root = nullptr;
      if (!qparse_do(ctx.get(), &root)) {
        auto cb = [](const char *msg, size_t size, uintptr_t data) {
          (void)size;
          (void)data;
          qerr << msg << std::endl;
        };

        qparse_dumps(ctx.get(), false, cb, 0);
        qerr << "Failed to parse source" << std::endl;
        return 1;
      }

      qxir_conf conf;
      qmodule qmod(lexer.get(), conf.get(), source.c_str());

      auto cb = [](const uint8_t *msg, size_t size, qxir_level_t lvl, uintptr_t data) {
        if (!data && lvl < QXIR_LEVEL_INFO) {
          return;
        }
        qerr << std::string_view((const char *)msg, size) << std::endl;
      };

      if (!qxir_lower(qmod.get(), root, true)) {
        qxir_diag_read(qmod.get(), QXIR_AUDIT_ALL,
                       mode.use_color ? QXIR_DIAG_COLOR : QXIR_DIAG_NOCOLOR, cb, verbose);
        return 1;
      }

      qxir_diag_read(qmod.get(), QXIR_AUDIT_ALL,
                     mode.use_color ? QXIR_DIAG_COLOR : QXIR_DIAG_NOCOLOR, cb, verbose);

      FILE *out_fp = nullptr;
      if (!output.empty()) {
        out_fp = fopen(output.c_str(), "w");
        if (!out_fp) {
          qerr << "Failed to open output file" << std::endl;
          return 1;
        }
      } else {
        out_fp = stdout;
      }

      static const std::unordered_map<std::string, qcode_lang_t> target_map = {
          {"c11", QCODE_C11},   {"c++", QCODE_CXX11},       {"ts", QCODE_TS},
          {"rust", QCODE_RUST}, {"python3", QCODE_PYTHON3}, {"csharp", QCODE_CSHARP}};

      qcode_conf qcode_conf;

      if (target_map.contains(target)) {
        if (!qcode_transcode(qmod.get(), qcode_conf.get(), target_map.at(target), QCODE_GOOGLE,
                             nullptr, out_fp)) {
          if (!output.empty()) fclose(out_fp);
          qerr << "Failed to generate code" << std::endl;
          return 1;
        }
      } else {
        if (target == "ir") {
          if (!qcode_ir(qmod.get(), qcode_conf.get(), stderr, out_fp)) {
            if (!output.empty()) fclose(out_fp);
            qerr << "Failed to generate code" << std::endl;
            return 1;
          }
        } else if (target == "asm") {
          if (!qcode_asm(qmod.get(), qcode_conf.get(), stderr, out_fp)) {
            if (!output.empty()) fclose(out_fp);
            qerr << "Failed to generate code" << std::endl;
            return 1;
          }
        } else if (target == "obj") {
          if (!qcode_obj(qmod.get(), qcode_conf.get(), stderr, out_fp)) {
            if (!output.empty()) fclose(out_fp);
            qerr << "Failed to generate code" << std::endl;
            return 1;
          }
        } else {
          qerr << "Unknown target specified" << std::endl;
        }
      }

      if (!output.empty()) fclose(out_fp);

      return 0;
    } else if (parser.is_used("--demangle")) {
      std::string input = parser.get<std::string>("--demangle");
      auto demangled_name = quixcc_cc_demangle(input.c_str());
      if (!demangled_name) {
        qerr << "Failed to demangle symbol" << std::endl;
        return 1;
      }

      qout << demangled_name.value() << std::endl;
      return 0;
    }

    qerr << "Unknown subcommand for dev" << std::endl;
    qerr << parser;

    return 1;
  }
}  // namespace qpkg::router

static bool do_libs_init() {
  if (!qcore_lib_init()) {
    qerr << "Failed to initialize QUIX-CORE library" << std::endl;
    return false;
  }

  if (!qlex_lib_init()) {
    qerr << "Failed to initialize QUIX-LEX library" << std::endl;
    return false;
  }

  if (!qprep_lib_init()) {
    qerr << "Failed to initialize QUIX-PREP library" << std::endl;
    return false;
  }

  if (!qparse_lib_init()) {
    qerr << "Failed to initialize QUIX-PARSE library" << std::endl;
    return false;
  }

  if (!qxir_lib_init()) {
    qerr << "Failed to initialize QUIX-IR library" << std::endl;
    return false;
  }

  if (!qcode_lib_init()) {
    qerr << "Failed to initialize QUIX-CODE library" << std::endl;
    return false;
  }

  return true;
}

static void do_libs_deinit() {
  qcode_lib_deinit();
  qxir_lib_deinit();
  qparse_lib_deinit();
  qprep_lib_deinit();
  qlex_lib_deinit();
  qcore_lib_deinit();
}

extern "C" __attribute__((visibility("default"))) int qpkg_command(int32_t argc, char *argv[],
                                                                   bool use_color) {
  QPKGMode mode;
  mode.use_color = use_color;
  qpkg::core::SetColorMode(mode.use_color);
  qpkg::core::SetDebugMode(false);

  ///===========================================================================
  /// BEGIN: Setup argument parsing and logging
  std::vector<std::string> args(argv, argv + argc);

  /* Configure Google logger */
  google::AddLogSink(&g_custom_log_sink);

  static ArgumentParser init_parser("init", "1.0", default_arguments::help);
  static ArgumentParser build_parser("build", "1.0", default_arguments::help);
  static ArgumentParser clean_parser("clean", "1.0", default_arguments::help);
  static ArgumentParser update_parser("update", "1.0", default_arguments::help);
  static ArgumentParser install_parser("install", "1.0", default_arguments::help);
  static ArgumentParser doc_parser("doc", "1.0", default_arguments::help);
  static ArgumentParser format_parser("format", "1.0", default_arguments::help);
  static ArgumentParser list_parser("list", "1.0", default_arguments::help);
  static ArgumentParser test_parser("test", "1.0", default_arguments::help);
  static ArgumentParser lsp_parser("lsp", "1.0", default_arguments::help);
  static ArgumentParser dev_parser("dev", "1.0", default_arguments::help);
  static std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>> dev_subparsers;
  static ArgumentParser program("qpkg", qpkg_deps_version_string());

  { /* Configure argument parser instances once */
    static std::once_flag parsers_inited;
    std::call_once(parsers_inited, [&]() {
      argparse_setup::setup_argparse(program, init_parser, build_parser, clean_parser,
                                     update_parser, install_parser, doc_parser, format_parser,
                                     list_parser, test_parser, lsp_parser, dev_parser,
                                     dev_subparsers);
    });
  }
  /// END: Setup argument parsing and logging
  ///===========================================================================

  ///===========================================================================
  /// BEGIN: Initialize dependencies
  class AutomaticLibInit {
  public:
    AutomaticLibInit() = default;
    bool do_init() { return do_libs_init(); }
    ~AutomaticLibInit() { do_libs_deinit(); }
  };

  AutomaticLibInit lib_init;
  if (!lib_init.do_init()) {
    qerr << "Failed to initialize libraries" << std::endl;
    return -1;
  }
  /// END: Initialize dependencies
  ///===========================================================================

  { /* Handle edge case for run scripts */
    if (args.size() >= 2 && args[1] == "run") {
      std::vector<std::string> run_args(args.begin() + 2, args.end());
      return qpkg::router::run_run_mode(run_args, mode);
    }
  }

  { /* Parse arguments */
    try {
      program.parse_args(args);
    } catch (const std::runtime_error &err) {
      qerr << err.what() << std::endl;
      qerr << program;
      return 1;
    }
  }

  { /* Conduct the routing */
    if (program["--license"] == true) {
      qout << FULL_LICENSE << std::endl;
      return 0;
    } else if (program.is_subcommand_used("init")) {
      return qpkg::router::run_init_mode(init_parser, mode);
    } else if (program.is_subcommand_used("build")) {
      return qpkg::router::run_build_mode(build_parser, mode);
    } else if (program.is_subcommand_used("clean")) {
      return qpkg::router::run_clean_mode(clean_parser, mode);
    } else if (program.is_subcommand_used("update")) {
      return qpkg::router::run_update_mode(update_parser, mode);
    } else if (program.is_subcommand_used("install")) {
      return qpkg::router::run_install_mode(install_parser, mode);
    } else if (program.is_subcommand_used("doc")) {
      return qpkg::router::run_doc_mode(doc_parser, mode);
    } else if (program.is_subcommand_used("format")) {
      return qpkg::router::run_format_mode(format_parser, mode);
    } else if (program.is_subcommand_used("list")) {
      return qpkg::router::run_list_mode(list_parser, mode);
    } else if (program.is_subcommand_used("test")) {
      return qpkg::router::run_test_mode(test_parser, mode);
    } else if (program.is_subcommand_used("lsp")) {
      return qpkg::router::run_lsp_mode(lsp_parser, mode);
    } else if (program.is_subcommand_used("dev")) {
      return qpkg::router::run_dev_mode(dev_parser, dev_subparsers, mode);
    } else {
      qerr << "No command specified" << std::endl;
      qerr << program;
      return 1;
    }
  }
}

#ifndef QPKG_LIBRARY

int main(int argc, char *argv[]) {
  const char *NO_COLOR = getenv("NO_COLOR");
  bool use_colors = NO_COLOR != NULL && NO_COLOR[0] != '\0' ? false : true;

  std::vector<char *> args(argv, argv + argc);
  for (auto it = args.begin(); it != args.end(); ++it) {
    if (strcmp(*it, "--no-color") == 0) {
      use_colors = false;
      args.erase(it);
      break;
    }
  }

  FLAGS_stderrthreshold = google::FATAL;

  google::InitGoogleLogging("qpkg");
  google::InstallFailureSignalHandler();

  return qpkg_command(args.size(), args.data(), use_colors);
}

#endif
