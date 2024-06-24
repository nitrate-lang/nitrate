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

#include <IR/Q/Variable.h>
#include <LibMacro.h>
#include <mangle/Symbol.h>

#include <thread>

const std::string libquixcc::Symbol::quix_abiprefix = "_Q";
const std::string libquixcc::Symbol::cxx_abiprefix = "_Z";
const std::string libquixcc::Symbol::c_abiprefix = "";

std::string libquixcc::Symbol::join(const std::string &a,
                                    const std::string &b) {
  if (a.empty()) return b;
  if (b.empty()) return a;
  return a + "::" + b;
}

std::string libquixcc::Symbol::join(const std::vector<std::string> &namespaces,
                                    const std::string &name) {
  std::string result;
  for (size_t i = 0; i < namespaces.size(); i++)
    result = join(result, namespaces[i]);

  return join(result, name);
}

std::string libquixcc::Symbol::mangle(const ir::q::Value *node,
                                      const std::string &prefix,
                                      ExportLangType lang) {
  switch (lang) {
    case ExportLangType::Default:
      return mangle_quix(node, prefix);
    case ExportLangType::C:
      return mangle_c(node, prefix);
    case ExportLangType::CXX:
      return mangle_cxx(node, prefix);
    case ExportLangType::DLang:
      throw std::runtime_error("DLang export not yet supported");
    default:
      throw std::runtime_error("Invalid export language type");
  }
}

libquixcc::ir::q::Value *libquixcc::Symbol::demangle(
    const std::string &mangled) {
  try {
    if (mangled.starts_with(quix_abiprefix)) {
      return demangle_quix(mangled);
    } else if (mangled.starts_with(cxx_abiprefix)) {
      return demangle_cxx(mangled);
    } else if (mangled.starts_with(c_abiprefix)) {
      return demangle_c(mangled);
    } else {
      return nullptr;
    }
  } catch (const std::exception &e) {
    std::cerr << "Demangling error: " << e.what() << std::endl;
    return nullptr;
  }
}

bool libquixcc::Symbol::demangle_tocode(const std::string &mangled,
                                        std::string &output) {
  auto node = demangle(mangled);
  if (node == nullptr) return false;

  output = node->to_string();

  if (output.ends_with(" = undef")) {
    output = output.substr(0, output.size() - 8);
  }

  return true;
}

LIB_EXPORT char *quixcc_demangle(const char *mangled) {
  /* no-op */
  if (!mangled) return nullptr;

  std::string output;
  if (!libquixcc::Symbol::demangle_tocode(mangled, output)) return nullptr;

  return strdup(output.c_str());
}