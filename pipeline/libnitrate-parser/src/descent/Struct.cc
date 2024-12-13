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

/// TODO: Cleanup this code; it's a mess from refactoring.

#include <nitrate-lexer/Lexer.h>

#include <descent/Recurse.hh>

using namespace npar;

static ExpressionList recurse_struct_attributes(npar_t &S, qlex_t &rd) {
  ExpressionList attributes;

  if (!next_if(qPuncLBrk)) {
    return attributes;
  }

  while (true) {
    let tok = peek();

    if (!tok.is(qEofF)) {
      if (next_if(qPuncRBrk)) {
        return attributes;
      }

      let attribute = recurse_expr(
          S, rd, {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncRBrk)});

      attributes.push_back(attribute);

      next_if(qPuncComa);
    } else {
      diagnostic
          << tok
          << "Encountered EOF while parsing composite definition attributes";
      return attributes;
    }
  }
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

static std::tuple<StructDefFields, StructDefMethods, StructDefStaticMethods>
recurse_struct_body(npar_t &S, qlex_t &rd) {
  /// TODO: Implement
  // qcore_implement();

  return {{}, {}, {}};
}

extern bool recurse_template_parameters(
    npar_t &S, qlex_t &rd, std::optional<TemplateParameters> &template_params);

npar::Stmt *npar::recurse_struct(npar_t &S, qlex_t &rd, CompositeType type) {
  let start_pos = current().start;

  let attributes = recurse_struct_attributes(S, rd);
  let name = recurse_struct_name(rd);

  std::optional<TemplateParameters> template_params;
  if (!recurse_template_parameters(S, rd, template_params)) {
    diagnostic << current() << "Failed to parse template parameters";
  }

  let terms = recurse_struct_terms(rd);

  let[fields, methods, static_methods] = recurse_struct_body(S, rd);

  let struct_defintion = make<StructDef>(
      type, std::move(attributes), SaveString(name), std::move(template_params),
      std::move(terms), std::move(fields), std::move(methods),
      std::move(static_methods));
  struct_defintion->set_offset(start_pos);

  return struct_defintion;
}
