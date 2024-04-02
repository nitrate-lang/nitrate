// This code is horrible
// ill come back to it later
/// TODO: Finish implementing the rest of the modes
/// TODO: polish the frontend
/// TODO: considier writing a python wrapper library for libquixcc.so

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <map>
#include <quixcc.hpp>
#include <thread>
#include <functional>
#include <optional>

constexpr const char *COPYRIGHT = "Copyright (C) 2024 Wesley C. Jones";
constexpr const char *VERSION_STR = "qcc version 0.1.0 [2024-03] (Debian 10.0.0-1)";
constexpr const char *FULL_LICENSE = R"(Copyright (C) 2023 Wesley C. Jones All rights reserved.

This work is a part of the QUIX Language compiler suite and is proprietary.
Any unauthorized use, copying, redistribution, derivation, modification,
or any other utilization of this work without the express written consent
of the Deltasploit Development Team is strictly prohibited.

For inquiries, open an issue at <https://github.com/Kracken256/quixcc>. The
Deltasploit Development Team will respond to your request in time.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.)";

static std::string program_name;

enum class OperatingMode
{
    ERROR,
    DISP_HELP,
    DISP_VERSION,
    DISP_LICENSE,

    IR,
    IR_BITCODE,

    ASSEMBLY,
    OBJECT,
    EXECUTABLE,
    SHELLCODE,

    SHARED_LIBRARY,
    STATIC_LIBRARY,

    DOCUMENTATION,
    QUIXCC_TARGET_DEFAULT = OperatingMode::EXECUTABLE
};

struct Options
{
    std::vector<std::string> include_paths;
    std::vector<std::string> library_paths;
    std::vector<std::string> libraries;
    std::vector<std::string> undefines;
    std::vector<std::string> settings;
    std::vector<std::string> sources;
    std::map<std::string, std::string> defines;
    std::string output;
    quixcc::Verbosity verbosity;
    quixcc::OptimizationLevel optimization;
    bool debug;
    OperatingMode mode;

    Options()
    {
        verbosity = quixcc::Verbosity::NORMAL;
        optimization = quixcc::OptimizationLevel::SPEED_1;
        debug = false;
        mode = OperatingMode::QUIXCC_TARGET_DEFAULT;
        output = "a.out";
    }

    bool validate()
    {
        return true;
    }
};

static void println(const std::string &msg = "")
{
    std::cout << msg << std::endl;
}

static void print_help()
{
    println("Usage: " + program_name + " [options] file...\n");
    println("Options:");
    println("  -h, --help                Display this message");
    println("  -v, --version             Display version information");
    println("  --license                 Display license information");
    println("  -o, --output <file>       Output to <file>");
    println("  -I<dir>                   Add <dir> to the include path");
    println("  -L<dir>                   Add <dir> to the library path");
    println("  -f<opt>                   Compiler settings. See docs for more info");
    println("  -l<name>                  Link with library <name>");
    println("  -D<name>[=<value>]        Define macro <name> with optional <value>");
    println("  -U<name>                  Undefine macro <name>");
    println("  -emit-ir                  Emit the intermediate representation (LLVM IR)");
    println("  -emit-bc                  Emit the intermediate representation (LLVM bitcode)");
    println("  -S                        Compile only; do not assemble or link");
    println("  -c                        Compile and assemble, but do not link");
    println("  -g                        Generate debug information");
    println("  -O0                       Disable all optimizations");
    println("  -Os                       Optimize for size");
    println("  -Oz                       Optimize for size (aggressive). Will apply post-processing");
    println("                            to the ELF file stripping unnecessary sections, applying");
    println("                            compression, and modifying the ELF header to reduce the file");
    println("                            size.");
    println("                            This may break compatibility with some tools and platforms.");
    println("  -O1                       Optimize for speed (level 1); default");
    println("  -O2                       Optimize for speed (level 2)");
    println("  -O3                       Optimize for speed (level 3)");
    println("  -O4                       Optimize -O3 and enable link-time optimization");
    println("  -vv                       Enable verbose output");
    println("  -vvv                      Enable very verbose output; includes compiler internals");
    println();
    println("Examples:");
    println(" Just build a single source and output `a.out`:");
    println("   " + program_name + " source.q");
    println();
    println(" Build a single source and output to a specific file:");
    println("   " + program_name + " source.q -o myapp");
    println();
    println(" Build a single source and output to a specific file with debug information:");
    println("   " + program_name + " source.q -o myapp -g");
    println();
    println(" Build a multiple sources with maximum optimizations, link a library, and output to a specific file:");
    println("   " + program_name + " source1.q source2.q -o myapp -O4 -lcrypto");
    println();
    println("(See license for legal information)");
}

