#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <map>
#include <thread>
#include <functional>
#include <optional>
#include <argparse.h>

#include <init/Package.hh>
#include <build/EngineBuilder.hh>
#include <clean/Cleanup.hh>

#include <core/Logger.hh>

constexpr const char *VERSION_STR = "qpkg version 0.1.0 [2024-03] (generic)";
constexpr const char *FULL_LICENSE = R"(This file is part of QUIX Compiler Suite.
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

static ArgumentParser init_parser("init", "initialize a new package");
static ArgumentParser build_parser("build", "compile packages and dependencies");
static ArgumentParser clean_parser("clean", "remove object files and cached files");
static ArgumentParser update_parser("update", "update package dependencies");
static ArgumentParser install_parser("install", "download and compile remote source");
static ArgumentParser doc_parser("doc", "show documentation for package or symbol");
static ArgumentParser env_parser("env", "print QUIX environment information");
static ArgumentParser fmt_parser("fmt", "reformat package sources");
static ArgumentParser list_parser("list", "list packages or modules");
static ArgumentParser run_parser("run", "compile and run QUIX program");
static ArgumentParser test_parser("test", "test packages");

static void setup_argparse_init(ArgumentParser &parser)
{
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

static void setup_argparse_build(ArgumentParser &parser)
{
    parser.add_argument("package-src")
        .help("path to package source")
        .nargs(1);

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
        .help("request optimization from build pipeline. not all pipelines will support this, and it may be ignored")
        .default_value(false)
        .implicit_value(true);

    optimization_group.add_argument("-Os", "--optimize-size")
        .help("request size optimization from build pipeline. not all pipelines will support this, and it may be ignored")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-g", "--debug")
        .help("request that the pipeline generate and preserve debug information. not all pipelines will support this, and it may be ignored")
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
        .help("do not verify OR require dependencies to be validly signed by a trusted source")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--trustkey")
        .help("add a trusted public key fingerprint that may be used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--trustkeys")
        .help("add a file containing trusted public key fingerprints that may be used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);
}

static void setup_argparse_clean(ArgumentParser &parser)
{
    parser.add_argument("package-src")
        .help("path to package source")
        .nargs(1);

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);
}

static void setup_argparse_update(ArgumentParser &parser)
{
    parser.add_argument("package-name")
        .help("name of package to update")
        .nargs(1);

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--supply-chain-insecure")
        .help("do not verify OR require dependencies to be validly signed by a trusted source")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--trustkey")
        .help("add a trusted public key fingerprint that may be used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--trustkeys")
        .help("add a file containing trusted public key fingerprints that may be used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);
}

static void setup_argparse_install(ArgumentParser &parser)
{
    parser.add_argument("src")
        .help("source of package to install")
        .nargs(1);

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
        .help("do not verify OR require dependencies to be validly signed by a trusted source")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--trustkey")
        .help("add a trusted public key fingerprint that may be used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--trustkeys")
        .help("add a file containing trusted public key fingerprints that may be used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);
}

static void setup_argparse_doc(ArgumentParser &parser)
{
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
        .help("digitally sign the documentation with the specified PKCS#12 certificate")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--certify-password")
        .help("password for the PKCS#12 certificate")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--supply-chain-insecure")
        .help("do not verify OR require dependencies to be validly signed by a trusted source")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--trustkey")
        .help("add a trusted public key fingerprint that may be used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--trustkeys")
        .help("add a file containing trusted public key fingerprints that may be used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);
}

static void setup_argparse_env(ArgumentParser &parser)
{
    parser.add_argument("key")
        .help("environment variable to query")
        .nargs(1);

    parser.add_argument("-noout", "--no-output")
        .help("do not print the value of the environment variable")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--base64")
        .help("print the value of the environment variable as a base64 encoded string")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--hex")
        .help("print the value of the environment variable as a hex encoded string")
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

static void setup_argparse_fmt(ArgumentParser &parser)
{
    parser.add_argument("package-src")
        .help("path to package source")
        .nargs(1);

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

static void setup_argparse_list(ArgumentParser &parser)
{
    parser.add_argument("-p", "--packages")
        .help("list all packages installed")
        .default_value(false)
        .implicit_value(true);
}

static void setup_argparse_run(ArgumentParser &parser)
{
    parser.add_argument("package-name")
        .help("name of package to run")
        .nargs(1);

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--supply-chain-insecure")
        .help("do not verify OR require dependencies to be validly signed by a trusted source")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--trustkey")
        .help("add a trusted public key fingerprint that may be used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--trustkeys")
        .help("add a file containing trusted public key fingerprints that may be used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);
}

static void setup_argparse_test(ArgumentParser &parser)
{
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
        .help("do not verify OR require dependencies to be validly signed by a trusted source")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("--trustkey")
        .help("add a trusted public key fingerprint that may be used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--trustkeys")
        .help("add a file containing trusted public key fingerprints that may be used to verify dependencies (only applies to this build)")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("-C", "--certify")
        .help("digitally sign the test reports with the specified PKCS#12 certificate")
        .default_value(std::string(""))
        .nargs(1);

    parser.add_argument("--certify-password")
        .help("password for the PKCS#12 certificate")
        .default_value(std::string(""))
        .nargs(1);
}

static void setup_argparse(ArgumentParser &parser)
{
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
    setup_argparse_run(run_parser);
    setup_argparse_test(test_parser);

    parser.add_subparser(init_parser);
    parser.add_subparser(build_parser);
    parser.add_subparser(clean_parser);
    parser.add_subparser(update_parser);
    parser.add_subparser(install_parser);
    parser.add_subparser(doc_parser);
    parser.add_subparser(env_parser);
    parser.add_subparser(fmt_parser);
    parser.add_subparser(list_parser);
    parser.add_subparser(run_parser);
    parser.add_subparser(test_parser);

    parser.add_argument("--license")
        .help("show license information")
        .default_value(false)
        .implicit_value(true);
}

static int run_init_mode(const ArgumentParser &parser)
{
    qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

    using namespace qpkg::init;

    PackageBuilder builder =
        PackageBuilder()
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

static int run_build_mode(const ArgumentParser &parser)
{
    qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

    qpkg::build::EngineBuilder builder;

    builder.set_package_src(parser.get<std::string>("package-src"));

    if (parser.is_used("--output"))
        builder.set_output(parser.get<std::string>("--output"));

    if (parser["--no-cache"] == true)
        builder.disable_cache();

    if (parser.is_used("--jobs"))
        builder.jobs(parser.get<uint32_t>("--jobs"));

    if (parser["--verbose"] == true)
        builder.verbose();

    if (parser["--optimize"] == true)
        builder.optimize();

    if (parser["--optimize-size"] == true)
        builder.optimize_size();

    if (parser["--debug"] == true)
        builder.debug();

    if (parser.is_used("--certify"))
        builder.certify(parser.get<std::string>("--certify"));

    if (parser.is_used("--certify-password"))
        builder.certify_password(parser.get<std::string>("--certify-password"));

    if (parser["--supply-chain-insecure"] == true)
        builder.disable_sigcheck();

    if (parser.is_used("--trustkey"))
        builder.trustkey(parser.get<std::string>("--trustkey"));

    if (parser.is_used("--trustkeys"))
        builder.trustkeys(parser.get<std::string>("--trustkeys"));

    return builder.build().run() ? 0 : -1;
}

static int run_clean_mode(const ArgumentParser &parser)
{
    qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

    return qpkg::clean::CleanPackageSource(parser.get<std::string>("package-src"), parser["--verbose"] == true) ? 0 : -1;
}

static int run_update_mode(const ArgumentParser &parser)
{
    qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

    (void)parser;
    std::cerr << "update not implemented yet" << std::endl;
    return 1;
}

static int run_install_mode(const ArgumentParser &parser)
{
    qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

    (void)parser;
    std::cerr << "install not implemented yet" << std::endl;
    return 1;
}

static int run_doc_mode(const ArgumentParser &parser)
{
    qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

    (void)parser;
    std::cerr << "doc not implemented yet" << std::endl;
    return 1;
}

static int run_env_mode(const ArgumentParser &parser)
{
    qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

    (void)parser;
    std::cerr << "env not implemented yet" << std::endl;
    return 1;
}

static int run_fmt_mode(const ArgumentParser &parser)
{
    qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

    (void)parser;
    std::cerr << "fmt not implemented yet" << std::endl;
    return 1;
}

static int run_list_mode(const ArgumentParser &parser)
{
    qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

    (void)parser;
    std::cerr << "list not implemented yet" << std::endl;
    return 1;
}

static int run_run_mode(const ArgumentParser &parser)
{
    qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

    (void)parser;
    std::cerr << "run not implemented yet" << std::endl;
    return 1;
}

static int run_test_mode(const ArgumentParser &parser)
{
    qpkg::core::FormatAdapter::PluginAndInit(parser["--verbose"] == true);

    (void)parser;
    std::cerr << "test not implemented yet" << std::endl;
    return 1;
}

int main(int argc, char *argv[])
{
    qpkg::core::FormatAdapter::PluginAndInit();

    std::vector<std::string> args(argv, argv + argc);
    ArgumentParser program("qpkg", VERSION_STR);
    setup_argparse(program);

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    if (program["--license"] == true)
    {
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
    else if (program.is_subcommand_used("run"))
        return run_run_mode(run_parser);
    else if (program.is_subcommand_used("test"))
        return run_test_mode(test_parser);
    else
    {
        std::cerr << "No command specified" << std::endl;
        std::cerr << program;
        return 1;
    }
}