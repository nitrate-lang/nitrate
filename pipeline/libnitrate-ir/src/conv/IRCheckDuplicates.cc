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

#define IRBUILDER_IMPL

#include <nitrate-core/Error.h>

#include <algorithm>
#include <nitrate-core/StringIntern.hh>
#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>
#include <nitrate-ir/Report.hh>
#include <unordered_map>

using namespace nr;

///=============================================================================

struct Conflict {
  std::string_view name;
  Expr *us;
  Kind us_kind;

  std::optional<Expr *> them;
  Kind them_kind;

  Conflict(std::string_view name, Kind us_kind, Expr *us, Kind them_kind,
           std::optional<Expr *> them)
      : name(name),
        us(us),
        us_kind(us_kind),
        them(them),
        them_kind(them_kind) {}

  Conflict() = default;
};

static void print_conflict_errors(const std::vector<Conflict> &conflicts,
                                  IReport *I) {
  static const std::unordered_map<Kind, std::string_view> kind_name = {
      {Kind::Function, "function"},
      {Kind::Variable, "variable"},
      {Kind::TypeDef, "named type"},
      {Kind::ScopedEnum, "named constant"},
  };

  for (const auto &conflict : conflicts) {
    I->report(nr::NameConflict, IC::Error,
              {kind_name.at(conflict.us_kind), " name '", conflict.name,
               "' is already defined as a ", kind_name.at(conflict.them_kind)},
              conflict.us->getLoc());
  }
}

bool NRBuilder::check_duplicates(Seq *, IReport *I) {
  std::vector<Conflict> conflicts;
  std::unordered_map<std::string_view, std::pair<Kind, Expr *>> names_map;

  {
    names_map.reserve(m_functions.size() + m_variables.size() +
                      m_named_types.size() + m_named_constant_group.size());

    std::for_each(m_functions.begin(), m_functions.end(), [&](auto x) {
      names_map[x.first] = {Kind::Function, x.second};
    });

    std::for_each(m_variables.begin(), m_variables.end(), [&](auto x) {
      names_map[x.first] = {Kind::Variable, x.second};
    });

    std::for_each(m_named_types.begin(), m_named_types.end(), [&](auto x) {
      names_map[x.first] = {Kind::TypeDef, x.second};
    });

    std::for_each(
        m_named_constant_group.begin(), m_named_constant_group.end(),
        [&](auto x) {
          std::for_each(x.second.begin(), x.second.end(), [&](auto y) {
            auto joined =
                qcore::save(std::string(x.first) + "::" + std::string(y.first));
            names_map[joined] = {Kind::ScopedEnum, y.second};
          });
        });
  }

  { /* Diagnose naming general conflicts */
    std::for_each(m_functions.begin(), m_functions.end(), [&](auto x) {
      auto it = names_map.find(x.first);
      if (it != names_map.end() && it->second.first != Kind::Function) {
        conflicts.push_back({x.first, Kind::Function, x.second,
                             it->second.first, it->second.second});
      }
    });

    std::for_each(m_variables.begin(), m_variables.end(), [&](auto x) {
      auto it = names_map.find(x.first);
      if (it != names_map.end() && it->second.first != Kind::Variable) {
        conflicts.push_back({x.first, Kind::Variable, x.second,
                             it->second.first, it->second.second});
      }
    });

    std::for_each(m_named_types.begin(), m_named_types.end(), [&](auto x) {
      auto it = names_map.find(x.first);
      if (it != names_map.end() && it->second.first != Kind::TypeDef) {
        conflicts.push_back({x.first, Kind::TypeDef, x.second, it->second.first,
                             it->second.second});
      }
    });

    std::for_each(
        m_named_constant_group.begin(), m_named_constant_group.end(),
        [&](auto x) {
          std::for_each(x.second.begin(), x.second.end(), [&](auto y) {
            auto named_constant =
                qcore::save(std::string(x.first) + "::" + std::string(y.first));

            auto it = names_map.find(named_constant);
            if (it != names_map.end() && it->second.first != Kind::ScopedEnum) {
              conflicts.push_back({named_constant, Kind::ScopedEnum, y.second,
                                   it->second.first, it->second.second});
            }
          });
        });
  }

  { /* Diagnose duplicate symbols */
    conflicts.reserve(
        m_duplicate_functions->size() + m_duplicate_variables->size() +
        m_duplicate_named_types->size() + m_duplicate_named_constants->size());

    std::for_each(m_duplicate_functions->begin(), m_duplicate_functions->end(),
                  [&](auto x) {
                    conflicts.push_back({x->getName(), Kind::Function, x,
                                         Kind::Function, std::nullopt});
                  });

    std::for_each(m_duplicate_variables->begin(), m_duplicate_variables->end(),
                  [&](auto x) {
                    conflicts.push_back({x->getName(), Kind::Variable, x,
                                         Kind::Variable, std::nullopt});
                  });

    std::for_each(m_duplicate_named_types->begin(),
                  m_duplicate_named_types->end(), [&](auto x) {
                    conflicts.push_back({x, Kind::TypeDef, m_named_types[x],
                                         Kind::TypeDef, std::nullopt});
                  });

    std::for_each(m_duplicate_named_constants->begin(),
                  m_duplicate_named_constants->end(), [&](auto x) {
                    conflicts.push_back({x, Kind::ScopedEnum, createIgn(),
                                         Kind::ScopedEnum, std::nullopt});
                  });

    /* Release the memory */
    m_duplicate_functions = std::nullopt;
    m_duplicate_variables = std::nullopt;
    m_duplicate_named_types = std::nullopt;
    m_duplicate_named_constants = std::nullopt;
  }

  print_conflict_errors(conflicts, I);

  return conflicts.empty();
}
