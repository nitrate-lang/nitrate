#include <clean/Cleanup.hh>
#include <filesystem>
#include <core/Logger.hh>
#include <conf/Parser.hh>
#include <conf/Validate.hh>

static std::optional<qpkg::conf::Config> get_config(const std::filesystem::path &base)
{
    if (std::filesystem::exists(base / "qpkg.yaml"))
    {
        auto c = qpkg::conf::YamlConfigParser().parsef(base / "qpkg.yaml");

        if (!c)
        {
            LOG(qpkg::core::ERROR) << "Failed to parse configuration file: " << base / "qpkg.yaml" << std::endl;
            return std::nullopt;
        }

        if (!qpkg::conf::ValidateConfig(*c, base))
        {
            LOG(qpkg::core::ERROR) << "Failed to validate configuration" << std::endl;
            return std::nullopt;
        }

        qpkg::conf::PopulateConfig(*c);

        return c;
    }
    else
    {
        LOG(qpkg::core::ERROR) << "No configuration file found in package source directory" << std::endl;
        return std::nullopt;
    }
}

static bool recursve_subpackages(const std::filesystem::path &base, bool verbose)
{
    auto c = get_config(base);

    if (!c)
        return false;

    auto packages = (*c)["packages"].as<std::vector<std::string>>();

    for (const auto &p : packages)
        qpkg::clean::CleanPackageSource(base / p, verbose);

    return true;
}

bool qpkg::clean::CleanPackageSource(const std::string &package_src, bool verbose)
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
        LOG(core::INFO) << "Cleaning package source recursively" << std::endl;
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

    recursve_subpackages(package_src_path, verbose);

    if (verbose)
        LOG(core::INFO) << "Package " << package_src << " cleaned" << std::endl;

    return true;
}