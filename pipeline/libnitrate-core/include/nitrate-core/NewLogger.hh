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

#ifndef __NITRATE_CORE_NEWLOGGER_H__
#define __NITRATE_CORE_NEWLOGGER_H__

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <optional>
#include <source_location>
#include <span>
#include <string_view>
#include <vector>

namespace ncc {
  enum class Severity {
    Trace,  /* Low-value, high-volume debug info (malloc, free, ...)*/
    Debug,  /* High-value, mid-volume debug info (init, major API calls, ...) */
    Info,   /* Relevant some like: upcoming feature notice, did you know? */
    Notice, /* Examples: Bad design choice, suboptimal code, ... */
    Warning,  /* Example: Design likely to cause problems, horrible code, ... */
    Error,    /* Example: Missing semicolon after statement */
    Critical, /* Example: Arena opened with 4 KB capacity, but 8 KB requested */
    Alert,    /* Example: Input pointer is not correctly aligned */
    Emergency /* Example: Segmentation fault */
  };

  using EC = std::uint64_t;

  class ECUnique {
    std::uint64_t m_ec;

  public:
    ECUnique(std::source_location loc = std::source_location::current());

    constexpr EC get() const { return m_ec; };
  };

  class ECBase {
    struct Details {
      std::vector<std::string> tags, fixes, examples, dev_notes, notes;
      std::string flagname, nice_name, details;
      Severity severity;

      constexpr Details() {
        flagname = nice_name = details = "";
        severity = Severity::Error;
      }
    };

    EC m_ec;
    Details m_details;
    std::string m_json;

    static std::optional<Details> LoadDetailsFromFile(std::string_view path);

  protected:
    virtual ECUnique GetIdentity(void) const = 0;
    virtual std::optional<std::string_view> GetDetailsPath(void) const {
      return std::nullopt;
    }

    void GetJsonRepresentation(std::ostream &os) const;
    void Finalize(void);

  public:
    virtual ~ECBase() = default;

    constexpr EC getKind() const { return m_ec; }

    constexpr std::string_view as_json() const { return m_json; }
    constexpr std::string_view flag_name() const { return m_details.flagname; }
    constexpr std::string_view nice_name() const { return m_details.nice_name; }
    constexpr std::string_view details() const { return m_details.details; }
    constexpr auto severity() const { return m_details.severity; }
    constexpr auto tags() const { return std::span(m_details.tags); }
    constexpr auto fixes() const { return std::span(m_details.fixes); }
    constexpr auto examples() const { return std::span(m_details.examples); }
    constexpr auto dev_notes() const { return std::span(m_details.dev_notes); }
    constexpr auto user_notes() const { return std::span(m_details.notes); }
  };

#define NCC_EC_DOMAIN(name)                         \
  class name : public ncc::ECBase {                 \
  protected:                                        \
    ncc::ECUnique GetIdentity() const override = 0; \
  };

#define NCC_EC(domain, name)                                               \
  static inline class name##Class final : public domain {                  \
  protected:                                                               \
    ncc::ECUnique GetIdentity() const override { return ncc::ECUnique(); } \
                                                                           \
  public:                                                                  \
    constexpr name##Class() { Finalize(); }                                \
  } name;

#define NCC_EC_EX(domain, name, metadata_path)                             \
  static inline class name##Class final : public domain {                  \
  protected:                                                               \
    ncc::ECUnique GetIdentity() const override { return ncc::ECUnique(); } \
    std::optional<std::string_view> GetDetailsPath() const override {      \
      return std::string_view(metadata_path);                              \
    }                                                                      \
                                                                           \
  public:                                                                  \
    constexpr name##Class() { Finalize(); }                                \
  } name;
}  // namespace ncc

#endif
