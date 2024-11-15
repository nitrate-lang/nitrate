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

#ifndef __NITRATE_QXIR_PASSES_LIST_H__
#define __NITRATE_QXIR_PASSES_LIST_H__

struct qmodule_t;

#include <nitrate-ir/Report.hh>

namespace nr::pass {
#define SEMANTIC_PASS(name) bool name(qmodule_t *mod, IReport *log);
#define TRANSFORM_PASS(name) bool name(qmodule_t *mod, IReport *log)
#define OPTIMIZE_PASS(name) bool name(qmodule_t *mod, IReport *log)

  TRANSFORM_PASS(ds_acyclic);
  TRANSFORM_PASS(ds_nullchk);
  TRANSFORM_PASS(ds_resolv);
  TRANSFORM_PASS(ds_flatten);
  TRANSFORM_PASS(ds_tyinfer);
  TRANSFORM_PASS(ds_mangle);
  TRANSFORM_PASS(ds_verify);
  TRANSFORM_PASS(ds_clean);
  TRANSFORM_PASS(ds_raii);

  SEMANTIC_PASS(chk_missing_return);
  SEMANTIC_PASS(chk_bad_cast);

#undef OPTIMIZE_PASS
#undef TRANSFORM_PASS
#undef SEMANTIC_PASS

}  // namespace nr::pass

#endif  // __NITRATE_QXIR_PASSES_LIST_H__
