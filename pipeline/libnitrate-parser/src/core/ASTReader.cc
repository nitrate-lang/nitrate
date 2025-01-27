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

#define AST_READER_IMPL

#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/AST.hh>
#include <nitrate-parser/ASTBase.hh>
#include <nitrate-parser/ASTReader.hh>

#include "core/SyntaxTree.pb.h"

using namespace ncc;
using namespace ncc::parse;
using namespace nitrate::parser::SyntaxTree;

AstReader::AstReader(std::istream& protobuf_data,
                     ReaderSourceManager source_manager) {
  Root root;
  if (!root.ParseFromIstream(&protobuf_data)) [[unlikely]] {
    return;
  }

  /// TODO: Transform AST structural representation to AST object representation
  qcore_implement();
  (void)source_manager;
}

AstReader::AstReader(std::string_view protobuf_data,
                     ReaderSourceManager source_manager) {
  Root root;
  if (!root.ParseFromArray(protobuf_data.data(), protobuf_data.size()))
      [[unlikely]] {
    return;
  }

  /// TODO: Transform AST structural representation to AST object representation
  qcore_implement();
  (void)source_manager;
}

auto AstReader::Get() -> std::optional<FlowPtr<Base>> {
  return m_root.has_value() ? std::make_optional(m_root.value()) : std::nullopt;
}
