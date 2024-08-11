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
#include <quix-core/Lib.h>
#include <quix-lexer/Lib.h>
#include <quix-parser/Lib.h>
#include <quix-qxir/Lib.h>

// #include <build/EngineBuilder.hh>
#include <clean/Cleanup.hh>
#include <core/Config.hh>
#include <core/Logger.hh>

#if QPKG_DEV_TOOLS
// #include <dev/bench/bench.hh>
// #include <dev/test/test.hh>
#endif
// #include <run/RunScript.hh>

// #include <filesystem>
// #include <functional>
#include <init/Package.hh>
// #include <install/Install.hh>
// #include <iostream>
// #include <map>
// #include <optional>
// #include <string>
// #include <thread>
// #include <vector>

#ifndef QPKG_ID
#warning "QPKG_ID not defined"
#define QPKG_ID "?"
#endif

static char *quixcc_cc_demangle(const char *mangled_name) {
  /// TODO: implement
  (void)mangled_name;
  std::cerr << "demangle not implemented yet" << std::endl;
  return nullptr;
}

static std::string qpkg_deps_version_string() {
#define QPKG_STABLE false /* TODO: Automate setting of 'is stable build' flag */

  std::stringstream ss;

  std::array<std::string_view, 4> QPKG_DEPS = {
      qcore_lib_version(), qlex_lib_version(), qparse_lib_version(), qxir_lib_version()

  };

  ss << "{\"ver\":\"" << QPKG_ID << "\",\"stable\":" << (QPKG_STABLE ? "true" : "false")
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

bool g_use_colors = true;

void setup_argparse_init(ArgumentParser &parser) {
  parser.add_argument("package-name").help("name of package to initialize").nargs(1);

  parser.add_argument("-o", "--output")
      .help("output directory")
      .default_value(std::string("."))
      .nargs(1);

  parser.add_argument("-V", "--verbose")
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

  parser.add_argument("-u", "--url").help("URL of package").default_value(std::string("")).nargs(1);

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

  parser.add_argument("-V", "--verbose")
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

  parser.add_argument("-V", "--verbose")
      .help("print verbose output")
      .default_value(false)
      .implicit_value(true);
}

void setup_argparse_update(ArgumentParser &parser) {
  parser.add_argument("-V", "--verbose")
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

  parser.add_argument("-V", "--verbose")
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

  parser.add_argument("-V", "--verbose")
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

  parser.add_argument("-V", "--verbose")
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
  parser.add_argument("-V", "--verbose")
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

  parser.add_argument("-V", "--verbose")
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

#if QPKG_DEV_TOOLS
void setup_argparse_dev(
    ArgumentParser &parser,
    std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>> &subparsers) {
  /*================= CONFIG BASIC =================*/
  parser.add_argument("-V", "--verbose")
      .help("print verbose output")
      .default_value(false)
      .implicit_value(true);

  /*================== OTHER STUFF =================*/
  parser.add_argument("--demangle").help("demangle QUIX symbol names").nargs(1);

  /*================= BENCH SUBPARSER =================*/
  auto bench = std::make_unique<ArgumentParser>("bench");

  bench->add_argument("-n", "--name")
      .choices("lexer", "parser", "quix-ir", "delta-ir", "llvm-ir", "llvm-codegen", "c11-codegen",
               "pipeline")
      .help("name of benchmark to run");

  bench->add_argument("--list")
      .help("list available benchmarks")
      .default_value(false)
      .implicit_value(true);

  subparsers["bench"] = std::move(bench);

  /*================= QPARSE SUBPARSER =================*/
  auto parse = std::make_unique<ArgumentParser>("parse");

  parse->add_argument("source").help("source file to parse").nargs(1);
  parse->add_argument("-o", "--output")
      .help("output file for parse tree")
      .default_value(std::string(""))
      .nargs(1);

  subparsers["parse"] = std::move(parse);

  /*================= QXIR SUBPARSER =================*/
  auto qxir = std::make_unique<ArgumentParser>("qxir");

  qxir->add_argument("source").help("source file to lower into QXIR").nargs(1);
  qxir->add_argument("-o", "--output")
      .help("output file for qxir tree")
      .default_value(std::string(""))
      .nargs(1);
  qxir->add_argument("-O", "--opts")
      .help("optimizations to apply to QXIR")
      .default_value(std::string(""))
      .nargs(1);

  subparsers["qxir"] = std::move(qxir);

  /*================= TEST SUBPARSER =================*/
  auto test = std::make_unique<ArgumentParser>("test");

  subparsers["test"] = std::move(test);

  parser.add_subparser(*subparsers["bench"]);
  parser.add_subparser(*subparsers["test"]);
  parser.add_subparser(*subparsers["parse"]);
  parser.add_subparser(*subparsers["qxir"]);
}

#endif

void setup_argparse(
    ArgumentParser &parser, ArgumentParser &init_parser, ArgumentParser &build_parser,
    ArgumentParser &clean_parser, ArgumentParser &update_parser, ArgumentParser &install_parser,
    ArgumentParser &doc_parser, ArgumentParser &format_parser, ArgumentParser &list_parser,
    ArgumentParser &test_parser
#if QPKG_DEV_TOOLS
    ,
    ArgumentParser &dev_parser,
    std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>> &dev_subparsers
#endif
) {
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
#if QPKG_DEV_TOOLS
  setup_argparse_dev(dev_parser, dev_subparsers);
#endif

  parser.add_subparser(init_parser);
  parser.add_subparser(build_parser);
  parser.add_subparser(clean_parser);
  parser.add_subparser(update_parser);
  parser.add_subparser(install_parser);
  parser.add_subparser(doc_parser);
  parser.add_subparser(format_parser);
  parser.add_subparser(list_parser);
  parser.add_subparser(test_parser);
#if QPKG_DEV_TOOLS
  parser.add_subparser(dev_parser);
#endif

  parser.add_argument("--license")
      .help("show license information")
      .default_value(false)
      .implicit_value(true);
}

int run_init_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true, g_use_colors);

  using namespace qpkg::init;

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

int run_build_mode(const ArgumentParser &parser) {
  (void)parser;

  std::cerr << "build not implemented yet" << std::endl;
  return 1;

  // qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true, g_use_colors);

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
  //   std::cerr << "Failed to construct engine" << std::endl;
  //   return -1;
  // }

  // return engine->run() ? 0 : -1;
}

int run_clean_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true, g_use_colors);

  return qpkg::clean::CleanPackageSource(parser.get<std::string>("package-src"),
                                         parser["--verbose"] == true)
             ? 0
             : -1;
}

