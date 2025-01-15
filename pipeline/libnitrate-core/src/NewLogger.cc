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
#include <nitrate-core/NewLogger.hh>

using namespace ncc;

NCC_EXPORT thread_local LoggerContext ncc::log;

NCC_EXPORT std::string ncc::Formatter(std::string_view msg, Sev sev) {
  std::stringstream ss;

  switch (sev) {
    case Trace: {
      ss << "\x1b[1mtrace:\x1b[0m ";
      break;
    }

    case Debug: {
      ss << "\x1b[1mdebug:\x1b[0m ";
      break;
    }

    case Info: {
      ss << "\x1b[37;1minfo:\x1b[0m ";
      break;
    }

    case Notice: {
      ss << "\x1b[37;1mnotice:\x1b[0m ";
      break;
    }

    case Warning: {
      ss << "\x1b[35;1mwarning:\x1b[0m ";
      break;
    }

    case Sev::Error: {
      ss << "\x1b[31;1merror:\x1b[0m ";
      break;
    }

    case Critical: {
      ss << "\x1b[31;1;4mcritical:\x1b[0m ";
      break;
    }

    case Alert: {
      ss << "\x1b[31;1;4malert:\x1b[0m ";
      break;
    }

    case Emergency: {
      ss << "\x1b[31;1;4memergency:\x1b[0m ";
      break;
    }
  }

  ss << msg;

  return ss.str();
}

NCC_EXPORT ECUnique::ECUnique(std::source_location loc) {
  std::stringstream ss;
  ss << loc.file_name() << ":" << loc.line() << ":" << loc.column();
  m_ec = std::hash<std::string>{}(ss.str());
}

NCC_EXPORT void ECBase::GetJsonRepresentation(std::ostream &os) const {
  os << "{\"flagname\":\"" << flag_name() << "\",\"nice_name\":\""
     << nice_name() << "\",\"details\":\"" << details() << "\",\"tags\":[";
  for (auto it = tags().begin(); it != tags().end(); ++it) {
    os << "\"" << *it << "\"";
    if (it + 1 != tags().end()) {
      os << ",";
    }
  }
  os << "],\"fixes\":[";
  for (auto it = fixes().begin(); it != fixes().end(); ++it) {
    os << "\"" << *it << "\"";
    if (it + 1 != fixes().end()) {
      os << ",";
    }
  }
  os << "],\"examples\":[";
  for (auto it = examples().begin(); it != examples().end(); ++it) {
    os << "\"" << *it << "\"";
    if (it + 1 != examples().end()) {
      os << ",";
    }
  }
  os << "],\"dev_notes\":[";
  for (auto it = dev_notes().begin(); it != dev_notes().end(); ++it) {
    os << "\"" << *it << "\"";
    if (it + 1 != dev_notes().end()) {
      os << ",";
    }
  }
  os << "],\"user_notes\":[";
  for (auto it = user_notes().begin(); it != user_notes().end(); ++it) {
    os << "\"" << *it << "\"";
    if (it + 1 != user_notes().end()) {
      os << ",";
    }
  }
  os << "]}";
}

NCC_EXPORT void ECBase::Finalize() {
  m_ec = GetIdentity().get();

  /* Try to load information about the error from disk */
  if (auto path = GetDetailsPath(); path.has_value()) {
    if (auto details_opt = LoadDetailsFromFile(*path);
        details_opt.has_value()) {
      m_details = *details_opt;
    }
  }

  /* Generate JSON representation */
  std::ostringstream oss;
  GetJsonRepresentation(oss);
  m_json = oss.str();
}

size_t LoggerContext::subscribe(LogCallback cb) {
  m_subscribers.push_back(cb);
  return m_subscribers.size() - 1;
}

void LoggerContext::unsubscribe(size_t idx) {
  if (idx < m_subscribers.size()) {
    m_subscribers.erase(m_subscribers.begin() + idx);
  }
}

void LoggerContext::unsubscribe_all() { m_subscribers.clear(); }

size_t LoggerContext::add_filter(LogFilterFunc filter) {
  m_filters.push_back(filter);
  return m_filters.size() - 1;
}

void LoggerContext::remove_filter(size_t idx) {
  if (idx < m_filters.size()) {
    m_filters.erase(m_filters.begin() + idx);
  }
}

void LoggerContext::remove_filter(LogFilterFunc filter) {
  m_filters.erase(std::remove(m_filters.begin(), m_filters.end(), filter),
                  m_filters.end());
}

void LoggerContext::clear_filters() { m_filters.clear(); }

void LoggerContext::publish(const std::string &msg, Sev sev,
                            const ECBase &ec) const {
  if (m_enabled) {
    bool emit = m_filters.empty() ||
                std::all_of(m_filters.begin(), m_filters.end(),
                            [&](const auto &f) { return f(msg, sev, ec); });

    if (emit) {
      for (const auto &sub : m_subscribers) {
        sub(msg, sev, ec);
      }
    }
  }
}
