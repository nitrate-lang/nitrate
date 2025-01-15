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

#include <algorithm>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/String.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/IRB/Builder.hh>
#include <nitrate-ir/diagnostic/Report.hh>
#include <unordered_map>

using namespace ncc;
using namespace ncc::ir;
using namespace ncc::ir::ec;

///=============================================================================

struct Conflict {
  std::string_view m_name;
  NullableFlowPtr<Expr> m_us;
  Kind m_us_kind;

  std::optional<FlowPtr<Expr>> m_them;
  Kind m_them_kind;

  Conflict(std::string_view name, Kind us_kind, FlowPtr<Expr> us,
           Kind them_kind, std::optional<FlowPtr<Expr>> them)
      : m_name(name),
        m_us(us),
        m_us_kind(us_kind),
        m_them(them),
        m_them_kind(them_kind) {}

  Conflict() = default;
};

static void PrintConflictErrors(const std::vector<Conflict> &conflicts,
                                  IReport *i) {
  static const std::unordered_map<Kind, std::string_view> kind_name = {
      {Kind::Function, "function"},
      {Kind::Variable, "variable"},
      {Kind::TypeDef, "named type"},
      {Kind::ScopedEnum, "named constant"},
  };

  for (const auto &conflict : conflicts) {
    // I->report(NameConflict, IC::Error,
    //           {kind_name.at(conflict.us_kind), " name '", conflict.name,
    //            "' is already defined as a ",
    //            kind_name.at(conflict.them_kind)},
    //           conflict.us.value()->getLoc());

    switch (conflict.m_us_kind) {
      case Kind::Function:
      case Kind::Variable: {
        ncc::Log << ConflictingSymbol << conflict.m_us.value()->GetLoc()
                 << conflict.m_name << kind_name.at(conflict.m_them_kind);
        break;
      }

      case Kind::TypeDef:
      case Kind::ScopedEnum: {
        ncc::Log << ConflictingType << conflict.m_us.value()->GetLoc()
                 << conflict.m_name << kind_name.at(conflict.m_them_kind);
        break;
      }
    }
  }
}

bool NRBuilder::CheckDuplicates(FlowPtr<Seq>, IReport *i) {
  std::vector<Conflict> conflicts;
  std::unordered_map<std::string_view, std::pair<Kind, FlowPtr<Expr>>>
      names_map;

  {
    names_map.reserve(m_functions.size() + m_variables.size() +
                      m_named_types.size() + m_named_constant_group.size());

    std::for_each(m_functions.begin(), m_functions.end(), [&](auto x) {
      names_map.insert({x.first, {Kind::Function, x.second}});
    });

    std::for_each(m_variables.begin(), m_variables.end(), [&](auto x) {
      names_map.insert({x.first, {Kind::Variable, x.second}});
    });

    std::for_each(m_named_types.begin(), m_named_types.end(), [&](auto x) {
      names_map.insert({x.first, {Kind::TypeDef, x.second}});
    });

    std::for_each(
        m_named_constant_group.begin(), m_named_constant_group.end(),
        [&](auto x) {
          std::for_each(x.second.begin(), x.second.end(), [&](auto y) {
            auto joined =
                string(std::string(x.first) + "::" + std::string(y.first));
            names_map.insert({joined, {Kind::ScopedEnum, y.second}});
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
                string(std::string(x.first) + "::" + std::string(y.first));

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
                    conflicts.push_back({x, Kind::TypeDef, m_named_types.at(x),
                                         Kind::TypeDef, std::nullopt});
                  });

    std::for_each(m_duplicate_named_constants->begin(),
                  m_duplicate_named_constants->end(), [&](auto x) {
                    conflicts.push_back({x, Kind::ScopedEnum, CreateIgn(),
                                         Kind::ScopedEnum, std::nullopt});
                  });

    /* Release the memory */
    m_duplicate_functions = std::nullopt;
    m_duplicate_variables = std::nullopt;
    m_duplicate_named_types = std::nullopt;
    m_duplicate_named_constants = std::nullopt;
  }

  PrintConflictErrors(conflicts, i);

  return conflicts.empty();
}
