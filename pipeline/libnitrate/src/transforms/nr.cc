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

#include <nitrate/code.h>

#include <core/SerialUtil.hh>
#include <core/Transform.hh>
#include <nitrate-core/Init.hh>
#include <nitrate-ir/Classes.hh>
#include <nitrate-ir/IRWriter.hh>
#include <nitrate-parser/ASTReader.hh>
#include <unordered_set>

using namespace ncc::ir;

CREATE_TRANSFORM(nit::nr) {
  (void)env;

  enum class OutMode {
    JSON,
    MsgPack,
  } out_mode = OutMode::JSON;

  if (opts.contains("-fuse-json") && opts.contains("-fuse-msgpack")) {
    qcore_logf(QCORE_ERROR, "Cannot use both JSON and MsgPack output.");
    return false;
  } else if (opts.contains("-fuse-msgpack")) {
    out_mode = OutMode::MsgPack;
  }

  std::optional<ncc::parse::Base *> root;

  if (source.peek() == '{') {
    root = ncc::parse::AST_JsonReader(source).get();
  } else {
    root = ncc::parse::AST_MsgPackReader(source).get();
  }

  if (!root.has_value()) {
    qcore_logf(QCORE_ERROR, "Failed to parse input.");
    return false;
  }

  if (auto module = nr_lower(root.value(), nullptr, true)) {
    switch (out_mode) {
      case OutMode::JSON: {
        auto writter = IR_JsonWriter(output);
        module->accept(writter);
        return false;
      }

      case OutMode::MsgPack: {
        auto writter = IR_MsgPackWriter(output);
        module->accept(writter);
        return false;
      }
    }
  } else {
    qcore_print(QCORE_ERROR, "Failed to lower IR module.\n");
    return false;
  }
}
