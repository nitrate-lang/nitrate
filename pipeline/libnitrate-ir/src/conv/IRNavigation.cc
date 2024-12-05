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

#include <nitrate-ir/IRBuilder.hh>
#include <nitrate-ir/IRGraph.hh>
#include <string_view>
#include <unordered_map>

using namespace nr;

static std::string join_name_segment(const std::string &a,
                                     const std::string &b) {
  if (!a.empty() && !b.empty()) {
    return a + "::" + b;
  } else {
    return a + b;
  }
}

static void drop_tail_scope(std::string &scope) {
  auto sep_it = scope.find_last_of("::");
  if (sep_it == std::string_view::npos) {
    scope = "";
  } else {
    scope = scope.substr(0, sep_it);
  }
}

template <typename T>
static std::optional<std::pair<T, std::string>> find_in_scope_map(
    const std::unordered_map<std::string_view, T> &map,
    std::string_view qualified_name) {
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

  std::optional<std::pair<T, std::string>> R;

  while (true) {
    the_case = join_name_segment(scope, name);

    auto it = map.find(the_case);
    if (it != map.end()) {
      R = {it->second, the_case};
      break;
    }

    if (scope.empty()) {
      break;
    } else {
      drop_tail_scope(scope);
    }
  };

  return R;
}

std::optional<std::pair<Expr *, std::string_view>> NRBuilder::resolve_name(
    std::string_view name, Kind kind) noexcept {
  std::optional<std::pair<Expr *, std::string>> R;

  switch (kind) {
    case Kind::TypeDef: {
      R = find_in_scope_map(m_named_types, name);
      break;
    }

    case Kind::ScopedEnum: {
      auto idx = name.find_last_of("::");
      if (idx != std::string_view::npos) {
        std::string_view basename = name.substr(0, idx - 1);
        std::string_view field_name = name.substr(idx + 1);

        if (const auto &submap =
                find_in_scope_map(m_named_constant_group, basename)) {
          auto it = submap.value().first.find(field_name);
          if (it != submap.value().first.end()) {
            R = {it->second, std::string(name)};
          }
        }
      }
      break;
    }

    case Kind::Function: {
      R = find_in_scope_map(m_functions, name);
      break;
    }

    case Kind::Variable: {
      R = find_in_scope_map(m_variables, name);
      break;
    }
  }

  if (!R.has_value()) {
    return std::nullopt;
  }

  return {{R->first, intern(R->second)}};
}
