////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///  ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░        ///
/// ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ///
///  ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░ ░▒▓██████▓▒░  ///
///    ░▒▓█▓▒░                                                               ///
///     ░▒▓██▓▒░                                                             ///
///                                                                          ///
///   * QUIX LANG COMPILER - The official compiler for the Quix language.    ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The QUIX Compiler Suite is free software; you can redistribute it or   ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The QUIX Compiler Suite is distributed in the hope that it will be     ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the QUIX Compiler Suite; if not, see                ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#define __QUIX_IMPL__

#include "LibMacro.h"
#include "parser/Parse.h"

using namespace qparse;
using namespace qparse::parser;

static bool parse_union_field(qparse_t &job, qlex_t *rd, CompositeField **node) {
  /**
   * @brief Parse a union struct field
   *
   * Format: "name: type [= expr],"
   */

  std::string name;
  qlex_tok_t tok;
  Type *type = nullptr;
  Expr *value = nullptr;

  { /*First token is the field name */
    tok = qlex_next(rd);
    if (!tok.is(qName)) {
      /// TODO: Write the ERROR message
      return false;
    }
    name = tok.as_string(rd);
  }

  { /* Next token should be a colon */
    tok = qlex_next(rd);
    if (!tok.is<qPuncColn>()) {
      /// TODO: Write the ERROR message
      return false;
    }
  }

  { /* Next section should be the field type */
    if (!parse_type(job, rd, &type)) {
      /// TODO: Write the ERROR message
      return false;
    }
  }

  /* Check for a default value */
  tok = qlex_next(rd);
  if (tok.is<qPuncComa>()) {
    *node = CompositeField::get(name, type);
    return true;
  }

  { /* Optional default value */
    if (!tok.is<qOpSet>()) {
      /// TODO: Write the ERROR message
      return false;
    }

    /* Parse the default value */
    if (!parse_expr(job, rd, {qlex_tok_t(qPunc, qPuncComa)}, &value)) {
      /// TODO: Write the ERROR message
      return false;
    }
  }

  /* Field ends with a comma */
  tok = qlex_next(rd);
  if (!tok.is<qPuncComa>()) {
    /// TODO: Write the ERROR message
    return false;
  }

  *node = CompositeField::get(name, type, value);
  return true;
}

bool parser::parse_union(qparse_t &job, qlex_t *rd, Stmt **node) {
  /**
   * @brief Parse a union composite type definition
   */

  qlex_tok_t tok;
  std::string name;
  UnionDefFields fields;
  UnionDefMethods methods;
  UnionDefStaticMethods static_methods;
  std::set<std::string> implements;
  Stmt *method = nullptr;
  FnDecl *fdecl = nullptr;
  FuncTy *ft = nullptr;
  CompositeField *field = nullptr;

  { /* First token should be the name of the definition */
    tok = qlex_next(rd);
    if (!tok.is(qName)) {
      /// TODO: Write the ERROR message
      return false;
    }

    name = tok.as_string(rd);
  }

  { /* Next token should be an open curly bracket */
    tok = qlex_next(rd);
    if (!tok.is<qPuncLCur>()) {
      /// TODO: Write the ERROR message
      return false;
    }
  }

  /* Parse the fields and methods */
  while (true) {
    { /* Check for the end of the content */
      tok = qlex_peek(rd);
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

    Visibility vis = Visibility::PRIVATE;

    { /* Check for visibility qualifiers */
      if (tok.is<qKPub>()) {
        vis = Visibility::PUBLIC;
        qlex_next(rd);
        tok = qlex_peek(rd);
      } else if (tok.is<qKSec>()) {
        vis = Visibility::PRIVATE;
        qlex_next(rd);
        tok = qlex_peek(rd);
      } else if (tok.is<qKPro>()) {
        vis = Visibility::PROTECTED;
        qlex_next(rd);
        tok = qlex_peek(rd);
      }
    }

    /* Check for a function definition */
    if (tok.is<qKFn>()) {
      qlex_next(rd);

      /* Parse the function definition */
      if (!parse_function(job, rd, &method)) {
        return false;
      }

      /* Assign the visibility to the method */
      static_cast<FnDecl *>(method)->set_visibility(vis);

      { /* Add the 'this' parameter to the method */
        FuncParam fn_this{"this", PtrTy::get(UnresolvedType::get(name)), nullptr};

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
        /// TODO: Write the ERROR message
        return false;
      }

      /* Parse the function definition */
      if (!parse_function(job, rd, &method)) {
        return false;
      }

      /* Assign the visibility to the method */
      static_cast<FnDecl *>(method)->set_visibility(vis);

      /* Add the method to the list */
      static_methods.push_back(static_cast<FnDecl *>(method));
    } else {
      /* Parse a normal field */
      if (!parse_union_field(job, rd, &field)) {
        return false;
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

  { /* The compiler may automatically generate traits for the definition */
    tok = qlex_peek(rd);
    if (!job.conf->has("-fno-auto-impl", "union")) {
      implements.insert("auto");
    }
  }

  { /* Check for an implementation/trait list */
    if (tok.is<qKImpl>()) {
      qlex_next(rd);
      tok = qlex_next(rd);

      { /* The implementation list should be enclosed in square brackets ex: [abc, hello] */
        if (!tok.is<qPuncLBrk>()) {
          /// TODO: Write the ERROR message
          return false;
        }
      }

      /* Parse an arbitrary number of trait names */
      while (true) {
        /* Check for termination */
        tok = qlex_next(rd);
        if (tok.is<qPuncRBrk>()) {
          break;
        }

        /* Ensure it is an identifier */
        if (!tok.is(qName)) {
          /// TODO: Write the ERROR message
          return false;
        }

        /* Add the trait to the list; Duplicate traits are ignored */
        implements.insert(tok.as_string(rd));

        /* Check for a comma */
        tok = qlex_peek(rd);
        if (tok.is<qPuncComa>()) {
          qlex_next(rd);
        }
      }
    }
  }

  UnionDef *sdef = UnionDef::get(name, nullptr, fields, methods, static_methods);
  sdef->add_tags(implements);
  *node = sdef;
  return true;
}