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

#include <IR/Q/QIR.h>
#include <IR/Q/Variable.h>
#include <LibMacro.h>
#include <core/Logger.h>
#include <libquixcc.h>
#include <preprocessor/Preprocessor.h>
#include <quixcc/EngineAPI.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <filesystem>
#include <string>
#include <vector>

using namespace libquixcc;

extern void quixcc_panic(std::string msg) noexcept;

///=============================================================================
/// BEGIN: QUIXCC QSYS REGISTRY
///=============================================================================

LIB_EXPORT bool quixcc_qsys_add(quixcc_job_t *job, uint32_t num,
                                quixcc_qsys_impl_t impl) {
  if (!job) quixcc_panic("quixcc_qsys_add: job is NULL");

  std::lock_guard<std::mutex> lock(job->m_lock);
  job->m_qsyscalls.Register(num, impl);

  return true;
}

LIB_EXPORT bool quixcc_qsys_remove(quixcc_job_t *job, uint32_t num) {
  if (!job) quixcc_panic("quixcc_qsys_remove: job is NULL");

  std::lock_guard<std::mutex> lock(job->m_lock);
  return job->m_qsyscalls.Unregister(num);
}

LIB_EXPORT bool quixcc_qsys_exists(quixcc_job_t *job, uint32_t num) {
  if (!job) quixcc_panic("quixcc_qsys_exists: job is NULL");

  std::lock_guard<std::mutex> lock(job->m_lock);
  return job->m_qsyscalls.IsRegistered(num);
}

LIB_EXPORT quixcc_qsys_impl_t quixcc_qsys_get(quixcc_job_t *job, uint32_t num) {
  if (!job) quixcc_panic("quixcc_qsys_get: job is NULL");

  std::lock_guard<std::mutex> lock(job->m_lock);
  auto impl = job->m_qsyscalls.Get(num);

  if (!impl) return nullptr;
  return impl.value();
}

LIB_EXPORT uint32_t *quixcc_qsys_list(quixcc_job_t *job, uint32_t *count) {
  if (!job) quixcc_panic("quixcc_qsys_list: job is NULL");
  if (!count) quixcc_panic("quixcc_qsys_list: count is NULL");

  std::lock_guard<std::mutex> lock(job->m_lock);

  auto list = job->m_qsyscalls.GetRegistered();
  *count = list.size();

  uint32_t *ret = (uint32_t *)malloc(sizeof(uint32_t) * list.size());
  if (!ret) quixcc_panic("quixcc_qsys_list: malloc() failed");

  for (size_t i = 0; i < list.size(); i++) {
    ret[i] = list[i];
  }

  return ret;
}

///=============================================================================
/// END: QUIXCC QSYS REGISTRY
///=============================================================================

///=============================================================================
/// BEGIN: QUIXCC ENGINE API
///=============================================================================

LIB_EXPORT quixcc_job_t *quixcc_engine_job(quixcc_engine_t *engine) {
  if (!engine) quixcc_panic("quixcc_engine_job: engine is NULL");
  return reinterpret_cast<libquixcc::PrepEngine *>(engine)->get_job();
}

LIB_EXPORT bool quixcc_engine_include(quixcc_engine_t *engine,
                                      const char *_include_path) {
  if (!engine) quixcc_panic("quixcc_engine_include: engine is NULL");
  if (!_include_path)
    quixcc_panic("quixcc_engine_include: include_path is NULL");

  std::string pathstr(_include_path);

  /*================ PARSE PATH =================*/
  std::vector<std::string> parts;
  std::string::size_type start = 0;
  std::string::size_type end = 0;

  while ((end = pathstr.find(':', start)) != std::string::npos) {
    parts.push_back(pathstr.substr(start, end - start));
    start = end + 1;
  }
  parts.push_back(pathstr.substr(start));

  /*================ VALIDATE PATH =================*/
  for (auto &part : parts) {
    if (part.empty()) {
      return false;
    }

    try {
      std::filesystem::path p(part);
      if (!std::filesystem::exists(p)) {
        return false;
      }
    } catch (std::exception &e) {
      return false;
    }
  }

  /*================ SET PATH =================*/
  reinterpret_cast<libquixcc::PrepEngine *>(engine)->set_include_path(pathstr);
  return true;
}

