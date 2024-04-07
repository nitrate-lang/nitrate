#ifndef __QPKG_CONF_PARSER_HH__
#define __QPKG_CONF_PARSER_HH__

#include <string>
#include <unordered_map>
#include <memory>
#include <optional>
#include <variant>
#include <stdexcept>
#include <set>
#include <conf/Config.hh>

namespace qpkg
{
    namespace conf
    {
        class IParser
        {
        public:
            virtual ~IParser() = default;

            /**
             * @brief Parse QPKG package configuration file
             *
             * @param filepath Configuration filepath
             * @return std::optional<Config> Configuration object
             * @note If any error occurs, the function returns an empty optional.
             */
            std::optional<Config> parsef(const std::string &filepath);

            /**
             * @brief Parse QPKG package configuration content
             *
             * @param data Configuration file content
             * @return std::optional<Config> Configuration object
             * @note If any error occurs, the function returns an empty optional.
             */
            virtual std::optional<Config> parse(const std::string &content) = 0;
        };

        class JsonConfigParser : public IParser
        {
        public:
            /**
             * @brief Parse QPKG package configuration content
             *
             * @param data Configuration file content
             * @return std::optional<Config> Configuration object
             * @note If any error occurs, the function returns an empty optional.
             */
            std::optional<Config> parse(const std::string &content) override;
        };

        class YamlConfigParser : public IParser
        {
        public:
            /**
             * @brief Parse QPKG package configuration content
             *
             * @param data Configuration file content
             * @return std::optional<Config> Configuration object
             * @note If any error occurs, the function returns an empty optional.
             */
            std::optional<Config> parse(const std::string &content) override;
        };

        class XmlConfigParser : public IParser
        {
        public:
            /**
             * @brief Parse QPKG package configuration content
             *
             * @param data Configuration file content
             * @return std::optional<Config> Configuration object
             * @note If any error occurs, the function returns an empty optional.
             */
            std::optional<Config> parse(const std::string &content) override;
        };
    }
}

#endif // __QPKG_CONF_PARSER_HH__
