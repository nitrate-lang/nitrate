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

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace libquixcc::ir;

static std::string serialize_name(const std::string &str) {
  std::stringstream ss;

  if (str.find("::") != std::string::npos) {
    size_t pos = 0;
    while (pos < str.size()) {
      size_t next = str.find("::", pos);
      if (next != std::string::npos) {
        ss << "N" << std::to_string(next - pos) << str.substr(pos, next - pos);
        pos = next + 2;
      } else {
        ss << "N" << std::to_string(str.size() - pos) << str.substr(pos);
        break;
      }
    }
  } else {
    ss << str.size() << str;
  }

  return ss.str();
}

static std::string deserialize_name(std::stringstream &str) {
  std::stringstream ss;

  char c;
  size_t size = 0, i = 0;
  bool first = true;

  if (str.peek() == 'N') {
    do {
      str.ignore();
      str >> size;

      if (!first) ss << "::";
      first = false;

      for (i = 0; i < size; i++) {
        str >> c;
        ss << c;
      }

    } while (str.peek() == 'N');

    return ss.str();
  }

  str >> size;

  for (i = 0; i < size; i++) {
    str >> c;
    ss << c;
  }

  return ss.str();
}

static std::string type_hash(const libquixcc::ir::q::Type *type) {
  auto uuid = type->hash();

  uint32_t hash;
  hash = (uuid.data[0] << 24);
  hash |= (uuid.data[1] << 16);
  hash |= (uuid.data[2] << 8);
  hash |= (uuid.data[3]);

  std::stringstream ss;
  ss << std::setw(8) << std::setfill('0') << std::hex << hash;

  return ss.str();
}

static std::string serialize_type(const libquixcc::ir::q::Type *type) {
  using namespace libquixcc;

  static std::map<int, std::string> basic_typesmap = {
      {(int)q::QType::I1, "y"},   {(int)q::QType::I8, "c"},
      {(int)q::QType::I16, "s"},  {(int)q::QType::I32, "i"},
      {(int)q::QType::I64, "l"},  {(int)q::QType::I128, "p"},
      {(int)q::QType::U8, "b"},   {(int)q::QType::U16, "w"},
      {(int)q::QType::U32, "d"},  {(int)q::QType::U64, "q"},
      {(int)q::QType::U128, "o"}, {(int)q::QType::F32, "g"},
      {(int)q::QType::F64, "e"},  {(int)q::QType::Void, "v"},
  };

  if (basic_typesmap.contains(type->ntype))
    return basic_typesmap.at(type->ntype);

  std::stringstream ss;

  if (type->is<q::Region>()) {
    auto region = type->as<q::Region>();
    ss << "R";
    ss << serialize_name(region->name);
    if (region->m_packed) ss << "X";
    if (region->m_ordered) ss << "Y";

    return ss.str();
  } else if (type->is<q::Union>()) {
    auto union_ = type->as<q::Union>();
    ss << "U";
    ss << serialize_name(union_->name);

    return ss.str();
  } else if (type->is<q::Array>()) {
    auto array = type->as<q::Array>();
    ss << "A";
    ss << serialize_type(array->type);
    ss << "_";
    ss << array->size;

    return ss.str();
  } else if (type->is<q::Vector>()) {
    auto vector = type->as<q::Vector>();
    ss << "V";
    ss << serialize_type(vector->type);

    return ss.str();
  } else if (type->is<q::Ptr>()) {
    auto ptr = type->as<q::Ptr>();
    ss << "P";
    ss << serialize_type(ptr->type);

    return ss.str();
  } else if (type->is<q::FType>()) {
    auto ftype = type->as<q::FType>();
    ss << "F";
    ss << serialize_type(ftype->ret);
    for (auto &arg : ftype->params) {
      ss << serialize_type(arg);
    }

    return ss.str();
  } else if (type->is<q::IntrinsicType>()) {
    auto intrinsic = type->as<q::IntrinsicType>();
    ss << "I";
    ss << serialize_name(q::intrinsic_type_names.at(intrinsic->name).data());

    return ss.str();
  }

  throw std::runtime_error("Unknown type: " + std::to_string((int)type->ntype));
}

