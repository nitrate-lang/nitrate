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

#include <LibMacro.h>
#include <lexer/Lex.h>
#include <libquixcc.h>
#include <quixcc/Quix.h>

#include <iostream>
#include <stdexcept>

using namespace libquixcc;

LIB_EXPORT void quixcc_lexconf(quixcc_job_t *job,
                               quixcc_lexer_config_t config) {
  std::lock_guard<std::mutex> lock(job->m_lock);
  // if (config & QUIXCC_LEXCONF_IGN_COM)
  //     job->m_prep->comments(false);
  // else
  //     job->m_prep->comments(true);
}

static quix_inline quixcc_sid_t publish_string(quixcc_job_t *job,
                                               const std::string &str) {
  if (job->m_sid_ctr == std::numeric_limits<quixcc_sid_t>::max())
    throw std::runtime_error("String ID counter overflow");

  auto sid = job->m_sid_ctr++;

  job->m_owned_strings[sid] = strdup(str.c_str());

  return sid;
}

static quix_inline void erase_sid(quixcc_job_t *job, quixcc_sid_t sid) {
  if (!job->m_owned_strings.contains(sid)) return;

  free(job->m_owned_strings[sid]);
  job->m_owned_strings.erase(sid);
}

static quix_inline bool check_and_init(quixcc_job_t *job) {
  bool preprocessor_config(quixcc_job_t * job,
                           std::unique_ptr<PrepEngine> & prep);

  if (job->m_prep) return true;

  job->m_prep = std::make_unique<PrepEngine>(*job);

  if (!preprocessor_config(job, job->m_prep)) return false;

  return true;
}

static quix_inline quixcc_tok_t fetch_token(quixcc_job_t *job) {
  auto tok = job->m_prep->peek();

  quixcc_tok_t ret;

  /* Token type */
  ret.ty = static_cast<quixcc_lex_type_t>(tok.type);

  /* Source info */
  ret.loc.column = tok.loc().col;
  ret.loc.line = tok.loc().line;
  ret.loc.voucher = publish_string(job, tok.loc().file.data());

  /* Token value */
  switch (ret.ty) {
    case QUIXCC_LEX_EOF:
      break;
    case QUIXCC_LEX_UNK:
      ret.val.voucher = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_IDENT:
      ret.val.voucher = publish_string(job, tok.as<std::string>());
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
      ret.val.voucher = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_FLOAT:
      ret.val.voucher = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_STR:
      ret.val.voucher = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_CHAR:
      ret.val.voucher = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_METABLK:
      ret.val.voucher = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_METASEG:
      ret.val.voucher = publish_string(job, tok.as<std::string>());
      break;
    case QUIXCC_LEX_NOTE:
      ret.val.voucher = publish_string(job, tok.as<std::string>());
      break;
  }

  return ret;
}

LIB_EXPORT quixcc_tok_t quixcc_next(quixcc_job_t *job) {
  /* Safe code is good code */
  assert(job != nullptr);

  std::lock_guard<std::mutex> lock(job->m_lock);

  quixcc_tok_t tokr{};
  tokr.ty = QUIXCC_LEX_EOF;

  /* Code is self-initializing */
  if (!check_and_init(job)) {
    return tokr;
  }

  auto tok = fetch_token(job);
  job->m_prep->next();
  return tok;
}

LIB_EXPORT quixcc_tok_t quixcc_peek(quixcc_job_t *job) {
  /* Safe code is good code */
  assert(job != nullptr);

  std::lock_guard<std::mutex> lock(job->m_lock);

  quixcc_tok_t tok{};
  tok.ty = QUIXCC_LEX_EOF;

  /* Code is self-initializing */
  if (!check_and_init(job)) return tok;

  return fetch_token(job);
}

LIB_EXPORT const char *quixcc_getstr(quixcc_job_t *job, quixcc_sid_t voucher) {
  /* Safe code is good code */
  assert(job != nullptr);

  std::lock_guard<std::mutex> lock(job->m_lock);

  if (!job->m_owned_strings.contains(voucher)) return nullptr;

  return job->m_owned_strings[voucher];
}

LIB_EXPORT void quixcc_tok_release(quixcc_job_t *job, quixcc_tok_t *tok) {
  /* Safe code is good code */
  assert(job != nullptr);
  assert(tok != nullptr);

  /* Require Lock */
  std::lock_guard<std::mutex> lock(job->m_lock);

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
      erase_sid(job, tok->val.voucher);
      tok->val.voucher = std::numeric_limits<quixcc_sid_t>::max();
      break;
    default:
      break;
  }

  erase_sid(job, tok->loc.voucher);
  tok->loc.voucher = std::numeric_limits<quixcc_sid_t>::max();
}

LIB_EXPORT size_t quixcc_tok_serialize(quixcc_job_t *job,
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
      return snprintf(buf, len, "%s", quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_IDENT:
      return snprintf(buf, len, "%s", quixcc_getstr(job, tok->val.voucher));
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
      return snprintf(buf, len, "%s", quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_FLOAT:
      return snprintf(buf, len, "%s", quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_STR:
      return snprintf(buf, len, "%s", quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_CHAR:
      return snprintf(buf, len, "%s", quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_METABLK:
      return snprintf(buf, len, "%s", quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_METASEG:
      return snprintf(buf, len, "%s", quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_NOTE:
      return snprintf(buf, len, "%s", quixcc_getstr(job, tok->val.voucher));
    default:
      return snprintf(buf, len, "UNK");
  }
}

LIB_EXPORT size_t quixcc_tok_humanize(quixcc_job_t *job,
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
      return snprintf(buf, len, "UNK(%s)",
                      quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_IDENT:
      return snprintf(buf, len, "IDENT(%s)",
                      quixcc_getstr(job, tok->val.voucher));
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
      return snprintf(buf, len, "INT(%s)",
                      quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_FLOAT:
      return snprintf(buf, len, "REAL(%s)",
                      quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_STR:
      return snprintf(buf, len, "STR(%s)",
                      quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_CHAR:
      return snprintf(buf, len, "CHAR(%s)",
                      quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_METABLK:
      return snprintf(buf, len, "METABLK(%s)",
                      quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_METASEG:
      return snprintf(buf, len, "METASEG(%s)",
                      quixcc_getstr(job, tok->val.voucher));
    case QUIXCC_LEX_NOTE:
      return snprintf(buf, len, "NOTE(%s)",
                      quixcc_getstr(job, tok->val.voucher));
    default:
      return snprintf(buf, len, "UNK");
  }
}