int run_update_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true, g_use_colors);

  (void)parser;
  std::cerr << "update not implemented yet" << std::endl;
  return 1;
}

int run_install_mode(const ArgumentParser &parser) {
  (void)parser;

  std::cerr << "install not implemented yet" << std::endl;
  return 1;

  // qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true, g_use_colors);

  // std::string url = parser.get<std::string>("src");
  // std::string dest, package_name;
  // bool overwrite = parser["--override"] == true;
  // bool global = parser["--global"] == true;

  // if (global) {
  //   /// TODO: fix platform-specific global install
  //   try {
  //     std::filesystem::create_directories(QPKG_GLOBAL_PACKAGE_DIR);
  //   } catch (std::filesystem::filesystem_error &e) {
  //     std::cerr << e.what() << std::endl;
  //     std::cerr << "Try running with higher permissions" << std::endl;
  //     return -1;
  //   }
  //   dest = QPKG_GLOBAL_PACKAGE_DIR;
  // } else {
  //   dest = ".";
  // }

  // if (!qpkg::install::install_from_url(url, dest, package_name, overwrite)) {
  //   return -1;
  // }

  // if (parser["--no-build"] == true) return 0;

  // qpkg::build::EngineBuilder builder;
  // std::filesystem::path dest_path = dest + "/" + package_name;
  // builder.set_package_src(dest_path.string());

  // if (global) {
  //   try {
  //     std::filesystem::path app_file = QPKG_GLOBAL_BINARY_DIR;
  //     app_file /= package_name;

  //     std::filesystem::create_directories(QPKG_GLOBAL_BINARY_DIR);

  //     builder.set_output(app_file.string());
  //   } catch (std::filesystem::filesystem_error &e) {
  //     std::cerr << e.what() << std::endl;
  //     std::cerr << "Try running with higher permissions" << std::endl;
  //     return -1;
  //   }
  // }

  // auto engine = builder.build();
  // if (!engine) {
  //   std::cerr << "Failed to construct engine" << std::endl;
  //   return -1;
  // }

  // if (!engine->run()) {
  //   std::cerr << "Failed to build package" << std::endl;
  //   return -1;
  // }

  // if (!global) {
  //   std::cout << "Package installed to: " << dest_path << std::endl;
  //   return 0;
  // }

  // return 0;
}

