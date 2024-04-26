////////////////////////////////////////////////////////////////////////////////////
///                                                                              ///
///    ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░          ///
///   ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░   ///
///    ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░    ///
///      ░▒▓█▓▒░                                                                 ///
///       ░▒▓██▓▒░                                                               ///
///                                                                              ///
///     * QUIX LANG COMPILER - The official compiler for the Quix language.      ///
///     * Copyright (C) 2020-2024 Wesley C. Jones                                ///
///                                                                              ///
///     The QUIX Compiler Suite is free software; you can redistribute it and/or ///
///     modify it under the terms of the GNU Lesser General Public               ///
///     License as published by the Free Software Foundation; either             ///
///     version 2.1 of the License, or (at your option) any later version.       ///
///                                                                              ///
///     The QUIX Compiler Suite is distributed in the hope that it will be       ///
///     useful, but WITHOUT ANY WARRANTY; without even the implied warranty of   ///
///     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        ///
///     Lesser General Public License for more details.                          ///
///                                                                              ///
///     You should have received a copy of the GNU Lesser General Public         ///
///     License along with the QUIX Compiler Suite; if not, see                  ///
///     <https://www.gnu.org/licenses/>.                                         ///
///                                                                              ///
////////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_IR_GENERIC_H__
#define __QUIXCC_IR_GENERIC_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <string>
#include <string_view>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <memory>
#include <optional>
#include <set>
#include <chrono>

#include <boost/serialization/access.hpp>
#include <boost/serialization/library_version_type.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/optional.hpp>

#include <boost/uuid/uuid.hpp>

namespace libquixcc
{
    namespace ir
    {
        enum class PrintMode
        {
            Text = 1,
            Bitcode = 2,
            Debug = 3,
            Unspecified = PrintMode::Text,
        };

        enum class DeserializeMode
        {
            Text = 1,
            Bitcode = 2,
            Unspecified = DeserializeMode::Text,
        };

        struct SourceLocation
        {
            const char *file;
            const char *function;
            unsigned line;
            unsigned column;

            SourceLocation(const char *file = __FILE__, const char *function = __PRETTY_FUNCTION__, unsigned line = __LINE__, unsigned column = 0)
                : file(file), function(function), line(line), column(column) {}

            SourceLocation(const SourceLocation &other) = default;
            SourceLocation(SourceLocation &&other) = default;
            SourceLocation &operator=(const SourceLocation &other) = default;

            friend std::ostream &operator<<(std::ostream &os, const SourceLocation &loc)
            {
                os << loc.file << ":" << loc.line << ":" << loc.column;
                return os;
            }

            template <typename Archive>
            void serialize(Archive &ar, const unsigned int version)
            {
                ar & file;
                ar & function;
                ar & line;
                ar & column;
            }
        };

        class Diagnostic
        {
            SourceLocation m_location;
            std::unordered_set<std::string> m_messages;
            std::chrono::time_point<std::chrono::system_clock> m_time;

        public:
            Diagnostic(const SourceLocation &location, const std::initializer_list<std::string> &messages) : m_location(location), m_messages(messages)
            {
                m_time = std::chrono::system_clock::now();
            }

            Diagnostic(const Diagnostic &other) = default;
            Diagnostic(Diagnostic &&other) = default;
            Diagnostic &operator=(const Diagnostic &other) = default;

            const SourceLocation &getLocation() const { return m_location; }
            const std::unordered_set<std::string> &getMessages() const { return m_messages; }
            const std::chrono::time_point<std::chrono::system_clock> &getTime() const { return m_time; }

            friend std::ostream &operator<<(std::ostream &os, const Diagnostic &diag)
            {
                os << diag.getLocation() << ": ";
                for (const auto &msg : diag.getMessages())
                {
                    os << msg << std::endl;
                }
                return os;
            }

