#ifndef __QPKG_INIT_PACKAGE_HH__
#define __QPKG_INIT_PACKAGE_HH__

#include <string>
#include <set>
#include <optional>
#include <filesystem>
#include <conf/Config.hh>

namespace qpkg
{
    namespace init
    {
        enum class PackageType
        {
            PROGRAM,
            STATICLIB,
            SHAREDLIB
        };

        class Package
        {
            std::filesystem::path m_output;
            std::string m_name;
            std::string m_license;
            std::string m_author;
            std::string m_email;
            std::string m_url;
            std::string m_version;
            PackageType m_type;
            bool m_verbose;
            bool m_force;

            bool createPackage();

            static bool validateName(const std::string &name);
            static bool validateVersion(const std::string &version);
            static bool validateEmail(const std::string &email);
            static bool validateUrl(const std::string &url);
            static bool validateLicense(const std::string &license);

            void writeGitIgnore();
            void writeLicense();

        public:
            Package(std::filesystem::path output,
                    std::string name,
                    std::string license,
                    std::string author,
                    std::string email,
                    std::string url,
                    std::string version,
                    PackageType type,
                    bool verbose,
                    bool force)
                : m_output(output),
                  m_name(name),
                  m_license(license),
                  m_author(author),
                  m_email(email),
                  m_url(url),
                  m_version(version),
                  m_type(type),
                  m_verbose(verbose),
                  m_force(force) {}

            bool create();
        };

        class PackageBuilder
        {
            std::string m_output;
            std::string m_name;
            std::string m_license;
            std::string m_author;
            std::string m_email;
            std::string m_url;
            std::string m_version;
            PackageType m_type;
            bool m_verbose;
            bool m_force;

        public:
            PackageBuilder() : m_verbose(false), m_force(false) {}

            PackageBuilder &output(const std::string &output);
            PackageBuilder &name(const std::string &name);
            PackageBuilder &license(const std::string &license);
            PackageBuilder &author(const std::string &author);
            PackageBuilder &email(const std::string &email);
            PackageBuilder &url(const std::string &url);
            PackageBuilder &version(const std::string &version);
            PackageBuilder &type(PackageType type);
            PackageBuilder &verbose(bool verbose);
            PackageBuilder &force(bool force);

            Package build();
        };
    }
}

#endif // __QPKG_INIT_PACKAGE_HH__