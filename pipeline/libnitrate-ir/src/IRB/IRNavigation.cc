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

#include <nitrate-core/Logger.hh>
#include <nitrate-core/String.hh>
#include <nitrate-ir/IR/Nodes.hh>
#include <nitrate-ir/IRB/Builder.hh>
#include <string_view>
#include <unordered_map>

using namespace ncc::ir;
using namespace ncc;

static auto JoinNameSegment(const std::string &a,
                            const std::string &b) -> std::string {
  if (!a.empty() && !b.empty()) {
    return a + "::" + b;
  } else {
    return a + b;
  }
}

static void DropTailScope(std::string &scope) {
  auto sep_it = scope.find_last_of("::");
  if (sep_it == std::string_view::npos) {
    scope = "";
  } else {
    scope = scope.substr(0, sep_it);
  }
}

template <typename T>
static auto FindInScopeMap(const std::unordered_map<std::string_view, T> &map,
                           std::string_view qualified_name)
    -> std::optional<std::pair<T, std::string>> {
  auto sep_it = qualified_name.find_last_of("$$");
  if (sep_it == std::string_view::npos) {
    auto it = map.find(qualified_name);
    return it == map.end() ? std::nullopt
                           : std::optional<std::pair<T, std::string>>(
                                 {it->second, std::string(qualified_name)});
  }

  std::string scope = std::string(qualified_name.substr(0, sep_it - 1));
  std::string name = std::string(qualified_name.substr(sep_it + 1));
  std::string the_case;

  std::optional<std::pair<T, std::string>> r;

  while (true) {
    the_case = JoinNameSegment(scope, name);

    auto it = map.find(the_case);
    if (it != map.end()) {
      r = {it->second, the_case};
      break;
    }

    if (scope.empty()) {
      break;
    } else {
      DropTailScope(scope);
    }
  };

  return r;
}

auto NRBuilder::ResolveName(std::string_view name, Kind kind)
    -> std::optional<std::pair<FlowPtr<Expr>, std::string_view>> {
  std::optional<std::pair<FlowPtr<Expr>, std::string>> r;

  switch (kind) {
    case Kind::TypeDef: {
      r = FindInScopeMap(m_named_types, name);
      break;
    }

    case Kind::ScopedEnum: {
      auto idx = name.find_last_of("::");
      if (idx != std::string_view::npos) {
        std::string_view basename = name.substr(0, idx - 1);
        std::string_view field_name = name.substr(idx + 1);

        if (const auto &submap =
                FindInScopeMap(m_named_constant_group, basename)) {
          auto it = submap.value().first.find(field_name);
          if (it != submap.value().first.end()) {
            r = {it->second, std::string(name)};
          }
        }
      }
      break;
    }

    case Kind::Function: {
      r = FindInScopeMap(m_functions, name);
      break;
    }

    case Kind::Variable: {
      r = FindInScopeMap(m_variables, name);
      break;
    }
  }

  if (!r.has_value()) {
    return std::nullopt;
  }

  return {{r->first, string(r->second)}};
}
