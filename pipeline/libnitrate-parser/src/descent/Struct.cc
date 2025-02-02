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

using namespace ncc;
using namespace ncc::lex;
using namespace ncc::parse;

auto Parser::PImpl::RecurseStructAttributes() -> ExpressionList {
  ExpressionList attributes;

  if (!NextIf(PuncLBrk)) {
    return attributes;
  }

  while (true) {
    if (NextIf(EofF)) [[unlikely]] {
      Log << SyntaxError << current() << "Encountered EOF while parsing struct attributes";
      break;
    }

    if (NextIf(PuncRBrk)) {
      break;
    }

    auto attribute = RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncRBrk),
    });

    attributes.push_back(attribute);

    NextIf(PuncComa);
  }

  return attributes;
}

auto Parser::PImpl::RecurseStructTerms() -> StructNames {
  StructNames names;

  if (!NextIf(PuncColn)) {
    return names;
  }

  bool enclosed = NextIf(PuncLBrk).has_value();

  if (enclosed) {
    while (true) {
      if (NextIf(EofF)) [[unlikely]] {
        Log << SyntaxError << current() << "Encountered EOF while parsing struct attributes";
        break;
      } else if (NextIf(PuncRBrk)) {
        break;
      }

      if (auto name = RecurseName(); !name.empty()) {
        names.push_back(name);
      } else {
        Log << SyntaxError << next() << "Expected identifier in struct terms";
        break;
      }

      if (NextIf(PuncComa)) {
        continue;
      }

      if (NextIf(PuncRBrk)) {
        break;
      }

      Log << SyntaxError << next() << "Expected ',' or ']' in struct terms";
      break;
    }
  } else {
    while (true) {
      if (NextIf(EofF)) [[unlikely]] {
        Log << SyntaxError << current() << "Encountered EOF while parsing struct attributes";
        break;
      }

      if (auto name = RecurseName(); !name.empty()) [[likely]] {
        names.push_back(name);
      } else {
        Log << SyntaxError << next() << "Expected identifier in struct terms";
        break;
      }

      if (!NextIf(PuncComa)) {
        break;
      }
    }
  }

  return names;
}

auto Parser::PImpl::RecurseStructFieldDefaultValue() -> NullableFlowPtr<Expr> {
  if (NextIf(OpSet)) {
    return RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncSemi),
        Token(Punc, PuncRCur),
    });
  }

  return std::nullopt;
}

void Parser::PImpl::RecurseStructField(Vis vis, bool is_static, StructFields &fields) {
  if (auto field_name = RecurseName(); !field_name.empty()) {
    if (NextIf(PuncColn)) {
      auto field_type = RecurseType();
      auto default_value = RecurseStructFieldDefaultValue();

      auto field = StructField(vis, is_static, field_name, field_type, default_value);

      fields.push_back(std::move(field));
    } else {
      Log << SyntaxError << current() << "Expected ':' after field name in struct";
    }
  } else {
    Log << SyntaxError << next() << "Expected field name in struct";
  }
}

void Parser::PImpl::RecurseStructMethodOrField(StructContent &body) {
  Vis vis = Vis::Sec;

  /* Parse visibility of member */
  if (NextIf(Sec)) {
    vis = Vis::Sec;
  } else if (NextIf(Pro)) {
    vis = Vis::Pro;
  } else if (NextIf(Pub)) {
    vis = Vis::Pub;
  }

  auto is_static_member = NextIf(Static).has_value();

  if (NextIf(Fn)) {
    auto method = RecurseFunction(false);

    if (is_static_member) {
      body.m_static_methods.emplace_back(vis, method);
    } else {
      body.m_methods.emplace_back(vis, method);
    }
  } else {
    RecurseStructField(vis, is_static_member, body.m_fields);
  }

  NextIf(PuncComa) || NextIf(PuncSemi);
}

auto Parser::PImpl::RecurseStructBody() -> Parser::PImpl::StructContent {
  StructContent body;

  if (NextIf(PuncSemi)) {
    return body;
  }

  if (!NextIf(PuncLCur)) [[unlikely]] {
    Log << SyntaxError << current() << "Expected '{' to start struct body";

    return body;
  }

  while (true) {
    if (NextIf(EofF)) {
      Log << SyntaxError << current() << "Encountered EOF while parsing struct body";
      break;
    }

    if (NextIf(PuncRCur)) {
      break;
    }

    RecurseStructMethodOrField(body);
  }

  return body;
}

auto Parser::PImpl::RecurseStruct(CompositeType struct_type) -> FlowPtr<Stmt> {
  auto start_pos = current().GetStart();
  auto struct_attributes = RecurseStructAttributes();
  auto struct_name = RecurseName();
  auto struct_template_params = RecurseTemplateParameters();
  auto struct_terms = RecurseStructTerms();
  auto [struct_fields, struct_methods, struct_static_methods] = RecurseStructBody();

  auto struct_defintion = CreateNode<Struct>(struct_type, struct_attributes, struct_name, struct_template_params,
                                             struct_terms, struct_fields, struct_methods, struct_static_methods)();
  struct_defintion->SetOffset(start_pos);

  return struct_defintion;
}
