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

#define QUIXCC_INTERNAL

#include "LibMacro.h"
#include "parser/Parse.h"
#include <quixcc/core/Logger.h>

using namespace libquixcc;
using namespace qparse;
using namespace qparse::parser;

static bool parse_union_field(quixcc_cc_job_t &job, Scanner *scanner, CompositeField **node) {
  /**
   * @brief Parse a union struct field
   *
   * Format: "name: type [= expr],"
   */

  std::string name;
  Token tok;
  Type *type = nullptr;
  Expr *value = nullptr;

  { /*First token is the field name */
    tok = scanner->next();
    if (!tok.is(tName)) {
      LOG(ERROR) << core::feedback[UNION_FIELD_MISSING_IDENTIFIER] << tok << std::endl;
      return false;
    }
    name = tok.as_string();
  }

  { /* Next token should be a colon */
    tok = scanner->next();
    if (!tok.is<Punctor>(Colon)) {
      LOG(ERROR) << core::feedback[UNION_FIELD_MISSING_COLON] << tok << std::endl;
      return false;
    }
  }

  { /* Next section should be the field type */
    if (!parse_type(job, scanner, &type)) {
      LOG(ERROR) << core::feedback[UNION_FIELD_TYPE_ERR] << name << tok << std::endl;
      return false;
    }
  }

  /* Check for a default value */
  tok = scanner->next();
  if (tok.is<Punctor>(Comma)) {
    *node = CompositeField::get(name, type);
    return true;
  }

  { /* Optional default value */
    if (!tok.is<Operator>(OpAssign)) {
      LOG(ERROR) << core::feedback[UNION_FIELD_MISSING_PUNCTOR] << name << tok << std::endl;
      return false;
    }

    /* Parse the default value */
    if (!parse_expr(job, scanner, {Token(tPunc, Comma)}, &value)) {
      LOG(ERROR) << core::feedback[UNION_FIELD_INIT_ERR] << name << tok << std::endl;
      return false;
    }
  }

  /* Field ends with a comma */
  tok = scanner->next();
  if (!tok.is<Punctor>(Comma)) {
    LOG(ERROR) << core::feedback[UNION_FIELD_MISSING_PUNCTOR] << name << tok << std::endl;
    return false;
  }

  *node = CompositeField::get(name, type, value);
  return true;
}

bool parser::parse_union(quixcc_cc_job_t &job, Scanner *scanner, Stmt **node) {
  /**
   * @brief Parse a union composite type definition
   */

  Token tok;
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
    tok = scanner->next();
    if (!tok.is(tName)) {
      LOG(ERROR) << core::feedback[UNION_DECL_MISSING_IDENTIFIER] << tok << std::endl;
      return false;
    }

    name = tok.as_string();
  }

  { /* Next token should be an open curly bracket */
    tok = scanner->next();
    if (!tok.is<Punctor>(OpenBrace)) {
      LOG(ERROR) << core::feedback[UNION_DEF_EXPECTED_OPEN_BRACE] << tok << std::endl;
      return false;
    }
  }

  /* Parse the fields and methods */
  while (true) {
    { /* Check for the end of the content */
      tok = scanner->peek();
      if (tok.is<Punctor>(CloseBrace)) {
        scanner->next();
        break;
      }
    }

    { /* Ignore free semicolons */
      if (tok.is<Punctor>(Semicolon)) {
        scanner->next();
        continue;
      }
    }

    Visibility vis = Visibility::PRIVATE;

    { /* Check for visibility qualifiers */
      if (tok.is<Keyword>(Keyword::Pub)) {
        vis = Visibility::PUBLIC;
        scanner->next();
        tok = scanner->peek();
      } else if (tok.is<Keyword>(Keyword::Sec)) {
        vis = Visibility::PRIVATE;
        scanner->next();
        tok = scanner->peek();
      } else if (tok.is<Keyword>(Keyword::Pro)) {
        vis = Visibility::PROTECTED;
        scanner->next();
        tok = scanner->peek();
      }
    }

    /* Check for a function definition */
    if (tok.is<Keyword>(Keyword::Fn)) {
      scanner->next();

      /* Parse the function definition */
      if (!parse_function(job, scanner, &method)) {
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
    } else if (tok.is<Keyword>(Keyword::Static)) {
      scanner->next();
      tok = scanner->next();

      /* Static fields are not currently supported */
      if (!tok.is<Keyword>(Keyword::Fn)) {
        LOG(ERROR) << core::feedback[UNION_DEF_EXPECTED_FN] << tok << std::endl;
        return false;
      }

      /* Parse the function definition */
      if (!parse_function(job, scanner, &method)) {
        return false;
      }

      /* Assign the visibility to the method */
      static_cast<FnDecl *>(method)->set_visibility(vis);

      /* Add the method to the list */
      static_methods.push_back(static_cast<FnDecl *>(method));
    } else {
      /* Parse a normal field */
      if (!parse_union_field(job, scanner, &field)) {
        return false;
      }

      /* Assign the visibility to the field */
      field->set_visibility(vis);

      fields.push_back(field);
    }
  }

  { /* Ignore optional semicolon */
    tok = scanner->peek();
    if (tok.is<Punctor>(Semicolon)) {
      scanner->next();
    }
  }

  { /* The compiler may automatically generate traits for the definition */
    tok = scanner->peek();
    if (!job.has("-fno-auto-impl", "union")) {
      implements.insert("auto");
    }
  }

  { /* Check for an implementation/trait list */
    if (tok.is<Keyword>(Keyword::Impl)) {
      scanner->next();
      tok = scanner->next();

      { /* The implementation list should be enclosed in square brackets ex: [abc, hello] */
        if (!tok.is<Punctor>(OpenBracket)) {
          LOG(ERROR) << core::feedback[UNION_DEF_EXPECTED_OPEN_BRACKET] << tok << std::endl;
          return false;
        }
      }

      /* Parse an arbitrary number of trait names */
      while (true) {
        /* Check for termination */
        tok = scanner->next();
        if (tok.is<Punctor>(CloseBracket)) {
          break;
        }

        /* Ensure it is an identifier */
        if (!tok.is(tName)) {
          LOG(ERROR) << core::feedback[UNION_DEF_EXPECTED_IDENTIFIER] << tok << std::endl;
          return false;
        }

        /* Add the trait to the list; Duplicate traits are ignored */
        implements.insert(tok.as_string());

        /* Check for a comma */
        tok = scanner->peek();
        if (tok.is<Punctor>(Comma)) {
          scanner->next();
        }
      }
    }
  }

  UnionDef *sdef = UnionDef::get(name, nullptr, fields, methods, static_methods);
  sdef->add_tags(implements);
  *node = sdef;
  return true;
}