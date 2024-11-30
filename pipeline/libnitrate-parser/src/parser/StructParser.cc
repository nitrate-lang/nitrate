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

#include <parser/Parse.h>

#include "nitrate-lexer/Lexer.h"

using namespace qparse;
using namespace qparse::parser;
using namespace qparse::diag;

bool qparse::parser::parse_attributes(qparse_t &job, qlex_t *rd,
                                      std::set<Expr *> &attributes) {
  qlex_tok_t tok = qlex_next(rd);

  { /* The implementation list should be enclosed in square brackets ex: [abc,
       hello] */
    if (!tok.is<qPuncLBrk>()) {
      syntax(tok, "Expected '[' after 'impl' in definition");
    }
  }

  /* Parse an arbitrary number of attributes */
  while (true) {
    /* Check for termination */
    tok = qlex_peek(rd);
    if (tok.is(qEofF)) {
      syntax(tok, "Unexpected end of file in definition");
      return false;
    }

    if (tok.is<qPuncRBrk>()) {
      qlex_next(rd);
      break;
    }

    Expr *impl = nullptr;

    if (!parse_expr(
            job, rd,
            {qlex_tok_t(qPunc, qPuncRBrk), qlex_tok_t(qPunc, qPuncComa)}, &impl,
            0)) {
      syntax(tok, "Failed to parse declaration attribute expression");
      return false;
    }

    attributes.insert(impl);

    /* Check for a comma */
    tok = qlex_peek(rd);
    if (tok.is<qPuncComa>()) {
      qlex_next(rd);
    }
  }

  return true;
}

bool parser::parse_composite_field(qparse_t &job, qlex_t *rd,
                                   StructField **node) {
  /*
   * Format: "name: type [= expr],"
   */

  std::string name;
  qlex_tok_t tok;
  Type *type = nullptr;
  Expr *value = nullptr;

  { /*First token is the field name */
    tok = qlex_next(rd);
    if (!tok.is(qName)) {
      syntax(tok, "Expected field name in composite definition");
    }
    name = tok.as_string(rd);
  }

  { /* Next token should be a colon */
    tok = qlex_next(rd);
    if (!tok.is<qPuncColn>()) {
      syntax(tok, "Expected colon after field name in composite definition");
    }
  }

  { /* Next section should be the field type */
    if (!parse_type(job, rd, &type)) {
      syntax(tok, "Expected field type in composite definition");
    }
  }

  /* Check for a default value */
  tok = qlex_peek(rd);
  if (tok.is<qPuncComa>() || tok.is<qPuncSemi>() || tok.is<qPuncRCur>()) {
    if (tok.is<qPuncComa>() || tok.is<qPuncSemi>()) {
      qlex_next(rd);
    }
    *node = StructField::get(name, type);
    (*node)->set_end_pos(tok.start);
    return true;
  }

  { /* Optional default value */
    if (!tok.is<qOpSet>()) {
      syntax(tok,
             "Expected '=' or ',' after field type in composite definition");
    }
    qlex_next(rd);

    /* Parse the default value */
    if (!parse_expr(job, rd,
                    {qlex_tok_t(qPunc, qPuncComa), qlex_tok_t(qPunc, qPuncSemi),
                     qlex_tok_t(qPunc, qPuncRCur)},
                    &value) ||
        !value) {
      syntax(tok, "Expected default value after '=' in composite definition");
      return false;
    }
  }

  *node = StructField::get(name, type, value);
  (*node)->set_end_pos(value->get_end_pos());

  return true;
}

bool parse_template_parameters(
    qparse_t &job, qlex_t *rd,
    std::optional<TemplateParameters> &template_params);

