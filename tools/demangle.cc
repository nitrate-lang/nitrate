#include <quixcc.h>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <mangled symbol>" << std::endl;
        return 1;
    }

    std::string mangled = argv[1];

    char *demangled = quixcc_demangle_symbol(mangled.c_str());
    if (!demangled)
    {
        std::cerr << "Invalid mangled symbol" << std::endl;
        return 1;
    }

    std::cout << demangled << std::endl;
    free(demangled);

    return 0;
}