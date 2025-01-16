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

ExpressionList Parser::PImpl::RecurseStructAttributes() {
  ExpressionList attributes;

  if (!next_if(PuncLBrk)) {
    return attributes;
  }

  while (true) {
    if (next_if(EofF)) [[unlikely]] {
      Log << SyntaxError << current()
          << "Encountered EOF while parsing struct attributes";
      break;
    }

    if (next_if(PuncRBrk)) {
      break;
    }

    auto attribute = RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncRBrk),
    });

    attributes.push_back(attribute);

    next_if(PuncComa);
  }

  return attributes;
}

string Parser::PImpl::RecurseStructName() {
  auto tok = next_if(Name);
  return tok ? tok->GetString() : "";
}

StructDefNames Parser::PImpl::RecurseStructTerms() {
  StructDefNames names;

  if (!next_if(PuncColn)) {
    return names;
  }

  while (true) {
    if (auto tok = next_if(Name)) {
      names.push_back(tok->GetString());
    } else {
      break;
    }

    next_if(PuncComa);
  }

  return names;
}

NullableFlowPtr<Expr> Parser::PImpl::RecurseStructFieldDefaultValue() {
  if (next_if(OpSet)) {
    return RecurseExpr({
        Token(Punc, PuncComa),
        Token(Punc, PuncSemi),
        Token(Punc, PuncRCur),
    });
  } else {
    return std::nullopt;
  }
}

void Parser::PImpl::RecurseStructField(Vis vis, bool is_static,
                                       StructDefFields &fields) {
  if (auto field_name = next_if(Name)) {
    if (next_if(PuncColn)) {
      auto field_type = RecurseType();
      auto default_value = RecurseStructFieldDefaultValue();

      auto field = StructField(vis, is_static, field_name->GetString(),
                               field_type, default_value);

      fields.push_back(std::move(field));
    } else {
      Log << SyntaxError << current()
          << "Expected ':' after field name in struct";
    }
  } else {
    Log << SyntaxError << next() << "Expected field name in struct";
  }
}

void Parser::PImpl::RecurseStructMethodOrField(StructContent &body) {
  Vis vis = Vis::Sec;

  /* Parse visibility of member */
  if (next_if(Sec)) {
    vis = Vis::Sec;
  } else if (next_if(Pro)) {
    vis = Vis::Pro;
  } else if (next_if(Pub)) {
    vis = Vis::Pub;
  }

  auto is_static_member = next_if(Static).has_value();

  if (next_if(Fn)) {
    auto method = RecurseFunction(false);

    if (is_static_member) {
      body.m_static_methods.push_back({vis, method});
    } else {
      body.m_methods.push_back({vis, method});
    }
  } else {
    RecurseStructField(vis, is_static_member, body.m_fields);
  }

  next_if(PuncComa) || next_if(PuncSemi);
}

Parser::PImpl::StructContent Parser::PImpl::RecurseStructBody() {
  StructContent body;

  if (!next_if(PuncLCur)) [[unlikely]] {
    Log << SyntaxError << current() << "Expected '{' to start struct body";
    return body;
  }

  while (true) {
    if (next_if(EofF)) {
      Log << SyntaxError << current()
          << "Encountered EOF while parsing struct body";
      break;
    }

    if (next_if(PuncRCur)) {
      break;
    }

    RecurseStructMethodOrField(body);
  }

  return body;
}

FlowPtr<Stmt> Parser::PImpl::RecurseStruct(CompositeType struct_type) {
  auto start_pos = current().GetStart();
  auto struct_attributes = RecurseStructAttributes();
  auto struct_name = RecurseStructName();
  auto struct_template_params = RecurseTemplateParameters();
  auto struct_terms = RecurseStructTerms();
  auto [struct_fields, struct_methods, struct_static_methods] =
      RecurseStructBody();

  auto struct_defintion = make<StructDef>(
      struct_type, struct_attributes, struct_name, struct_template_params,
      struct_terms, struct_fields, struct_methods, struct_static_methods)();
  struct_defintion->SetOffset(start_pos);

  return struct_defintion;
}
