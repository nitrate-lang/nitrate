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

#define LIBNITRATE_INTERNAL

#include <nitrate-core/Lib.h>
#include <nitrate/code.h>

#include <SerialUtil.hh>
#include <functional>
#include <nitrate-core/Classes.hh>
#include <nitrate-ir/Classes.hh>
#include <string_view>
#include <unordered_set>

static bool impl_use_json(qmodule_t *R, FILE *O) {
  /// TODO: Do correct JSON serialization

  (void)R;
  (void)O;

  return true;
}

static bool impl_use_msgpack(qmodule_t *R, FILE *O) {
  /// TODO: Do correct MsgPack serialization

  return impl_use_json(R, O);
}

bool impl_subsys_qxir(std::shared_ptr<std::istream> source, FILE *output,
                      std::function<void(const char *)> diag_cb,
                      const std::unordered_set<std::string_view> &opts) {
  enum class OutMode {
    JSON,
    MsgPack,
  } out_mode = OutMode::JSON;

  if (opts.contains("-fuse-json") && opts.contains("-fuse-msgpack")) {
    qcore_print(QCORE_ERROR, "Cannot use both JSON and MsgPack output.");
    return false;
  } else if (opts.contains("-fuse-msgpack")) {
    out_mode = OutMode::MsgPack;
  }

  qxir_conf conf;

  { /* Should the ir use the crashguard signal handler? */
    if (opts.contains("-fir-crashguard=off")) {
      qxir_conf_setopt(conf.get(), QQK_CRASHGUARD, QQV_OFF);
    } else if (opts.contains("-fparse-crashguard=on")) {
      qxir_conf_setopt(conf.get(), QQK_CRASHGUARD, QQV_ON);
    }
  }

  (void)source;

  qmodule ir_module(nullptr, conf.get(), nullptr);

  bool ok = qxir_lower(ir_module.get(), nullptr, true);
  if (!ok) {
    diag_cb("Failed to lower IR module.\n");
    return false;
  }

  switch (out_mode) {
    case OutMode::JSON:
      ok = impl_use_json(ir_module.get(), output);
      break;
    case OutMode::MsgPack:
      ok = impl_use_msgpack(ir_module.get(), output);
      break;
  }

  return ok;
}
