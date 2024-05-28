#include <core/Writer.hh>
#include <iostream>
#include <mutex>

static std::mutex g_mutex;

void qpkg::core::log_owrite(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(g_mutex);

    std::cout.flush();
    std::cout << msg << std::endl;
    std::cout.flush();
}

void qpkg::core::log_ewrite(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(g_mutex);

    std::cerr.flush();
    std::cerr << msg << std::endl;
    std::cerr.flush();
}