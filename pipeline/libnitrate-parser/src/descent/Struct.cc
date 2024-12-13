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

/// TODO: Source location

/// TODO: Cleanup this code; it's a mess from refactoring.

#include <nitrate-lexer/Lexer.h>
#include <nitrate-parser/Node.h>

#include <descent/Recurse.hh>

bool npar::recurse_attributes(npar_t &S, qlex_t &rd,
                              std::set<Expr *> &attributes) {
  qlex_tok_t tok = next();

  { /* The implementation list should be enclosed in square brackets ex: [abc,
       hello] */
    if (!tok.is<qPuncLBrk>()) {
      diagnostic << tok << "Expected '[' after 'impl' in definition";
    }
  }

  /* Parse an arbitrary number of attributes */
  while (true) {
    /* Check for termination */
    tok = peek();
    if (tok.is(qEofF)) {
      diagnostic << tok << "Unexpected end of file in definition";
      return false;
    }

    if (tok.is<qPuncRBrk>()) {
      next();
      break;
    }

    Expr *impl = recurse_expr(
        S, rd, {qlex_tok_t(qPunc, qPuncRBrk), qlex_tok_t(qPunc, qPuncComa)});

    attributes.insert(impl);

    /* Check for a comma */
    tok = peek();
    if (tok.is<qPuncComa>()) {
      next();
    }
  }

  return true;
}

npar::Stmt *npar::recurse_composite_field(npar_t &S, qlex_t &rd) {
  /*
   * Format: "name: type [= expr],"
   */

  std::string name;
  qlex_tok_t tok;
  Type *type = nullptr;
  Expr *value = nullptr;

  { /*First token is the field name */
    tok = next();
    if (!tok.is(qName)) {
      diagnostic << tok << "Expected field name in composite definition";
      return mock_stmt(QAST_STRUCT_FIELD);
    }
    name = tok.as_string(&rd);
  }

  { /* Next token should be a colon */
    tok = next();
    if (!tok.is<qPuncColn>()) {
      diagnostic << tok
                 << "Expected colon after field name in composite definition";
      return mock_stmt(QAST_STRUCT_FIELD);
    }
  }

  { /* Next section should be the field type */
    type = recurse_type(S, rd);
  }

  /* Check for a default value */
  tok = peek();
  if (tok.is<qPuncComa>() || tok.is<qPuncSemi>() || tok.is<qPuncRCur>()) {
    if (tok.is<qPuncComa>() || tok.is<qPuncSemi>()) {
      next();
    }
    auto R = make<StructField>(SaveString(name), type, nullptr, Vis::Sec);

    return R;
  }

  { /* Optional default value */
    if (!tok.is<qOpSet>()) {
      diagnostic
          << tok
          << "Expected '=' or ',' after field type in composite definition";
      return mock_stmt(QAST_STRUCT_FIELD);
    }
    next();

    /* Parse the default value */
    value = recurse_expr(
        S, rd,
        {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncSemi),
         qlex_tok_t(qPunc, qPuncRCur)});
  }

  auto R = make<StructField>(SaveString(name), type, value, Vis::Sec);

  return R;
}

bool recurse_template_parameters(
    npar_t &S, qlex_t &rd,
    std::optional<npar::TemplateParameters> &template_params);

npar::Stmt *npar::recurse_struct(npar_t &S, qlex_t &rd, CompositeType type) {
  qlex_tok_t tok;
  std::string name;
  StructDefFields fields;
  StructDefMethods methods;
  StructDefStaticMethods static_methods;
  Stmt *method = nullptr;
  FnDef *fdecl = nullptr;
  FuncTy *ft = nullptr;
  Stmt *field = nullptr;
  StructDef *sdef = make<StructDef>(SaveString(""));

  sdef->set_composite_type(type);

  { /* First token should be the name of the definition */
    tok = next();
    if (tok.is(qName)) {
      name = tok.as_string(&rd);
    } else {
      diagnostic << tok << "Expected struct name in struct definition";
      return mock_stmt(QAST_STRUCT);
    }
  }

  {
    if (!recurse_template_parameters(S, rd, sdef->get_template_params())) {
      return mock_stmt(QAST_STRUCT);
    }
  }

  { /* Next token should be an open curly bracket */
    tok = next();
    if (!tok.is<qPuncLCur>()) {
      diagnostic << tok
                 << "Expected '{' after struct name in struct definition";
      return mock_stmt(QAST_STRUCT);
    }
  }

  /* Parse the fields and methods */
  while (true) {
    { /* Check for the end of the content */
      tok = peek();
      if (tok.is(qEofF)) {
        diagnostic << tok << "Unexpected end of file in struct definition";
        return mock_stmt(QAST_STRUCT);
      }
      if (tok.is<qPuncRCur>()) {
        next();
        break;
      }
    }

    { /* Ignore free semicolons */
      if (tok.is<qPuncSemi>()) {
        next();
        continue;
      }
    }

    Vis vis = Vis::Sec;

    { /* Check for visibility qualifiers */
      if (tok.is<qKPub>()) {
        vis = Vis::Pub;
        next();
        tok = peek();
      } else if (tok.is<qKSec>()) {
        vis = Vis::Sec;
        next();
        tok = peek();
      } else if (tok.is<qKPro>()) {
        vis = Vis::Pro;
        next();
        tok = peek();
      }
    }

    /* Check for a function definition */
    if (tok.is<qKFn>()) {
      next();

      /* Parse the function definition */
      method = recurse_function(S, rd);

      { /* Add the 'this' parameter to the method */
        FuncParam fn_this{SaveString("this"),
                          make<RefTy>(make<NamedTy>(SaveString(name))),
                          nullptr};

        if (method->is<FnDef>()) {
          fdecl = static_cast<FnDef *>(method);
          ft = fdecl->get_type();
          ft->get_params().insert(ft->get_params().begin(), fn_this);
          fdecl->set_type(ft);
        } else {
          fdecl = static_cast<FnDef *>(method);
          ft = fdecl->get_type();
          ft->get_params().insert(ft->get_params().begin(), fn_this);
          fdecl->set_type(ft);
        }
      }

      /* Add the method to the list */
      methods.push_back(static_cast<FnDef *>(method));
    } else if (tok.is<qKStatic>()) {
      next();
      tok = next();

      /* Static fields are not currently supported */
      if (!tok.is<qKFn>()) {
        diagnostic << tok
                   << "Expected function definition after 'static' in struct "
                      "definition";
        return mock_stmt(QAST_STRUCT);
      }

      /* Parse the function definition */
      method = recurse_function(S, rd);

      /* Add the method to the list */
      static_methods.push_back(static_cast<FnDef *>(method));
    } else {
      /* Parse a normal field */
      field = recurse_composite_field(S, rd);

      tok = peek();
      if (tok.is<qPuncComa>() || tok.is<qPuncSemi>()) {
        next();
      }

      /* Assign the visibility to the field */
      if (field->is(QAST_STRUCT_FIELD)) {
        static_cast<StructField *>(field)->set_visibility(vis);
      }

      fields.push_back(field);
    }
  }

  { /* Ignore optional semicolon */
    tok = peek();
    if (tok.is<qPuncSemi>()) {
      next();
    }
  }

  sdef->set_name(SaveString(name));
  sdef->get_fields() = std::move(fields);
  sdef->get_methods() = std::move(methods);
  sdef->get_static_methods() = std::move(static_methods);

  return sdef;
}