int run_doc_mode(const ArgumentParser &parser) {
  enum class OFormat { Html, Plain, Pdf, Json, Xml, ReactJS } oformat;

  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true, g_use_colors);

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
    std::cerr << "Exactly one output format must be specified" << std::endl;
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
      std::cerr << "HTML not implemented yet" << std::endl;
      return 1;
    case OFormat::Plain:
      std::cerr << "Plain not implemented yet" << std::endl;
      return 1;
    case OFormat::Pdf:
      std::cerr << "PDF not implemented yet" << std::endl;
      return 1;
    case OFormat::Json:
      std::cerr << "JSON not implemented yet" << std::endl;
      return 1;
    case OFormat::Xml:
      std::cerr << "XML not implemented yet" << std::endl;
      return 1;
    case OFormat::ReactJS:
      std::cerr << "ReactJS not implemented yet" << std::endl;
      return 1;

    default:
      std::cerr << "Unknown output format" << std::endl;
      return -1;
  }
}

int run_format_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true, g_use_colors);

  (void)parser;
  std::cerr << "format not implemented yet" << std::endl;
  return 1;
}

int run_list_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true, g_use_colors);

  (void)parser;
  std::cerr << "list not implemented yet" << std::endl;
  return 1;
}

int run_run_mode(const std::vector<std::string> &args) {
  (void)args;

  std::cerr << "run not implemented yet" << std::endl;
  return 1;

  // qpkg::core::FormatAdapter::PluginAndInit(false, g_use_colors);

  // if (args.size() < 1) {
  //   std::cerr << "No script specified" << std::endl;
  //   return -1;
  // }

  // qpkg::run::RunScript script(args[0]);
  // if (!script.is_okay()) return -1;

  // return script.run(args);
}

int run_test_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true, g_use_colors);

  (void)parser;
  std::cerr << "test not implemented yet" << std::endl;
  return 1;
}

#if QPKG_DEV_TOOLS

namespace qpkg::dev::bench {
  int run_benchmark_lexer() {
    /// TODO: implement
    std::cerr << "benchmark lexer not implemented yet" << std::endl;
    return 1;
  }

  int run_benchmark_parser() {
    /// TODO: implement
    std::cerr << "benchmark parser not implemented yet" << std::endl;
    return 1;
  }

  int run_benchmark_quix_ir() {
    /// TODO: implement
    std::cerr << "benchmark quix-ir not implemented yet" << std::endl;
    return 1;
  }

  int run_benchmark_delta_ir() {
    /// TODO: implement
    std::cerr << "benchmark delta-ir not implemented yet" << std::endl;
    return 1;
  }

  int run_benchmark_llvm_ir() {
    /// TODO: implement
    std::cerr << "benchmark llvm-ir not implemented yet" << std::endl;
    return 1;
  }

  int run_benchmark_llvm_codegen() {
    /// TODO: implement
    std::cerr << "benchmark llvm-codegen not implemented yet" << std::endl;
    return 1;
  }

  int run_benchmark_c11_codegen() {
    /// TODO: implement
    std::cerr << "benchmark c11-codegen not implemented yet" << std::endl;
    return 1;
  }

  int run_benchmark_pipeline() {
    /// TODO: implement
    std::cerr << "benchmark pipeline not implemented yet" << std::endl;
    return 1;
  }
}  // namespace qpkg::dev::bench

namespace qpkg::dev::test {
  int run_tests() {
    /// TODO: implement tests
    std::cerr << "test not implemented yet" << std::endl;
    return 1;
  }
}  // namespace qpkg::dev::test

