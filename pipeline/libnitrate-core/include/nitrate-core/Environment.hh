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

#ifndef __NITRATE_CORE_ENV_H__
#define __NITRATE_CORE_ENV_H__

#include <nitrate-core/Allocate.hh>
#include <nitrate-core/IEnvironment.hh>
#include <nitrate-core/String.hh>
#include <optional>
#include <ranges>
#include <unordered_map>

namespace ncc {
  class NCC_EXPORT Environment final : public IEnvironment {
    std::unordered_map<string, string> m_data;

    void SetupDefaultKeys();

  public:
    Environment() { SetupDefaultKeys(); }
    ~Environment() override = default;

    Environment(const Environment &) = delete;
    Environment(Environment &&) = delete;
    Environment &operator=(const Environment &) = delete;
    Environment &operator=(Environment &&) = delete;

    auto Contains(std::string_view key) -> bool override;
    auto Get(string key) -> std::optional<string> override;
    void Set(string key, std::optional<string> value) override;

    auto GetKeys() const { return m_data | std::ranges::views::keys; }
  };

}  // namespace ncc

#endif  // __NITRATE_CORE_ENV_H__
