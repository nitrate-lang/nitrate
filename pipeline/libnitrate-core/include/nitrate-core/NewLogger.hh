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
  enum Sev {
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
    ECUnique(std::source_location loc = std::source_location::current());

    constexpr EC get() const { return m_ec; };
  };

  using LogFormatterFunc = std::function<std::string(std::string_view, Sev)>;

  std::string Formatter(std::string_view msg, Sev sev);

  class ECBase {
    struct Details {
      std::vector<std::string> tags, fixes, examples, dev_notes, notes;
      std::string flagname, nice_name, details;

      constexpr Details() { flagname = nice_name = details = ""; }
    };

    EC m_ec;
    Details m_details;
    std::string m_json;

    static std::optional<Details> LoadDetailsFromFile(std::string_view path);

  protected:
    virtual ECUnique GetIdentity() const = 0;

    virtual std::optional<std::string_view> GetDetailsPath() const {
      return std::nullopt;
    }

    virtual LogFormatterFunc GetFormatter() const = 0;
    void GetJsonRepresentation(std::ostream &os) const;
    void Finalize();

  public:
    virtual ~ECBase() = default;

    constexpr EC getKind() const { return m_ec; }

    constexpr std::string_view as_json() const { return m_json; }
    constexpr std::string_view flag_name() const { return m_details.flagname; }
    constexpr std::string_view nice_name() const { return m_details.nice_name; }
    constexpr std::string_view details() const { return m_details.details; }
    constexpr auto tags() const { return std::span(m_details.tags); }
    constexpr auto fixes() const { return std::span(m_details.fixes); }
    constexpr auto examples() const { return std::span(m_details.examples); }
    constexpr auto dev_notes() const { return std::span(m_details.dev_notes); }
    constexpr auto user_notes() const { return std::span(m_details.notes); }

    std::string format(std::string_view msg, Sev sev) const {
      return GetFormatter()(msg, sev);
    }
  };

#define NCC_EC_GROUP(name)                                   \
  class name : public ncc::ECBase {                          \
  protected:                                                 \
    ncc::ECUnique GetIdentity() const override = 0;          \
    ncc::LogFormatterFunc GetFormatter() const override = 0; \
  };

#define NCC_EC(group, name)                                                \
  static inline class name##Class final : public group {                   \
  protected:                                                               \
    ncc::ECUnique GetIdentity() const override { return ncc::ECUnique(); } \
    ncc::LogFormatterFunc GetFormatter() const override {                  \
      return ncc::Formatter;                                               \
    }                                                                      \
                                                                           \
  public:                                                                  \
    constexpr name##Class() { Finalize(); }                                \
  } name;

#define NCC_EC_EX(group, name, formatter, ...)                                \
  static inline class name##Class final : public group {                      \
  protected:                                                                  \
    ncc::ECUnique GetIdentity() const override { return ncc::ECUnique(); }    \
    ncc::LogFormatterFunc GetFormatter() const override { return formatter; } \
    std::optional<std::string_view> GetDetailsPath() const override {         \
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
    LogStream(LogCallback pub) : m_recv(pub) {}
    LogStream(LogStream &&) = default;

    ~LogStream() {
      if (m_recv) {
        m_recv(m_ss.str(), m_severity, m_ec ? *m_ec : UnknownEC);
      }
    }

    template <typename T>
    void write(const T &value) {
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

#define NCC_EC_FILTER(name, msg, sev, ec) \
  static inline bool name(const std::string &msg, Sev sev, const ECBase &ec)

  class NCC_EXPORT LoggerContext final {
    std::vector<LogCallback> m_subscribers;
    std::vector<LogFilterFunc> m_filters;
    bool m_enabled = true;

  public:
    LoggerContext() = default;
    ~LoggerContext() = default;

    size_t subscribe(LogCallback cb);
    void unsubscribe(size_t idx);
    void unsubscribe_all();

    size_t add_filter(LogFilterFunc filter);
    void remove_filter(size_t idx);
    void remove_filter(LogFilterFunc filter);
    void clear_filters();

    void operator+=(LogFilterFunc filter) { add_filter(filter); }
    void operator-=(LogFilterFunc filter) { remove_filter(filter); }
    void operator+=(LogCallback cb) { subscribe(cb); }

    void enable() { m_enabled = true; }
    void disable() { m_enabled = false; }
    bool enabled() const { return m_enabled; }

    void publish(const std::string &msg, Sev sev, const ECBase &ec) const;
  };

  LogStream operator<<(LoggerContext log, const auto &value) {
    LogStream stream([log](auto msg, auto sev, const auto &ec) {
      log.publish(msg, sev, ec);
    });

    stream.write(value);

    return stream;
  };

  LogStream operator<<(LogStream &&stream, const auto &value) {
    stream.write(value);
    return std::move(stream);
  };

  extern thread_local LoggerContext log;
}  // namespace ncc

#endif
