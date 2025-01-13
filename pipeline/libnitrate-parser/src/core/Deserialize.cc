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

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <functional>
#include <iostream>
#include <nitrate-core/Logger.hh>
#include <nitrate-core/Macro.hh>
#include <nitrate-parser/ASTReader.hh>

using namespace ncc::parse;

void AST_JsonReader::parse_stream(std::istream& is) {
  rapidjson::Document doc;
  rapidjson::IStreamWrapper wrapper(is);
  doc.ParseStream(wrapper);

  std::function<bool(const rapidjson::Value& obj)> handle_value =
      [&](const rapidjson::Value& obj) -> bool {
    switch (obj.GetType()) {
      case rapidjson::kNullType: {
        null();
        return true;
      }

      case rapidjson::kFalseType: {
        boolean(false);
        return true;
      }

      case rapidjson::kTrueType: {
        boolean(true);
        return true;
      }

      case rapidjson::kObjectType: {
        begin_obj(obj.MemberCount());
        for (let member : obj.GetObject()) {
          if (member.name.IsString()) {
            str(std::string_view(member.name.GetString(),
                                 member.name.GetStringLength()));
          } else {
            return false;
          }
          handle_value(member.value);
        }
        end_obj();
        return true;
      }

      case rapidjson::kArrayType: {
        begin_arr(obj.Size());
        for (let elem : obj.GetArray()) {
          handle_value(elem);
        }
        end_arr();
        return true;
      }

      case rapidjson::kStringType: {
        str(std::string_view(obj.GetString(), obj.GetStringLength()));
        return true;
      }

      case rapidjson::kNumberType: {
        if (obj.IsUint64()) {
          uint(obj.GetUint64());
          return true;
        } else {
          return false;
        }
      }
    }
  };

  if (!doc.HasParseError()) {
    handle_value(doc);
  }
}

///===========================================================================///

void AST_MsgPackReader::parse_stream(std::istream& is) {
  (void)is;
  /// TODO: Implement MsgPack parsing
  qcore_implement();
}
