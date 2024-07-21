////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
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

#include <chrono>
#include <cstdlib>
#include <quixcc/plugin/EngineAPI.h>
#include <quixcc/preprocessor/QSys.h>
#include <thread>

QSYS_DEFINE(qsys_time, "Get the timestamp local or UTC") {
  QSYS_ARGASSERT(qsys_time, 1);
  QSYS_ARG_STRING(qsys_time, is_local, 0);

  (void)is_local_len;

  if (strcmp(is_local, "true") == 0) {
    time_t t = time(0);
    struct tm *now = localtime(&t);
    char buf[80] = {0};
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", now);
    return strdup(buf);
  } else {
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::milliseconds ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return strdup(std::to_string(ms.count()).c_str());
  }
}

QSYS_DEFINE(qsys_sleep, "Sleep for a specified number of milliseconds") {
  QSYS_ARGASSERT(qsys_sleep, 1);
  QSYS_ARG_INT64(qsys_sleep, ms, 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  return strdup("");
}

QSYS_NOT_IMPLEMENTED(qsys_crypto_random);
QSYS_NOT_IMPLEMENTED(qsys_hash);
QSYS_NOT_IMPLEMENTED(qsys_insmod);
QSYS_NOT_IMPLEMENTED(qsys_rmmod);
QSYS_NOT_IMPLEMENTED(qsys_ioctl);
