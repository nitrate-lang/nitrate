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
#include <functional>
#include <iostream>
#include <nitrate-core/Macro.hh>
#include <optional>
#include <source_location>
#include <span>
#include <sstream>
#include <string_view>
#include <vector>

namespace ncc {
  enum Sev : uint8_t {
    Trace,  /* Low-value, high-volume debug info (malloc, free, ...)*/
    Debug,  /* High-value, mid-volume debug info (init, major API calls, ...) */
    Info,   /* Examples: upcoming feature notice, did you know about...? */
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
    explicit ECUnique(
        std::source_location loc = std::source_location::current());

    [[nodiscard]] constexpr auto Get() const -> EC { return m_ec; };
  };

  using LogFormatterFunc = std::function<std::string(std::string_view, Sev)>;

  auto Formatter(std::string_view msg, Sev sev) -> std::string;

  class ECBase {
    struct ECDetails {
      std::vector<std::string> m_tags, m_fixes, m_examples, m_dev_notes,
          m_notes;
      std::string m_flagname, m_nice_name, m_details;
    };

    EC m_ec = 0;
    ECDetails m_details;
    std::string m_json;

    static auto LoadDetailsFromFile(std::string_view path)
        -> std::optional<ECDetails>;

  protected:
    [[nodiscard]] virtual auto GetIdentity() const -> ECUnique = 0;

    [[nodiscard]] virtual auto GetDetailsPath() const
        -> std::optional<std::string_view> {
      return std::nullopt;
    }

    [[nodiscard]] virtual auto GetFormatter() const -> LogFormatterFunc = 0;
    void GetJsonRepresentation(std::ostream &os) const;
    void Finalize();

  public:
    constexpr ECBase() = default;
    virtual ~ECBase() = default;

    [[nodiscard]] constexpr auto GetKind() const -> EC { return m_ec; }

    [[nodiscard]] constexpr auto AsJson() const {
      return std::string_view(m_json);
    }
    [[nodiscard]] constexpr auto FlagName() const {
      return std::string_view(m_details.m_flagname);
    }
    [[nodiscard]] constexpr auto NiceName() const {
      return std::string_view(m_details.m_nice_name);
    }
    [[nodiscard]] constexpr auto Details() const {
      return std::string_view(m_details.m_details);
    }
    [[nodiscard]] constexpr auto Tags() const {
      return std::span(m_details.m_tags);
    }
    [[nodiscard]] constexpr auto Fixes() const {
      return std::span(m_details.m_fixes);
    }
    [[nodiscard]] constexpr auto Examples() const {
      return std::span(m_details.m_examples);
    }
    [[nodiscard]] constexpr auto DevNotes() const {
      return std::span(m_details.m_dev_notes);
    }
    [[nodiscard]] constexpr auto UserNotes() const {
      return std::span(m_details.m_notes);
    }

    [[nodiscard]] auto Format(std::string_view msg, Sev sev) const {
      return GetFormatter()(msg, sev);
    }
  };

#define NCC_EC_GROUP(name)                                           \
  class name : public ncc::ECBase {                                  \
  protected:                                                         \
    auto GetIdentity() const -> ncc::ECUnique override = 0;          \
    auto GetFormatter() const -> ncc::LogFormatterFunc override = 0; \
  };

#define NCC_EC(group, name)                                       \
  static inline class name##Class final : public group {          \
  protected:                                                      \
    auto GetIdentity() const -> ncc::ECUnique override {          \
      return ncc::ECUnique();                                     \
    }                                                             \
    auto GetFormatter() const -> ncc::LogFormatterFunc override { \
      return ncc::Formatter;                                      \
    }                                                             \
                                                                  \
  public:                                                         \
    constexpr name##Class() { Finalize(); }                       \
  } name;

#define NCC_EC_EX(group, name, formatter, ...)                                \
  static inline class name##Class final : public group {                      \
  protected:                                                                  \
    auto GetIdentity() const -> ncc::ECUnique override {                      \
      return ncc::ECUnique();                                                 \
    }                                                                         \
    auto GetFormatter() const -> ncc::LogFormatterFunc override {             \
      return formatter;                                                       \
    }                                                                         \
    auto GetDetailsPath() const -> std::optional<std::string_view> override { \
      return std::string_view("" __VA_ARGS__);                                \
    }                                                                         \
                                                                              \
  public:                                                                     \
    constexpr name##Class() { Finalize(); }                                   \
  } name;

  NCC_EC_GROUP(CoreEC);
  NCC_EC(CoreEC, UnknownEC);

  ///=========================================================================///

  using LogCallback =
      std::function<void(const std::string &, Sev, const ECBase &)>;

  class LogStream final {
    std::stringstream m_ss;
    Sev m_severity = Sev::Error;
    const ECBase *m_ec = nullptr;
    LogCallback m_recv;

  public:
    explicit LogStream(LogCallback pub) : m_recv(std::move(pub)) {}
    LogStream(LogStream &&) = default;

    ~LogStream() {
      if (m_recv) {
        m_recv(m_ss.str(), m_severity, m_ec != nullptr ? *m_ec : UnknownEC);
      }
    }

    template <typename T>
    void Write(const T &value) {
      if constexpr (std::is_base_of_v<ECBase, T>) {
        m_ec = &value; /* ECBase children must have static lifetime */
      } else if constexpr (std::is_same_v<Sev, T>) {
        m_severity = value;
      } else {
        m_ss << value;
      }
    }
  };

  using LogFilterFunc = bool (*)(const std::string &, Sev, const ECBase &);

#define NCC_EC_FILTER(__name, __msg, __sev, __ec)                \
  static inline auto __name(const std::string &__msg, Sev __sev, \
                            const ECBase &__ec) -> bool

  NCC_EC_FILTER(TraceFilter, msg, sev, ec);

  class NCC_EXPORT LoggerContext final {
    std::vector<LogCallback> m_subscribers;
    std::vector<LogFilterFunc> m_filters;
    bool m_enabled = true;

  public:
    LoggerContext() = default;
    ~LoggerContext() = default;

    auto Subscribe(LogCallback cb) -> size_t;
    void Unsubscribe(size_t idx);
    void UnsubscribeAll();

    auto AddFilter(LogFilterFunc filter) -> size_t;
    void RemoveFilter(size_t idx);
    void RemoveFilter(LogFilterFunc filter);
    void ClearFilters();

    void operator+=(LogFilterFunc filter) { AddFilter(filter); }
    void operator-=(LogFilterFunc filter) { RemoveFilter(filter); }
    void operator+=(LogCallback cb) { Subscribe(std::move(cb)); }

    void Enable() { m_enabled = true; }
    void Disable() { m_enabled = false; }
    [[nodiscard]] auto Enabled() const -> bool { return m_enabled; }

    void Publish(const std::string &msg, Sev sev, const ECBase &ec) const;
  };

  auto operator<<(LoggerContext log, const auto &value) -> LogStream {
    LogStream stream([log](auto msg, auto sev, const auto &ec) {
      log.Publish(msg, sev, ec);
    });

    stream.Write(value);

    return stream;
  };

  auto operator<<(LogStream &&stream, const auto &value) -> LogStream {
    stream.Write(value);
    return std::move(stream);
  };

  extern thread_local LoggerContext Log;
}  // namespace ncc

#endif
