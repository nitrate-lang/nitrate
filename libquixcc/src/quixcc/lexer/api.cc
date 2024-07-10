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

#include <core/Macro.h>
#include <core/QuixJob.h>
#include <quixcc/Library.h>
#include <quixcc/lexer/Lex.h>

#include <iostream>
#include <stdexcept>

using namespace libquixcc;

LIB_EXPORT void quixcc_lexconf(quixcc_cc_job_t *job,
                               quixcc_lexer_config_t config) {
  std::lock_guard<std::mutex> lock(job->m_lock);
}

const char *publish_string(quixcc_cc_job_t *job, std::string_view str) {
  return job->m_owned_strings.insert(std::string(str)).first->c_str();
}

static quix_inline void erase_sid(quixcc_cc_job_t *job, const char *sid) {
  for (auto it = job->m_owned_strings.begin(); it != job->m_owned_strings.end();
       it++) {
    if (strcmp(it->c_str(), sid) == 0) {
      job->m_owned_strings.erase(it);
      break;
    }
  }
}

static quix_inline bool check_and_init(quixcc_cc_job_t *job) {
  if (job->m_scanner) return true;

  job->m_scanner = std::make_unique<StreamLexer>();
  job->m_scanner->set_source(job->m_in, job->m_filename.top().c_str());

  for (size_t i = 0; i < job->m_options.m_count; i++) {
    if (strcmp(job->m_options.m_options[i], "-fkeep-comments") == 0) {
      job->m_scanner->comments(false);
      break;
    }
  }

  return true;
}

static quix_inline quixcc_tok_t fetch_token(quixcc_cc_job_t *job) {
  auto tok = job->m_scanner->peek();

  quixcc_tok_t ret;

  /* Token type */
  ret.ty = static_cast<quixcc_lex_type_t>(tok.type());

  /* Source info */
  ret.loc.column = tok.loc().col();
  ret.loc.line = tok.loc().line();
  ret.loc.file = publish_string(job, tok.loc().file());

  /* Token value */
  switch (ret.ty) {
    case QUIXCC_LEX_EOF:
      break;
    case QUIXCC_LEX_UNK:
      ret.val.data = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_IDENT:
      ret.val.data = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_KW:
      ret.val.kw = static_cast<quixcc_lex_kw_t>(tok.as<Keyword>());
      break;
    case QUIXCC_LEX_OP:
      ret.val.op = static_cast<quixcc_lex_op_t>(tok.as<Operator>());
      break;
    case QUIXCC_LEX_PUNCT:
      ret.val.punct = static_cast<quixcc_lex_punct_t>(tok.as<Punctor>());
      break;
    case QUIXCC_LEX_INT:
      ret.val.data = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_FLOAT:
      ret.val.data = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_STR:
      ret.val.data = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_CHAR:
      ret.val.data = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_METABLK:
      ret.val.data = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_METASEG:
      ret.val.data = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_NOTE:
      ret.val.data = publish_string(job, tok.as<std::string>());
      break;
  }

  return ret;
}

LIB_EXPORT quixcc_tok_t quixcc_cc_next(quixcc_cc_job_t *job) {
  /* Safe code is good code */
  assert(job != nullptr);

  /* This function is not thread-safe on the same job */

  quixcc_tok_t tokr{};
  tokr.ty = QUIXCC_LEX_EOF;

  /* Code is self-initializing */
  if (!check_and_init(job)) {
    return tokr;
  }

  auto tok = fetch_token(job);
  job->m_scanner->next();
  return tok;
}

LIB_EXPORT quixcc_tok_t quixcc_cc_peek(quixcc_cc_job_t *job) {
  /* Safe code is good code */
  assert(job != nullptr);

  /* This function is not thread-safe on the same job */

  quixcc_tok_t tok{};
  tok.ty = QUIXCC_LEX_EOF;

  /* Code is self-initializing */
  if (!check_and_init(job)) return tok;

  return fetch_token(job);
}

LIB_EXPORT bool quixcc_cc_tokeq(quixcc_tok_t a, quixcc_tok_t b) {
  if (a.ty != b.ty) return false;

  switch (a.ty) {
    case QUIXCC_LEX_EOF:
      return true;
    case QUIXCC_LEX_UNK:
    case QUIXCC_LEX_IDENT:
    case QUIXCC_LEX_INT:
    case QUIXCC_LEX_FLOAT:
    case QUIXCC_LEX_STR:
    case QUIXCC_LEX_CHAR:
    case QUIXCC_LEX_METABLK:
    case QUIXCC_LEX_METASEG:
    case QUIXCC_LEX_NOTE: {
      if (!a.val.data || !b.val.data) return false;
      return strcmp(a.val.data, b.val.data) == 0;
    }
    case QUIXCC_LEX_KW:
      return a.val.kw == b.val.kw;
    case QUIXCC_LEX_OP:
      return a.val.op == b.val.op;
    case QUIXCC_LEX_PUNCT:
      return a.val.punct == b.val.punct;
    default:
      return false;
  }
}

