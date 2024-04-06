#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <map>
#include <thread>
#include <functional>
#include <optional>
#include <argparse.h>

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

int main(int argc, char *argv[])
{
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