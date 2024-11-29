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

#include <algorithm>
#include <unordered_map>

#include "nitrate-ir/Report.hh"
#define IRBUILDER_IMPL

#include <nitrate-core/Error.h>

#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>

using namespace nr;

///=============================================================================

struct Conflict {
  std::string_view name;
  Expr *expr;
  Kind kind;
};

static void print_conflict_errors(const std::vector<Conflict> &conflicts,
                                  IReport *I) {
  for (const auto &conflict : conflicts) {
    switch (conflict.kind) {
      case Kind::Function: {
        I->report(nr::FunctionRedefinition, IC::Error, {conflict.name},
                  conflict.expr->getLoc());
        break;
      }

      case Kind::Variable: {
        I->report(nr::VariableRedefinition, IC::Error, {conflict.name},
                  conflict.expr->getLoc());
        break;
      }

      default:
        qcore_implement();
        break;
    }
  }
}

bool NRBuilder::check_duplicates(Seq *root, IReport *I) noexcept {
  bool ok = true;

  std::vector<Conflict> conflicts;
  std::unordered_map<std::string_view, Kind> names_map;

  {
    names_map.reserve(m_functions.size() + m_variables.size());

    std::for_each(m_functions.begin(), m_functions.end(),
                  [&](auto x) { names_map[x.first] = Kind::Function; });

    std::for_each(m_variables.begin(), m_variables.end(),
                  [&](auto x) { names_map[x.first] = Kind::Variable; });
  }

  { /* Diagnose duplicate symbols */
    conflicts.reserve(m_duplicate_functions->size() +
                      m_duplicate_variables->size());

    std::for_each(m_duplicate_functions->begin(), m_duplicate_functions->end(),
                  [&](auto x) {
                    conflicts.push_back({x->getName(), x, Kind::Function});
                  });

    std::for_each(m_duplicate_variables->begin(), m_duplicate_variables->end(),
                  [&](auto x) {
                    conflicts.push_back({x->getName(), x, Kind::Variable});
                  });

    /* Release the memory */
    m_duplicate_functions = std::nullopt;
    m_duplicate_variables = std::nullopt;
  }

  print_conflict_errors(conflicts, I);

  return ok;
}
