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
///     * Copyright (C) 2024 Wesley C. Jones                                     ///
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
#include <algorithm>
#include <functional>
#include <optional>
#include <set>
#include <stack>
#include <queue>
#include <sstream>
#include <chrono>

#include <boost/serialization/access.hpp>
#include <boost/serialization/library_version_type.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/optional.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/endian/conversion.hpp>

namespace libquixcc
{
    namespace ir
    {
        enum class PrintMode
        {
            Text = 1,
            Debug = 3,
            Unspecified = PrintMode::Text,
        };

        enum class DeserializeMode
        {
            Text = 1,
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
            Diagnostic(const std::initializer_list<std::string> &messages, const SourceLocation &location = SourceLocation()) : m_location(location), m_messages(messages)
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
            Result() : m_diagnostic(nullptr), m_value(T()) {}
            Result(const T &value) : m_diagnostic(nullptr), m_value(value) {}
            Result(const Diagnostic &diag) : m_diagnostic(new Diagnostic(diag)), m_value(T()) {}
            Result(const Diagnostic &diag, const T &value) : m_diagnostic(new Diagnostic(diag)), m_value(value) {}
            ~Result()
            {
                if (m_diagnostic)
                {
                    delete m_diagnostic;
                    m_diagnostic = nullptr;
                }
            }

            Result(const Result &other)
            {
                m_diagnostic = other.m_diagnostic;
                m_value = other.m_value;
            }

            Result(Result &&other)
            {
                m_diagnostic = other.m_diagnostic;
                m_value = other.m_value;
                other.m_diagnostic = nullptr;
            }

            Result &operator=(const Result &other)
            {
                if (this != &other)
                {
                    m_diagnostic = other.m_diagnostic;
                    m_value = other.m_value;
                }
                return *this;
            }

            T &get() { return m_value; }
            const T &get() const { return m_value; }
            const Diagnostic *problem() const { return m_diagnostic; }
            bool is_ok() const { return m_diagnostic == nullptr; }
            bool operator!() const { return !is_ok(); }

            friend std::ostream &operator<<(std::ostream &os, const Result &result)
            {
                if (result.is_ok())
                    os << result.get();
                else
                    os << *result.problem();
                return os;
            }

            T &operator*() { return m_value; }
            const T &operator*() const { return m_value; }
            T *operator->() { return m_value; }
            const T *operator->() const { return m_value; }
        };

        template <typename X, typename Y>
        X *qir_cast(Y *node)
        {
            return reinterpret_cast<X *>(node);
        }

        template <typename X, typename Y>
        const X *qir_cast(const Y *node)
        {
            return reinterpret_cast<const X *>(node);
        }

        template <typename X, typename Y>
        std::shared_ptr<X> qir_cast(const std::shared_ptr<Y> &node)
        {
            return std::reinterpret_pointer_cast<X>(node);
        }

        template <typename X, typename Y>
        std::shared_ptr<const X> qir_cast(const std::shared_ptr<const Y> &node)
        {
            return std::reinterpret_pointer_cast<const X>(node);
        }

        template <auto T = 0, typename W = double>
        class Value
        {
        protected:
            /* Node Serialization */
            virtual Result<bool> print_impl(std::ostream &os, bool debug) const = 0;

            /* Node Deserialization */
            virtual Result<bool> deserialize_impl(std::istream &is) = 0;

            /* Graph Operations & Caching */
            virtual boost::uuids::uuid hash_impl() const = 0;

            /* IR Module Verification */
            virtual bool verify_impl() const = 0;

            std::set<std::pair<std::shared_ptr<Value>, W>> m_children;

        public:
            Value() = default;
            virtual ~Value() = default;
            Value(const Value &other) = delete;
            Value(Value &&other) = delete;
            Value &operator=(const Value &other) = delete;

            template <typename U>
            bool is() const
            {
                return typeid(*this) == typeid(U);
            }

            template <typename U>
            bool is(const U *other) const
            {
                if (typeid(*this) != typeid(*other))
                    return false;

                return *qir_cast<const U>(this) == *other;
            }

            /* Write IR to Output Stream */
            template <PrintMode mode = PrintMode::Unspecified>
            Result<bool> print(std::ostream &os, bool newline = false) const
            {
                switch (mode)
                {
                case PrintMode::Debug:
                {
                    Result<bool> result(print_impl(os, true));
                    if (newline)
                        os << std::endl;
                    return result;
                }
                default:
                {
                    Result<bool> result(print_impl(os, false));
                    if (newline)
                        os << std::endl;
                    return result;
                }
                }
            }

