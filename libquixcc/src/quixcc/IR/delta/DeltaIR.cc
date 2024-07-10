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

#include <quixcc/IR/delta/Cast.h>
#include <quixcc/IR/delta/Control.h>
#include <quixcc/IR/delta/DeltaIR.h>
#include <quixcc/IR/delta/Math.h>
#include <quixcc/IR/delta/Memory.h>
#include <quixcc/IR/delta/Segment.h>
#include <quixcc/IR/delta/Type.h>
#include <quixcc/IR/delta/Variable.h>

bool libquixcc::ir::delta::IRDelta::print_impl(std::ostream &os,
                                               PState &state) const {
  if (state.modinfo) {
    os << "use QDelta_1_0;\n";
    os << "; ModuleID = '" << m_name << "'\n";

    if (!m_root) return true;

    os << "; ModuleHash = '";
    m_root->printid(os);
    os << "'\n; Tags = [";

    for (auto it = m_tags.begin(); it != m_tags.end(); it++) {
      os << *it;

      if (std::next(it) != m_tags.end()) os << ", ";
    }

    os << "]\n\n";
  }

  if (!m_root->print(os, state)) return false;

  if (state.modinfo) {
    os << "\n; End of module '";
    this->m_root->printid(os);

    os << "'\n";
  }

  return true;
}

bool libquixcc::ir::delta::IRDelta::verify_impl() const {
  if (!m_root) return false;

  return m_root->verify();
}

std::string_view libquixcc::ir::delta::IRDelta::ir_dialect_name_impl() const {
  return "QIR-Delta";
}

unsigned int libquixcc::ir::delta::IRDelta::ir_dialect_version_impl() const {
  return 1;
}

std::string_view libquixcc::ir::delta::IRDelta::ir_dialect_family_impl() const {
  return "DeltaIR";
}

std::string_view libquixcc::ir::delta::IRDelta::ir_dialect_description_impl()
    const {
  return "Quix Delta Intermediate Representation (QIR-Delta-V1.0) is an "
         "intermediate representation for the Quix language. ... (write "
         "something useful here)";
}
