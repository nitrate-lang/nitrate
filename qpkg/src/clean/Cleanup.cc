#include <clean/Cleanup.hh>
#include <filesystem>
#include <core/Logger.hh>

bool qpkg::clean::CleanPackageSource(const std::string &package_src, bool recursive, bool verbose)
{
    if (verbose)
        LOG_ENABLE(core::DEBUG);
    
    std::filesystem::path package_src_path(package_src);

    if (!std::filesystem::exists(package_src_path))
    {
        LOG(core::ERROR) << "Package source path does not exist: " << package_src << std::endl;
        return false;
    }

    if (!std::filesystem::is_directory(package_src_path))
    {
        LOG(core::ERROR) << "Package source path is not a directory: " << package_src << std::endl;
        return false;
    }

    if (verbose)
    {
        if (recursive)
            LOG(core::INFO) << "Cleaning package source recursively" << std::endl;
        else
            LOG(core::INFO) << "Cleaning package source non-recursively" << std::endl;
    }

    std::filesystem::path cache_dir = package_src_path / ".qpkg" / "cache";
    std::filesystem::path build_dir = package_src_path / ".qpkg" / "build";

    if (std::filesystem::exists(cache_dir))
    {
        if (verbose)
            LOG(core::INFO) << "Removing cache directory: " << cache_dir << std::endl;

        std::filesystem::remove_all(cache_dir);
    }

    if (std::filesystem::exists(build_dir))
    {
        if (verbose)
            LOG(core::INFO) << "Removing build directory: " << build_dir << std::endl;
        std::filesystem::remove_all(build_dir);
    }

    /// TODO: implement recursive cleaning

    if (verbose)
        LOG(core::INFO) << "Package " << package_src << " cleaned" << std::endl;

    return true;
}