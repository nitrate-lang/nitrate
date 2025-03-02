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
#include <nitrate-ir/IR.hh>
#include <nitrate-ir/Module.hh>
#include <nitrate-ir/ToJson.hh>
#include <nitrate-ir/ToMsgPack.hh>
#include <nitrate-parser/ASTReader.hh>
#include <unordered_set>

using namespace ncc;
using namespace ncc::ir;

CREATE_TRANSFORM(nit::nr) {
  (void)env;

  enum class OutMode {
    JSON,
    MsgPack,
  } out_mode = OutMode::JSON;

  if (opts.contains("-fuse-json") && opts.contains("-fuse-msgpack")) {
    Log << "Cannot use both JSON and MsgPack output.";
    return false;
  }

  if (opts.contains("-fuse-msgpack")) {
    out_mode = OutMode::MsgPack;
  }

  std::string source_str(std::istreambuf_iterator<char>(source), {});

  auto pool = DynamicArena();
  auto root = ncc::parse::AstReader(source_str, pool).Get();
  if (!root.has_value()) {
    Log << "Failed to parse input.";
    return false;
  }

  if (auto module = NrLower(root.value().get(), nullptr, true)) {
    switch (out_mode) {
      case OutMode::JSON: {
        auto writter = IRJsonWriter(output);
        module->Accept(writter);
        return false;
      }

      case OutMode::MsgPack: {
        auto writter = IRMsgPackWriter(output);
        module->Accept(writter);
        return false;
      }
    }
  } else {
    Log << "Failed to lower IR module.";
    return false;
  }
}
