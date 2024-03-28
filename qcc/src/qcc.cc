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
    LINTER,
    IR,
    IR_BITCODE,
    ASSEMBLY,
    OBJECT,
    EXECUTABLE,
    UPX_COMPRESSED_EXECUTABLE,
    SHELLCODE,
    SHARED_LIBRARY,
    STATIC_LIBRARY,
    DOCUMENTATION,
    QUIXCC_TARGET_DEFAULT = OperatingMode::EXECUTABLE
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
    println("  -l, --license             Display license information");
    println("  -o, --output <file>       Output to <file>");
    println("  -I, --include <dir>       Add <dir> to the include path");
    println("  -L, --lib <dir>           Add <dir> to the library path");
    println("  -f<opt>                   Compiler settings. See docs for more info");
    println("  -l<name>                  Link with library <name>");
    println("  -D<name>[=<value>]        Define macro <name> with optional <value>");
    println("  -U<name>                  Undefine macro <name>");
    println("  -X                        Lex only; do not preprocess, compile, assemble or link");
    println("  -E                        Preprocess only; do not compile, assemble or link");
    println("  -S                        Compile only; do not assemble or link");
    println("  -c                        Compile and assemble, but do not link");
    println("  -g                        Generate debug information");
    println("  -O0                       Disable optimizations");
    println("  -Os                       Optimize for size");
    println("  -Oz                       Optimize for size (aggressive). Will apply post-processing");
    println("                            to the ELF file stripping unnecessary sections, applying");
    println("                            compression, and modifying the ELF header to reduce the file");
    println("                            size. This will also apply UPX compression if the tool is installed.");
    println("                            This may break compatibility with some tools and platforms.");
    println("  -O1                       Optimize for speed (level 1); default");
    println("  -O2                       Optimize for speed (level 2)");
    println("  -O3                       Optimize for speed (level 3)");
    println("  -O4                       Optimize -O3 and enable link-time optimization");
    println("  -O5                       Optimize -O4 and enable whole program optimization");
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
    println("   " + program_name + " source1.q source2.q -o myapp -O5 -lcrypto");
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

static OperatingMode get_operating_mode(const std::vector<std::string> &args, std::vector<std::string> &filtered, std::string &errmsg)
{
    (void)errmsg;
    (void)filtered;
    (void)args;

    errmsg = "Not implemented yet";
    return OperatingMode::DISP_LICENSE;
}

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv + argc);

    if (args.empty())
    {
        std::cerr << "Internal error: 0 arguments" << std::endl;
        return 1;
    }

    if (!quixcc_init())
    {
        std::cerr << "Failed to initialize QUIXCC" << std::endl;
        return 2;
    }

    program_name = args[0];

    std::string errmsg;

    switch (get_operating_mode(args, args, errmsg))
    {
    case OperatingMode::ERROR:
        std::cerr << errmsg << std::endl;
        print_help();
        return 1;
    case OperatingMode::DISP_HELP:
        print_help();
        return 0;
    case OperatingMode::DISP_VERSION:
        print_version();
        return 0;
    case OperatingMode::DISP_LICENSE:
        print_license();
        return 0;

    default:
        /// TODO: implement the rest of the modes
        break;
    }

    return 0;
}