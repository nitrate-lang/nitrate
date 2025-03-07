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

#ifndef __NITRATE_CORE_LOGGER_H__
#define __NITRATE_CORE_LOGGER_H__

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
    Trace,     /* Low-value, high-volume debug info (malloc, free, ...)*/
    Debug,     /* High-value, mid-volume debug info (init, major API calls, ...) */
    Info,      /* Examples: upcoming feature notice, did you know about...? */
    Notice,    /* Examples: Bad design choice, suboptimal code, ... */
    Warning,   /* Example: Design likely to cause problems, horrible code, ... */
    Error,     /* Example: Missing semicolon after statement */
    Critical,  /* Example: Arena opened with 4 KB capacity, but 8 KB requested */
    Alert,     /* Example: Input pointer is not correctly aligned */
    Emergency, /* Example: Segmentation fault */
    Raw,       /* Raw output, no formatting */
    Sev_MaxValue = Raw
  };

  static inline std::string_view ToString(Sev sev) {
    switch (sev) {
      case Sev::Trace:
        return "trace";

      case Sev::Debug:
        return "debug";

      case Sev::Info:
        return "info";

      case Sev::Notice:
        return "notice";

      case Sev::Warning:
        return "warning";

      case Sev::Error:
        return "error";

      case Sev::Critical:
        return "critical";

      case Sev::Alert:
        return "alert";

      case Sev::Emergency:
        return "emergency";

      case Sev::Raw:
        return "raw";
    }
  }

  static inline std::ostream &operator<<(std::ostream &os, Sev sev) {
    os << ToString(sev);
    return os;
  }

  using EC = std::uint64_t;

  class ECUnique {
    std::uint64_t m_ec;

  public:
    explicit ECUnique(std::source_location loc = std::source_location::current());

    [[nodiscard]] constexpr auto Get() const -> EC { return m_ec; };
  };

  using LogFormatterFunc = std::function<std::string(std::string_view, Sev)>;

  auto Formatter(std::string_view msg, Sev sev) -> std::string;

  struct ECDetails {
    std::vector<std::string> m_tags, m_fixes, m_examples, m_dev_notes, m_notes;
    std::string m_flagname, m_nice_name, m_details;

    bool operator==(const ECDetails &rhs) const {
      return m_flagname == rhs.m_flagname && m_nice_name == rhs.m_nice_name && m_details == rhs.m_details &&
             m_tags == rhs.m_tags && m_fixes == rhs.m_fixes && m_examples == rhs.m_examples &&
             m_dev_notes == rhs.m_dev_notes && m_notes == rhs.m_notes;
    }
  };

  class NCC_EXPORT ECBase {
    EC m_ec = 0;
    ECDetails m_details;
    std::string m_json;

  protected:
    [[nodiscard]] virtual auto GetIdentity() const -> ECUnique = 0;

    [[nodiscard]] virtual auto GetDetailsPath() const -> std::optional<std::string_view> { return std::nullopt; }

    [[nodiscard]] virtual auto GetFormatter() const -> LogFormatterFunc = 0;
    void GetJsonRepresentation(std::ostream &os) const;
    void Finalize();

  public:
    constexpr ECBase() = default;
    virtual ~ECBase() = default;

    static auto ParseJsonECConfig(std::string_view json) -> std::optional<ECDetails>;

    [[nodiscard]] constexpr auto GetKind() const -> EC { return m_ec; }

    [[nodiscard]] constexpr auto AsJson() const { return std::string_view(m_json); }
    [[nodiscard]] constexpr auto FlagName() const { return std::string_view(m_details.m_flagname); }
    [[nodiscard]] constexpr auto NiceName() const { return std::string_view(m_details.m_nice_name); }
    [[nodiscard]] constexpr auto Details() const { return std::string_view(m_details.m_details); }
    [[nodiscard]] constexpr auto Tags() const { return std::span(m_details.m_tags); }
    [[nodiscard]] constexpr auto Fixes() const { return std::span(m_details.m_fixes); }
    [[nodiscard]] constexpr auto Examples() const { return std::span(m_details.m_examples); }
    [[nodiscard]] constexpr auto DevNotes() const { return std::span(m_details.m_dev_notes); }
    [[nodiscard]] constexpr auto UserNotes() const { return std::span(m_details.m_notes); }

    [[nodiscard]] auto Format(std::string_view msg, Sev sev) const { return GetFormatter()(msg, sev); }
  };

