////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#include <cstddef>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-ir/ABI/Name.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <sstream>

using namespace ncc;
using namespace ncc::ir;

namespace util {
  constexpr std::array<char, 256> ns_valid_chars = []() {
    auto is_alnum = [](char ch) constexpr {
      return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') ||
             (ch >= 'A' && ch <= 'Z');
    };

    std::array<char, 256> valid_chars = {};
    valid_chars.fill(false);

    for (size_t i = 0; i < 256; i++) {
      if (is_alnum(i) || i == '_' || i == '$') {
        valid_chars[i] = true;
      }
    }

    return valid_chars;
  }();

  static void escape_string(std::ostream &ss, std::string_view input) {
    ss << '"';

    for (char ch : input) {
      switch (ch) {
        case '"':
          ss << "\\\"";
          break;
        case '\\':
          ss << "\\\\";
          break;
        case '\b':
          ss << "\\b";
          break;
        case '\f':
          ss << "\\f";
          break;
        case '\n':
          ss << "\\n";
          break;
        case '\r':
          ss << "\\r";
          break;
        case '\t':
          ss << "\\t";
          break;
        case '\0':
          ss << "\\0";
          break;
        default:
          if (ch >= 32 && ch < 127) {
            ss << ch;
          } else {
            char hex[5];
            snprintf(hex, sizeof(hex), "\\x%02x", (int)(uint8_t)ch);
            ss << hex;
          }
          break;
      }
    }

    ss << '"';
  }
}  // namespace util

namespace ncc::ir::abi::azide {
  static void nslv_encode(std::string_view input, std::ostream &ss) {
    std::string buf;

    for (size_t state = 0, i = 0; i < input.size(); i++) {
      switch (state) {
        case 0: {
          if (input[i] == ':') {
            state = 1;
          } else {
            buf.push_back(input[i]);
          }
          break;
        }
        case 1: {
          if (input[i] != ':') {
            ss << buf.size() << buf;
            buf.clear();
            buf.push_back(input[i]);
            state = 0;
          }
          break;
        }
      }
    }

    if (!buf.empty()) {
      ss << buf.size() << buf;
    }
  }

  static bool nslv_decode(std::string_view &input, std::ostream &ss) {
    std::string buf;
    bool first = true;

    for (size_t state = 0, i = 0; i < input.size(); i++) {
      switch (state) {
        case 0: {
          if (std::isdigit(input[i])) {
            buf.push_back(input[i]);
          } else if (i != 0 && buf.empty()) {
            input.remove_prefix(i);
            return true;
          } else {
            state = 1;
            i--;
          }
          break;
        }
        case 1: {
          if (buf.empty()) {
            return false;
          }

          int size = std::stoi(buf);
          buf.clear();

          if (i + size > input.size()) {
            return false;
          }

          std::string_view part = input.substr(i, size);

          if (!std::all_of(part.begin(), part.end(),
                           [](char ch) { return util::ns_valid_chars[ch]; })) {
            return false;
          }

          if (first) {
            ss << part;
            first = false;
          } else {
            ss << "::" << part;
          }

          i += size - 1;
          state = 0;
          break;
        }
      }
    }

    return true;
  }

