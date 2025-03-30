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

#include <descent/Recurse.hh>
#include <nitrate-parser/ASTStmt.hh>

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

struct StructContent {
  std::vector<StructField> m_fields;
  std::vector<StructFunction> m_methods;
};

static auto RecurseStructTerms(GeneralParser::Context &m) -> std::vector<string> {
  std::vector<string> names;

  if (!m.NextIf<PuncColn>()) {
    return names;
  }

  bool enclosed = m.NextIf<PuncLBrk>().has_value();

  if (enclosed) {
    while (true) {
      if (m.IsEof()) [[unlikely]] {
        Log << ParserSignal << m.Current() << "Encountered EOF while parsing struct attributes";
        break;
      } else if (m.NextIf<PuncRBrk>()) {
        break;
      }

      if (auto name = m.RecurseName()) {
        names.push_back(name);
      } else {
        Log << ParserSignal << m.Next() << "Expected identifier in struct terms";
        break;
      }

      if (m.NextIf<PuncComa>()) {
        continue;
      }

      if (m.NextIf<PuncRBrk>()) {
        break;
      }

      Log << ParserSignal << m.Next() << "Expected ',' or ']' in struct terms";
      break;
    }
  } else {
    while (true) {
      if (m.IsEof()) [[unlikely]] {
        Log << ParserSignal << m.Current() << "Encountered EOF while parsing struct attributes";
        break;
      }

      if (auto name = m.RecurseName()) [[likely]] {
        names.push_back(name);
      } else {
        Log << ParserSignal << m.Next() << "Expected identifier in struct terms";
        break;
      }

      if (!m.NextIf<PuncComa>()) {
        break;
      }
    }
  }

  return names;
}

static auto RecurseStructFieldDefaultValue(GeneralParser::Context &m) -> NullableFlowPtr<Expr> {
  if (m.NextIf<OpSet>()) {
    return m.RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncSemi),
        Token(Punc, PuncRCur),
    });
  }

  return std::nullopt;
}

static void RecurseStructField(GeneralParser::Context &m, Vis vis, bool is_static, std::vector<StructField> &fields) {
  if (auto field_name = m.RecurseName()) {
    if (m.NextIf<PuncColn>()) {
      auto field_type = m.RecurseType();
      auto default_value = RecurseStructFieldDefaultValue(m);

      auto field = StructField(vis, is_static, field_name, field_type, default_value);

      fields.push_back(std::move(field));
    } else {
      Log << ParserSignal << m.Current() << "Expected ':' after field name in struct";
    }
  } else {
    Log << ParserSignal << m.Next() << "Expected field name in struct";
  }
}

static void RecurseStructMethodOrField(GeneralParser::Context &m, StructContent &body) {
  Vis vis = Vis::Sec;

  /* Parse visibility of member */
  if (m.NextIf<Sec>()) {
    vis = Vis::Sec;
  } else if (m.NextIf<Pro>()) {
    vis = Vis::Pro;
  } else if (m.NextIf<Pub>()) {
    vis = Vis::Pub;
  }

  auto is_static_member = m.NextIf<Static>().has_value();

  if (m.NextIf<Fn>()) {
    if (is_static_member) {
      Log << ParserSignal << m.Current() << "Static member function is not allowed";
    }

    auto method = m.RecurseFunction(false);
    body.m_methods.emplace_back(vis, method);
  } else {
    RecurseStructField(m, vis, is_static_member, body.m_fields);
  }

  m.NextIf<PuncComa>() || m.NextIf<PuncSemi>();
}

static auto RecurseStructBody(GeneralParser::Context &m) -> StructContent {
  StructContent body;

  if (m.NextIf<PuncSemi>()) {
    return body;
  }

  if (!m.NextIf<PuncLCur>()) [[unlikely]] {
    Log << ParserSignal << m.Current() << "Expected '{' to start struct body";

    return body;
  }

  while (true) {
    if (m.IsEof()) {
      Log << ParserSignal << m.Current() << "Encountered EOF while parsing struct body";
      break;
    }

    if (m.NextIf<PuncRCur>()) {
      break;
    }

    RecurseStructMethodOrField(m, body);
  }

  return body;
}

auto GeneralParser::Context::RecurseStruct(CompositeType struct_type) -> FlowPtr<Expr> {
  auto start_pos = Current().GetStart();
  auto struct_attributes = RecurseAttributes("struct");
  auto struct_name = RecurseName();
  auto struct_template_params = RecurseTemplateParameters();
  auto struct_terms = RecurseStructTerms(m);
  auto [struct_fields, struct_methods] = RecurseStructBody(m);

  auto struct_defintion = CreateStruct(struct_type, struct_name, struct_template_params, struct_fields, struct_methods,
                                       struct_terms, struct_attributes);
  struct_defintion->SetOffset(start_pos);

  return struct_defintion;
}
