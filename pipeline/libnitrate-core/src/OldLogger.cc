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

#include <cstdio>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <sstream>
#include <string>

using namespace ncc;

static thread_local std::stringstream ThreadLogBuffer;

extern "C" NCC_EXPORT void QCoreBegin() { ThreadLogBuffer.str(""); }

extern "C" NCC_EXPORT void QCoreEnd(QCoreLog level) {
  std::string message = ThreadLogBuffer.str();

  while (message.ends_with("\n")) {
    message.pop_back();
  }

  switch (level) {
    case QCORE_DEBUG: {
      ncc::Log << ncc::Debug << message;
      break;
    }

    case QCORE_INFO: {
      ncc::Log << Info << message;
      break;
    }

    case QCORE_WARN: {
      ncc::Log << Warning << message;
      break;
    }

    case QCORE_ERROR: {
      ncc::Log << Error << message;
      break;
    }

    case QCORE_FATAL: {
      ncc::Log << Emergency << message;
      break;
    }
  }
}

extern "C" NCC_EXPORT int QCoreVWriteF(const char *fmt, va_list args) {
  char *buffer = nullptr;
  int size = vasprintf(&buffer, fmt, args);
  if (size < 0) {
    qcore_panic("Failed to allocate memory for log message.");
  }

  ThreadLogBuffer << std::string_view(buffer, size);

  free(buffer);

  return size;
}
