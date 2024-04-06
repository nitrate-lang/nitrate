#ifndef __QPKG_CONF_PARSER_HH__
#define __QPKG_CONF_PARSER_HH__

#include <string>
#include <unordered_map>
#include <memory>
#include <optional>
#include <variant>
#include <stdexcept>

namespace qpkg
{
    namespace conf
    {
        enum class ConfigItemSerializationTarget
        {
            JSON
        };

        class ConfigSerializerException : public std::runtime_error
        {
        public:
            ConfigSerializerException(const std::string &message) : std::runtime_error(message) {}
        };

        struct ConfigValue
        {
            std::variant<std::string, int64_t, bool> m_value;

            ConfigValue() = default;
            ConfigValue(std::variant<std::string, int64_t, bool> value) : m_value(value) {}

            template <typename T>
            T as() const
            {
                return std::get<T>(m_value);
            }

            template <typename T>
            bool is() const
            {
                return std::holds_alternative<T>(m_value);
            }

            bool operator==(const ConfigValue &value) const
            {
                return m_value == value.m_value;
            }
        };

        struct ConfigGroup
        {
            std::unordered_map<std::string, ConfigValue> m_items;

            ConfigGroup() = default;

            /**
             * @brief Dump the configuration tree to a string
             *
             * @param target Serialization target
             * @return std::string Serialized configuration tree
             * @note If the target is not supported, the function throws ConfigSerializerException.
             */
            std::string dump(ConfigItemSerializationTarget target = ConfigItemSerializationTarget::JSON) const;

            inline const ConfigValue &operator[](const std::string &key) const { return m_items.at(key); }

            template <typename T>
            void set(const std::string &key, T value)
            {
                m_items[key] = ConfigValue(value);
            }

            template <typename T>
            bool has(const std::string &key) const
            {
                return m_items.contains(key) && m_items.at(key).is<T>();
            }

            template <typename T>
            bool is(const std::string &key, T value) const
            {
                return has<T>(key) && m_items.at(key).as<T>() == value;
            }

            bool operator==(const ConfigGroup &grp) const
            {
                return m_items == grp.m_items;
            }
        };

        struct Config
        {
            ConfigGroup m_root;
            int m_version;

            Config() : m_version(0) {}
            Config(ConfigGroup grp, int version = 0)
                : m_root(std::move(grp)), m_version(version) {}

            /**
             * @brief Dump the configuration to a string
             *
             * @param target Serialization target
             * @return std::string Serialized configuration
             * @note If the target is not supported, the function throws ConfigSerializerException.
             */
            std::string dump(ConfigItemSerializationTarget target = ConfigItemSerializationTarget::JSON) const;

            bool operator==(const Config &cfg) const
            {
                return m_version == cfg.m_version && m_root == cfg.m_root;
            }
        };

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
