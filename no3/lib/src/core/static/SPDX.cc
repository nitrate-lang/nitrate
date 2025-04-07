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
#include <core/static/SPDX.hh>
#include <functional>
#include <nitrate-core/Assert.hh>
#include <nitrate-core/CatchAll.hh>
#include <nitrate-core/Logger.hh>
#include <vector>

using namespace ncc;

static auto Levenstein(std::string_view a, std::string_view b) -> size_t {
  /// BY: https://chatgpt.com/share/67c8cc55-73dc-8001-b7ca-7da50b9d5410

  size_t m = a.size();
  size_t n = b.size();
  std::vector<std::vector<size_t>> memo(m + 1, std::vector<size_t>(n + 1, -1));

  std::function<size_t(size_t, size_t)> dp = [&](size_t i, size_t j) -> size_t {
    if (memo[i][j] != static_cast<size_t>(-1)) {
      return memo[i][j];
    }

    if (i == m) {
      return memo[i][j] = n - j;
    }

    if (j == n) {
      return memo[i][j] = m - i;
    }

    if (a[i] == b[j]) {
      return memo[i][j] = dp(i + 1, j + 1);
    }

    return memo[i][j] = 1 + std::min({dp(i, j + 1), dp(i + 1, j), dp(i + 1, j + 1)});
  };

  return dp(0, 0);
}

auto no3::constants::FindClosestSPDXLicense(std::string query) -> std::string_view {
  std::transform(query.begin(), query.end(), query.begin(), ::tolower);

  size_t minv = -1;
  std::string_view mini;

  qcore_assert(!SPDX_IDENTIFIERS.empty());

  for (const auto& [lowercase_spdx, case_sensitive_spdx] : SPDX_IDENTIFIERS) {
    size_t const dist = Levenstein(lowercase_spdx, query);
    if (dist < minv) {
      minv = dist;
      mini = case_sensitive_spdx;
    }
  }

  return mini;
}

auto no3::constants::IsExactSPDXLicenseMatch(std::string_view query) -> bool {
  std::string lowercase_query(query);
  std::transform(lowercase_query.begin(), lowercase_query.end(), lowercase_query.begin(), ::tolower);
  return SPDX_IDENTIFIERS.contains(lowercase_query);
}
