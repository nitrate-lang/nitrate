////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///           ░▒▓██████▓▒░░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░                  ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓███████▓▒░░▒▓███████▓▒░░▒▓█▓▒▒▓███▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///          ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░           ///
///           ░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░            ///
///             ░▒▓█▓▒░                                                      ///
///              ░▒▓██▓▒░                                                    ///
///                                                                          ///
///   * QUIX PACKAGE MANAGER - The official tool for the Quix language.      ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __QPKG_CORE_COLLECTOR_HH__
#define __QPKG_CORE_COLLECTOR_HH__

#include <core/Adapter.hh>
#include <cstdint>
#include <map>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <tuple>

namespace qpkg {
  namespace core {
    class LoggerStream {
      std::ostringstream &m_stream;
      std::function<void(const std::string &)> m_flush;

    public:
      LoggerStream(std::ostringstream &stream, std::function<void(const std::string &)> flush);
      LoggerStream &operator<<(std::ostream &(*f)(std::ostream &));

      template <class T>
      LoggerStream &operator<<(const T &msg) {
        m_stream << msg;
        return *this;
      }
    };

    class IFrontLogger {
    public:
      virtual IFrontLogger &push(const std::string &msg, float weight) = 0;
      virtual LoggerStream stream(float weight) = 0;
    };

    class ThreadLogger : public IFrontLogger {
      std::ostringstream m_stream;
      LoggerFlushCallback m_flush;
      Level m_level;

    public:
      ThreadLogger(Level level, LoggerFlushCallback flush);
      virtual ~ThreadLogger() = default;

      static ThreadLogger &get(Level level, LoggerFlushCallback flush);

      ThreadLogger &push(const std::string &msg, float weight) override;
      LoggerStream stream(float weight) override;
    };

    class GlobalCollector {
      LoggerFlushCallback m_flush;

      GlobalCollector() = default;
      GlobalCollector(const GlobalCollector &) = delete;
      GlobalCollector &operator=(const GlobalCollector &) = delete;

    public:
      static GlobalCollector &get();
      void setWriter(LoggerFlushCallback new_flush);
      void push(const std::string &msg, Level lvl, float weight);

      ThreadLogger &debug();
      ThreadLogger &good();
      ThreadLogger &bold();
      ThreadLogger &info();
      ThreadLogger &warn();
      ThreadLogger &error();
      ThreadLogger &fatal();
    };

    class Debug {
    public:
      static void push(const std::string &msg, float weight = 0) {
        GlobalCollector::get().debug().push(msg, weight);
      }
    };

    class Good {
    public:
      static void push(const std::string &msg, float weight = 0) {
        GlobalCollector::get().good().push(msg, weight);
      }
    };

    class Bold {
    public:
      static void push(const std::string &msg, float weight = 0) {
        GlobalCollector::get().bold().push(msg, weight);
      }
    };

    class Info {
    public:
      static void push(const std::string &msg, float weight = 0) {
        GlobalCollector::get().info().push(msg, weight);
      }
    };

    class Warn {
    public:
      static void push(const std::string &msg, float weight = 0) {
        GlobalCollector::get().warn().push(msg, weight);
      }
    };

    class Error {
    public:
      static void push(const std::string &msg, float weight = 0) {
        GlobalCollector::get().error().push(msg, weight);
      }
    };

    class Fatal {
    public:
      static void push(const std::string &msg, float weight = 0) {
        GlobalCollector::get().fatal().push(msg, weight);
      }
    };

    class Process {
      enum class Phase {
        PENDING,
        RUNNING,
        HALTED,
      };

      std::string m_name;
      Phase m_phase;

      Process(const std::string &name);

    public:
      static Process create(const std::string &name);

      Process &debug(const std::string &msg, float weight = 0);
      Process &good(const std::string &msg, float weight = 0);
      Process &bold(const std::string &msg, float weight = 0);
      Process &info(const std::string &msg, float weight = 0);
      Process &warn(const std::string &msg, float weight = 0);
      Process &error(const std::string &msg, float weight = 0);
      Process &fatal(const std::string &msg, float weight = 0);

      LoggerStream stream(Level lvl, float weight = 0);
      void done();

      void operator+=(float weight);
    };

    LoggerStream LOG(Level lvl);
  }  // namespace core
}  // namespace qpkg

#endif  // __QPKG_CORE_COLLECTOR_HH__