            template <typename Archive>
            void serialize(Archive &ar, const unsigned int version)
            {
                ar & m_location;
                ar & m_messages;
                ar & m_time;
            }
        };

        template <typename T>
        class Result
        {
            Diagnostic *m_diagnostic;
            T m_value;

        public:
            Result(const T &value) : m_diagnostic(nullptr), m_value(value) {}
            Result(const Diagnostic &diag) : m_diagnostic(new Diagnostic(diag)), m_value(T()) {}
            Result(const Diagnostic &diag, const T &value) : m_diagnostic(new Diagnostic(diag)), m_value(value) {}
            inline ~Result()
            {
                if (m_diagnostic)
                {
                    delete m_diagnostic;
                    m_diagnostic = nullptr;
                }
            }

            Result(const Result &other) = delete;
            inline Result(Result &&other)
            {
                m_diagnostic = other.m_diagnostic;
                m_value = other.m_value;
                other.m_diagnostic = nullptr;
            }
            Result &operator=(const Result &other) = delete;

            T &get() { return m_value; }
            const T &get() const { return m_value; }
            const Diagnostic *problem() const { return m_diagnostic; }
            bool is_ok() const { return m_diagnostic == nullptr; }
            bool operator!() const { return !is_ok(); }

            friend std::ostream &operator<<(std::ostream &os, const Result &result)
            {
                if (result.is_ok())
                {
                    os << result.get();
                }
                else
                {
                    os << *result.problem();
                }
                return os;
            }

            T &operator*() { return m_value; }
            const T &operator*() const { return m_value; }
            T *operator->() { return m_value; }
            const T *operator->() const { return m_value; }
        };

        template <auto T, auto U>
        class IRGeneric;

        template <typename T>
        class IRRegistry
        {
            std::unordered_map<T, std::string_view> m_registry;

            IRRegistry() = default;
            IRRegistry(const IRRegistry &other) = delete;
            IRRegistry(IRRegistry &&other) = delete;
            IRRegistry &operator=(const IRRegistry &other) = delete;

        public:
            static IRRegistry &getInstance()
            {
                static IRRegistry instance;
                return instance;
            }

            template <auto U, auto V>
            inline void registerIRType(const IRGeneric<U, V> &type)
            {
                m_registry[type.getIRType()] = type.dialect_name();
            }

            template <auto U, auto V>
            inline void unregisterIRType(const IRGeneric<U, V> &type)
            {
                m_registry.erase(type.getIRType());
            }

            bool isRegistered(T type) const
            {
                return m_registry.contains(type);
            }

            std::string_view getIRTypeName(T type) const
            {
                if (!isRegistered(type))
                    return "";

                return m_registry.at(type);
            }

            std::set<T> getRegisteredIRTypes() const
            {
                std::set<T> types;
                for (const auto &[type, name] : m_registry)
                    types.insert(type);
                return types;
            }
        };

        template <auto T>
        class NodeIterator;

        template <auto T>
        class Value
        {
        protected:
            const static auto m_kind = T;

            /* Node Serialization */
            virtual Result<bool> print_text_impl(std::ostream &os, bool debug) const = 0;
            virtual Result<bool> print_bitcode_impl(std::ostream &os) const = 0;

            /* Node Deserialization */
            virtual Result<bool> deserialize_text_impl(std::istream &is) = 0;
            virtual Result<bool> deserialize_bitcode_impl(std::istream &is) = 0;

            /* Graph Operations & Caching */
            virtual boost::uuids::uuid graph_hash_impl() const = 0;

            /* IR Module Verification */
            virtual bool verify_impl() const = 0;

            std::vector<std::shared_ptr<Value>> m_children;

        public:
            Value() = default;
            virtual ~Value() = default;
            Value(const Value &other) = delete;
            Value(Value &&other) = delete;
            Value &operator=(const Value &other) = delete;