            /* Write UUID to Output Stream */
            inline void printid(std::ostream &os) const { os << boost::uuids::to_string(hash()); }

            /* Read IR from Input Stream */
            template <DeserializeMode mode = DeserializeMode::Unspecified>
            Result<bool> deserialize(std::istream &is)
            {
                switch (mode)
                {
                default:
                    return deserialize_impl(is);
                }
            }

            /* Verify IR Module */
            inline bool verify() const { return verify_impl(); }

            /* Count Nodes in IR Graph */
            inline size_t node_count() const { return m_children.size() + 1; }

            /* Calculate a cryptographic hash of the IR Graph */
            inline boost::uuids::uuid hash() const { return hash_impl(); }

            /* Iterate over the IR Graph */
            bool has_children() const { return !m_children.empty(); }
            bool empty() const { return m_children.empty(); }

            auto begin() { return m_children.begin(); }
            auto end() { return m_children.end(); }
            const auto cbegin() const { return m_children.cbegin(); }
            const auto cend() const { return m_children.cend(); }

            const std::set<std::pair<std::shared_ptr<Value>, W>> &children() const { return m_children; }

            Value<T> &operator[](const std::string_view &uuid)
            {
                for (const auto &child : m_children)
                {
                    if (child.first->hash() == boost::uuids::string_generator()(uuid.data()))
                        return *child.first;
                }

                throw std::out_of_range("Value not found");
            }

            const Value<T> &operator[](const std::string_view &uuid) const
            {
                for (const auto &child : m_children)
                {
                    if (child.first->hash() == boost::uuids::string_generator()(uuid.data()))
                        return *child.first;
                }

                throw std::out_of_range("Value not found");
            }

            Value<T> &add_child(std::shared_ptr<Value<T>> child, W weight = W())
            {
                m_children.insert(std::make_pair(child, weight));
                return *this;
            }

            Value<T> &remove_child(size_t index)
            {
                m_children.erase(m_children.begin() + index);
                return *this;
            }

            Value<T> &remove_child(const Value<T> *child)
            {
                m_children.erase(std::remove_if(m_children.begin(), m_children.end(), [child](const auto &pair)
                                                { return pair.first.get() == child; }),
                                 m_children.end());
                return *this;
            }

            Value<T> &remove_child(std::function<bool(const Value<T> *)> predicate)
            {
                m_children.erase(std::remove_if(m_children.begin(), m_children.end(), [predicate](const auto &pair)
                                                { return predicate(pair.first.get()); }),
                                 m_children.end());
                return *this;
            }

            bool operator==(const Value &other) const { return hash() == other.hash(); }
            bool operator>(const Value &other) const { return hash() > other.hash(); }

            /* Graph Algorithims */
            bool is_cyclic() const
            {
                std::set<const Value<T> *> visited;
                std::stack<const Value<T> *> stack;
                stack.push(this);

                while (!stack.empty())
                {
                    const Value<T> *current = stack.top();
                    stack.pop();

                    if (visited.contains(current))
                        return true;

                    visited.insert(current);

                    for (const auto &child : current->m_children)
                    {
                        stack.push(child.first.get());
                    }
                }

                return false;
            }

            inline bool is_acyclic() const { return !is_cyclic(); }

            bool contains(std::function<bool(const Value<T> *)> predicate) const
            {
                std::set<const Value<T> *> visited;
                std::stack<const Value<T> *> stack;
                stack.push(this);

                while (!stack.empty())
                {
                    const Value<T> *current = stack.top();
                    stack.pop();

                    if (visited.contains(current))
                        continue;

                    visited.insert(current);

                    if (predicate(current))
                        return true;

                    for (const auto &child : current->m_children)
                    {
                        stack.push(child.first.get());
                    }
                }

                return false;
            }

            std::vector<Value<T> *> find_shortest_path(std::function<bool(Value<T> *)> predicate)
            {
                assert(predicate != nullptr);

                std::vector<Value<T> *> path;
                std::set<Value<T> *> visited;
                std::stack<Value<T> *> stack;
                std::unordered_map<Value<T> *, Value<T> *> parent;

                stack.push(this);
                parent[this] = nullptr;

                while (!stack.empty())
                {
                    Value<T> *current = stack.top();
                    stack.pop();

                    if (visited.contains(current))
                        continue;

                    visited.insert(current);

                    if (predicate(current))
                    {
                        Value<T> *current_node = current;
                        while (current_node != nullptr)
                        {
                            path.push_back(current_node);
                            current_node = parent[current_node];
                        }

                        std::reverse(path.begin(), path.end());
                        return path;
                    }

                    for (auto &child : current->m_children)
                    {
                        stack.push(child.first.get());
                        parent[child.first.get()] = current;
                    }
                }

                return path;
            }