#define NCC_EC_GROUP(name)                                                         \
  class name : public ncc::ECBase {                                                \
  protected:                                                                       \
    [[nodiscard]] auto GetIdentity() const -> ncc::ECUnique override = 0;          \
    [[nodiscard]] auto GetFormatter() const -> ncc::LogFormatterFunc override = 0; \
  };

#define NCC_EC(group, name)                                                                              \
  static inline class name##Class final : public group {                                                 \
  protected:                                                                                             \
    [[nodiscard]] auto GetIdentity() const -> ncc::ECUnique override { return ncc::ECUnique(); }         \
    [[nodiscard]] auto GetFormatter() const -> ncc::LogFormatterFunc override { return ncc::Formatter; } \
                                                                                                         \
  public:                                                                                                \
    constexpr name##Class() { Finalize(); }                                                              \
  } name;

#define NCC_EC_EX(group, name, formatter, ...)                                                      \
  static inline class name##Class final : public group {                                            \
  protected:                                                                                        \
    [[nodiscard]] auto GetIdentity() const -> ncc::ECUnique override { return ncc::ECUnique(); }    \
    [[nodiscard]] auto GetFormatter() const -> ncc::LogFormatterFunc override { return formatter; } \
    [[nodiscard]] auto GetDetailsPath() const -> std::optional<std::string_view> override {         \
      std::string_view path;                                                                        \
      path = "" __VA_ARGS__;                                                                        \
      return path.empty() ? std::nullopt : std::make_optional(path);                                \
    }                                                                                               \
                                                                                                    \
  public:                                                                                           \
    constexpr name##Class() { Finalize(); }                                                         \
  } name;

  NCC_EC_GROUP(CoreEC);
  NCC_EC(CoreEC, UnknownEC);

  ///=========================================================================///

  struct LogMessage {
    const std::string &m_message;
    Sev m_sev;
    const ECBase &m_by;
  };

  using LogCallback = std::function<void(const LogMessage &)>;

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
        m_recv(LogMessage{m_ss.str(), m_severity, m_ec != nullptr ? *m_ec : UnknownEC});
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

  using LogSubscriberID = size_t;

  class NCC_EXPORT LoggerContext final : public std::ostream {
  public:
    class Subscriber {
      friend class LoggerContext;

      LogCallback m_cb;
      LogSubscriberID m_id;
      bool m_suspended;

    public:
      Subscriber(LogCallback cb, LogSubscriberID id, bool suspended)
          : m_cb(std::move(cb)), m_id(id), m_suspended(suspended) {}

      [[nodiscard]] auto ID() const -> LogSubscriberID { return m_id; }
      [[nodiscard]] auto Callback() const -> const LogCallback & { return m_cb; }
      [[nodiscard]] bool IsSuspended() const { return m_suspended; }
    };

  private:
    std::vector<Subscriber> m_subs;
    LogSubscriberID m_sub_id_ctr = 0;
    bool m_enabled = true;

  public:
    LoggerContext() = default;
    LoggerContext(const LoggerContext &o) : m_subs(o.m_subs), m_enabled(o.m_enabled) {}
    ~LoggerContext() override = default;

    auto Subscribe(LogCallback cb) -> LogSubscriberID;
    void Unsubscribe(LogSubscriberID id);
    void UnsubscribeAll();
    const auto &SubscribersList() const { return m_subs; }

    void Suspend(LogSubscriberID id);
    void SuspendAll();
    void Resume(LogSubscriberID id);
    void ResumeAll();

    LogSubscriberID operator+=(LogCallback cb) { return Subscribe(std::move(cb)); }

    void Enable() { m_enabled = true; }
    void Disable() { m_enabled = false; }
    [[nodiscard]] auto Enabled() const -> bool { return m_enabled; }

    void Reset() {
      m_subs.clear();
      m_enabled = true;
      m_sub_id_ctr = 0;
    }

    void Publish(const std::string &msg, Sev sev, const ECBase &ec) const;
  };

  auto operator<<(LoggerContext log, const auto &value) -> LogStream {
    LogStream stream([log](const LogMessage &m) { log.Publish(m.m_message, m.m_sev, m.m_by); });

    stream.Write(value);

    return stream;
  };

  auto operator<<(LogStream &&stream, const auto &value) -> LogStream {
    stream.Write(value);
    return std::move(stream);
  };

  /** This logger may be used prior to nitrate-core initialization */
  extern thread_local LoggerContext Log;
}  // namespace ncc

#endif