LIB_EXPORT bool quixcc_engine_message(quixcc_engine_t *engine,
                                      quixcc_message_t mode, const char *format,
                                      ...) {
  if (!engine) quixcc_panic("quixcc_engine_message: engine is NULL");
  if (!format) quixcc_panic("quixcc_engine_message: message is NULL");

  auto job = quixcc_engine_job(engine);

  /*================ CONFIGURE LOGGER (IF NOT ALREADY) =================*/
  LoggerConfigure(*job);

  /*================ LOG MESSAGE =================*/
  va_list args;
  va_start(args, format);
  char *buffer = nullptr;
  vasprintf(&buffer, format, args);
  va_end(args);

  std::string message(buffer);
  free(buffer);

  switch (mode) {
    case QUIXCC_MESSAGE_DEBUG:
      LOG(DEBUG) << message << std::endl;
      break;
    case QUIXCC_MESSAGE_INFO:
      LOG(INFO) << message << std::endl;
      break;
    case QUIXCC_MESSAGE_WARNING:
      LOG(WARN) << message << std::endl;
      break;
    case QUIXCC_MESSAGE_ERROR:
      LOG(ERROR) << message << std::endl;
      break;
    case QUIXCC_MESSAGE_FAILED:
      LOG(FAILED) << message << std::endl;
      break;
    case QUIXCC_MESSAGE_FATAL:
      LOG(FATAL) << message << std::endl;
      break;
    default:
      return false;
  }

  return true;
}

LIB_EXPORT bool quixcc_engine_emit(quixcc_engine_t *engine, quixcc_tok_t tok) {
  if (!engine) quixcc_panic("quixcc_engine_emit: engine is NULL");

  auto job = quixcc_engine_job(engine);

  Token token;

  const char *s;

#define GETSTR(x) ((s = quixcc_getstr(job, x.val.voucher)) ? s : "")

  switch (tok.ty) {
    case QUIXCC_LEX_EOF:
      token = Token(TT::Eof, "");
      break;
    case QUIXCC_LEX_IDENT:
      token = Token(TT::Identifier, GETSTR(tok));
      break;
    case QUIXCC_LEX_KW:
      token = Token(TT::Keyword, (Keyword)tok.val.kw);
      break;
    case QUIXCC_LEX_OP:
      token = Token(TT::Operator, (Operator)tok.val.op);
      break;
    case QUIXCC_LEX_PUNCT:
      token = Token(TT::Punctor, (Punctor)tok.val.punct);
      break;
    case QUIXCC_LEX_INT:
      token = Token(TT::Integer, GETSTR(tok));
      break;
    case QUIXCC_LEX_FLOAT:
      token = Token(TT::Float, GETSTR(tok));
      break;
    case QUIXCC_LEX_STR:
      token = Token(TT::String, GETSTR(tok));
      break;
    case QUIXCC_LEX_CHAR:
      token = Token(TT::Char, GETSTR(tok));
      break;
    case QUIXCC_LEX_METABLK:
      token = Token(TT::MacroBlock, GETSTR(tok));
      break;
    case QUIXCC_LEX_METASEG:
      token = Token(TT::MacroSingleLine, GETSTR(tok));
      break;
    case QUIXCC_LEX_NOTE:
      token = Token(TT::Comment, GETSTR(tok));
      break;
    case QUIXCC_LEX_UNK:
      token = Token(TT::Unknown, "");
      break;
    default:
      return false;
  }

  reinterpret_cast<libquixcc::PrepEngine *>(engine)->emit(token);
  return true;
}