            size_t dfs_preorder(std::function<void(Value<T> *)> callback)
            {
                size_t count = 0;
                std::set<Value<T> *> visited;
                std::stack<Value<T> *> stack;
                stack.push(this);

                while (!stack.empty())
                {
                    Value<T> *current = stack.top();
                    stack.pop();

                    if (visited.contains(current))
                        continue;

                    visited.insert(current);

                    callback(current);
                    count++;

                    for (const auto &child : current->m_children)
                    {
                        stack.push(child.first.get());
                    }
                }

                return count;
            }

            size_t dfs_preorder(std::function<void(const Value<T> *)> callback) const
            {
                return const_cast<Value<T> *>(this)->dfs_preorder([&callback](Value<T> *node)
                                                                  { callback(node); });
            }

            size_t dfs_postorder(std::function<void(Value<T> *)> callback)
            {
                size_t count = 0;
                std::set<Value<T> *> visited;
                std::stack<Value<T> *> stack;
                stack.push(this);

                while (!stack.empty())
                {
                    Value<T> *current = stack.top();
                    stack.pop();

                    if (visited.contains(current))
                        continue;

                    visited.insert(current);

                    for (const auto &child : current->m_children)
                    {
                        stack.push(child.first.get());
                    }

                    callback(current);
                    count++;
                }

                return count;
            }

            size_t dfs_postorder(std::function<void(const Value<T> *)> callback) const
            {
                return const_cast<Value<T> *>(this)->dfs_postorder([&callback](Value<T> *node)
                                                                   { callback(node); });
            }

            size_t bfs_preorder(std::function<void(Value<T> *)> callback)
            {
                size_t count = 0;
                std::set<Value<T> *> visited;
                std::queue<Value<T> *> queue;
                queue.push(this);

                while (!queue.empty())
                {
                    Value<T> *current = queue.front();
                    queue.pop();

                    if (visited.contains(current))
                        continue;

                    visited.insert(current);

                    callback(current);
                    count++;

                    for (const auto &child : current->m_children)
                    {
                        queue.push(child.first.get());
                    }
                }

                return count;
            }

            size_t bfs_preorder(std::function<void(const Value<T> *)> callback) const
            {
                return const_cast<Value<T> *>(this)->bfs_preorder([&callback](Value<T> *node)
                                                                  { callback(node); });
            }

            size_t bfs_postorder(std::function<void(Value<T> *)> callback)
            {
                size_t count = 0;
                std::set<Value<T> *> visited;
                std::queue<Value<T> *> queue;
                queue.push(this);

                while (!queue.empty())
                {
                    Value<T> *current = queue.front();
                    queue.pop();

                    if (visited.contains(current))
                        continue;

                    visited.insert(current);

                    for (const auto &child : current->m_children)
                    {
                        queue.push(child.first.get());
                    }

                    callback(current);
                    count++;
                }

                return count;
            }

            size_t bfs_postorder(std::function<void(const Value<T> *)> callback) const
            {
                return const_cast<Value<T> *>(this)->bfs_postorder([&callback](Value<T> *node)
                                                                   { callback(node); });
            }
        };

        class Hasher
        {
            boost::uuids::detail::sha1 m_hasher;

        public:
            Hasher() = default;

            template <typename T>
            Hasher &add(const T value)
            {
                std::stringstream ss;
                value->print(ss);
                m_hasher.process_bytes(ss.str().data(), ss.str().size());
                return *this;
            }

            template <typename T, typename W>
            Hasher &add(const std::set<std::pair<T, W>> &values)
            {
                for (const auto &value : values)
                {
                    add(value.first);

                    /* TODO: fix this */
                    m_hasher.process_bytes(reinterpret_cast<const void *>(&value.second), sizeof(W));
                }
                return *this;
            }

            inline Hasher &add(const std::string_view &value)
            {
                m_hasher.process_bytes(value.data(), value.size());
                return *this;
            }