            /* Write IR to Output Stream */
            template <PrintMode mode = PrintMode::Unspecified>
            Result<bool> print(std::ostream &os) const
            {
                switch (mode)
                {
                case PrintMode::Bitcode:
                    return print_bitcode_impl(os);
                case PrintMode::Debug:
                    return print_text_impl(os, true);
                default:
                    return print_text_impl(os, false);
                }
            }

            /* Read IR from Input Stream */
            template <DeserializeMode mode = DeserializeMode::Unspecified>
            Result<bool> deserialize(std::istream &is)
            {
                switch (mode)
                {
                case DeserializeMode::Bitcode:
                    return deserialize_bitcode_impl(is);
                default:
                    return deserialize_text_impl(is);
                }
            }

            /* Verify IR Module */
            inline bool verify() const { return verify_impl(); }

            /* Count Nodes in IR Graph */
            inline size_t node_count() const { return m_children.size() + 1; }

            /* Calculate a cryptographic hash of the IR Graph */
            inline boost::uuids::uuid hash() const { return graph_hash_impl(); }

            /* Iterate over the IR Graph */
            bool hasChildren() const { return !m_children.empty(); }

            NodeIterator<T> begin() { return NodeIterator(this, 0); }
            NodeIterator<T> end() { return NodeIterator(this, m_children.size()); }

            Value &getChild(size_t index) { return *m_children.at(index); }

            bool operator==(const Value &other) const { return hash() == other.hash(); }

            template <auto U>
            bool isof() const
            {
                static_assert(std::is_enum_v<decltype(U)>, "IR Type must be an enumeration");
                static_assert(std::is_same_v<decltype(U), decltype(T)>, "IR Type must match Value Type");
                return false;
            }

            template <typename U>
            bool is() const
            {
                return typeid(U) == typeid(*this);
            }
        };

        template <auto T>
        class NodeIterator
        {
            friend class Value<T>;

            Value<T> *m_node;
            size_t m_index;

            NodeIterator(Value<T> *node, size_t index) : m_node(node), m_index(index) {}

        public:
            NodeIterator(const NodeIterator &other) = default;
            NodeIterator(NodeIterator &&other) = default;
            NodeIterator &operator=(const NodeIterator &other) = default;

            inline NodeIterator<T> &operator++()
            {
                m_index++;
                return *this;
            }
            inline NodeIterator<T> operator++(int)
            {
                NodeIterator tmp(*this);
                operator++();
                return tmp;
            }
            inline bool operator==(const NodeIterator &other) const { return m_node == other.m_node && m_index == other.m_index; }
            Value<T> &operator*() const
            {
                return m_node->getChild(m_index);
            }

            Value<T> *operator->() const
            {
                return &m_node->getChild(m_index);
            }
        };

        template <auto T, auto U>
        class IRGeneric
        {
        protected:
            std::shared_ptr<Value<U>> m_root;
            const static auto m_ir_type = T;

            /* Module Serialization */
            virtual Result<bool> print_text_impl(std::ostream &os, bool debug) const = 0;
            virtual Result<bool> print_bitcode_impl(std::ostream &os) const = 0;

            /* Module Deserialization */
            virtual Result<bool> deserialize_text_impl(std::istream &is) = 0;
            virtual Result<bool> deserialize_bitcode_impl(std::istream &is) = 0;

            /* Graph Operations & Caching */
            virtual size_t node_count_impl() const = 0;
            virtual boost::uuids::uuid graph_hash_impl() const = 0;
            virtual std::string_view graph_hash_algorithm_name_impl() const = 0;

            /* IR Dialect Information */
            virtual std::string_view ir_dialect_name_impl() const = 0;
            virtual unsigned ir_dialect_version_impl() const = 0;
            virtual std::string_view ir_dialect_family_impl() const = 0;
            virtual std::string_view ir_dialect_description_impl() const = 0;

            /* IR Module Verification */
            virtual bool verify_impl() const = 0;

