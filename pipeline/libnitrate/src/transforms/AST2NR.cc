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

#define LIBNITRATE_INTERNAL

#include <nitrate-core/Lib.h>
#include <nitrate/code.h>

#include <core/SerialUtil.hh>
#include <core/Transformer.hh>
#include <functional>
#include <nitrate-core/Classes.hh>
#include <nitrate-ir/Classes.hh>
#include <nitrate-ir/Writer.hh>
#include <nitrate-parser/ASTReader.hh>
#include <string_view>
#include <unordered_set>

bool nit::nr(std::istream &source, std::ostream &output,
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

  std::optional<npar_node_t *> root;

  if (source.peek() == '{') {
    root = npar::AST_JsonReader(source).get();
  } else {
    root = npar::AST_MsgPackReader(source).get();
  }

  if (!root.has_value()) {
    qcore_print(QCORE_ERROR, "Failed to parse input.");
    return false;
  }

  qmodule ir_module;

  bool ok =
      nr_lower(&ir_module.get(), root.value(), nullptr, diag_cb != nullptr);
  if (!ok) {
    diag_cb("Failed to lower IR module.\n");
    return false;
  }

  switch (out_mode) {
    case OutMode::JSON: {
      auto writter = nr::NR_JsonWriter(output);
      ir_module.get()->accept(writter);
      return false;
    }

    case OutMode::MsgPack: {
      auto writter = nr::NR_MsgPackWriter(output);
      ir_module.get()->accept(writter);
      return false;
    }
  }
}