  static void MangleTypeRecurse(auto n, std::ostream &ss) {
    /**
     * @brief Name mangling for Nitrate is inspired by the Itanium C++ ABI.
     * @ref https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling
     *
     *  <builtin-type> ::= v	# void
     *                 ::= w	# wchar_t
     *                 ::= b	# bool
     *                 ::= c	# char
     *                 ::= a	# signed char
     *                 ::= h	# unsigned char
     *                 ::= s	# short
     *                 ::= t	# unsigned short
     *                 ::= i	# int
     *                 ::= j	# unsigned int
     *                 ::= l	# long
     *                 ::= m	# unsigned long
     *                 ::= x	# long long, __int64
     *                 ::= y	# unsigned long long, __int64
     *                 ::= n	# __int128
     *                 ::= o	# unsigned __int128
     *                 ::= f	# float
     *                 ::= d	# double
     *                 ::= e	# long double, __float80
     *                 ::= g	# __float128
     *                 ::= z	# ellipsis
     *                 ::= Dd # IEEE 754r decimal floating point (64 bits)
     *                 ::= De # IEEE 754r decimal floating point (128 bits)
     *                 ::= Df # IEEE 754r decimal floating point (32 bits)
     *                 ::= Dh # IEEE 754r half-precision floating point (16
     * bits)
     *                 ::= DF <number> _ # ISO/IEC TS 18661 binary floating
     * point type _FloatN (N bits), C++23 std::floatN_t
     *                 ::= DF <number> x # IEEE extended precision formats, C23
     * _FloatNx (N bits)
     *                 ::= DF16b # C++23 std::bfloat16_t
     *                 ::= DB <number> _        # C23 signed _BitInt(N)
     *                 ::= DB <instantiation-dependent expression> _ # C23
     * signed _BitInt(N)
     *                 ::= DU <number> _        # C23 unsigned _BitInt(N)
     *                 ::= DU <instantiation-dependent expression> _ # C23
     * unsigned _BitInt(N)
     *                 ::= Di # char32_t
     *                 ::= Ds # char16_t
     *                 ::= Du # char8_t
     *                 ::= Da # auto
     *                 ::= Dc # decltype(auto)
     *                 ::= Dn # std::nullptr_t (i.e., decltype(nullptr))
     *                 ::= [DS] DA  # N1169 fixed-point [_Sat] T _Accum
     *                 ::= [DS] DR  # N1169 fixed-point [_Sat] T _Fract
     *                 ::= u <source-name> [<template-args>] # vendor extended
     * type
     *
     *  <fixed-point-size>
     *                 ::= s # short
     *                 ::= t # unsigned short
     *                 ::= i # plain
     *                 ::= j # unsigned
     *                 ::= l # long
     *                 ::= m # unsigned long
     */

    switch (n->GetKind()) {
      case IR_tU1: {
        ss << 'b';
        break;
      }

      case IR_tU8: {
        ss << 'h';
        break;
      }

      case IR_tU16: {
        ss << 't';
        break;
      }

      case IR_tU32: {
        ss << 'j';
        break;
      }

      case IR_tU64: {
        ss << 'm';
        break;
      }

      case IR_tU128: {
        ss << 'o';
        break;
      }

      case IR_tI8: {
        ss << 'a';
        break;
      }

      case IR_tI16: {
        ss << 's';
        break;
      }

      case IR_tI32: {
        ss << 'i';
        break;
      }

      case IR_tI64: {
        ss << 'l';
        break;
      }

      case IR_tI128: {
        ss << 'n';
        break;
      }

      case IR_tF16_TY: {
        ss << "Dh";
        break;
      }

      case IR_tF32_TY: {
        ss << "Df";
        break;
      }

      case IR_tF64_TY: {
        ss << "Dd";
        break;
      }

      case IR_tF128_TY: {
        ss << "De";
        break;
      }

      case IR_tVOID: {
        ss << 'v';
        break;
      }

      case IR_tPTR: {
        ss << 'P';
        MangleTypeRecurse(n->template as<PtrTy>()->getPointee(), ss);
        break;
      }

      case IR_tCONST: {
        ss << 'K';
        MangleTypeRecurse(n->template as<ConstTy>()->getItem(), ss);
        break;
      }

      case IR_tOPAQUE: {
        ss << 'N';
        nslv_encode(n->template as<OpaqueTy>()->getName(), ss);
        ss << 'E';
        break;
      }

      case IR_tSTRUCT: {
        /**
         * @brief Unlike C++, Nitrate encodes field types into the name.
         * Making any changes to a struct will break ABI compatibility
         * at link time avoiding runtime UB.
         */

        ss << 'c';
        for (auto field : n->template as<StructTy>()->getFields()) {
          MangleTypeRecurse(field, ss);
        }
        ss << 'E';
        break;
      }

      case IR_tUNION: {
        /**
         * @brief Unlike C++, Nitrate encodes field types into the name.
         * Making any changes to a union will break ABI compatibility
         * at link time avoiding runtime UB.
         */

        ss << 'u';
        for (auto field : n->template as<StructTy>()->getFields()) {
          MangleTypeRecurse(field, ss);
        }
        ss << 'E';
        break;
      }

      case IR_tARRAY: {
        ss << 'A';
        ss << n->template as<ArrayTy>()->getCount();
        ss << '_';
        MangleTypeRecurse(n->template as<ArrayTy>()->getElement(), ss);
        break;
      }

      case IR_tFUNC: {
        /**
         * @brief Unlike C++, Nitrate also encodes the parameter types
         * into the name. This is to avoid runtime UB when calling
         * functions with the wrong number of arguments.
         * These bugs will be caught at link time.
         */

        ss << 'F';
        auto *fn = n->template as<FnTy>();
        MangleTypeRecurse(fn->getReturn(), ss);
        for (auto param : fn->getParams()) {
          MangleTypeRecurse(param, ss);
        }
        if (fn->isVariadic()) {
          ss << '_';
        }
        ss << 'E';
        break;
      }

      default: {
        qcore_panicf("Unknown type kind: %d", (int)n->GetKind());
      }
    }
  }

