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
#include <core/Logger.h>
#include <libquixcc.h>
#include <preprocessor/Preprocessor.h>
#include <quixcc/EngineAPI.h>
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
                                      quixcc_message_t mode,
                                      const char *message) {
  if (!engine) quixcc_panic("quixcc_engine_message: engine is NULL");
  if (!message) quixcc_panic("quixcc_engine_message: message is NULL");

  auto job = quixcc_engine_job(engine);

  /*================ CONFIGURE LOGGER (IF NOT ALREADY) =================*/
  LoggerConfigure(*job);

  /*================ LOG MESSAGE =================*/
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

bool qsys_foo(quixcc_engine_t *engine, uint32_t num, quixcc_expr_t **args,
              uint32_t argc) {
  if (argc != 3) {
    quixcc_engine_message(engine, QUIXCC_MESSAGE_ERROR,
                          "qsys_foo: expected 3 arguments");
    return false;
  }

  std::cerr << "QSYS FOO INVOKED" << std::endl;

  return true;
}

bool qsys_bar(quixcc_engine_t *engine, uint32_t num, quixcc_expr_t **args,
              uint32_t argc) {
  if (argc != 3) {
    quixcc_engine_message(engine, QUIXCC_MESSAGE_ERROR,
                          "qsys_barr: expected 3 arguments");
    return false;
  }

  std::cerr << "QSYS BAR INVOKED" << std::endl;

  return true;
}

///=============================================================================
/// END: QUIXCC ENGINE API
///=============================================================================
