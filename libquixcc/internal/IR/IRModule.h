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

#include <core/Logger.h>

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
        struct PState
        {
            size_t ind;

            PState()
            {
                ind = 0;
            }
        };

        template <auto V>
        class Node
        {
        protected:
            /* Node Serialization */
            virtual bool print_impl(std::ostream &os, PState &state) const = 0;

            /* Graph Operations & Caching */
            virtual boost::uuids::uuid hash_impl() const = 0;

            /* IR Module Verification */
            virtual bool verify_impl() const = 0;

            const static auto _IR_type = V;

        public:
            Node() = default;
            virtual ~Node() = default;

            template <typename T>
            bool is() const
            {
                return dynamic_cast<const T *>(this) != nullptr;
            }

            template <typename T>
            bool is(const T *other) const
            {
                return reinterpret_cast<const T *>(this) == other; 
            }

            template <typename T>
            const T *as() const
            {
                if (!is<T>())
                    throw std::runtime_error("Invalid cast from `" + std::string(typeid(*this).name()) + "` to `" + std::string(typeid(T).name()) + "`");

                return reinterpret_cast<const T *>(this);
            }

            /* Write IR to Output Stream */
            bool print(std::ostream &os, PState &state) const
            {
                return print_impl(os, state);
            }

            std::string to_string() const
            {
                std::stringstream ss;
                PState state;
                print(ss, state);
                return ss.str();
            }

            /* Write UUID to Output Stream */
            inline void printid(std::ostream &os) const { os << boost::uuids::to_string(hash()); }

            /* Verify IR Module */
            inline bool verify() const { return verify_impl(); }

            /* Calculate a cryptographic hash of the IR Graph */
            inline boost::uuids::uuid hash() const { return hash_impl(); }

            bool operator==(const Node &other) const { return hash() == other.hash(); }
            bool operator>(const Node &other) const { return hash() > other.hash(); }

            int32_t ntype = -1;
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
                PState state;
                value->print(ss, state);
                m_hasher.process_bytes(ss.str().data(), ss.str().size());
                return *this;
            }

            template <typename T>
            Hasher &add(const std::vector<T> &values)
            {
                for (const auto &value : values)
                {
                    add(value);
                }
                return *this;
            }

            template <typename T>
            Hasher &add(const std::set<T> &values)
            {
                for (const auto &value : values)
                {
                    add(value);
                }
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

            inline Hasher &add(size_t value)
            {
                m_hasher.process_bytes(reinterpret_cast<const void *>(&value), sizeof(size_t));
                return *this;
            }

            inline Hasher &add(bool value)
            {
                m_hasher.process_bytes(reinterpret_cast<const void *>(&value), sizeof(bool));
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

        template <auto T, typename U>
        class IRModule
        {
        protected:
            std::string m_name;
            U m_root = nullptr;
            const static auto m_ir_type = T;

            /* Module Serialization */
            virtual bool print_impl(std::ostream &os, PState &state) const = 0;

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

                m_name = name;
            }

            virtual ~IRModule() = default;
            IRModule(const IRModule &other) = delete;
            IRModule(IRModule &&other) = delete;
            IRModule &operator=(const IRModule &other) = delete;

            auto getIRType() const { return m_ir_type; }

            /* Write IR to Output Stream */
            bool print(std::ostream &os) const
            {
                PState state;
                return print_impl(os, state);
            }

            std::string to_string() const
            {
                std::ostringstream ss;
                if (!print(ss))
                    throw std::runtime_error("Failed to print IR Module");
                return ss.str();
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
            virtual U root() { return m_root; }
            virtual const U root() const { return m_root; }

            IRModule<T, U> &assign(U root)
            {
                m_root = root;
                return *this;
            }
        };

    } // namespace ir
} // namespace libquixcc

#endif // __QUIXCC_IR_GENERIC_H__