        public:
            IRGeneric()
            {
                static_assert(std::is_enum_v<decltype(T)>, "IR Type must be an enumeration");
                static_assert(std::is_enum_v<decltype(U)>, "IR Node Type must be an enumeration");
                static_assert(!std::is_same_v<decltype(T), decltype(U)>, "IR Type and Node Type must be different enumerations");

                IRRegistry<decltype(T)>::getInstance().registerIRType(*this);
            }

            virtual ~IRGeneric() = default;
            IRGeneric(const IRGeneric &other) = delete;
            IRGeneric(IRGeneric &&other) = delete;
            IRGeneric &operator=(const IRGeneric &other) = delete;

            auto getIRType() const { return m_ir_type; }

            /* Write IR to Output Stream */
            template <PrintMode mode = PrintMode::Unspecified>
            Result<bool> print(std::ostream &os) const
            {
                switch (mode)
                {
                case PrintMode::Bitcode:
                    return print_bitcode_impl(os);
                case PrintMode::Debug:
                    return print_text_impl(os, true);
                default:
                    return print_text_impl(os, false);
                }
            }

            /* Read IR from Input Stream */
            template <DeserializeMode mode = DeserializeMode::Unspecified>
            Result<bool> deserialize(std::istream &is)
            {
                switch (mode)
                {
                case DeserializeMode::Bitcode:
                    return deserialize_bitcode_impl(is);
                default:
                    return deserialize_text_impl(is);
                }
            }

            /* Verify IR Module */
            inline bool verify() const { return verify_impl(); }

            /* Count Nodes in IR Graph */
            inline size_t node_count() const { return node_count_impl(); }

            /* Calculate a cryptographic hash of the IR Graph */
            inline boost::uuids::uuid hash() const { return graph_hash_impl(); }

            /* Get the name of the hash algorithm used to calculate the IR Graph hash */
            inline std::string_view hash_algorithm() const { return graph_hash_algorithm_name_impl(); }

            /* Get the name of the IR Dialect */
            inline std::string_view dialect_name() const { return ir_dialect_name_impl(); }

            /* Get the version of the IR Dialect */
            inline unsigned dialect_version() const { return ir_dialect_version_impl(); }

            /* Get the family of the IR Dialect */
            inline std::string_view dialect_family() const { return ir_dialect_family_impl(); }

            /* Get a description of the IR Dialect */
            inline std::string_view dialect_description() const { return ir_dialect_description_impl(); }

            /* Get Entry Point for IR Graph */
            virtual std::shared_ptr<Value<U>> getRoot() { return m_root; }
            virtual const std::shared_ptr<Value<U>> getRoot() const { return m_root; }
        };

        template <typename T, typename U>
        std::shared_ptr<T> QIR_ROOT(const U &_module)
        {
            return std::reinterpret_pointer_cast<T>(_module->getRoot());
        }

        template <typename T, typename U>
        T *qir_cast(U *node)
        {
            T *c;
            if ((c = dynamic_cast<T *>(node)))
                return c;
            throw std::bad_cast();
        }

        template <typename T, typename U>
        const T *qir_cast(const U *node)
        {
            const T *c;
            if ((c = dynamic_cast<const T *>(node)))
                return c;
            throw std::bad_cast();
        }

        template <typename T, typename U>
        std::shared_ptr<T> qir_cast(const std::shared_ptr<U> &node)
        {
            std::shared_ptr<T> c;
            if ((c = std::reinterpret_pointer_cast<T>(node)))
                return c;
            throw std::bad_cast();
        }

        template <typename T, typename U>
        std::shared_ptr<const T> qir_cast(const std::shared_ptr<const U> &node)
        {
            std::shared_ptr<const T> c;
            if ((c = std::reinterpret_pointer_cast<const T>(node)))
                return c;
            throw std::bad_cast();
        }
    } // namespace ir
} // namespace libquixcc

#endif // __QUIXCC_IR_GENERIC_H__