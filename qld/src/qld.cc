#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <map>
#include <thread>
#include <functional>
#include <optional>

constexpr const char *COPYRIGHT = "Copyright (C) 2024 Wesley C. Jones";
constexpr const char *VERSION_STR = "qld version 0.1.0 [2024-03] (Debian 10.0.0-1)";
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

static std::string program_name;

enum class OperatingMode
{
    ERROR,
    DISP_HELP,
    DISP_VERSION,
    DISP_LICENSE,

    DYNAMIC_EXECUTABLE,
    STATIC_EXECUTABLE,
    SHARED_LIBRARY,
    STATIC_LIBRARY,

    DOCUMENTATION,
    QUIXCC_TARGET_DEFAULT = OperatingMode::DYNAMIC_EXECUTABLE
};

enum class OptimizationLevel
{
    DEFAULT,
    LTO,
};

struct Options
{
    std::vector<std::string> library_paths;
    std::vector<std::string> libraries;
    std::vector<std::string> settings;
    std::vector<std::string> objects;
    std::vector<std::string> platform_options;
    std::string output;
    OptimizationLevel optimization;
    bool debug;
    bool verbose;
    bool strip;
    bool pack;
    OperatingMode mode;

    Options()
    {
        verbose = false;
        optimization = OptimizationLevel::DEFAULT;
        debug = false;
        mode = OperatingMode::QUIXCC_TARGET_DEFAULT;
    }

    bool validate()
    {
        try
        {
            for (const auto &obj : objects)
            {
                if (!std::filesystem::exists(obj))
                {
                    std::cerr << "Error: file not found: " << obj << std::endl;
                    return false;
                }
            }

            for (const auto &path : library_paths)
            {
                if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
                {
                    std::cerr << "Error: library path not found: " << path << std::endl;
                    return false;
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }

        switch (mode)
        {
        case OperatingMode::ERROR:
            return false;
        case OperatingMode::DISP_HELP:
        case OperatingMode::DISP_VERSION:
        case OperatingMode::DISP_LICENSE:
            return true;
        case OperatingMode::DYNAMIC_EXECUTABLE:
        case OperatingMode::STATIC_EXECUTABLE:
        case OperatingMode::SHARED_LIBRARY:
            if (pack && (mode != OperatingMode::DYNAMIC_EXECUTABLE && mode != OperatingMode::STATIC_EXECUTABLE))
            {
                std::cerr << "Error: packing is only supported for executables" << std::endl;
                return false;
            }
            if (output.empty())
            {
                std::cerr << "Error: missing output file" << std::endl;
                return false;
            }
            if (objects.empty())
            {
                std::cerr << "Error: missing input files" << std::endl;
                return false;
            }
            return true;
        case OperatingMode::STATIC_LIBRARY:
            if (pack)
            {
                std::cerr << "Error: packing is not supported for static libraries" << std::endl;
                return false;
            }
            if (output.empty())
            {
                std::cerr << "Error: missing output file" << std::endl;
                return false;
            }
            if (!output.ends_with(".a"))
            {
                std::cerr << "Error: output file must have .a extension" << std::endl;
                return false;
            }
            if (objects.empty())
            {
                std::cerr << "Error: missing input files" << std::endl;
                return false;
            }
            if (strip)
            {
                std::cerr << "Error: cannot strip symbols from a static library" << std::endl;
                return false;
            }
            return true;
        default:
            return false;
        }
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
    println("  -L<dir>                   Add <dir> to the library path");
    println("  -f<opt>                   Linker settings. See docs for more info");
    println("  -l<name>                  Link with library <name>");
    println("  -flto                     Enable link-time optimization");
    println("  -vv                       Enable verbose output");
    println("  -static                   Link statically into an executable");
    println("  -staticlib                Link as a static library");
    println("  -sharedlib                Link as a shared library");
    println("  -pack                     UPX pack the executable");
    println("  -F=<opt>                  Set a platform-specific option");
    println("  -s, --strip               Strip symbols from output");
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
        else if (*it == "-flto")
        {
            options.optimization = OptimizationLevel::LTO;
        }
        else if (*it == "-vv")
        {
            options.verbose = true;
        }
        else if (*it == "-static")
        {
            options.mode = OperatingMode::STATIC_EXECUTABLE;
        }
        else if (*it == "-staticlib")
        {
            options.mode = OperatingMode::STATIC_LIBRARY;
        }
        else if (*it == "-sharedlib")
        {
            options.mode = OperatingMode::SHARED_LIBRARY;
        }
        else if (*it == "-s" || *it == "--strip")
        {
            options.strip = true;
        }
        else if (it->starts_with("-F="))
        {
            auto sub = it->substr(3);
            if (sub.empty())
            {
                std::cerr << "Error: missing argument for -F" << std::endl;
                return std::nullopt;
            }
            options.platform_options.push_back(sub);
        }
        else if (*it == "-pack")
        {
            options.pack = true;
        }
        else
        {
            if (!std::filesystem::exists(*it))
            {
                std::cerr << "Error: unknown option " << *it << std::endl;
                return std::nullopt;
            }

            options.objects.push_back(*it);
        }
    }

    if (options.mode == OperatingMode::STATIC_LIBRARY && options.output.empty() && !options.objects.empty())
    {
        // strip file extension from first object file
        auto obj = options.objects[0];
        auto pos = obj.find_last_of('.');
        if (pos != std::string::npos)
            obj = obj.substr(0, pos);

        options.output = "lib" + obj + ".a";
    }
    else if (options.mode == OperatingMode::SHARED_LIBRARY && options.output.empty() && !options.objects.empty())
    {
        // strip file extension from first object file
        auto obj = options.objects[0];
        auto pos = obj.find_last_of('.');
        if (pos != std::string::npos)
            obj = obj.substr(0, pos);

        options.output = "lib" + obj + ".so";
    }
    else if (options.output.empty())
    {
        options.output = "a.out";
    }

    return options;
}

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv + argc);

    program_name = args[0];

    std::optional<Options> options;

    if (options = parse_options(args), !options.has_value())
        return 2;

    if (!options->validate())
        return 3;

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
    case OperatingMode::STATIC_LIBRARY:
    {
        std::string cmd = "ar rcs " + options->output;

        for (const auto &obj : options->objects)
            cmd += " " + obj;

        return std::system(cmd.c_str());
    }
    default:
        break;
    }

    std::string cmd = "cc -o " + options->output;

    for (const auto &obj : options->objects)
        cmd += " '" + obj + "'";

    for (const auto &lib : options->libraries)
        cmd += " -l" + lib;

    for (const auto &path : options->library_paths)
        cmd += " -L" + path;

    if (options->optimization == OptimizationLevel::LTO)
        cmd += " -flto";

    if (options->verbose)
        cmd += " -v";

    if (options->strip)
        cmd += " -s";

    for (const auto &opt : options->platform_options)
        cmd += " " + opt;

    switch (options->mode)
    {
    case OperatingMode::STATIC_EXECUTABLE:
        cmd += " -static";
        break;
    case OperatingMode::SHARED_LIBRARY:
        cmd += " -shared";
        break;
    default:
        break;
    }

    int s;
    if ((s = std::system(cmd.c_str())) != 0)
        return s;

    if (options->pack)
        return std::system(("upx " + options->output).c_str());

    return 0;
}