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

#include <nitrate-lexer/Lexer.h>
#include <nitrate-parser/Node.h>

#include <decent/Recurse.hh>

bool qparse::recurse_attributes(qparse_t &S, qlex_t &rd,
                                std::set<Expr *> &attributes) {
  qlex_tok_t tok = next();

  { /* The implementation list should be enclosed in square brackets ex: [abc,
       hello] */
    if (!tok.is<qPuncLBrk>()) {
      syntax(tok, "Expected '[' after 'impl' in definition");
    }
  }

  /* Parse an arbitrary number of attributes */
  while (true) {
    /* Check for termination */
    tok = peek();
    if (tok.is(qEofF)) {
      syntax(tok, "Unexpected end of file in definition");
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

qparse::Decl *qparse::recurse_composite_field(qparse_t &S, qlex_t &rd) {
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
      syntax(tok, "Expected field name in composite definition");
      return mock_decl(QAST_NODE_STRUCT_FIELD);
    }
    name = tok.as_string(&rd);
  }

  { /* Next token should be a colon */
    tok = next();
    if (!tok.is<qPuncColn>()) {
      syntax(tok, "Expected colon after field name in composite definition");
      return mock_decl(QAST_NODE_STRUCT_FIELD);
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
    auto R = StructField::get(name, type);
    R->set_end_pos(tok.start);
    return R;
  }

  { /* Optional default value */
    if (!tok.is<qOpSet>()) {
      syntax(tok,
             "Expected '=' or ',' after field type in composite definition");
      return mock_decl(QAST_NODE_STRUCT_FIELD);
    }
    next();

    /* Parse the default value */
    value = recurse_expr(
        S, rd,
        {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncSemi),
         qlex_tok_t(qPunc, qPuncRCur)});
  }

  auto R = StructField::get(name, type, value);
  R->set_end_pos(value->get_end_pos());

  return R;
}

bool recurse_template_parameters(
    qparse_t &S, qlex_t &rd,
    std::optional<qparse::TemplateParameters> &template_params);

qparse::Stmt *qparse::recurse_struct(qparse_t &S, qlex_t &rd,
                                     CompositeType type) {
  qlex_tok_t tok;
  std::string name;
  StructDefFields fields;
  StructDefMethods methods;
  StructDefStaticMethods static_methods;
  std::set<Expr *> attributes;
  Stmt *method = nullptr;
  FnDecl *fdecl = nullptr;
  FuncTy *ft = nullptr;
  Decl *field = nullptr;
  StructDef *sdef = StructDef::get();

  sdef->set_composite_type(type);

  { /* First token should be the name of the definition */
    tok = next();
    if (tok.is(qName)) {
      name = tok.as_string(&rd);
    } else {
      syntax(tok, "Expected struct name in struct definition");
      return mock_stmt(QAST_NODE_STRUCT);
    }
  }

  {
    if (!recurse_template_parameters(S, rd, sdef->get_template_params())) {
      return mock_stmt(QAST_NODE_STRUCT);
    }
  }

  { /* Next token should be an open curly bracket */
    tok = next();
    if (!tok.is<qPuncLCur>()) {
      syntax(tok, "Expected '{' after struct name in struct definition");
      return mock_stmt(QAST_NODE_STRUCT);
    }
  }

  /* Parse the fields and methods */
  while (true) {
    { /* Check for the end of the content */
      tok = peek();
      if (tok.is(qEofF)) {
        syntax(tok, "Unexpected end of file in struct definition");
        return mock_stmt(QAST_NODE_STRUCT);
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

    Vis vis = Vis::PRIVATE;

    { /* Check for visibility qualifiers */
      if (tok.is<qKPub>()) {
        vis = Vis::PUBLIC;
        next();
        tok = peek();
      } else if (tok.is<qKSec>()) {
        vis = Vis::PRIVATE;
        next();
        tok = peek();
      } else if (tok.is<qKPro>()) {
        vis = Vis::PROTECTED;
        next();
        tok = peek();
      }
    }

    /* Check for a function definition */
    if (tok.is<qKFn>()) {
      next();

      /* Parse the function definition */
      method = recurse_function(S, rd);

      /* Assign the visibility to the method */
      static_cast<FnDecl *>(method)->set_visibility(vis);

      { /* Add the 'this' parameter to the method */
        FuncParam fn_this{"this", RefTy::get(NamedTy::get(name)), nullptr};

        if (method->is<FnDecl>()) {
          fdecl = static_cast<FnDecl *>(method);
          ft = fdecl->get_type();
          ft->get_params().insert(ft->get_params().begin(), fn_this);
          fdecl->set_type(ft);
        } else {
          fdecl = static_cast<FnDecl *>(method);
          ft = fdecl->get_type();
          ft->get_params().insert(ft->get_params().begin(), fn_this);
          fdecl->set_type(ft);
        }
      }

      /* Add the method to the list */
      methods.push_back(static_cast<FnDecl *>(method));
    } else if (tok.is<qKStatic>()) {
      next();
      tok = next();

      /* Static fields are not currently supported */
      if (!tok.is<qKFn>()) {
        syntax(
            tok,
            "Expected function definition after 'static' in struct definition");
        return mock_stmt(QAST_NODE_STRUCT);
      }

      /* Parse the function definition */
      method = recurse_function(S, rd);

      /* Assign the visibility to the method */
      static_cast<FnDecl *>(method)->set_visibility(vis);

      /* Add the method to the list */
      static_methods.push_back(static_cast<FnDecl *>(method));
    } else {
      /* Parse a normal field */
      field = recurse_composite_field(S, rd);

      tok = peek();
      if (tok.is<qPuncComa>() || tok.is<qPuncSemi>()) {
        next();
      }

      /* Assign the visibility to the field */
      field->set_visibility(vis);

      fields.push_back(field);
    }
  }

  { /* Ignore optional semicolon */
    tok = peek();
    if (tok.is<qPuncSemi>()) {
      next();
    }
  }

  tok = peek();
  { /* Check for an implementation/trait list */
    if (tok.is<qKWith>()) {
      next();
      if (!recurse_attributes(S, rd, attributes)) {
        return mock_stmt(QAST_NODE_STRUCT);
      }
    }
  }

  sdef->set_name(name);
  sdef->get_fields() = std::move(fields);
  sdef->get_methods() = std::move(methods);
  sdef->get_static_methods() = std::move(static_methods);
  sdef->get_tags().insert(attributes.begin(), attributes.end());

  return sdef;
}
