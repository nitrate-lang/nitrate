////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __QUIXCC_IR_GENERIC_H__
#define __QUIXCC_IR_GENERIC_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <core/Exception.h>
#include <core/Logger.h>

#include <algorithm>
#include <boost/endian/conversion.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace libquixcc {
namespace ir {
struct PState {
  size_t ind;
  bool modinfo = true;
  int last_node_type = -1;

  PState() { ind = 0; }
};

template <auto V>
class Node {
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
  bool is() const {
#if !defined(NDEBUG)
    uintptr_t p = reinterpret_cast<uintptr_t>(this);
    if (p == 0) {
      LOG(FATAL) << "Invalid cast from nullptr to `" << typeid(T).name() << "`"
                 << std::endl;
      throw core::Exception();
    }
#endif
    return typeid(T) == typeid(*this);
  }

  template <typename T>
  bool is(const T *other) const {
#if !defined(NDEBUG)
    uintptr_t p = reinterpret_cast<uintptr_t>(this);
    if (p == 0) {
      LOG(FATAL) << "Invalid cast from nullptr to `" << typeid(T).name() << "`"
                 << std::endl;
      throw core::Exception();
    }
    if (!other) {
      LOG(FATAL) << "Invalid cast from `" << typeid(*this).name()
                 << "` to nullptr" << std::endl;
      throw core::Exception();
    }
#endif
    return reinterpret_cast<const T *>(this) == other;
  }

  template <typename T>
  const T *as() const {
#if !defined(NDEBUG)
    uintptr_t ptr = reinterpret_cast<uintptr_t>(this);
    if (ptr == 0) {
      LOG(FATAL) << "Invalid cast from nullptr to `" << typeid(T).name() << "`"
                 << std::endl;
      throw core::Exception();
    }
#endif
    auto p = dynamic_cast<const T *>(this);

    if (p) return p;

    LOG(FATAL) << "Invalid cast from `" << typeid(*this).name() << "` to `"
               << typeid(T).name() << "`" << std::endl;
    throw core::Exception();
  }

  template <typename T>
  T *as() {
#if !defined(NDEBUG)
    uintptr_t ptr = reinterpret_cast<uintptr_t>(this);
    if (ptr == 0) {
      LOG(FATAL) << "Invalid cast from nullptr to `" << typeid(T).name() << "`"
                 << std::endl;
      throw core::Exception();
    }
#endif

    auto p = dynamic_cast<T *>(this);

    if (p) return p;

    LOG(FATAL) << "Invalid cast from `" << typeid(*this).name() << "` to `"
               << typeid(T).name() << "`" << std::endl;
    throw core::Exception();
  }

  /* Write IR to Output Stream */
  bool print(std::ostream &os, PState &state) const {
    return print_impl(os, state);
  }

  std::string to_string() const {
    std::stringstream ss;
    PState state;
    print(ss, state);
    return ss.str();
  }

  /* Write UUID to Output Stream */
  inline void printid(std::ostream &os) const {
    os << boost::uuids::to_string(hash());
  }

  /* Verify IR Module */
  inline bool verify() const { return verify_impl(); }

  /* Calculate a cryptographic hash of the IR Graph */
  inline boost::uuids::uuid hash() const { return hash_impl(); }

  bool operator==(const Node &other) const { return hash() == other.hash(); }
  bool operator>(const Node &other) const { return hash() > other.hash(); }

  int32_t ntype = -1;
};

class Hasher {
  boost::uuids::detail::sha1 m_hasher;

 public:
  Hasher() = default;

  template <typename T>
  Hasher &add(const T value) {
    std::stringstream ss;
    PState state;
    value->print(ss, state);
    m_hasher.process_bytes(ss.str().data(), ss.str().size());
    return *this;
  }

  template <typename T>
  Hasher &add(const std::vector<T> &values) {
    for (const auto &value : values) {
      add(value);
    }
    return *this;
  }

  template <typename T>
  Hasher &add(const std::set<T> &values) {
    for (const auto &value : values) {
      add(value);
    }
    return *this;
  }

  inline Hasher &add(const std::string_view &value) {
    m_hasher.process_bytes(value.data(), value.size());
    return *this;
  }

  inline Hasher &add(const char *value) {
    m_hasher.process_bytes(value, std::strlen(value));
    return *this;
  }

  inline Hasher &add(size_t value) {
    m_hasher.process_bytes(reinterpret_cast<const void *>(&value),
                           sizeof(size_t));
    return *this;
  }

  inline Hasher &add(bool value) {
    m_hasher.process_bytes(reinterpret_cast<const void *>(&value),
                           sizeof(bool));
    return *this;
  }

  inline Hasher &add(const std::string &value) {
    m_hasher.process_bytes(value.data(), value.size());
    return *this;
  }

  /* Generate tag based on source metadata */
  inline Hasher &gettag(const std::string_view &tag = __PRETTY_FUNCTION__) {
    m_hasher.process_bytes(tag.data(), tag.size());
    return *this;
  }

  inline boost::uuids::uuid hash() {
    boost::uuids::detail::sha1::digest_type digest;
    m_hasher.get_digest(digest);
    boost::uuids::uuid ret;
    auto p = ret.begin();
    for (std::size_t i{}; i != 4; p += 4, ++i) {
      auto const d = boost::endian::native_to_big(digest[i]);
      std::memcpy(p, &d, 4);
    }
    return ret;
  }
};

template <auto T, typename U>
class IRModule {
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
  IRModule(const std::string_view &name) {
    static_assert(std::is_enum_v<decltype(T)>,
                  "IR Type must be an enumeration");

    m_name = name;
  }

  virtual ~IRModule() = default;
  IRModule(const IRModule &other) = delete;
  IRModule(IRModule &&other) = delete;
  IRModule &operator=(const IRModule &other) = delete;

  auto getIRType() const { return m_ir_type; }

  /* Write IR to Output Stream */
  bool print(std::ostream &os, bool modinfo = true) const {
    PState state;
    state.modinfo = modinfo;
    return print_impl(os, state);
  }

  std::string to_string(bool modinfo = true) const {
    std::ostringstream ss;
    if (!print(ss, modinfo))
      throw std::runtime_error("Failed to print IR Module");
    return ss.str();
  }

  /* Verify IR Module */
  inline bool verify() const { return verify_impl(); }

  /* Calculate a cryptographic hash of the IR Graph */
  inline boost::uuids::uuid hash() const {
    return m_root ? m_root->hash() : boost::uuids::nil_uuid();
  }

  /* Get the name of the IR Dialect */
  inline std::string_view dialect_name() const {
    return ir_dialect_name_impl();
  }

  /* Get the version of the IR Dialect */
  inline unsigned dialect_version() const { return ir_dialect_version_impl(); }

  /* Get the family of the IR Dialect */
  inline std::string_view dialect_family() const {
    return ir_dialect_family_impl();
  }

  /* Get a description of the IR Dialect */
  inline std::string_view dialect_description() const {
    return ir_dialect_description_impl();
  }

  /* Get Entry Point for IR Graph */
  virtual U root() { return m_root; }
  virtual const U root() const { return m_root; }

  IRModule<T, U> &assign(U root) {
    m_root = root;
    return *this;
  }
};

}  // namespace ir
}  // namespace libquixcc

#endif  // __QUIXCC_IR_GENERIC_H__