int run_dev_mode(
    const ArgumentParser &parser,
    const std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>> &subparsers) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true, g_use_colors);

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

    if (bench_parser["--list"] == true) {
      std::cout << "Available benchmarks:" << std::endl;
      std::cout << "  lexer" << std::endl;
      std::cout << "  parser" << std::endl;
      std::cout << "  quix-ir" << std::endl;
      std::cout << "  delta-ir" << std::endl;
      std::cout << "  llvm-ir" << std::endl;
      std::cout << "  llvm-codegen" << std::endl;
      std::cout << "  c11-codegen" << std::endl;
      std::cout << "  pipeline" << std::endl;
      return 0;
    }

    if (!bench_parser.is_used("--name")) {
      std::cerr << "No benchmark specified" << std::endl;
      std::cerr << bench_parser;
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
      std::cerr << "Unknown benchmark specified" << std::endl;
      std::cerr << bench_parser;
      return 1;
    }

    switch (bench_type) {
      case Benchmark::LEXER:
        return qpkg::dev::bench::run_benchmark_lexer();
      case Benchmark::PARSER:
        return qpkg::dev::bench::run_benchmark_parser();
      case Benchmark::Q_IR:
        return qpkg::dev::bench::run_benchmark_quix_ir();
      case Benchmark::DELTA_IR:
        return qpkg::dev::bench::run_benchmark_delta_ir();
      case Benchmark::LLVM_IR:
        return qpkg::dev::bench::run_benchmark_llvm_ir();
      case Benchmark::LLVM_CODEGEN:
        return qpkg::dev::bench::run_benchmark_llvm_codegen();
      case Benchmark::C11_CODEGEN:
        return qpkg::dev::bench::run_benchmark_c11_codegen();
      case Benchmark::PIPELINE:
        return qpkg::dev::bench::run_benchmark_pipeline();
      default:
        std::cerr << "Unknown benchmark name: " << bench_name << std::endl;
        return 1;
    }

    return 0;
  } else if (parser.is_subcommand_used("test")) {
    return qpkg::dev::test::run_tests();
  } else if (parser.is_subcommand_used("parse")) {
    auto &parse_parser = *subparsers.at("parse");

    std::string source = parse_parser.get<std::string>("source");
    std::string output = parse_parser.get<std::string>("--output");

    FILE *fp = fopen(source.c_str(), "r");
    if (!fp) {
      std::cerr << "Failed to open source file" << std::endl;
      return 1;
    }

    qlex_t *lexer = qlex_new(fp, source.c_str());
    if (!lexer) {
      fclose(fp);
      std::cerr << "Failed to create lexer" << std::endl;
      return 1;
    }

    qparse_conf_t *pconf = qparse_conf_new(true);
    if (!pconf) {
      qlex_free(lexer);
      fclose(fp);
      std::cerr << "Failed to create parser configuration" << std::endl;
      return 1;
    }

    qparse_t *ctx = qparse_new(lexer, pconf);
    if (!ctx) {
      qparse_conf_free(pconf);
      qlex_free(lexer);
      fclose(fp);
      std::cerr << "Failed to create parser context" << std::endl;
      return 1;
    }

    qcore_arena_t arena;
    qcore_arena_open(&arena);
    qparse_node_t *root = nullptr;
    if (!qparse_do(ctx, &arena, &root)) {
      auto cb = [](const char *msg, size_t size, uintptr_t data) {
        (void)size;
        (void)data;
        std::cerr << msg << std::endl;
      };

      qparse_dumps(ctx, false, cb, 0);
      qcore_arena_close(&arena);
      qparse_free(ctx);
      qparse_conf_free(pconf);
      qlex_free(lexer);
      fclose(fp);
      std::cerr << "Failed to parse source" << std::endl;
      return 1;
    }

    size_t out_len = 0;
    char *out_str = qparse_repr(root, false, 2, &arena, &out_len);
    if (!out_str) {
      qcore_arena_close(&arena);
      qparse_free(ctx);
      qparse_conf_free(pconf);
      qlex_free(lexer);
      fclose(fp);
      std::cerr << "Failed to generate parse tree" << std::endl;
      return 1;
    }

    FILE *out_fp = nullptr;
    if (!output.empty()) {
      out_fp = fopen(output.c_str(), "w");
      if (!out_fp) {
        qcore_arena_close(&arena);
        qparse_free(ctx);
        qparse_conf_free(pconf);
        qlex_free(lexer);
        fclose(fp);
        std::cerr << "Failed to open output file" << std::endl;
        return 1;
      }
    } else {
      out_fp = stdout;
    }

    fwrite(out_str, 1, out_len, out_fp);

    if (!output.empty()) fclose(out_fp);

    qcore_arena_close(&arena);
    qparse_free(ctx);
    qparse_conf_free(pconf);
    qlex_free(lexer);
    fclose(fp);

    return 0;
  } else if (parser.is_subcommand_used("qxir")) {
    auto &qxir_parser = *subparsers.at("qxir");

    std::string source = qxir_parser.get<std::string>("source");
    std::string output = qxir_parser.get<std::string>("--output");
    std::string opts = qxir_parser.get<std::string>("--opts");

    FILE *fp = fopen(source.c_str(), "r");
    if (!fp) {
      std::cerr << "Failed to open source file" << std::endl;
      return 1;
    }

    qlex_t *lexer = qlex_new(fp, source.c_str());
    if (!lexer) {
      fclose(fp);
      std::cerr << "Failed to create lexer" << std::endl;
      return 1;
    }

    qparse_conf_t *pconf = qparse_conf_new(true);
    if (!pconf) {
      qlex_free(lexer);
      fclose(fp);
      std::cerr << "Failed to create parser configuration" << std::endl;
      return 1;
    }

    qparse_t *ctx = qparse_new(lexer, pconf);
    if (!ctx) {
      qparse_conf_free(pconf);
      qlex_free(lexer);
      fclose(fp);
      std::cerr << "Failed to create parser context" << std::endl;
      return 1;
    }

    qcore_arena_t arena;
    qcore_arena_open(&arena);
    qparse_node_t *root = nullptr;
    if (!qparse_do(ctx, &arena, &root)) {
      auto cb = [](const char *msg, size_t size, uintptr_t data) {
        (void)size;
        (void)data;
        std::cerr << msg << std::endl;
      };

      qparse_dumps(ctx, false, cb, 0);
      qcore_arena_close(&arena);
      qparse_free(ctx);
      qparse_conf_free(pconf);
      qlex_free(lexer);
      fclose(fp);
      std::cerr << "Failed to parse source" << std::endl;
      return 1;
    }

    qxir_conf_t *conf = qxir_conf_new(true);
    if (!conf) {
      qcore_arena_close(&arena);
      qparse_free(ctx);
      qparse_conf_free(pconf);
      qlex_free(lexer);
      fclose(fp);
      std::cerr << "Failed to create QXIR configuration" << std::endl;
      return 1;
    }

    qxir_t *qxir = qxir_new(root, conf);
    if (!qxir) {
      qxir_conf_free(conf);
      qcore_arena_close(&arena);
      qparse_free(ctx);
      qparse_conf_free(pconf);
      qlex_free(lexer);
      fclose(fp);
      std::cerr << "Failed to create QXIR context" << std::endl;
      return 1;
    }

    qxir_node_t *qxir_root = nullptr;
    if (!qxir_do(qxir, &arena, &qxir_root)) {
      qxir_free(qxir);
      qxir_conf_free(conf);
      qcore_arena_close(&arena);
      qparse_free(ctx);
      qparse_conf_free(pconf);
      qlex_free(lexer);
      fclose(fp);
      std::cerr << "Failed to lower source to QXIR" << std::endl;
      return 1;
    }

    size_t out_len = 0;
    char *out_str = qxir_repr(qxir_root, false, 2, &arena, &out_len);
    if (!out_str) {
      qxir_free(qxir);
      qxir_conf_free(conf);
      qcore_arena_close(&arena);
      qparse_free(ctx);
      qparse_conf_free(pconf);
      qlex_free(lexer);
      fclose(fp);
      std::cerr << "Failed to generate QXIR tree" << std::endl;
      return 1;
    }

    FILE *out_fp = nullptr;
    if (!output.empty()) {
      out_fp = fopen(output.c_str(), "w");
      if (!out_fp) {
        qxir_free(qxir);
        qxir_conf_free(conf);
        qcore_arena_close(&arena);
        qparse_free(ctx);
        qparse_conf_free(pconf);
        qlex_free(lexer);
        fclose(fp);
        std::cerr << "Failed to open output file" << std::endl;
        return 1;
      }
    } else {
      out_fp = stdout;
    }

    fwrite(out_str, 1, out_len, out_fp);

    if (!output.empty()) fclose(out_fp);

    qxir_free(qxir);
    qxir_conf_free(conf);
    qcore_arena_close(&arena);
    qparse_free(ctx);
    qparse_conf_free(pconf);
    qlex_free(lexer);
    fclose(fp);

    std::cout << "opts: " << opts << std::endl;
    return 0;
  } else if (parser.is_used("--demangle")) {
    std::string input = parser.get<std::string>("--demangle");
    char *demangled_name = quixcc_cc_demangle(input.c_str());
    if (!demangled_name) {
      std::cerr << "Failed to demangle symbol" << std::endl;
      return 1;
    }

    std::cout << demangled_name << std::endl;

    free(demangled_name);
    return 0;
  }

  std::cerr << "Unknown subcommand for dev" << std::endl;
  std::cerr << parser;

  return 1;
}
#endif

