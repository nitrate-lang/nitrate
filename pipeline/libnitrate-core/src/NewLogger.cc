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

#include <nitrate-core/NewLogger.hh>

using namespace ncc;

CPP_EXPORT thread_local LoggerContext ncc::log;

CPP_EXPORT std::string ncc::Formatter(std::string_view msg, Sev sev) {
  return std::string(msg);
}

CPP_EXPORT ECUnique::ECUnique(std::source_location loc) {
  std::stringstream ss;
  ss << loc.file_name() << ":" << loc.line() << ":" << loc.column();
  m_ec = std::hash<std::string>{}(ss.str());
}

CPP_EXPORT void ECBase::GetJsonRepresentation(std::ostream &os) const {
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

CPP_EXPORT void ECBase::Finalize(void) {
  m_ec = GetIdentity().get();

  /* Try to load information about the error from disk */
  if (auto path = GetDetailsPath(); path.has_value()) {
    if (auto details = LoadDetailsFromFile(*path); details.has_value()) {
      m_details = *details;
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
