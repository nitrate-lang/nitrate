#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <map>
#include <thread>
#include <functional>
#include <optional>
#include <argparse.h>

#include <cache/CacheDirectory.hh>

constexpr const char *COPYRIGHT = "Copyright (C) 2024 Wesley C. Jones";
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

static void setup_argparse_build(argparse::ArgumentParser &parser)
{
    (void)parser;

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
        .help("number of jobs to run simultaneously. -1 for auto")
        .default_value(-1)
        .nargs(1);

    parser.add_argument("-v", "--verbose")
        .help("print verbose output")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-O", "--optimize")
        .help("request optimization from build pipeline. not all pipelines will support this, and it may be ignored")
        .default_value(false)
        .implicit_value(true);

    parser.add_argument("-OS", "--optimize-size")
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

static void setup_argparse_clean(argparse::ArgumentParser &parser)
{
    parser.add_argument("package-src")
        .help("path to package source")
        .nargs(1);

    parser.add_argument("-r", "--recursive")
        .help("remove all cached files in the package and its dependencies")
        .default_value(false)
        .implicit_value(true);
}

static void setup_argparse_update(argparse::ArgumentParser &parser)
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

static void setup_argparse_install(argparse::ArgumentParser &parser)
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

static void setup_argparse_doc(argparse::ArgumentParser &parser)
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

static void setup_argparse_env(argparse::ArgumentParser &parser)
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

static void setup_argparse_fmt(argparse::ArgumentParser &parser)
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

static void setup_argparse_list(argparse::ArgumentParser &parser)
{
    parser.add_argument("-p", "--packages")
        .help("list all packages installed")
        .default_value(false)
        .implicit_value(true);
}

static void setup_argparse_run(argparse::ArgumentParser &parser)
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

static void setup_argparse_test(argparse::ArgumentParser &parser)
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

static void setup_argparse(argparse::ArgumentParser &parser)
{
    using namespace argparse;

    static ArgumentParser build("build", "compile packages and dependencies");
    static ArgumentParser clean("clean", "remove object files and cached files");
    static ArgumentParser update("update", "update package dependencies");
    static ArgumentParser install("install", "download and compile remote source");
    static ArgumentParser doc("doc", "show documentation for package or symbol");
    static ArgumentParser env("env", "print QUIX environment information");
    static ArgumentParser fmt("fmt", "reformat package sources");
    static ArgumentParser list("list", "list packages or modules");
    static ArgumentParser run("run", "compile and run QUIX program");
    static ArgumentParser test("test", "test packages");

    setup_argparse_build(build);
    setup_argparse_clean(clean);
    setup_argparse_update(update);
    setup_argparse_install(install);
    setup_argparse_doc(doc);
    setup_argparse_env(env);
    setup_argparse_fmt(fmt);
    setup_argparse_list(list);
    setup_argparse_run(run);
    setup_argparse_test(test);

    parser.add_subparser(build);
    parser.add_subparser(clean);
    parser.add_subparser(update);
    parser.add_subparser(install);
    parser.add_subparser(doc);
    parser.add_subparser(env);
    parser.add_subparser(fmt);
    parser.add_subparser(list);
    parser.add_subparser(run);
    parser.add_subparser(test);

    parser.add_argument("--license")
        .help("show license information")
        .default_value(false)
        .implicit_value(true);
}

static int run_build_mode(const argparse::ArgumentParser &parser)
{
    (void)parser;
    std::cerr << "build not implemented yet" << std::endl;
    return 1;
}

static int run_clean_mode(const argparse::ArgumentParser &parser)
{
    (void)parser;
    std::cerr << "clean not implemented yet" << std::endl;
    return 1;
}

static int run_update_mode(const argparse::ArgumentParser &parser)
{
    (void)parser;
    std::cerr << "update not implemented yet" << std::endl;
    return 1;
}

static int run_install_mode(const argparse::ArgumentParser &parser)
{
    (void)parser;
    std::cerr << "install not implemented yet" << std::endl;
    return 1;
}

static int run_doc_mode(const argparse::ArgumentParser &parser)
{
    (void)parser;
    std::cerr << "doc not implemented yet" << std::endl;
    return 1;
}

static int run_env_mode(const argparse::ArgumentParser &parser)
{
    (void)parser;
    std::cerr << "env not implemented yet" << std::endl;
    return 1;
}

static int run_fmt_mode(const argparse::ArgumentParser &parser)
{
    (void)parser;
    std::cerr << "fmt not implemented yet" << std::endl;
    return 1;
}

static int run_list_mode(const argparse::ArgumentParser &parser)
{
    (void)parser;
    std::cerr << "list not implemented yet" << std::endl;
    return 1;
}

static int run_run_mode(const argparse::ArgumentParser &parser)
{
    (void)parser;
    std::cerr << "run not implemented yet" << std::endl;
    return 1;
}

static int run_test_mode(const argparse::ArgumentParser &parser)
{
    (void)parser;
    std::cerr << "test not implemented yet" << std::endl;
    return 1;
}

static void quick_test()
{
}

int main(int argc, char *argv[])
{
    quick_test();
    return 0;

    std::vector<std::string> args(argv, argv + argc);
    argparse::ArgumentParser program("qpkg", VERSION_STR);
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

    if (program.is_subcommand_used("build"))
        return run_build_mode(program);
    else if (program.is_subcommand_used("clean"))
        return run_clean_mode(program);
    else if (program.is_subcommand_used("update"))
        return run_update_mode(program);
    else if (program.is_subcommand_used("install"))
        return run_install_mode(program);
    else if (program.is_subcommand_used("doc"))
        return run_doc_mode(program);
    else if (program.is_subcommand_used("env"))
        return run_env_mode(program);
    else if (program.is_subcommand_used("fmt"))
        return run_fmt_mode(program);
    else if (program.is_subcommand_used("list"))
        return run_list_mode(program);
    else if (program.is_subcommand_used("run"))
        return run_run_mode(program);
    else if (program.is_subcommand_used("test"))
        return run_test_mode(program);
    else
    {
        std::cerr << "No command specified" << std::endl;
        std::cerr << program;
        return 1;
    }
}