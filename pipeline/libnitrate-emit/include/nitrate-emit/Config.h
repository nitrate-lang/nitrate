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

#ifndef __NITRATE_CODEGEN_CONFIG_H__
#define __NITRATE_CODEGEN_CONFIG_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/// TODO: Replace with c++ interface

#if defined(__cplusplus) && defined(__NITRATE_CODEGEN_IMPL__)
#include <initializer_list>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QCodegenConfig QCodegenConfig;

enum QcodeKeyT {
  QCK_UNKNOWN = 0,
  QCK_CRASHGUARD,
  QCV_FASTERROR,
};

enum QcodeValT {
  QCV_UNKNOWN = 0,
  QCV_TRUE,
  QCV_FALSE,
  QCV_ON = QCV_TRUE,
  QCV_OFF = QCV_FALSE,
};

///==========================================================================///

#if defined(__cplusplus) && defined(__NITRATE_CODEGEN_IMPL__)
}
typedef struct QcodeSettingT {
  QcodeKeyT m_key;
  QcodeValT m_value;

#if defined(__cplusplus) && defined(__NITRATE_CODEGEN_IMPL__)
  constexpr QcodeSettingT(const std::initializer_list<int> &list)
      : m_key(static_cast<QcodeKeyT>(list.begin()[0])),
        m_value(static_cast<QcodeValT>(list.begin()[1])) {}
#endif
} QcodeSettingT;

extern "C" {
#else

struct QcodeSettingT {
  QcodeKeyT m_key;
  QcodeValT m_value;
};

#endif

QCodegenConfig *QcodeConfNew(bool use_defaults);
void QcodeConfFree(QCodegenConfig *conf);
bool QcodeConfSetopt(QCodegenConfig *conf, QcodeKeyT key, QcodeValT value);
bool QcodeConfGetopt(QCodegenConfig *conf, QcodeKeyT key, QcodeValT *value);
QcodeSettingT *QcodeConfGetopts(QCodegenConfig *conf, size_t *count);
void QcodeConfClear(QCodegenConfig *conf);

#ifdef __cplusplus
}
#endif

#endif  // __NITRATE_CODEGEN_CONFIG_H__
