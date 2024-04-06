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
constexpr const char *VERSION_STR = "qpkg version 0.1.0 [2024-03] (Debian 10.0.0-1)";
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

int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv + argc);

    std::cerr << "qpkg not implemented yet" << std::endl;

    return 1;
}