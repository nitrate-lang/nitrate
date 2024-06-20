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

#include <bench/bench.hh>
#include <build/EngineBuilder.hh>
#include <clean/Cleanup.hh>
#include <core/Logger.hh>
#include <filesystem>
#include <functional>
#include <init/Package.hh>
#include <iostream>
#include <map>
#include <optional>
#include <run/RunScript.hh>
#include <string>
#include <thread>
#include <vector>

constexpr const char *VERSION_STR = "qpkg version 0.1.0 [2024-03] (generic)";
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

void setup_argparse_init(ArgumentParser &parser) {
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

  parser.add_argument("-V", "--version")
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
  parser.add_argument("package-name")
      .help("name of package to update")
      .nargs(1);

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

  parser.add_argument("-u", "--utility")
      .help("install package as a utility")
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
  parser.add_argument("package-src")
      .help("name of package to document")
      .nargs(1);

  parser.add_argument("-f", "--format")
      .help("output format")
      .choices("HTML", "TXT", "PDF")
      .default_value(std::string("HTML"))
      .nargs(1);

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
      .default_value(1)
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

void setup_argparse_env(ArgumentParser &parser) {
  parser.add_argument("key").help("environment variable to query").nargs(1);

  parser.add_argument("-noout", "--no-output")
      .help("do not print the value of the environment variable")
      .default_value(false)
      .implicit_value(true);

  parser.add_argument("--base64")
      .help(
          "print the value of the environment variable as a base64 encoded "
          "string")
      .default_value(false)
      .implicit_value(true);

  parser.add_argument("--hex")
      .help(
          "print the value of the environment variable as a hex encoded string")
      .default_value(false)
      .implicit_value(true);

  parser.add_argument("--json")
      .help("print the key-value pair as a JSON object")
      .default_value(false)
      .implicit_value(true);

  parser.add_argument("--yaml")
      .help("print the key-value pair as a YAML object")
      .default_value(false)
      .implicit_value(true);

  parser.add_argument("--toml")
      .help("print the key-value pair as a TOML object")
      .default_value(false)
      .implicit_value(true);

  parser.add_argument("--xml")
      .help("print the key-value pair as an XML object")
      .default_value(false)
      .implicit_value(true);

  parser.add_argument("--csv")
      .help("print the key-value pair as a CSV object")
      .default_value(false)
      .implicit_value(true);
}

void setup_argparse_fmt(ArgumentParser &parser) {
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
  parser.add_argument("-p", "--packages")
      .help("list all packages installed")
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

  parser.add_argument("-f", "--format")
      .help("output format for reports")
      .choices("HTML", "TXT", "PDF", "LOG", "JSON", "XML", "CSV")
      .default_value(std::string("HTML"))
      .nargs(1);

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
      .default_value(1)
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

void setup_argparse_dev(
    ArgumentParser &parser,
    std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>>
        &subparsers) {
  /*================= CONFIG BASIC =================*/
  parser.add_argument("-v", "--verbose")
      .help("print verbose output")
      .default_value(false)
      .implicit_value(true);

  /*================= BENCH SUBPARSER =================*/
  auto bench =
      std::make_unique<ArgumentParser>("bench", "run internal benchmarks");

  bench->add_argument("-n", "--name")
      .choices("lexer", "parser", "quix-ir", "delta-ir", "llvm-ir",
               "llvm-codegen", "c11-codegen", "cxx-codegen", "pipeline")
      .help("name of benchmark to run");

  bench->add_argument("--list")
      .help("list available benchmarks")
      .default_value(false)
      .implicit_value(true);

  subparsers["bench"] = std::move(bench);

  parser.add_subparser(*subparsers["bench"]);
}

void setup_argparse(
    ArgumentParser &parser, ArgumentParser &init_parser,
    ArgumentParser &build_parser, ArgumentParser &clean_parser,
    ArgumentParser &update_parser, ArgumentParser &install_parser,
    ArgumentParser &doc_parser, ArgumentParser &env_parser,
    ArgumentParser &fmt_parser, ArgumentParser &list_parser,
    ArgumentParser &test_parser, ArgumentParser &dev_parser,
    std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>>
        &dev_subparsers) {
  using namespace argparse;

  setup_argparse_init(init_parser);
  setup_argparse_build(build_parser);
  setup_argparse_clean(clean_parser);
  setup_argparse_update(update_parser);
  setup_argparse_install(install_parser);
  setup_argparse_doc(doc_parser);
  setup_argparse_env(env_parser);
  setup_argparse_fmt(fmt_parser);
  setup_argparse_list(list_parser);
  setup_argparse_test(test_parser);
  setup_argparse_dev(dev_parser, dev_subparsers);

  parser.add_subparser(init_parser);
  parser.add_subparser(build_parser);
  parser.add_subparser(clean_parser);
  parser.add_subparser(update_parser);
  parser.add_subparser(install_parser);
  parser.add_subparser(doc_parser);
  parser.add_subparser(env_parser);
  parser.add_subparser(fmt_parser);
  parser.add_subparser(list_parser);
  parser.add_subparser(test_parser);
  parser.add_subparser(dev_parser);

  parser.add_argument("--license")
      .help("show license information")
      .default_value(false)
      .implicit_value(true);
}

int run_init_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

  using namespace qpkg::init;

  PackageBuilder builder = PackageBuilder()
                               .output(parser.get<std::string>("--output"))
                               .name(parser.get<std::string>("package-name"))
                               .license(parser.get<std::string>("--license"))
                               .author(parser.get<std::string>("--author"))
                               .email(parser.get<std::string>("--email"))
                               .url(parser.get<std::string>("--url"))
                               .version(parser.get<std::string>("--version"))
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
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

  qpkg::build::EngineBuilder builder;

  builder.set_package_src(parser.get<std::string>("package-src"));

  if (parser.is_used("--output"))
    builder.set_output(parser.get<std::string>("--output"));

  if (parser["--no-cache"] == true) builder.disable_cache();

  if (parser.is_used("--jobs")) builder.jobs(parser.get<uint32_t>("--jobs"));

  if (parser["--verbose"] == true) builder.verbose();

  if (parser["--optimize"] == true) builder.optimize();

  if (parser["--optimize-size"] == true) builder.optimize_size();

  if (parser["--debug"] == true) builder.debug();

  if (parser.is_used("--certify"))
    builder.certify(parser.get<std::string>("--certify"));

  if (parser.is_used("--certify-password"))
    builder.certify_password(parser.get<std::string>("--certify-password"));

  if (parser["--supply-chain-insecure"] == true) builder.disable_sigcheck();

  if (parser.is_used("--trustkey"))
    builder.trustkey(parser.get<std::string>("--trustkey"));

  if (parser.is_used("--trustkeys"))
    builder.trustkeys(parser.get<std::string>("--trustkeys"));

  auto engine = builder.build();
  if (!engine) {
    std::cerr << "Failed to construct engine" << std::endl;
    return -1;
  }

  return engine->run() ? 0 : -1;
}

int run_clean_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

  return qpkg::clean::CleanPackageSource(parser.get<std::string>("package-src"),
                                         parser["--verbose"] == true)
             ? 0
             : -1;
}