static void print_version()
{
    println(VERSION_STR);
}

static void print_license()
{
    println(VERSION_STR);
    println();
    println(FULL_LICENSE);
}

static bool init(const std::vector<std::string> &args)
{
    if (args.empty())
    {
        std::cerr << "Internal error: 0 arguments" << std::endl;
        return false;
    }

    if (!quixcc_init())
    {
        std::cerr << "Failed to initialize QUIXCC" << std::endl;
        return false;
    }

    return true;
}

static std::optional<Options> parse_options(const std::vector<std::string> &args)
{
    Options options;

    if (args.size() == 1)
    {
        std::cerr << "Error: no input files" << std::endl;
        options.mode = OperatingMode::DISP_HELP;
        return options;
    }

    auto it = args.begin() + 1;

    for (; it != args.end(); ++it)
    {
        if (*it == "-h" || *it == "--help")
        {
            options.mode = OperatingMode::DISP_HELP;
            return options;
        }
        else if (*it == "-v" || *it == "--version")
        {
            options.mode = OperatingMode::DISP_VERSION;
            return options;
        }
        else if (*it == "--license")
        {
            options.mode = OperatingMode::DISP_LICENSE;
            return options;
        }
        else if (*it == "-o" || *it == "--output")
        {
            if (++it == args.end())
            {
                std::cerr << "Error: missing argument for -o" << std::endl;
                return std::nullopt;
            }

            options.output = *it;
        }
        else if (it->starts_with("-I"))
        {
            auto sub = it->substr(2);
            if (sub.empty())
            {
                std::cerr << "Error: missing argument for -I" << std::endl;
                return std::nullopt;
            }
            options.include_paths.push_back(sub);
        }
        else if (it->starts_with("-L"))
        {
            auto sub = it->substr(2);
            if (sub.empty())
            {
                std::cerr << "Error: missing argument for -L" << std::endl;
                return std::nullopt;
            }
            options.library_paths.push_back(sub);
        }
        else if (it->starts_with("-f"))
        {
            if (it->size() == 2)
            {
                std::cerr << "Error: missing argument for -f" << std::endl;
                return std::nullopt;
            }
            options.settings.push_back(*it);
        }
        else if (it->starts_with("-l"))
        {
            auto sub = it->substr(2);
            if (sub.empty())
            {
                std::cerr << "Error: missing argument for -l" << std::endl;
                return std::nullopt;
            }
            options.libraries.push_back(sub);
        }
        else if (it->starts_with("-D"))
        {
            std::string define = it->substr(2);
            if (define.empty())
            {
                std::cerr << "Error: missing argument for -D" << std::endl;
                return std::nullopt;
            }

            auto pos = define.find('=');

            if (pos == std::string::npos)
                options.defines[define] = "1";
            else
                options.defines[define.substr(0, pos)] = define.substr(pos + 1);
        }
        else if (it->starts_with("-U"))
        {
            auto sub = it->substr(2);
            if (sub.empty())
            {
                std::cerr << "Error: missing argument for -U" << std::endl;
                return std::nullopt;
            }
            options.undefines.push_back(sub);
        }
        else if (*it == "-emit-ir")
        {
            options.mode = OperatingMode::IR;
        }
        else if (*it == "-emit-bc")
        {
            options.mode = OperatingMode::IR_BITCODE;
        }
        else if (*it == "-S")
        {
            options.mode = OperatingMode::ASSEMBLY;
        }
        else if (*it == "-c")
        {
            options.mode = OperatingMode::OBJECT;
        }
        else if (*it == "-g")
        {
            options.debug = true;
        }
        else if (*it == "-O0")
        {
            options.optimization = quixcc::OptimizationLevel::NONE;
        }
        else if (*it == "-Os")
        {
            options.optimization = quixcc::OptimizationLevel::SIZE;
        }
        else if (*it == "-Oz")
        {
            options.optimization = quixcc::OptimizationLevel::AGGRESSIVE_SIZE;
        }
        else if (*it == "-O1")
        {
            options.optimization = quixcc::OptimizationLevel::SPEED_1;
        }
        else if (*it == "-O2")
        {
            options.optimization = quixcc::OptimizationLevel::SPEED_2;
        }
        else if (*it == "-O3")
        {
            options.optimization = quixcc::OptimizationLevel::SPEED_3;
        }
        else if (*it == "-O4")
        {
            options.optimization = quixcc::OptimizationLevel::SPEED_4;
        }
        else if (*it == "-vv")
        {
            options.verbosity = quixcc::Verbosity::VERBOSE;
        }
        else if (*it == "-vvv")
        {
            options.verbosity = quixcc::Verbosity::VERY_VERBOSE;
        }
        else
        {
            if (!std::filesystem::exists(*it))
            {
                std::cerr << "Error: unknown option " << *it << std::endl;
                return std::nullopt;
            }

            options.sources.push_back(*it);
        }
    }

    return options;
}

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv + argc);

    if (program_name = args[0], !init(args))
        return 1;

    std::optional<Options> options;

    if (options = parse_options(args), !options.has_value())
        return 2;

    if (!options->validate())
        return 3;

    auto builder = quixcc::CompilerBuilder();

    for (const auto &path : options->sources)
        builder.add_source(path);
    for (const auto &path : options->include_paths)
        builder.add_include(path);
    for (const auto &path : options->library_paths)
        builder.add_library(path);
    for (const auto &lib : options->libraries)
        builder.link_library(lib);
    for (const auto &[name, value] : options->defines)
        builder.define(name, value);
    for (const auto &name : options->undefines)
        builder.undefine(name);
    for (const auto &setting : options->settings)
        builder.set_flag(setting);
    builder.set_output(options->output);
    builder.set_verbosity(options->verbosity);
    builder.set_optimization(options->optimization);
    builder.set_debug(options->debug);

    switch (options->mode)
    {
    case OperatingMode::DISP_HELP:
        print_help();
        return 0;
    case OperatingMode::DISP_VERSION:
        print_version();
        return 0;
    case OperatingMode::DISP_LICENSE:
        print_license();
        return 0;
    case OperatingMode::IR:
        builder.opt("-emit-ir");
        break;
    case OperatingMode::IR_BITCODE:
        builder.opt("-emit-bc");
        break;
    case OperatingMode::ASSEMBLY:
        builder.opt("-S");
        break;
    case OperatingMode::OBJECT:
        builder.opt("-c");
        break;
    case OperatingMode::EXECUTABLE:
        break;
    case OperatingMode::SHELLCODE:
        throw std::runtime_error("Not implemented");
    case OperatingMode::SHARED_LIBRARY:
        throw std::runtime_error("Not implemented");
    case OperatingMode::STATIC_LIBRARY:
        throw std::runtime_error("Not implemented");
    case OperatingMode::DOCUMENTATION:
        throw std::runtime_error("Not implemented");

    default:
        return 4;
    }

    auto compiler = builder.build();

    if (compiler.run().puts().ok())
        return 0;

    std::remove(options->output.c_str());
    return -1;
}