LIB_EXPORT void quixcc_cc_tok_release(quixcc_cc_job_t *job, quixcc_tok_t *tok) {
  /* Safe code is good code */
  assert(job != nullptr);
  assert(tok != nullptr);

  /* This function is not thread-safe on the same job */

  switch (tok->ty) {
    case QUIXCC_LEX_UNK:
    case QUIXCC_LEX_IDENT:
    case QUIXCC_LEX_INT:
    case QUIXCC_LEX_FLOAT:
    case QUIXCC_LEX_STR:
    case QUIXCC_LEX_CHAR:
    case QUIXCC_LEX_METABLK:
    case QUIXCC_LEX_METASEG:
    case QUIXCC_LEX_NOTE:
      erase_sid(job, tok->val.data);
      tok->val.data = NULL;
      break;
    default:
      break;
  }

  erase_sid(job, tok->loc.file);
  tok->loc.file = NULL;
}

LIB_EXPORT size_t quixcc_cc_tok_serialize(quixcc_cc_job_t *job,
                                          const quixcc_tok_t *tok, char *buf,
                                          size_t len) {
  /* Safe code is good code */
  assert(job != nullptr);
  assert(tok != nullptr);
  assert(buf != nullptr);

  switch (tok->ty) {
    case QUIXCC_LEX_EOF:
      return snprintf(buf, len, "EOF");
    case QUIXCC_LEX_UNK:
      return snprintf(buf, len, "%s", tok->val.data);
    case QUIXCC_LEX_IDENT:
      return snprintf(buf, len, "%s", tok->val.data);
    case QUIXCC_LEX_KW:
      return snprintf(
          buf, len, "%s",
          keyword_map_inverse.at(static_cast<Keyword>(tok->val.kw)).data());
    case QUIXCC_LEX_OP:
      return snprintf(
          buf, len, "%s",
          operator_map_inverse.at(static_cast<Operator>(tok->val.op)).data());
    case QUIXCC_LEX_PUNCT:
      return snprintf(
          buf, len, "%s",
          punctor_map_inverse.at(static_cast<Punctor>(tok->val.punct)).data());
    case QUIXCC_LEX_INT:
      return snprintf(buf, len, "%s", tok->val.data);
    case QUIXCC_LEX_FLOAT:
      return snprintf(buf, len, "%s", tok->val.data);
    case QUIXCC_LEX_STR:
      return snprintf(buf, len, "%s", tok->val.data);
    case QUIXCC_LEX_CHAR:
      return snprintf(buf, len, "%s", tok->val.data);
    case QUIXCC_LEX_METABLK:
      return snprintf(buf, len, "%s", tok->val.data);
    case QUIXCC_LEX_METASEG:
      return snprintf(buf, len, "%s", tok->val.data);
    case QUIXCC_LEX_NOTE:
      return snprintf(buf, len, "%s", tok->val.data);
    default:
      return snprintf(buf, len, "UNK");
  }
}

LIB_EXPORT size_t quixcc_cc_tok_humanize(quixcc_cc_job_t *job,
                                         const quixcc_tok_t *tok, char *buf,
                                         size_t len) {
  /* Safe code is good code */
  assert(job != nullptr);
  assert(tok != nullptr);
  assert(buf != nullptr);

  switch (tok->ty) {
    case QUIXCC_LEX_EOF:
      return snprintf(buf, len, "EOF()");
    case QUIXCC_LEX_UNK:
      return snprintf(buf, len, "UNK(%s)", tok->val.data);
    case QUIXCC_LEX_IDENT:
      return snprintf(buf, len, "IDENT(%s)", tok->val.data);
    case QUIXCC_LEX_KW:
      return snprintf(
          buf, len, "KW(%s)",
          keyword_map_inverse.at(static_cast<Keyword>(tok->val.kw)).data());
    case QUIXCC_LEX_OP:
      return snprintf(
          buf, len, "OP(%s)",
          operator_map_inverse.at(static_cast<Operator>(tok->val.op)).data());
    case QUIXCC_LEX_PUNCT:
      return snprintf(
          buf, len, "PUNCT(%s)",
          punctor_map_inverse.at(static_cast<Punctor>(tok->val.punct)).data());
    case QUIXCC_LEX_INT:
      return snprintf(buf, len, "INT(%s)", tok->val.data);
    case QUIXCC_LEX_FLOAT:
      return snprintf(buf, len, "REAL(%s)", tok->val.data);
    case QUIXCC_LEX_STR:
      return snprintf(buf, len, "STR(%s)", tok->val.data);
    case QUIXCC_LEX_CHAR:
      return snprintf(buf, len, "CHAR(%s)", tok->val.data);
    case QUIXCC_LEX_METABLK:
      return snprintf(buf, len, "METABLK(%s)", tok->val.data);
    case QUIXCC_LEX_METASEG:
      return snprintf(buf, len, "METASEG(%s)", tok->val.data);
    case QUIXCC_LEX_NOTE:
      return snprintf(buf, len, "NOTE(%s)", tok->val.data);
    default:
      return snprintf(buf, len, "UNK");
  }
}