  static bool DemangleTypeRecurse(std::string_view &name, std::ostream &ss) {
    static std::unordered_map<char, std::string_view> basic_types = {
        {'b', "u1"},  {'h', "u8"},   {'t', "u16"},  {'j', "u32"},
        {'m', "u64"}, {'o', "u128"}, {'a', "i8"},   {'s', "i16"},
        {'i', "i32"}, {'l', "i64"},  {'n', "i128"}, {'v', "void"},
    };

    if (name.empty()) {
      return false;
    }

    auto it = basic_types.find(name[0]);
    if (it != basic_types.end()) {
      ss << it->second;
      name.remove_prefix(1);
      return true;
    }

    switch (name[0]) {
      case 'D': {
        if (name.size() < 2) {
          return false;
        }

        switch (name[1]) {
          case 'h': {
            ss << "f16";
            name.remove_prefix(2);
            return true;
          }

          case 'f': {
            ss << "f32";
            name.remove_prefix(2);
            return true;
          }

          case 'd': {
            ss << "f64";
            name.remove_prefix(2);
            return true;
          }

          case 'e': {
            ss << "f128";
            name.remove_prefix(2);
            return true;
          }

          default: {
            return false;
          }
        }
      }

      case 'P': {
        ss << '*';
        name.remove_prefix(1);
        return DemangleTypeRecurse(name, ss);
      }

      case 'K': {
        ss << "const<";
        name.remove_prefix(1);
        auto ret = DemangleTypeRecurse(name, ss);
        ss << ">";
        return ret;
      }

      case 'N': {
        ss << "opaque ";
        name.remove_prefix(1);
        if (!nslv_decode(name, ss)) {
          return false;
        }

        return true;
      }

      case 'c': {
        ss << "(";
        name.remove_prefix(1);
        bool first = true;
        while (!name.empty() && name[0] != 'E') {
          if (first) {
            first = false;
          } else {
            ss << ", ";
          }

          if (!DemangleTypeRecurse(name, ss)) {
            return false;
          }
        }
        ss << ")";
        name.remove_prefix(1);
        return true;
      }

      case 'u': {
        ss << "union {";
        name.remove_prefix(1);
        bool first = true;
        while (!name.empty() && name[0] != 'E') {
          if (first) {
            first = false;
          } else {
            ss << ", ";
          }

          if (!DemangleTypeRecurse(name, ss)) {
            return false;
          }
        }
        ss << "}";
        name.remove_prefix(1);
        return true;
      }

      case 'A': {
        ss << "[";
        name.remove_prefix(1);
        size_t count = 0;
        while (!name.empty() && name[0] != '_') {
          if (!std::isdigit(name[0])) {
            return false;
          }

          count = count * 10 + (name[0] - '0');
          name.remove_prefix(1);
        }
        name.remove_prefix(1);
        if (!DemangleTypeRecurse(name, ss)) {
          return false;
        }
        ss << "; " << count << "]";

        return true;
      }

      case 'F': {
        ss << "fn (";
        name.remove_prefix(1);
        std::stringstream return_ty, params;
        if (!DemangleTypeRecurse(name, return_ty)) {
          return false;
        }

        bool first = true;
        size_t i = 0;
        while (!name.empty() && name[0] != 'E') {
          if (first) {
            first = false;
          } else {
            params << ", ";
          }

          params << "_" << i++ << ": ";
          if (!DemangleTypeRecurse(name, params)) {
            return false;
          }

          if (!name.empty() && name[0] == '_') {
            params << ", ...";
            name.remove_prefix(1);
            break;
          }
        }
        name.remove_prefix(1);

        ss << params.str() << "): " << return_ty.str();
        return true;
      }

      default: {
        return false;
      }
    }
  }