            inline Hasher &add(const char *value)
            {
                m_hasher.process_bytes(value, std::strlen(value));
                return *this;
            }

            inline Hasher &add(const std::string &value)
            {
                m_hasher.process_bytes(value.data(), value.size());
                return *this;
            }

            /* Generate tag based on source metadata */
            inline Hasher &gettag(const std::string_view &tag = __PRETTY_FUNCTION__)
            {
                m_hasher.process_bytes(tag.data(), tag.size());
                return *this;
            }

            inline boost::uuids::uuid hash()
            {

                boost::uuids::detail::sha1::digest_type digest;
                m_hasher.get_digest(digest);
                boost::uuids::uuid ret;
                auto p = ret.begin();
                for (std::size_t i{}; i != 4; p += 4, ++i)
                {
                    auto const d = boost::endian::native_to_big(digest[i]);
                    std::memcpy(p, &d, 4);
                }
                return ret;
            }
        };

        template <auto T, auto U>
        class IRModule
        {
        protected:
            std::shared_ptr<Value<U>> m_root;
            std::string m_name;
            const static auto m_ir_type = T;

            /* Module Serialization */
            virtual Result<bool> print_impl(std::ostream &os, bool debug) const = 0;

            /* Module Deserialization */
            virtual Result<bool> deserialize_impl(std::istream &is) = 0;

            /* IR Dialect Information */
            virtual std::string_view ir_dialect_name_impl() const = 0;
            virtual unsigned ir_dialect_version_impl() const = 0;
            virtual std::string_view ir_dialect_family_impl() const = 0;
            virtual std::string_view ir_dialect_description_impl() const = 0;

            /* IR Module Verification */
            virtual bool verify_impl() const = 0;

        public:
            IRModule(const std::string_view &name)
            {
                static_assert(std::is_enum_v<decltype(T)>, "IR Type must be an enumeration");
                static_assert(std::is_enum_v<decltype(U)>, "IR Node Type must be an enumeration");
                static_assert(!std::is_same_v<decltype(T), decltype(U)>, "IR Type and Node Type must be different enumerations");

                m_name = name;
            }

            virtual ~IRModule() = default;
            IRModule(const IRModule &other) = delete;
            IRModule(IRModule &&other) = delete;
            IRModule &operator=(const IRModule &other) = delete;

            auto getIRType() const { return m_ir_type; }

            /* Write IR to Output Stream */
            template <PrintMode mode = PrintMode::Unspecified>
            Result<bool> print(std::ostream &os) const
            {
                switch (mode)
                {
                case PrintMode::Debug:
                {
                    Result<bool> result = print_impl(os, true);
                    os.flush();
                    return result;
                }
                default:
                {
                    Result<bool> result = print_impl(os, false);
                    os.flush();
                    return result;
                }
                }
            }

            /* Read IR from Input Stream */
            template <DeserializeMode mode = DeserializeMode::Unspecified>
            Result<bool> deserialize(std::istream &is)
            {
                switch (mode)
                {
                default:
                    return deserialize_impl(is);
                }
            }

            /* Verify IR Module */
            inline bool verify() const { return verify_impl(); }

            /* Calculate a cryptographic hash of the IR Graph */
            inline boost::uuids::uuid hash() const { return m_root ? m_root->hash() : boost::uuids::nil_uuid(); }

            /* Get the name of the IR Dialect */
            inline std::string_view dialect_name() const { return ir_dialect_name_impl(); }

            /* Get the version of the IR Dialect */
            inline unsigned dialect_version() const { return ir_dialect_version_impl(); }

            /* Get the family of the IR Dialect */
            inline std::string_view dialect_family() const { return ir_dialect_family_impl(); }

            /* Get a description of the IR Dialect */
            inline std::string_view dialect_description() const { return ir_dialect_description_impl(); }

            /* Get Entry Point for IR Graph */
            virtual std::shared_ptr<Value<U>> &root() { return m_root; }
            virtual const std::shared_ptr<Value<U>> &root() const { return m_root; }

            IRModule<T, U> &assign(std::shared_ptr<Value<U>> root)
            {
                m_root = root;
                return *this;
            }
        };

        template <typename T, typename U>
        std::shared_ptr<T> QIR_ROOT(const U &_module)
        {
            return std::reinterpret_pointer_cast<T>(_module->root());
        }

    } // namespace ir
} // namespace libquixcc

void x();

#endif // __QUIXCC_IR_GENERIC_H__