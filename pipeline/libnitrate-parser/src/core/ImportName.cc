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

#include <nitrate-core/Logger.hh>
#include <nitrate-parser/Package.hh>

using namespace ncc::parse;

bool ImportName::Validate(const std::string &name) {
  /// TODO: Validate the import name
  (void)name;
  return true;
}

ImportName::ImportName(std::string name) {
  if (Validate(name)) {
    m_name = std::move(name);
    return;
  }

  Log << Trace << "ImportName: Not valid: " << name;
}

auto ImportName::GetChain() const -> std::vector<std::string_view> {
  std::vector<std::string_view> chain;
  std::string_view name = *m_name;

  while (!name.empty()) {
    auto pos = name.find_first_of("::");
    if (pos == std::string_view::npos) {
      chain.push_back(name);
      break;
    }

    chain.push_back(name.substr(0, pos));
    name.remove_prefix(pos + 2);
  }

  Log << Trace << "ImportName: \"" << *m_name << "\" -> " << chain.size() << " parts";

  return chain;
}

std::ostream &ncc::parse::operator<<(std::ostream &os, const ImportName &name) {
  if (name.IsValid()) {
    os << name.GetName();
  } else {
    os << "<invalid>";
  }

  return os;
}