  static std::string MangleSymbol(std::string_view name, auto type) {
    std::stringstream ss;

    ss << "_Q";  // Nitrate ABI prefix
    nslv_encode(name, ss);
    MangleTypeRecurse(type, ss);
    ss << "_0";  // ABI version 0

    return ss.str();
  }

  static std::optional<std::string> DemangleSymbol(std::string_view name) {
    if (name.size() < 2) {
      printf("Failed to decode Nitrate ABI prefix\n");
      return std::nullopt;
    }

    name.remove_prefix(2);  // Remove Nitrate ABI prefix

    if (!name.ends_with("_0")) {  // Version check
      printf("Failed to decode ABI version\n");
      return std::nullopt;
    }

    name.remove_suffix(2);  // Remove ABI version

    std::stringstream ss;

    ss << "{\"name\":";

    { /* Write the symbol name */
      std::stringstream name_ss;
      if (!nslv_decode(name, name_ss)) {
        printf("Failed to decode namespace size value\n");
        return std::nullopt;
      }

      util::escape_string(ss, name_ss.str());
    }

    ss << ",\"type\":";

    { /* Write the type */
      std::stringstream type_ss;
      if (!DemangleTypeRecurse(name, type_ss)) {
        printf("Failed to decode type\n");
        return std::nullopt;
      }

      util::escape_string(ss, type_ss.str());
    }

    ss << "}";

    return ss.str();
  }
}  // namespace ncc::ir::abi::azide

namespace ncc::ir::abi::c {
  static std::string mangle_c_abi(std::string_view name, auto) {
    std::string s = std::string(name);
    std::replace(s.begin(), s.end(), ':', '_');
    return s;
  }

  static std::string demangle_c_abi(std::string_view name) {
    /**
     * @brief There isn't really anything to do here.
     * The C ABI is weak and doesn't encode type information.
     */

    return std::string(name);
  }
}  // namespace ncc::ir::abi::c

///=============================================================================

NCC_EXPORT std::optional<std::string> ncc::ir::MangleTypeName(
    FlowPtr<Type> type, AbiTag abi) {
  switch (abi) {
    case AbiTag::C: {
      return "";
    }

    case AbiTag::Nitrate:
    case AbiTag::Internal: {
      std::stringstream ss;
      abi::azide::MangleTypeRecurse(type, ss);
      return ss.str();
    }
  }
}

NCC_EXPORT std::optional<std::string> ncc::ir::GetMangledSymbolName(
    FlowPtr<Expr> symbol, AbiTag abi) {
  auto name = symbol->getName();

  if (auto type_opt = symbol->getType()) {
    switch (abi) {
      case AbiTag::C: {
        return abi::c::mangle_c_abi(name, type_opt.value());
      }

      case AbiTag::Nitrate:
      case AbiTag::Internal: {
        return abi::azide::MangleSymbol(name, type_opt.value());
      }
    }
  } else {
    return std::nullopt;
  }
}

NCC_EXPORT std::optional<std::string> ncc::ir::ExpandSymbolName(
    std::string_view mangled_name) {
  if (mangled_name.empty()) {
    return std::nullopt;
  }

  if (mangled_name.starts_with("_Q")) {
    return abi::azide::DemangleSymbol(mangled_name);
  }

  std::stringstream ss;
  if (abi::azide::DemangleTypeRecurse(mangled_name, ss)) {
    return ss.str();
  } else {
    return abi::c::demangle_c_abi(mangled_name);
  }
}

NCC_EXPORT NullableFlowPtr<Expr> ncc::ir::GetSymbolFromMangledName(
    std::string_view mangled_name) {
  /// TODO: Implement this function
  qcore_implement();
}

NCC_EXPORT NullableFlowPtr<Type> ncc::ir::GetTypeFromMangledName(
    std::string_view mangled_name) {
  /// TODO: Implement this function
  qcore_implement();
}