bool parser::parse_struct(qparse_t &job, qlex_t *rd, Stmt **node) {
  /**
   * @brief Parse a struct composite type definition
   */

  qlex_tok_t tok;
  std::string name;
  StructDefFields fields;
  StructDefMethods methods;
  StructDefStaticMethods static_methods;
  std::set<Expr *> attributes;
  Stmt *method = nullptr;
  FnDecl *fdecl = nullptr;
  FuncTy *ft = nullptr;
  StructField *field = nullptr;
  StructDef *sdef = StructDef::get();

  { /* First token should be the name of the definition */
    tok = qlex_next(rd);
    if (tok.is(qName)) {
      name = tok.as_string(rd);
    } else {
      syntax(tok, "Expected struct name in struct definition");
    }
  }

  {
    if (!parse_template_parameters(job, rd, sdef->get_template_params())) {
      return false;
    }
  }

  { /* Next token should be an open curly bracket */
    tok = qlex_next(rd);
    if (!tok.is<qPuncLCur>()) {
      syntax(tok, "Expected '{' after struct name in struct definition");
    }
  }

  /* Parse the fields and methods */
  while (true) {
    { /* Check for the end of the content */
      tok = qlex_peek(rd);
      if (tok.is(qEofF)) {
        syntax(tok, "Unexpected end of file in struct definition");
        return false;
      }
      if (tok.is<qPuncRCur>()) {
        qlex_next(rd);
        break;
      }
    }

    { /* Ignore free semicolons */
      if (tok.is<qPuncSemi>()) {
        qlex_next(rd);
        continue;
      }
    }

    Vis vis = Vis::PRIVATE;

    { /* Check for visibility qualifiers */
      if (tok.is<qKPub>()) {
        vis = Vis::PUBLIC;
        qlex_next(rd);
        tok = qlex_peek(rd);
      } else if (tok.is<qKSec>()) {
        vis = Vis::PRIVATE;
        qlex_next(rd);
        tok = qlex_peek(rd);
      } else if (tok.is<qKPro>()) {
        vis = Vis::PROTECTED;
        qlex_next(rd);
        tok = qlex_peek(rd);
      }
    }

    /* Check for a function definition */
    if (tok.is<qKFn>()) {
      qlex_next(rd);

      /* Parse the function definition */
      if (!parse_function(job, rd, &method) || !method) {
        syntax(tok, "Expected function definition in struct definition");
        return false;
      }

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
      qlex_next(rd);
      tok = qlex_next(rd);

      /* Static fields are not currently supported */
      if (!tok.is<qKFn>()) {
        syntax(
            tok,
            "Expected function definition after 'static' in struct definition");
      }

      /* Parse the function definition */
      if (!parse_function(job, rd, &method) || !method) {
        syntax(tok, "Expected function definition in struct definition");
        return false;
      }

      /* Assign the visibility to the method */
      static_cast<FnDecl *>(method)->set_visibility(vis);

      /* Add the method to the list */
      static_methods.push_back(static_cast<FnDecl *>(method));
    } else {
      /* Parse a normal field */
      if (!parse_composite_field(job, rd, &field)) {
        syntax(tok, "Expected field definition in struct definition");
        return false;
      }

      tok = qlex_peek(rd);
      if (tok.is<qPuncComa>() || tok.is<qPuncSemi>()) {
        qlex_next(rd);
      }

      /* Assign the visibility to the field */
      field->set_visibility(vis);

      fields.push_back(field);
    }
  }

  { /* Ignore optional semicolon */
    tok = qlex_peek(rd);
    if (tok.is<qPuncSemi>()) {
      qlex_next(rd);
    }
  }

  tok = qlex_peek(rd);
  { /* Check for an implementation/trait list */
    if (tok.is<qKWith>()) {
      qlex_next(rd);
      if (!parse_attributes(job, rd, attributes)) {
        return false;
      }
    }
  }

  sdef->set_name(name);
  sdef->get_fields() = std::move(fields);
  sdef->get_methods() = std::move(methods);
  sdef->get_static_methods() = std::move(static_methods);
  sdef->get_tags().insert(attributes.begin(), attributes.end());

  *node = sdef;

  return true;
}
