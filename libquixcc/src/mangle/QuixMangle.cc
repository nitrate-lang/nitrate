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

#define QUIXCC_INTERNAL

#include <IR/Q/Type.h>
#include <IR/Q/Variable.h>
#include <LibMacro.h>
#include <core/Logger.h>
#include <mangle/Symbol.h>

static std::string wrap_tag(const std::string &tag) {
  std::string s;

  for (size_t i = 0; i < tag.size(); i++) {
    if (tag.at(i) == ':') {
      s += "__";
      i++;
    } else {
      s += tag.at(i);
    }
  }

  return std::to_string(s.size()) + s;
}

static bool unwrap_tags(const std::string &input,
                        std::vector<std::string> &out) {
  size_t i = 0;
  try {
    while (i < input.size()) {
      std::string len;
      while (i < input.size() && std::isdigit(input.at(i))) {
        len += input.at(i);
        i++;
      }

      if (len.empty()) return false;

      size_t l = std::stoi(len);
      out.push_back(input.substr(i, l));

      i += l;
    }
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Out of range");
    return false;
  }

  return true;
}

using namespace libquixcc::ir;

static std::string serialize_type(
    const libquixcc::ir::q::Type *type,
    std::set<const libquixcc::ir::q::Type *> visited) {
  using namespace libquixcc;

  static std::map<int, std::string> basic_typesmap = {
      {(int)q::NodeType::I1, "y"},   {(int)q::NodeType::U8, "b"},
      {(int)q::NodeType::U16, "w"},  {(int)q::NodeType::U32, "d"},
      {(int)q::NodeType::U64, "q"},  {(int)q::NodeType::U128, "o"},
      {(int)q::NodeType::I8, "c"},   {(int)q::NodeType::I16, "s"},
      {(int)q::NodeType::I32, "i"},  {(int)q::NodeType::I64, "l"},
      {(int)q::NodeType::I128, "p"}, {(int)q::NodeType::F32, "g"},
      {(int)q::NodeType::F64, "e"},  {(int)q::NodeType::Void, "v"},
  };

  if (basic_typesmap.contains(type->ntype))
    return basic_typesmap.at(type->ntype);

  if (visited.contains(type)) {
    switch ((q::NodeType)type->ntype) {
      case q::NodeType::Group:
        return "t" + wrap_tag(static_cast<const q::Group *>(type)->name);
      case q::NodeType::Region:
        return "j" + wrap_tag(static_cast<const q::Region *>(type)->name);
      case q::NodeType::Union:
        return "u" + wrap_tag(static_cast<const q::Union *>(type)->name);
      default:
        throw std::runtime_error("Unknown type: " +
                                 std::to_string((int)type->ntype));
    }
  }

  if (type->is<q::Group>()) {
    visited.insert(type);

    const q::Group *st = static_cast<const q::Group *>(type);
    return "t" + wrap_tag(st->name);
  } else if (type->is<q::Region>()) {
    visited.insert(type);

    const q::Region *st = static_cast<const q::Region *>(type);
    return "j" + wrap_tag(st->name);
  } else if (type->is<q::Union>()) {
    visited.insert(type);

    const q::Union *st = static_cast<const q::Union *>(type);
    return "u" + wrap_tag(st->name);
  } else if (type->is<q::Array>()) {
    const q::Array *st = static_cast<const q::Array *>(type);
    std::string s;
    s += wrap_tag(serialize_type(st->type, visited));
    s += wrap_tag("x" + std::to_string(st->size));
    return "a" + s;
  } else if (type->is<q::Vector>()) {
    const q::Vector *st = static_cast<const q::Vector *>(type);
    std::string s;
    s += wrap_tag(serialize_type(st->type, visited));
    return "r" + s;
  } else if (type->is<q::Ptr>()) {
    const q::Ptr *st = static_cast<const q::Ptr *>(type);
    return "p" + wrap_tag(serialize_type(st->type, visited));
  } else if (type->is<q::FType>()) {
    const q::FType *st = static_cast<const q::FType *>(type);
    std::string s;
    s += wrap_tag(serialize_type(st->ret, visited));

    for (auto param : st->params) s += wrap_tag(serialize_type(param, visited));

    std::string prop;

    if (st->m_foreign) prop += "f";
    if (st->m_noexcept) prop += "n";
    if (st->m_pure) prop += "p";
    if (st->m_thread_safe) prop += "t";
    if (st->m_variadic) prop += "v";

    s += wrap_tag(prop);

    return "f" + s;
  }

  throw std::runtime_error("Unknown type: " + std::to_string((int)type->ntype));
}

