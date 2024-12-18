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

using namespace ncc::parse;

struct StructContent {
  StructDefFields fields;
  StructDefMethods methods;
  StructDefStaticMethods static_methods;
};

extern std::optional<TemplateParameters> recurse_template_parameters(
    npar_t &S, qlex_t &rd);

static ExpressionList recurse_struct_attributes(npar_t &S, qlex_t &rd) {
  ExpressionList attributes;

  if (!next_if(qPuncLBrk)) {
    return attributes;
  }

  while (true) {
    if (next_if(qEofF)) {
      diagnostic
          << current()
          << "Encountered EOF while parsing composite definition attributes";
      break;
    }

    if (next_if(qPuncRBrk)) {
      break;
    }

    let attribute = recurse_expr(
        S, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRBrk)});

    attributes.push_back(attribute);

    next_if(qPuncComa);
  }

  return attributes;
}

static std::string_view recurse_struct_name(qlex_t &rd) {
  if (let tok = next_if(qName)) {
    return tok->as_string(&rd);
  } else {
    return "";
  }
}

static StructDefNames recurse_struct_terms(qlex_t &rd) {
  StructDefNames names;

  if (!next_if(qPuncColn)) {
    return names;
  }

  while (true) {
    if (let tok = next_if(qName)) {
      names.push_back(SaveString(tok->as_string(&rd)));

      next_if(qPuncComa);
    } else {
      return names;
    }
  }
}

static std::optional<Expr *> recurse_struct_field_default_value(npar_t &S,
                                                                qlex_t &rd) {
  if (next_if(qOpSet)) {
    return recurse_expr(
        S, rd,
        {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncSemi),
         qlex_tok_t(qPunc, qPuncRCur)});
  } else {
    return std::nullopt;
  }
}

static void recurse_struct_field(npar_t &S, qlex_t &rd, Vis vis, bool is_static,
                                 StructDefFields &fields) {
  /* Must consume token to avoid infinite loop on error */
  if (let name = next(); name.is(qName)) {
    let field_name = name.as_string(&rd);

    if (next_if(qPuncColn)) {
      let field_type = recurse_type(S, rd);
      let default_value = recurse_struct_field_default_value(S, rd);

      let field = StructField(vis, is_static, SaveString(field_name),
                              field_type, std::move(default_value));

      fields.push_back(std::move(field));
    } else {
      diagnostic << current() << "Expected ':' after field name in struct";
    }
  } else {
    diagnostic << current() << "Expected field name in struct";
  }
}

static void recurse_struct_method_or_field(npar_t &S, qlex_t &rd,
                                           StructContent &body) {
  Vis vis = Vis::Sec;

  /* Parse visibility of member */
  if (next_if(qKSec)) {
    vis = Vis::Sec;
  } else if (next_if(qKPro)) {
    vis = Vis::Pro;
  } else if (next_if(qKPub)) {
    vis = Vis::Pub;
  }

  /* Is the member static? */
  bool is_static = next_if(qKStatic).has_value();

  if (next_if(qKFn)) { /* Parse method */
    let method = recurse_function(S, rd, false);

    if (is_static) {
      body.static_methods.push_back({vis, method});
    } else {
      body.methods.push_back({vis, method});
    }
  } else { /* Parse field */
    recurse_struct_field(S, rd, vis, is_static, body.fields);
  }

  next_if(qPuncComa) || next_if(qPuncSemi);
}

static StructContent recurse_struct_body(npar_t &S, qlex_t &rd) {
  StructContent body;

  if (!next_if(qPuncLCur)) {
    diagnostic << current() << "Expected '{' to start struct body";
    return body;
  }

  while (true) {
    if (next_if(qPuncRCur)) {
      break;
    }

    if (next_if(qEofF)) {
      diagnostic << current() << "Encountered EOF while parsing struct body";
      break;
    }

    recurse_struct_method_or_field(S, rd, body);
  }

  return body;
}

Stmt *ncc::parse::recurse_struct(npar_t &S, qlex_t &rd, CompositeType type) {
  let start_pos = current().start;
  let attributes = recurse_struct_attributes(S, rd);
  let name = recurse_struct_name(rd);
  let template_params = recurse_template_parameters(S, rd);
  let terms = recurse_struct_terms(rd);
  let[fields, methods, static_methods] = recurse_struct_body(S, rd);

  let struct_defintion = make<StructDef>(
      type, std::move(attributes), SaveString(name), std::move(template_params),
      std::move(terms), std::move(fields), std::move(methods),
      std::move(static_methods));

  struct_defintion->set_offset(start_pos);

  return struct_defintion;
}
