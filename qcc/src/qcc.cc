// This code is horrible
// ill come back to it later
/// TODO: fix this code

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <map>
#include <quixcc.hpp>
#include <thread>
#include <functional>
#include <argparse.h>

static void SetupArgs(argparse::ArgumentParser &parser)
{
    static argparse::ArgumentParser build("build"), run("run"), clean("check"), install("install"), uninstall("uninstall"), _new("new"), help("help");

    parser.add_subparser(build);
    parser.add_subparser(run);
    parser.add_subparser(clean);
    parser.add_subparser(install);
    parser.add_subparser(uninstall);
    parser.add_subparser(_new);
    parser.add_subparser(help);
}

typedef std::function<int(const std::vector<std::string> &args)> SubcommandFn;
static std::map<std::string, SubcommandFn> _g_subcommands;

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("quixcc", "0.1");
    SetupArgs(program);

    try
    {
        /* Parse command line arguments */
        program.parse_args(argc, argv);
        return true;
    }
    catch (const std::exception &err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return false;
    }
}