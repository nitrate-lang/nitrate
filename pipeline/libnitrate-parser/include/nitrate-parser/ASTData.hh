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

#ifndef __NITRATE_AST_sSTRUCTURAL_DATA_H__
#define __NITRATE_AST_sSTRUCTURAL_DATA_H__

#include <memory>
#include <nitrate-core/Allocate.hh>
#include <nitrate-core/FlowPtr.hh>
#include <nitrate-core/NullableFlowPtr.hh>
#include <nitrate-core/String.hh>
#include <nitrate-parser/ASTFwd.hh>
#include <variant>
#include <vector>

namespace ncc::parse {
  enum class Vis : uint8_t {
    Pub = 0,
    Sec = 1,
    Pro = 2,
  };

  enum class ImportMode : uint8_t {
    Code,
    String,
  };

  using CallArg = std::pair<string, FlowPtr<Expr>>;
  using TemplateParameter = std::tuple<string, FlowPtr<Type>, NullableFlowPtr<Expr>>;
  using FuncParam = std::tuple<string, FlowPtr<Type>, NullableFlowPtr<Expr>>;

  class StructField {
    string m_name;
    NullableFlowPtr<Expr> m_value;
    FlowPtr<Type> m_type;
    Vis m_vis;
    bool m_is_static;

  public:
    StructField(Vis vis, bool is_static, string name, FlowPtr<Type> type, NullableFlowPtr<Expr> value)
        : m_name(name), m_value(std::move(value)), m_type(std::move(type)), m_vis(vis), m_is_static(is_static) {}

    [[nodiscard]] auto GetVis() const { return m_vis; }
    [[nodiscard]] auto IsStatic() const { return m_is_static; }
    [[nodiscard]] auto GetName() const { return m_name; }
    [[nodiscard]] auto GetType() const { return m_type; }
    [[nodiscard]] auto GetValue() const { return m_value; }
  };

  struct StructFunction {
    Vis m_vis;
    FlowPtr<Function> m_func;

    StructFunction(Vis vis, FlowPtr<Function> func) : m_vis(vis), m_func(std::move(func)) {}
  };

}  // namespace ncc::parse

#endif