static const libquixcc::ir::q::Type *deserialize_type_inner(
    const std::string &type,
    std::map<std::string, libquixcc::ir::q::Type *> &prev) {
  using namespace libquixcc;

  static std::map<std::string, const q::Type *> basic_typesmap = {
      {"y", q::I1::create()},   {"b", q::U8::create()},
      {"w", q::U16::create()},  {"d", q::U32::create()},
      {"q", q::U64::create()},  {"o", q::U128::create()},
      {"c", q::I8::create()},   {"s", q::I16::create()},
      {"i", q::I32::create()},  {"l", q::I64::create()},
      {"p", q::I128::create()}, {"g", q::F32::create()},
      {"e", q::F64::create()},  {"v", q::Void::create()}};

  if (basic_typesmap.contains(type)) return basic_typesmap.at(type);

  try {
    if (type.at(0) == 't') {
      std::vector<std::string> fields;
      if (!unwrap_tags(type.substr(1), fields)) return nullptr;

      if (fields.size() < 1) return nullptr;

      return q::Group::create(fields.at(0), {});
    } else if (type.at(0) == 'j') {
      std::vector<std::string> fields;
      if (!unwrap_tags(type.substr(1), fields)) return nullptr;

      if (fields.size() < 1) return nullptr;

      return q::Region::create(fields.at(0), {});
    } else if (type.at(0) == 'u') {
      std::vector<std::string> fields;
      if (!unwrap_tags(type.substr(1), fields)) return nullptr;

      if (fields.size() < 1) return nullptr;

      return q::Union::create(fields.at(0), {});
    } else if (type.at(0) == 'a') {
      std::vector<std::string> fields;
      if (!unwrap_tags(type.substr(1), fields)) return nullptr;

      if (fields.size() < 2) return nullptr;

      const q::Type *t;
      if ((t = deserialize_type_inner(fields.at(0), prev)) == nullptr)
        return nullptr;

      return q::Array::create(t, std::stoi(fields.at(1)));
    } else if (type.at(0) == 'r') {
      std::vector<std::string> fields;
      if (!unwrap_tags(type.substr(1), fields)) return nullptr;

      if (fields.size() < 1) return nullptr;

      const q::Type *t;
      if ((t = deserialize_type_inner(fields.at(0), prev)) == nullptr)
        return nullptr;

      return q::Vector::create(t);
    } else if (type.at(0) == 'p') {
      std::vector<std::string> fields;
      if (!unwrap_tags(type.substr(1), fields)) return nullptr;

      if (fields.size() < 1) return nullptr;

      const q::Type *t;
      if ((t = deserialize_type_inner(fields.at(0), prev)) == nullptr)
        return nullptr;

      return q::Ptr::create(t);
    } else if (type.at(0) == 'f') {
      std::vector<std::string> fields;
      if (!unwrap_tags(type.substr(1), fields)) return nullptr;

      if (fields.size() < 3) return nullptr;

      const q::Type *ret;
      if ((ret = deserialize_type_inner(fields.at(0), prev)) == nullptr)
        return nullptr;

      std::vector<const q::Type *> params;
      for (size_t i = 1; i < fields.size() - 1; i++) {
        const q::Type *t;
        if ((t = deserialize_type_inner(fields.at(i), prev)) == nullptr)
          return nullptr;

        params.push_back(t);
      }

      std::string prop = fields.back();
      bool foreign = false, _noexcept = false, pure = false,
           thread_safe = false, variadic = false;
      for (size_t i = 0; i < prop.size(); i++) {
        switch (prop.at(i)) {
          case 'f':
            foreign = true;
            break;
          case 'n':
            _noexcept = true;
            break;
          case 'p':
            pure = true;
            break;
          case 't':
            thread_safe = true;
            break;
          case 'v':
            variadic = true;
            break;
          default:
            return nullptr;
        }
      }

      return q::FType::create(params, ret, variadic, pure, thread_safe, foreign,
                              _noexcept);
    }

    throw std::runtime_error("Unknown type 2");
  } catch (std::exception &e) {
    return nullptr;
  }
}

static const libquixcc::ir::q::Type *deserialize_type(
    const std::string &input) {
  std::map<std::string, libquixcc::ir::q::Type *> prev;

  auto type = deserialize_type_inner(input, prev);
  if (!type) return nullptr;

  return type;
}

std::string libquixcc::Symbol::mangle_quix(const libquixcc::ir::q::Value *node,
                                           const std::string &prefix) {
  std::string res = quix_abiprefix;
  std::set<const libquixcc::ir::q::Type *> visited;

  switch ((q::NodeType)node->ntype) {
    case q::NodeType::Global: {
      res += "g";
      auto var = static_cast<const q::Global *>(node);
      res += wrap_tag(join(prefix, var->name));
      res += wrap_tag(serialize_type(var->type, visited));

      std::string flags;
      if (var->_atomic) flags += "a";
      if (var->_volatile) flags += "o";

      res += wrap_tag(flags);

      return res;
    }

    default:
      break;
  }

  return "";
}

const libquixcc::ir::q::Value *libquixcc::Symbol::demangle_quix(
    std::string input) {
  try {
    input = input.substr(quix_abiprefix.size());

    std::vector<std::string> parts;

    switch (input.at(0)) {
      case 'g': {
        if (!unwrap_tags(input.substr(1), parts)) return nullptr;

        if (parts.size() < 3) return nullptr;

        auto t = deserialize_type(parts.at(1));
        if (!t) return nullptr;

        bool atomic = false, _volatile = false;
        std::string flags = parts.at(2);
        for (size_t i = 0; i < flags.size(); i++) {
          switch (flags.at(i)) {
            case 'a':
              atomic = true;
              break;
            case 'o':
              _volatile = true;
              break;
            default:
              return nullptr;
          }
        }

        return q::Global::create(parts.at(0), t, nullptr, _volatile, atomic,
                                 false);
      }

      default:
        return nullptr;
    }
  } catch (std::exception &e) {
    throw std::runtime_error("Error demangling: " + std::string(e.what()));
    return nullptr;
  }
}