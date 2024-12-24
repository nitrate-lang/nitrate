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

ExpressionList Parser::recurse_struct_attributes() {
  ExpressionList attributes;

  if (!next_if(PuncLBrk)) {
    return attributes;
  }

  while (true) {
    if (next_if(EofF)) {
      diagnostic
          << current()
          << "Encountered EOF while parsing composite definition attributes";
      break;
    }

    if (next_if(PuncRBrk)) {
      break;
    }

    auto attribute =
        recurse_expr({Token(Punc, PuncComa), Token(Punc, PuncRBrk)});

    attributes.push_back(attribute);

    next_if(PuncComa);
  }

  return attributes;
}

string Parser::recurse_struct_name() {
  if (auto tok = next_if(Name)) {
    return tok->as_string();
  } else {
    return "";
  }
}

StructDefNames Parser::recurse_struct_terms() {
  StructDefNames names;

  if (!next_if(PuncColn)) {
    return names;
  }

  while (true) {
    if (auto tok = next_if(Name)) {
      names.push_back(tok->as_string());

      next_if(PuncComa);
    } else {
      return names;
    }
  }
}

NullableFlowPtr<Expr> Parser::recurse_struct_field_default_value() {
  if (next_if(OpSet)) {
    return recurse_expr(
        {Token(Punc, PuncComa), Token(Punc, PuncSemi), Token(Punc, PuncRCur)});
  } else {
    return std::nullopt;
  }
}

void Parser::recurse_struct_field(Vis vis, bool is_static,
                                  StructDefFields &fields) {
  /* Must consume token to avoid infinite loop on error */
  if (auto name = next(); name.is(Name)) {
    auto field_name = name.as_string();

    if (next_if(PuncColn)) {
      auto field_type = recurse_type();
      auto default_value = recurse_struct_field_default_value();

      auto field =
          StructField(vis, is_static, field_name, field_type, default_value);

      fields.push_back(std::move(field));
    } else {
      diagnostic << current() << "Expected ':' after field name in struct";
    }
  } else {
    diagnostic << current() << "Expected field name in struct";
  }
}

void Parser::recurse_struct_method_or_field(StructContent &body) {
  Vis vis = Vis::Sec;

  /* Parse visibility of member */
  if (next_if(Sec)) {
    vis = Vis::Sec;
  } else if (next_if(Pro)) {
    vis = Vis::Pro;
  } else if (next_if(Pub)) {
    vis = Vis::Pub;
  }

  /* Is the member static? */
  bool is_static = next_if(Static).has_value();

  if (next_if(Fn)) { /* Parse method */
    auto method = recurse_function(false);

    if (is_static) {
      body.static_methods.push_back({vis, method});
    } else {
      body.methods.push_back({vis, method});
    }
  } else { /* Parse field */
    recurse_struct_field(vis, is_static, body.fields);
  }

  next_if(PuncComa) || next_if(PuncSemi);
}

Parser::StructContent Parser::recurse_struct_body() {
  StructContent body;

  if (!next_if(PuncLCur)) {
    diagnostic << current() << "Expected '{' to start struct body";
    return body;
  }

  while (true) {
    if (next_if(PuncRCur)) {
      break;
    }

    if (next_if(EofF)) {
      diagnostic << current() << "Encountered EOF while parsing struct body";
      break;
    }

    recurse_struct_method_or_field(body);
  }

  return body;
}

FlowPtr<Stmt> Parser::recurse_struct(CompositeType type) {
  auto start_pos = current().get_start();
  auto attributes = recurse_struct_attributes();
  auto name = recurse_struct_name();
  auto template_params = recurse_template_parameters();
  auto terms = recurse_struct_terms();
  auto [fields, methods, static_methods] = recurse_struct_body();

  auto struct_defintion =
      make<StructDef>(type, attributes, name, template_params, terms, fields,
                      methods, static_methods)();

  struct_defintion->set_offset(start_pos);

  return struct_defintion;
}
