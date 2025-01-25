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

#ifndef __NO3_CONF_PARSER_HH__
#define __NO3_CONF_PARSER_HH__

#include <conf/Config.hh>
#include <optional>
#include <string>

namespace no3::conf {
  class IParser {
  public:
    virtual ~IParser() = default;

    /**
     * @brief Parse NO3 package configuration file
     *
     * @param filepath Configuration filepath
     * @return std::optional<Config> Configuration object
     * @note If any error occurs, the function returns an empty optional.
     */
    auto Parsef(const std::string &filepath) -> std::optional<Config>;

    /**
     * @brief Parse NO3 package configuration content
     *
     * @param data Configuration file content
     * @return std::optional<Config> Configuration object
     * @note If any error occurs, the function returns an empty optional.
     */
    virtual auto Parse(const std::string &content) -> std::optional<Config> = 0;
  };

  class YamlConfigParser : public IParser {
  public:
    /**
     * @brief Parse NO3 package configuration content
     *
     * @param data Configuration file content
     * @return std::optional<Config> Configuration object
     * @note If any error occurs, the function returns an empty optional.
     */
    auto Parse(const std::string &content) -> std::optional<Config> override;
  };
}  // namespace no3::conf

#endif  // __NO3_CONF_PARSER_HH__