int qpkg_main(std::vector<std::string> args) {
  if (args.size() >= 2 && args[1] == "run") {
    std::vector<std::string> run_args(args.begin() + 2, args.end());
    return run_run_mode(run_args);
  }

  ArgumentParser init_parser("init");
  ArgumentParser build_parser("build");
  ArgumentParser clean_parser("clean");
  ArgumentParser update_parser("update");
  ArgumentParser install_parser("install");
  ArgumentParser doc_parser("doc");
  ArgumentParser format_parser("format");
  ArgumentParser list_parser("list");
  ArgumentParser test_parser("test");
#if QPKG_DEV_TOOLS
  ArgumentParser dev_parser("dev");

  std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>> dev_subparsers;
#endif

  ArgumentParser program("qpkg", qpkg_deps_version_string());
  setup_argparse(program, init_parser, build_parser, clean_parser, update_parser, install_parser,
                 doc_parser, format_parser, list_parser, test_parser
#if QPKG_DEV_TOOLS
                 ,
                 dev_parser, dev_subparsers
#endif
  );

  try {
    program.parse_args(args);
  } catch (const std::runtime_error &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  if (program["--license"] == true) {
    std::cout << FULL_LICENSE << std::endl;
    return 0;
  }

  if (program.is_subcommand_used("init"))
    return run_init_mode(init_parser);
  else if (program.is_subcommand_used("build"))
    return run_build_mode(build_parser);
  else if (program.is_subcommand_used("clean"))
    return run_clean_mode(clean_parser);
  else if (program.is_subcommand_used("update"))
    return run_update_mode(update_parser);
  else if (program.is_subcommand_used("install"))
    return run_install_mode(install_parser);
  else if (program.is_subcommand_used("doc"))
    return run_doc_mode(doc_parser);
  else if (program.is_subcommand_used("format"))
    return run_format_mode(format_parser);
  else if (program.is_subcommand_used("list"))
    return run_list_mode(list_parser);
  else if (program.is_subcommand_used("test"))
    return run_test_mode(test_parser);
#if QPKG_DEV_TOOLS
  else if (program.is_subcommand_used("dev"))
    return run_dev_mode(dev_parser, dev_subparsers);
#endif
  else {
    std::cerr << "No command specified" << std::endl;
    std::cerr << program;
    return 1;
  }
}

int main(int argc, char *argv[]) {
  if (getenv("QUIXCC_COLOR") != nullptr) {
    if (std::string(getenv("QUIXCC_COLOR")) == "0")
      g_use_colors = false;
    else
      g_use_colors = true;
  }

  qpkg::core::FormatAdapter::PluginAndInit(false, g_use_colors);

  std::vector<std::string> args(argv, argv + argc);

  if (!qcore_lib_init()) {
    std::cerr << "Failed to initialize QUIX-CORE library" << std::endl;
    return -1;
  }

  if (!qlex_lib_init()) {
    std::cerr << "Failed to initialize QUIX-LEX library" << std::endl;
    return -1;
  }

  if (!qparse_lib_init()) {
    std::cerr << "Failed to initialize QUIX-PARSE library" << std::endl;
    return -1;
  }

  int ret = qpkg_main(args);

  qparse_lib_deinit();
  qlex_lib_deinit();
  qcore_lib_deinit();

  return ret;
}
