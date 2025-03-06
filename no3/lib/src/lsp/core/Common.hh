#pragma once

#include <boost/flyweight.hpp>
#include <string>

namespace no3::lsp {
  using String = boost::flyweight<std::string>;

  constexpr std::string_view kLicenseText =
      R"(The Nitrate LSP Server is part of Nitrate Compiler Suite.
Copyright (C) 2024 Wesley C. Jones

The Nitrate Compiler Suite is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.))";
}  // namespace no3::lsp