LIB_EXPORT const char *quixcc_expr_to_string(quixcc_expr_t *expr, size_t *len) {
  if (!expr) quixcc_panic("quixcc_expr_to_string: expr is NULL");
  if (!len) quixcc_panic("quixcc_expr_to_string: len is NULL");

  auto qir = reinterpret_cast<libquixcc::ir::q::QModule *>(expr);
  auto root = qir->root();

  if (root->children.size() != 1) {
    return nullptr;
  }

  if (!qir->root()->children[0]->is<ir::q::String>()) {
    return nullptr;
  }

  auto &str = root->children[0]->as<ir::q::String>()->value;
  *len = str.size();

  return str.c_str();
}

LIB_EXPORT bool quixcc_expr_to_int64(quixcc_expr_t *expr, int64_t *out) {
  if (!expr) quixcc_panic("quixcc_expr_to_int64: expr is NULL");
  if (!out) quixcc_panic("quixcc_expr_to_int64: out is NULL");

  *out = INT64_MIN;

  auto qir = reinterpret_cast<libquixcc::ir::q::QModule *>(expr);
  auto root = qir->root();

  if (root->children.size() != 1) {
    return false;
  }

  if (!qir->root()->children[0]->is<ir::q::Number>()) {
    return false;
  }

  auto &intstr = root->children[0]->as<ir::q::Number>()->value;

  try {
    *out = std::stoll(intstr);
    return true;
  } catch (std::exception &e) {
    return false;
  }
}

extern quixcc_sid_t publish_string(quixcc_job_t *job, std::string_view str);

LIB_EXPORT quixcc_tok_t quixcc_tok_new(quixcc_engine_t *engine,
                                       quixcc_lex_type_t ty, const char *str) {
  if (!engine) quixcc_panic("quixcc_tok_new: engine is NULL");
  if (!str) quixcc_panic("quixcc_tok_new: str is NULL");

  auto job = quixcc_engine_job(engine);
  auto sid = publish_string(job, str);

  quixcc_tok_t tok;
  tok.ty = ty;
  tok.val.voucher = sid;
  tok.loc.line = 0;
  tok.loc.column = 0;
  tok.loc.voucher = QUIXCC_SID_NAN;
  return tok;
}

LIB_EXPORT quixcc_tok_t quixcc_tok_new_ex(quixcc_engine_t *engine,
                                          quixcc_lex_type_t ty, const char *str,
                                          size_t len) {
  if (!engine) quixcc_panic("quixcc_tok_new: engine is NULL");
  if (!str) quixcc_panic("quixcc_tok_new: str is NULL");

  auto job = quixcc_engine_job(engine);
  auto sid = publish_string(job, std::string_view(str, len));

  quixcc_tok_t tok;
  tok.ty = ty;
  tok.val.voucher = sid;
  tok.loc.line = 0;
  tok.loc.column = 0;
  tok.loc.voucher = QUIXCC_SID_NAN;
  return tok;
}

LIB_EXPORT quixcc_tok_t quixcc_tok_new_kw(quixcc_engine_t *engine,
                                          quixcc_lex_kw_t kw) {
  quixcc_tok_t tok;
  tok.ty = QUIXCC_LEX_KW;
  tok.val.kw = kw;
  tok.loc.line = 0;
  tok.loc.column = 0;
  tok.loc.voucher = QUIXCC_SID_NAN;
  return tok;
}

LIB_EXPORT quixcc_tok_t quixcc_tok_new_op(quixcc_engine_t *engine,
                                          quixcc_lex_op_t op) {
  quixcc_tok_t tok;
  tok.ty = QUIXCC_LEX_OP;
  tok.val.op = op;
  tok.loc.line = 0;
  tok.loc.column = 0;
  tok.loc.voucher = QUIXCC_SID_NAN;
  return tok;
}

LIB_EXPORT quixcc_tok_t quixcc_tok_new_punct(quixcc_engine_t *engine,
                                             quixcc_lex_punct_t punct) {
  quixcc_tok_t tok;
  tok.ty = QUIXCC_LEX_PUNCT;
  tok.val.punct = punct;
  tok.loc.line = 0;
  tok.loc.column = 0;
  tok.loc.voucher = QUIXCC_SID_NAN;
  return tok;
}

///=============================================================================
/// END: QUIXCC ENGINE API
///=============================================================================