int run_update_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

  (void)parser;
  std::cerr << "update not implemented yet" << std::endl;
  return 1;
}

int run_install_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

  (void)parser;
  std::cerr << "install not implemented yet" << std::endl;
  return 1;
}

int run_doc_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

  (void)parser;
  std::cerr << "doc not implemented yet" << std::endl;
  return 1;
}

int run_env_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

  (void)parser;
  std::cerr << "env not implemented yet" << std::endl;
  return 1;
}

int run_fmt_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

  (void)parser;
  std::cerr << "fmt not implemented yet" << std::endl;
  return 1;
}

int run_list_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

  (void)parser;
  std::cerr << "list not implemented yet" << std::endl;
  return 1;
}

int run_run_mode(const std::vector<std::string> &args) {
  qpkg::core::FormatAdapter::PluginAndInit(false);

  if (args.size() < 1) {
    std::cerr << "No script specified" << std::endl;
    return -1;
  }

  qpkg::run::RunScript script(args[0]);
  if (!script.is_okay()) return -1;

  return script.run(args);
}

int run_test_mode(const ArgumentParser &parser) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

  (void)parser;
  std::cerr << "test not implemented yet" << std::endl;
  return 1;
}

int run_dev_mode(
    const ArgumentParser &parser,
    const std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>>
        &subparsers) {
  qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

  if (parser.is_subcommand_used("bench")) {
    enum class Benchmark {
      LEXER,
      PARSER,
      Q_IR,
      DELTA_IR,
      LLVM_IR,
      LLVM_CODEGEN,
      C11_CODEGEN,
      CXX_CODEGEN,
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
      std::cout << "  cxx-codegen" << std::endl;
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
    else if (bench_name == "cxx-codegen")
      bench_type = Benchmark::CXX_CODEGEN;
    else if (bench_name == "pipeline")
      bench_type = Benchmark::PIPELINE;
    else {
      std::cerr << "Unknown benchmark specified" << std::endl;
      std::cerr << bench_parser;
      return 1;
    }

    switch (bench_type) {
      case Benchmark::LEXER:
        return qpkg::bench::run_benchmark_lexer();
      case Benchmark::PARSER:
        return qpkg::bench::run_benchmark_parser();
      case Benchmark::Q_IR:
        return qpkg::bench::run_benchmark_quix_ir();
      case Benchmark::DELTA_IR:
        return qpkg::bench::run_benchmark_delta_ir();
      case Benchmark::LLVM_IR:
        return qpkg::bench::run_benchmark_llvm_ir();
      case Benchmark::LLVM_CODEGEN:
        return qpkg::bench::run_benchmark_llvm_codegen();
      case Benchmark::C11_CODEGEN:
        return qpkg::bench::run_benchmark_c11_codegen();
      case Benchmark::CXX_CODEGEN:
        return qpkg::bench::run_benchmark_cxx_codegen();
      case Benchmark::PIPELINE:
        return qpkg::bench::run_benchmark_pipeline();
      default:
        std::cerr << "Unknown benchmark name: " << bench_name << std::endl;
        return 1;
    }

    return 0;
  }

  std::cerr << "Unknown subcommand for dev" << std::endl;
  std::cerr << parser;

  return 1;
}

int qpkg_main(std::vector<std::string> args) {
  if (args.size() >= 2 && args[1] == "run") {
    std::vector<std::string> run_args(args.begin() + 2, args.end());
    return run_run_mode(run_args);
  }

  ArgumentParser init_parser("init", "initialize a new package");
  ArgumentParser build_parser("build", "compile packages and dependencies");
  ArgumentParser clean_parser("clean", "remove object files and cached files");
  ArgumentParser update_parser("update", "update package dependencies");
  ArgumentParser install_parser("install",
                                "download and compile remote source");
  ArgumentParser doc_parser("doc", "show documentation for package or symbol");
  ArgumentParser env_parser("env", "print QUIX environment information");
  ArgumentParser fmt_parser("fmt", "reformat package sources");
  ArgumentParser list_parser("list", "list packages or modules");
  ArgumentParser test_parser("test", "test packages");
  ArgumentParser dev_parser("dev", "run internal development");

  std::unordered_map<std::string_view, std::unique_ptr<ArgumentParser>>
      dev_subparsers;

  ArgumentParser program("qpkg", VERSION_STR);
  setup_argparse(program, init_parser, build_parser, clean_parser,
                 update_parser, install_parser, doc_parser, env_parser,
                 fmt_parser, list_parser, test_parser, dev_parser,
                 dev_subparsers);

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
  else if (program.is_subcommand_used("env"))
    return run_env_mode(env_parser);
  else if (program.is_subcommand_used("fmt"))
    return run_fmt_mode(fmt_parser);
  else if (program.is_subcommand_used("list"))
    return run_list_mode(list_parser);
  else if (program.is_subcommand_used("test"))
    return run_test_mode(test_parser);
  else if (program.is_subcommand_used("dev"))
    return run_dev_mode(dev_parser, dev_subparsers);
  else {
    std::cerr << "No command specified" << std::endl;
    std::cerr << program;
    return 1;
  }
}

int main(int argc, char *argv[]) {
  qpkg::core::FormatAdapter::PluginAndInit();

  std::vector<std::string> args(argv, argv + argc);

  return qpkg_main(args);
}