static q::Type *deserialize_type(std::stringstream &str) {
  char c;

  str >> c;

  switch (c) {
    case 'y':
      return q::I1::create();
    case 'c':
      return q::I8::create();
    case 's':
      return q::I16::create();
    case 'i':
      return q::I32::create();
    case 'l':
      return q::I64::create();
    case 'p':
      return q::I128::create();
    case 'b':
      return q::U8::create();
    case 'w':
      return q::U16::create();
    case 'd':
      return q::U32::create();
    case 'q':
      return q::U64::create();
    case 'o':
      return q::U128::create();
    case 'g':
      return q::F32::create();
    case 'e':
      return q::F64::create();
    case 'v':
      return q::Void::create();
    case 'R': {
      auto name = deserialize_name(str);
      bool packed = false;
      bool ordered = false;

      while (str.peek() == 'X' || str.peek() == 'Y') {
        char c;
        str >> c;
        if (c == 'X') packed = true;
        if (c == 'Y') ordered = true;
      }

      return q::Region::create(name, {}, packed, ordered);
    }
    case 'U': {
      auto name = deserialize_name(str);
      return q::Union::create(name, {});
    }
    case 'A': {
      auto type = deserialize_type(str);

      if (str.peek() != '_') {
        std::cerr << "Error deserializing array type" << std::endl;
        return nullptr;
      }

      str.ignore();
      size_t size = 0;
      str >> size;

      return q::Array::create(type, size);
    }
    case 'V': {
      auto type = deserialize_type(str);
      return q::Vector::create(type);
    }
    case 'P': {
      auto type = deserialize_type(str);
      return q::Ptr::create(type);
    }
    case 'F': {
      auto ret = deserialize_type(str);
      std::vector<q::Type *> params;

      while (str.peek() != '_') {
        auto type = deserialize_type(str);
        if (!type) {
          std::cerr << "Error deserializing function type" << std::endl;
          return nullptr;
        }
        params.push_back(type);
      }

      return q::FType::create(params, ret);
    }
    case 'I': {
      auto name = deserialize_name(str);
      if (!q::intrinsic_type_map.contains(name)) {
        std::cerr << "Error deserializing intrinsic type" << std::endl;
        return nullptr;
      }

      return q::IntrinsicType::create(q::intrinsic_type_map.at(name));
    }
    default:
      break;
  }
  return nullptr;
}

std::string libquixcc::Symbol::mangle_quix(const libquixcc::ir::q::Value *node,
                                           const std::string &prefix) {
  std::stringstream res;

  switch ((q::QType)node->ntype) {
    case q::QType::Global: {
      auto global = node->as<q::Global>();
      if (!global->type->is<q::FType>()) {
        /* Normal global variable */

        res << quix_abiprefix;
        res << "G";
        res << serialize_name(global->name);
        res << serialize_type(global->type);
        res << "_";
        res << type_hash(global->type);
      } else {
        auto ftype = global->type->as<q::FType>();
        /* Function */

        res << quix_abiprefix;
        res << "F";
        res << serialize_name(global->name);
        res << serialize_type(ftype->ret);
        for (auto &arg : ftype->params) {
          res << serialize_type(arg);
        }
        res << "_";
        res << type_hash(global->type);
      }
      return res.str();
    }

    default:
      return "!!!";
  }
}

libquixcc::ir::q::Value *libquixcc::Symbol::demangle_quix(std::string input) {
  try {
    input = input.substr(quix_abiprefix.size());

    std::vector<std::string> parts;

    switch (input.at(0)) {
      case 'G': {
        std::stringstream ss(input.substr(1));

        std::string name;
        q::Type *type;
        std::string hash;

        name = deserialize_name(ss);
        type = deserialize_type(ss);
        if (!type) {
          std::cerr << "Error demangling type 0: " << input << std::endl;
          return nullptr;
        }

        if (ss.peek() == '_') {
          ss.ignore();
          hash = ss.str();
        }

        return q::Global::create(name, type, nullptr, false, false, true);
      }
      case 'F': {
        std::stringstream ss(input.substr(1));

        std::string name;
        q::Type *ret;
        std::vector<q::Type *> params;
        std::string hash;

        name = deserialize_name(ss);
        ret = deserialize_type(ss);
        if (!ret) {
          std::cerr << "Error demangling type 1: " << input << std::endl;
          return nullptr;
        }

        while (ss.peek() != '_') {
          auto type = deserialize_type(ss);
          if (!type) {
            std::cerr << "Error demangling type 2: " << input << std::endl;
            return nullptr;
          }
          params.push_back(type);
        }

        if (ss.peek() == '_') {
          ss.ignore();
          hash = ss.str();
        }

        return q::Global::create(name, q::FType::create(params, ret), nullptr,
                                 false, false, true);
      }
      default:
        return nullptr;
    }
  } catch (std::exception &e) {
    throw std::runtime_error("Error demangling: " + std::string(e.what()));
    return nullptr;
  }
}