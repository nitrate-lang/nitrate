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
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <qast/Parser.h>
#include <quixcc/IR/Q/QIR.h>
#include <quixcc/IR/delta/DeltaIR.h>
#include <quixcc/Library.h>
#include <quixcc/core/Exception.h>
#include <quixcc/core/Logger.h>
#include <quixcc/generate/Generate.h>
#include <quixcc/lexer/Lex.h>
#include <quixcc/llvm/LLVMWrapper.h>
// #include <quixcc/mutate/Routine.h>
#include <quixcc/optimizer/Optimizer.h>
#include <quixcc/preprocessor/Preprocessor.h>
#include <quixcc/preprocessor/QSys.h>
#include <quixcc/solver/Solver.h>
#include <setjmp.h>
#include <signal.h>

#include <atomic>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#define PRUNE_DEBUG_MESSAGES 1

using namespace libquixcc;
using namespace libquixcc::core;

extern std::atomic<bool> g_is_initialized;
extern std::atomic<uint64_t> g_num_of_contexts;
extern std::mutex g_library_lock;
thread_local uint8_t libquixcc::quixcc::g_target_word_size;

static thread_local jmp_buf g_tls_exception;
static thread_local bool g_tls_exception_set = false;

extern void quixcc_print_stacktrace();
extern void quixcc_print_general_fault_message();

static void *safe_realloc(void *ptr, size_t size) {
  void *new_ptr = realloc(ptr, size);
  if (!new_ptr)
    quixcc_panic("out of memory");

  return new_ptr;
}

static char *safe_strdup(const char *str) {
  char *new_str = strdup(str);
  if (!new_str)
    quixcc_panic("out of memory");
  return new_str;
}

static libquixcc::quixcc::quixcc_uuid_t quixcc_uuid() {
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  static_assert((sizeof(libquixcc::quixcc::quixcc_uuid_t::data) | sizeof(uuid.data)) == 16,
                "UUID type size mismatch");

  libquixcc::quixcc::quixcc_uuid_t id;
  memcpy(id.data, uuid.data, sizeof(libquixcc::quixcc::quixcc_uuid_t));

  return id;
}

LIB_EXPORT quixcc_cc_job_t *quixcc_cc_new() {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_cc_new(). "
                 "quitcc_new() "
                 "attempted to compensate for this error, but quitcc_init() failed to "
                 "initialize.");
  }

  /* Acquire a lock on the library state. This is for MT-safe cache management
   */
  std::lock_guard<std::mutex> lock(g_library_lock);

  /* Allocate a new job using raw pointers to be friendly with C */
  quixcc_cc_job_t *job = new quixcc_cc_job_t();
  job->m_id = quixcc_uuid();

  /* Clear structures */
  memset(&job->m_options, 0, sizeof(libquixcc::quixcc::quixcc_cc_options_t));
  memset(&job->m_result, 0, sizeof(quixcc_status_t));

  /* Clear all pointers & values */
  job->m_in = job->m_out = nullptr;
  job->m_priority = 0;
  job->m_debug = job->m_tainted = job->m_running = false;
  job->m_wordsize = 64;
  job->m_triple = llvm::sys::getDefaultTargetTriple();

  qsys::bind_qsyscalls(job);

  /* Set magic structure field */
  job->m_magic = libquixcc::quixcc::JOB_MAGIC;

  g_num_of_contexts++;

  // LoggerConfigure(*job);

  return job;
}

LIB_EXPORT bool quixcc_cc_dispose(quixcc_cc_job_t *job) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_cc_dispose(). "
                 "quixcc_cc_dispose() attempted to compensate for this error, but "
                 "quitcc_init() failed to initialize.");
  }

  /* no-op */
  if (!job)
    return false;

  /* User may have passed an invalid job pointer */
  if (job->m_magic != libquixcc::quixcc::JOB_MAGIC) /* We can't handle this, so panic */
    quixcc_panic("A libquixcc library contract violation occurred: An invalid job "
                 "pointer was passed to quixcc_cc_dispose().");

  bool lockable = job->m_lock.try_lock();
  if (!lockable)
    return false;

  /* Free Options array */
  for (uint32_t i = 0; i < job->m_options.m_count; i++) {
    free((void *)job->m_options.m_options[i]);
    job->m_options.m_options[i] = nullptr;
  }
  if (job->m_options.m_options)
    free(job->m_options.m_options);
  memset(&job->m_options, 0, sizeof(libquixcc::quixcc::quixcc_cc_options_t));

  /* Free messages array */
  for (uint32_t i = 0; i < job->m_result.m_count; i++) {
    quixcc_msg_t *msg = job->m_result.m_messages[i];
    assert(msg);
    assert(msg->message);

    free((void *)msg->message);
    memset(msg, 0, sizeof(quixcc_msg_t));
    free(msg);
  }

  if (job->m_result.m_messages)
    free(job->m_result.m_messages);
  memset(&job->m_result, 0, sizeof(quixcc_status_t));

  /* Clear all pointers & values */
  /* The FILE handles are owned by the caller; we don't close them */
  job->m_in = job->m_out = nullptr;
  job->m_priority = 0;
  job->m_debug = job->m_tainted = false;

  /* Hopefully, this will cache library usage errors */
  job->m_magic = 0;

  job->m_lock.unlock();

  delete job; // Destruct C++ object members implicitly

  g_num_of_contexts--;

  return true;
}

LIB_EXPORT void quixcc_cc_option(quixcc_cc_job_t *job, const char *opt, bool enable) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_cc_option(). "
                 "quixcc_cc_option() attempted to compensate for this error, but "
                 "quitcc_init() failed to initialize.");
  }

  /* no-op */
  if (!job || !opt)
    return;

  /* User may have passed an invalid job pointer */
  if (job->m_magic != libquixcc::quixcc::JOB_MAGIC) /* We can't handle this, so panic */
    quixcc_panic("A libquixcc library contract violation occurred: An invalid job "
                 "pointer was passed to quixcc_cc_option().");

  std::lock_guard<std::mutex> lock(job->m_lock);

  /* Remove the option if it already exists */
  for (uint32_t i = 0; i < job->m_options.m_count; i++) {
    const char *option = job->m_options.m_options[i];
    if (strcmp(option, opt) == 0) {
      free((void *)option);

      job->m_options.m_options[i] = job->m_options.m_options[job->m_options.m_count - 1];
      job->m_options.m_options[job->m_options.m_count - 1] = nullptr;
      job->m_options.m_count--;
      break;
    }
  }

  if (enable) {
    /* Enable it */
    job->m_options.m_options = (const char **)safe_realloc(
        job->m_options.m_options, (job->m_options.m_count + 1) * sizeof(const char *));
    job->m_options.m_options[job->m_options.m_count++] = safe_strdup(opt);
  }
}

LIB_EXPORT void quixcc_cc_source(quixcc_cc_job_t *job, FILE *in, const char *filename) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_cc_source(). "
                 "quixcc_cc_source() attempted to compensate for this error, but "
                 "quitcc_init() failed to initialize.");
  }

  /* no-op */
  if (!job || !in || !filename)
    return;

  /* User may have passed an invalid job pointer */
  if (job->m_magic != libquixcc::quixcc::JOB_MAGIC) /* We can't handle this, so panic */
    quixcc_panic("A libquixcc library contract violation occurred: An invalid job "
                 "pointer was passed to quixcc_cc_source().");

  std::lock_guard<std::mutex> lock(job->m_lock);

  /* Its the callers responsibility to make sure this is a valid file */
  if (fseek(in, 0, SEEK_SET) != 0)
    abort();

  job->m_in = in;
  job->m_filename.push(filename);
}

LIB_EXPORT bool quixcc_cc_target(quixcc_cc_job_t *job, const char *_llvm_triple) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_target(). "
                 "quixcc_target() attempted to compensate for this error, but "
                 "quitcc_init() failed to initialize.");
  }

  /* no-op */
  if (!job || !_llvm_triple)
    return false;

  /* User may have passed an invalid job pointer */
  if (job->m_magic != libquixcc::quixcc::JOB_MAGIC) /* We can't handle this, so panic */
    quixcc_panic("A libquixcc library contract violation occurred: An invalid job "
                 "pointer was passed to quixcc_target().");

  std::lock_guard<std::mutex> lock(job->m_lock);

  std::string new_triple, err;

  try {
    /* An empty string means use the default target */
    if (_llvm_triple[0] == '\0')
      new_triple = llvm::sys::getDefaultTargetTriple();
    else
      new_triple = _llvm_triple;
  } catch (std::exception &) {
    return false;
  }

  if (llvm::TargetRegistry::lookupTarget(new_triple, err) == nullptr) {
    LOG(ERROR) << log::raw << "invalid target triple: " << new_triple << std::endl;
    return false;
  }

  job->m_triple = new_triple;

  return true;
}

LIB_EXPORT bool quixcc_cc_cpu(quixcc_cc_job_t *job, const char *cpu) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_cpu(). "
                 "quixcc_cpu() "
                 "attempted to compensate for this error, but quitcc_init() failed to "
                 "initialize.");
  }

  /* no-op */
  if (!job || !cpu)
    return false;

  /* User may have passed an invalid job pointer */
  if (job->m_magic != libquixcc::quixcc::JOB_MAGIC) /* We can't handle this, so panic */
    quixcc_panic("A libquixcc library contract violation occurred: An invalid job "
                 "pointer was passed to quixcc_cpu().");

  std::lock_guard<std::mutex> lock(job->m_lock);

  job->m_cpu = cpu;

  return true;
}

LIB_EXPORT void quixcc_cc_output(quixcc_cc_job_t *job, FILE *out, FILE **old_out) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_cc_output(). "
                 "quixcc_cc_output() attempted to compensate for this error, but "
                 "quitcc_init() failed to initialize.");
  }

  /* no-op */
  if (!job || !out)
    return;

  /* User may have passed an invalid job pointer */
  if (job->m_magic != libquixcc::quixcc::JOB_MAGIC) /* We can't handle this, so panic */
    quixcc_panic("A libquixcc library contract violation occurred: An invalid job "
                 "pointer was passed to quixcc_cc_output().");

  std::lock_guard<std::mutex> lock(job->m_lock);

  if (old_out)
    *old_out = job->m_out;

  job->m_out = out;
}

static std::string get_datetime() {
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
  return buf;
}

static bool quixcc_mutate_ptree(quixcc_cc_job_t *job, qast::Node *ptree) {
  // Mutation mutator;
  // mutator.add_routine(mutate::MethodToFunc);
  // mutator.add_routine(mutate::DiscoverNamedConstructs);
  // mutator.add_routine(mutate::ResolveNamedConstructs);
  // mutator.add_routine(mutate::SubsystemCollapse);
  // mutator.run(job, ptree);

  return true;
}

static bool quixcc_qualify(quixcc_cc_job_t *job, std::unique_ptr<ir::q::QModule> &module) {
  /// TODO: implement semantic analysis
  return true;
}

static bool get_include_directories(quixcc_cc_job_t *job, std::set<std::string> &dirs) {
  for (uint32_t i = 0; i < job->m_options.m_count; i++) {
    std::string option = job->m_options.m_options[i];

    if (option.size() < 3)
      continue;

    if (option.starts_with("-I")) {
      dirs.insert(option.substr(2));
      continue;
    }
  }

  return true;
}

static bool verify_user_constant_string(const std::string &s) {
  int state = 0;

  for (size_t i = 1; i < s.size() - 1; i++) {
    char c = s[i];

    switch (state) {
    case 0:
      if (c == '\\')
        state = 1;
      if (c == '"')
        return false;
      if (!std::isprint(c))
        return false;
      break;
    case 1:
      if (c == '"')
        state = 0;
      else if (c == '\\')
        state = 0;
      else if (c == 'n' || c == 'r' || c == 't' || c == '0' || c == 'x')
        state = 0;
      else
        return false;
      break;
    }
  }

  return state == 0;
}

static bool verify_user_constant(const std::string &key, const std::string &value) {
  if (key.empty())
    return false;

  bool key_valid = std::isalpha(key[0]) || key[0] == '_';

  for (char c : key) {
    if (!std::isalnum(c) && c != '_') {
      key_valid = false;
      break;
    }
  }

  if (!key_valid)
    return false;

  if (value.empty())
    return true;

  if (value == "true" || value == "false")
    return true;

  bool is_int = true;
  for (char c : value) {
    if (!std::isdigit(c)) {
      is_int = false;
      break;
    }
  }

  if (is_int)
    return true;

  return verify_user_constant_string(value);
}

static bool get_compile_time_user_constants(quixcc_cc_job_t *job,
                                            std::map<std::string, std::string> &constants) {
  auto argmap = job->m_argset;

  for (auto &arg : argmap) {
    if (arg.first.starts_with("-D")) {
      std::string key = arg.first.substr(2);
      std::string value = arg.second;

      if (!verify_user_constant(key, value)) {
        LOG(ERROR) << log::raw << "invalid user constant: " << key << std::endl;
        return false;
      }
      constants[key] = value;
    }
  }

  return true;
}

static bool get_env_constants(quixcc_cc_job_t *job, std::map<std::string, std::string> &constants) {
  for (char **env = environ; *env; env++) {
    std::string var = *env;
    if (var.find("QUIXCC_VAR_") == 0) {
      size_t pos = var.find('=');
      if (pos != std::string::npos) {
        std::string key = var.substr(4, pos - 4);
        std::string value = var.substr(pos + 1);

        if (!verify_user_constant(key, value)) {
          LOG(ERROR) << log::raw << "invalid user constant: " << key << std::endl;
          return false;
        }
        constants[key] = value;
      }
    }
  }

  return true;
}

bool preprocessor_config(quixcc_cc_job_t *job, std::unique_ptr<PrepEngine> &prep) {
  std::set<std::string> dirs;
  std::map<std::string, std::string> constants;

  prep->setup();

  if (!get_include_directories(job, dirs)) {
    LOG(ERROR) << "failed to get include directories" << std::endl;
    return false;
  }
  for (auto &dir : dirs)
    prep->addpath(dir);
  if (!get_compile_time_user_constants(job, constants)) {
    LOG(ERROR) << "failed to get compile time user constants" << std::endl;
    return false;
  }
  for (auto &constant : constants)
    prep->set_static(constant.first, constant.second);

  if (!get_env_constants(job, constants)) {
    LOG(ERROR) << "failed to get environment constants" << std::endl;
    return false;
  }
  for (auto &constant : constants)
    prep->set_static(constant.first, constant.second);

  if (!prep->set_source(job->m_in, job->m_filename.top())) {
    LOG(ERROR) << "failed to set source" << std::endl;
    return false;
  }

  return true;
}

static bool compile(quixcc_cc_job_t *job) {
  // auto ptree = std::make_shared<Ptree>();
  qast::Block *ptree = nullptr;

  if (job->has("-emit-prep")) {
    LOG(DEBUG) << "Preprocessing only" << std::endl;
    std::unique_ptr<PrepEngine> prep = std::make_unique<PrepEngine>(*job);
    if (!preprocessor_config(job, prep))
      return false;
    if (job->has("-fkeep-comments")) {
      prep->comments(false);
    }

    // Generate output
    std::string tmp;
    std::optional<Token> tok;
    while ((tok = prep->next())->type() != tEofF) {
      tmp = tok->serialize();
      fwrite(tmp.c_str(), 1, tmp.size(), job->m_out);
      fputc('\n', job->m_out);
    }
    fflush(job->m_out);
    return true;
  }

  if (job->has("-emit-tokens")) {
    LOG(DEBUG) << "Lexing only" << std::endl;
    StreamLexer lexer;
    if (job->has("-fkeep-comments")) {
      lexer.comments(false);
    }

    if (!lexer.set_source(job->m_in, job->m_filename.top())) {
      LOG(ERROR) << "failed to set source" << std::endl;
      return false;
    }

    // Generate output
    std::string tmp;
    Token tok;
    while ((tok = lexer.next()).type() != tEofF) {
      tmp = tok.serialize();
      fwrite(tmp.c_str(), 1, tmp.size(), job->m_out);
      fputc('\n', job->m_out);
    }
    fflush(job->m_out);
    return true;
  }

  {
    ///=========================================
    /// BEGIN: PREPROCESSOR/LEXER
    auto prep = std::make_unique<PrepEngine>(*job);
    LOG(DEBUG) << "Preprocessing source" << std::endl;
    if (!preprocessor_config(job, prep))
      return false;
    if (job->has("-fkeep-comments")) {
      prep->comments(false);
    }
    LOG(DEBUG) << "Finished preprocessing source" << std::endl;
    /// END:   PREPROCESSOR/LEXER
    ///=========================================

    ///=========================================
    /// BEGIN: PARSER
    LOG(DEBUG) << "Building Ptree 1" << std::endl;
    if (!qast::parser::parse(*job, prep.get(), &ptree, false))
      return false;
    LOG(DEBUG) << "Finished building Ptree 1" << std::endl;

    if (job->has("-emit-parse")) {
      auto serial = ptree->to_string(job->has("-emit-minify"));
      if (fwrite(serial.c_str(), 1, serial.size(), job->m_out) != serial.size())
        return false;
      fflush(job->m_out);

      LOG(DEBUG) << "Parse only" << std::endl;
      return true;
    }
    /// END:   PARSER
    ///=========================================
  } /* Destruct PrepEngine */

  ///=========================================
  /// BEGIN: INTERMEDIATE PROCESSING
  if (!quixcc_mutate_ptree(job, ptree) || job->m_tainted)
    return false;
  /// END:   INTERMEDIATE PROCESSING
  ///=========================================

  ///=========================================
  /// BEGIN: OPTIMIZATION PIPELINE
  auto QIR = std::make_unique<ir::q::QModule>(job->m_filename.top());
  if (!QIR->from_ptree(job, ptree)) /* This will modify the Ptree */
    return false;

  if (!job->has("-fno-check")) // -fno-check disables semantic analysis
  {
    ///=========================================
    /// BEGIN: SEMANTIC ANALYSIS
    LOG(DEBUG) << "Performing semantic analysis" << std::endl;
    if (!quixcc_qualify(job, QIR)) {
      LOG(ERROR) << "failed to qualify program" << std::endl;
      return false;
    }
    LOG(DEBUG) << "Finished semantic analysis" << std::endl;
    /// END:   SEMANTIC ANALYSIS
    ///=========================================
  }

  {
    ///=========================================
    /// BEGIN: INTERMEDIATE PROCESSING
    auto solvermgr = solver::SolPassMgrFactory::CreateStandard();
    if (!solvermgr->run_passes(*job, QIR)) {
      LOG(ERROR) << "failed to run solver passes" << std::endl;
      return false;
    }
    /// END:   INTERMEDIATE PROCESSING
    ///=========================================
  }

  {
    LOG(DEBUG) << "Optimizing Quix IR" << std::endl;

    using namespace optimizer;
    OptLevel opt;
    if (job->has("-O0"))
      opt = OptLevel::O0;
    else if (job->has("-O1"))
      opt = OptLevel::O1;
    else if (job->has("-O2"))
      opt = OptLevel::O2;
    else if (job->has("-O3"))
      opt = OptLevel::O3;
    else if (job->has("-Os"))
      opt = OptLevel::Os;
    else if (job->has("-Oz"))
      opt = OptLevel::Oz;
    else
      opt = OptLevel::O2;

    std::unique_ptr<OptPassManager> architecture_opt, behavior_opt, general_opt;

    architecture_opt = OptPassMgrFactory::create(opt, OptType::Architecture);
    behavior_opt = OptPassMgrFactory::create(opt, OptType::Behavior);
    general_opt = OptPassMgrFactory::create(opt, OptType::General);

    architecture_opt->optimize_phase_order();
    behavior_opt->optimize_phase_order();
    general_opt->optimize_phase_order();

    if (!architecture_opt->run_passes(*job, QIR)) {
      LOG(FATAL) << "Architecture optimization failed; aborting" << std::endl;
      return false;
    }

    if (!behavior_opt->run_passes(*job, QIR)) {
      LOG(FATAL) << "Behavior optimization failed; aborting" << std::endl;
      return false;
    }

    if (!general_opt->run_passes(*job, QIR)) {
      LOG(FATAL) << "General optimization failed; aborting" << std::endl;
      return false;
    }

    LOG(DEBUG) << "Optimization pipeline complete" << std::endl;
  }

  if (job->has("-emit-quix-ir")) {
    auto serial = QIR->to_string();
    if (fwrite(serial.c_str(), 1, serial.size(), job->m_out) != serial.size())
      return false;
    fflush(job->m_out);

    LOG(DEBUG) << "Quix IR only" << std::endl;
    return true;
  }

  auto DIR = std::make_unique<ir::delta::IRDelta>(job->m_filename.top());
  if (!DIR->from_qir(job, QIR))
    return false;

  if (job->has("-emit-delta-ir")) {
    auto serial = DIR->to_string();
    if (fwrite(serial.c_str(), 1, serial.size(), job->m_out) != serial.size())
      return false;
    fflush(job->m_out);

    LOG(DEBUG) << "Delta IR only" << std::endl;
    return true;
  }
  /// END:   OPTIMIZATION PIPELINE
  ///=========================================

  ///=========================================
  /// BEGIN: GENERATOR
  LOG(DEBUG) << "Generating output" << std::endl;
  if (!codegen::generate(*job, DIR)) /* Apply LLVM optimizations */
  {
    LOG(ERROR) << "failed to generate output" << std::endl;
    return false;
  }
  /// END: GENERATOR
  ///=========================================

  fflush(job->m_out);
  return true;
}

static bool verify_build_option(const std::string &option, const std::string &value) {
  const static std::set<std::string> static_options = {
      "-S",              // assembly output
      "-emit-tokens",    // lexer output (no preprocessing)
      "-emit-prep",      // preprocessor/Lexer output
      "-emit-parse",     // parse tree output
      "-emit-minify",    // minified output
      "-emit-ir",        // IR output
      "-emit-quix-ir",   // Quix IR output
      "-emit-delta-ir",  // Delta IR output
      "-emit-c11",       // C11 output
      "-emit-bc",        // bitcode output
      "-c",              // compile only
      "-O0",             // optimization levels
      "-O1",             // optimization levels
      "-O2",             // optimization levels
      "-O3",             // optimization levels
      "-Os",             // optimization levels
      "-g",              // debug information
      "-flto",           // link time optimization
      "-fPIC",           // position independent code
      "-fPIE",           // position independent executable
      "-v",              // verbose
      "-fcoredump",      // dump core on crash
      "-fkeep-comments", // keep comments from scanner
  };
  const static std::vector<std::pair<std::regex, std::regex>> static_regexes = {
      {std::regex("-D[a-zA-Z_][a-zA-Z0-9_]*"), std::regex("[a-zA-Z0-9_ ]*")},
      {std::regex("-I.+"), std::regex("")},
      {std::regex("-fno-auto-impl"), std::regex("function|group|struct|region|union")},
  };

  if (static_options.contains(option))
    return true;

  for (auto &regex : static_regexes) {
    if (std::regex_match(option, regex.first) && std::regex_match(value, regex.second))
      return true;
  }

  return false;
}

static bool verify_build_option_conflicts(quixcc_cc_job_t *job) {
  (void)job;
  return true;
}

static bool build_argmap(quixcc_cc_job_t *job) {
  const static std::set<char> okay_prefixes = {'f', 'O', 'P', 'I', 'e', 'D', 'W',
                                               'm', 'c', 'S', 'g', 's', 'v'};

  for (uint32_t i = 0; i < job->m_options.m_count; i++) {
    std::string option = job->m_options.m_options[i];

    if (option.size() == 2 && option[0] == '-' && !okay_prefixes.contains(option[1])) {
      LOG(ERROR) << log::raw << "invalid build option: " << option << std::endl;
      return false;
    }

    size_t pos = option.find('=');
    std::string key = option.substr(0, pos);
    std::string value;

    if (pos != std::string::npos)
      value = option.substr(pos + 1);

    if (!verify_build_option(key, value)) {
      LOG(ERROR) << log::raw << "invalid build option: " << key << std::endl;
      return false;
    }

    if (key == "-v" && value.empty())
      job->m_debug = true;

    job->m_argset.push_back({key, value});

    if (!value.empty())
      LOG(DEBUG) << log::raw << "Added switch entry: " << key << "=" << value << std::endl;
    else
      LOG(DEBUG) << log::raw << "Added switch entry: " << key << std::endl;
  }

  return verify_build_option_conflicts(job);
}

void quixcc_fault_handler(int sig) {
  /*
      Lock all threads to prevent multiple error messages
  */
  static std::mutex mutex;
  std::lock_guard<std::mutex> lock(mutex);

  signal(SIGINT, SIG_IGN);
  signal(SIGILL, SIG_IGN);
  signal(SIGFPE, SIG_IGN);
  signal(SIGSEGV, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  signal(SIGABRT, SIG_IGN);

  switch (sig) {
  case SIGINT:
    std::cerr << "SIGINT: libquixcc was interrupted. compilation aborted." << std::endl;
    break;
  case SIGILL:
    std::cerr << "SIGILL: libquixcc tried to execute an illegal instruction. "
                 "compilation aborted."
              << std::endl;
    break;
  case SIGFPE:
    std::cerr << "SIGFPE: libquixcc tried to execute an illegal floating "
                 "point operation. compilation aborted."
              << std::endl;
    break;
  case SIGSEGV:
    std::cerr << "SIGSEGV: libquixcc tried to access an invalid memory location "
                 "leading to a segmentation fault. compilation aborted."
              << std::endl;
    break;
  case SIGTERM:
    std::cerr << "SIGTERM: libquixcc was terminated. compilation aborted." << std::endl;
    break;
  case SIGABRT:
    std::cerr << "SIGABRT: libquixcc encountered an internal error. "
                 "compilation aborted."
              << std::endl;
    break;
  default:
    std::cerr << "libquixcc encountered an unexpected signal. compilation aborted." << std::endl;
    break;
  }

  std::cerr << '\n';
  quixcc_print_general_fault_message();
  std::cerr << "\n";
  quixcc_print_stacktrace();
  std::cerr << "\n";

  LOG(WARN) << "Attemping to recover from `fatal` error by `longjmp()`ing into "
               "another thread which is hopefully okay."
            << std::endl;
  LOG(WARN) << "INTERNAL COMPILER NOTICE: THIS PROCESS IS HAS BEEN TAINTED. IT "
               "IS NOW A WIRED-MACHINE. TERMINATE ASAP."
            << std::endl;

  if (g_tls_exception_set) {
    /* Attempt to recover by longjmp'ing into the job-execution thread */
    /* I say `attempt` because it feels like there are many ways this could go
       wrong, even in a static-binary. */

    // Details @ https://man7.org/linux/man-pages/man3/longjmp.3.html
    longjmp(g_tls_exception, 1);
  } else {
    abort();
  }
}

static bool execute_job(quixcc_cc_job_t *job) {
  if (!job->m_in || !job->m_out || job->m_filename.empty())
    return false;

  try {
    LoggerConfigure(*job);
    job->m_inner.setup(job->m_filename.top());

    LOG(DEBUG) << log::raw << "Starting quixcc run @ " << get_datetime() << std::endl;

    if (!build_argmap(job)) {
      LOG(ERROR) << "failed to build argmap" << std::endl;
      return false;
    }
  } catch (Exception &e) {
    LOG(FAILED) << "Compilation failed: " << e.what() << std::endl;
    return false;
  } catch (std::exception &e) {
    LOG(FAILED) << "Compilation failed: " << e.what() << std::endl;
    return false;
  } catch (...) {
    LOG(FAILED) << "Compilation failed" << std::endl;
    return false;
  }

  if (job->has("-fcoredump")) {
    if (!compile(job))
      LOG(ERROR) << "Compilation failed" << std::endl;

    LOG(DEBUG) << "Compilation successful" << std::endl;
    LOG(DEBUG) << log::raw << "Finished quixcc run @ " << get_datetime() << std::endl;

    job->m_result.m_success = true;
    return true;
  } else {
    try {
      if (!compile(job))
        LOG(ERROR) << "Compilation failed" << std::endl;

      LOG(DEBUG) << "Compilation successful" << std::endl;
      LOG(DEBUG) << log::raw << "Finished quixcc run @ " << get_datetime() << std::endl;

      job->m_result.m_success = true;
      return true;
    } catch (ProgrammaticPreprocessorException &) {
      LOG(FAILED) << "Compilation was programmatically aborted while "
                     "preprocessing source"
                  << std::endl;
      return false;
    } catch (PreprocessorException &e) {
      LOG(FAILED) << log::raw << "Compilation was aborted while preprocessing source: " << e.what()
                  << std::endl;
      return false;
    } catch (ParseException &e) {
      LOG(FAILED) << log::raw << "Compilation was aborted while parsing source: " << e.what()
                  << std::endl;
      return false;
    } catch (Exception &e) {
      LOG(FAILED) << log::raw << "Compilation failed" << std::endl;
      return false;
    } catch (std::runtime_error &e) {
      LOG(FAILED) << log::raw << "Compilation failed: " << e.what() << std::endl;
      return false;
    } catch (std::exception &e) {
      LOG(FAILED) << log::raw << "Compilation failed: " << e.what() << std::endl;
      return false;
    } catch (const char *e) {
      LOG(FAILED) << log::raw << "Compilation failed: " << e << std::endl;
      return false;
    } catch (...) {
      LOG(FAILED) << "Compilation failed" << std::endl;
      return false;
    }
  }

  return false;
}

static uint8_t get_target_word_size(quixcc_cc_job_t *job) {
  //
  return 8;
}

LIB_EXPORT bool quixcc_cc_run(quixcc_cc_job_t *job) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_run(). "
                 "quixcc_run() "
                 "attempted to compensate for this error, but quitcc_init() failed to "
                 "initialize.");
  }

  /* no-op */
  if (!job)
    return false;

  /* User may have passed an invalid job pointer */
  if (job->m_magic != libquixcc::quixcc::JOB_MAGIC) /* We can't handle this, so panic */
    quixcc_panic("A libquixcc library contract violation occurred: An invalid job "
                 "pointer was passed to quixcc_run().");

  std::lock_guard<std::mutex> lock(job->m_lock);
  static std::mutex siglock;

  /* Install signal handlers to catch fatal memory errors */
  siglock.lock();
  sighandler_t old_handlers[4];
  old_handlers[0] = signal(SIGILL, quixcc_fault_handler);
  old_handlers[1] = signal(SIGFPE, quixcc_fault_handler);
  old_handlers[2] = signal(SIGSEGV, quixcc_fault_handler);
  old_handlers[3] = signal(SIGABRT, quixcc_fault_handler);
  siglock.unlock();

  /* Every compiler job must have its own thread-local storage */
  bool status = false;
  std::thread t([&] {
    /* This is a dirty hack to `catch` segfaults and still be able to return */

    bool has_core_dump = false;
    for (uint32_t i = 0; i < job->m_options.m_count; i++) {
      if (std::string(job->m_options.m_options[i]) == "-fcoredump") {
        has_core_dump = true;
        break;
      }
    }

    /* Set the target word size */
    job->m_wordsize = get_target_word_size(job);
    libquixcc::quixcc::g_target_word_size = job->m_wordsize;

    if (!has_core_dump) {
      /* We capture the local environment: (stack pointer, PC, registers, etc.)
       */
      if (setjmp(g_tls_exception) == 0) {
        /* Okay, now the confusing part.
           The above call to `setjmp()` captures the 'point-of-return' or
           'jump-point' at the exact location it is called from.

           This fact requires us to install logic to detect whether or not we on
           the non-setjmp (normal/common) path or the setjmp path to avoid
           recursion and chaos.

           If the thread-local handler is not set, we are on the normal path.
        */
        g_tls_exception_set = true;
        status = execute_job(job);
      }
    } else {
      status = execute_job(job);
    }
  });

  t.join();

  /* Restore signal handlers */
  siglock.lock();
  signal(SIGILL, old_handlers[0]);
  signal(SIGFPE, old_handlers[1]);
  signal(SIGSEGV, old_handlers[2]);
  signal(SIGABRT, old_handlers[3]);
  siglock.unlock();

  return status;
}

LIB_EXPORT const quixcc_status_t *quixcc_cc_status(quixcc_cc_job_t *job) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_status(). "
                 "quixcc_status() attempted to compensate for this error, but "
                 "quitcc_init() failed to initialize.");
  }

  /* no-op */
  if (!job)
    return nullptr;

  /* User may have passed an invalid job pointer */
  if (job->m_magic != libquixcc::quixcc::JOB_MAGIC) /* We can't handle this, so panic */
    quixcc_panic("A libquixcc library contract violation occurred: An invalid job "
                 "pointer was passed to quixcc_status().");

  bool lockable = job->m_lock.try_lock();
  if (!lockable)
    return nullptr;

  quixcc_status_t *result = &job->m_result;

#if PRUNE_DEBUG_MESSAGES
  // Remove debug messages
  uint32_t i = 0;
  while (i < result->m_count) {
    quixcc_msg_t *msg = result->m_messages[i];
    if (msg->m_level != QUIXCC_DEBUG) {
      i++;
      continue;
    }

    if (msg->message)
      free((void *)msg->message);
    msg->message = nullptr;
    free(msg);

    memmove(&result->m_messages[i], &result->m_messages[i + 1],
            (result->m_count - i - 1) * sizeof(quixcc_msg_t *));
    result->m_count--;
  }
#endif

  job->m_lock.unlock();

  return result;
}

LIB_EXPORT char **quixcc_cc_compile(FILE *in, FILE *out, const char *options[]) {
  if (!g_is_initialized && !quixcc_lib_init()) {
    quixcc_panic("A libquixcc library contract violation occurred: A successful call to "
                 "quixcc_lib_init() is required before calling quixcc_compile(). "
                 "quixcc_compile() attempted to compensate for this error, but "
                 "quitcc_init() failed to initialize.");
  }

  /* no-op */
  if (!in || !out)
    return nullptr;

  quixcc_cc_job_t *job = quixcc_cc_new();

  /* No need for locks here */
  quixcc_cc_source(job, in, "stdin");
  quixcc_cc_output(job, out, nullptr);

  /* Set options */
  if (options) {
    for (uint32_t i = 0; options[i]; i++)
      quixcc_cc_option(job, options[i], true);
  }

  /* Run the job */
  if (quixcc_cc_run(job)) {
    quixcc_cc_dispose(job);
    return nullptr; // success
  }

  /* Copy messages */
  char **messages = (char **)malloc((job->m_result.m_count + 1) * sizeof(char *));
  for (uint32_t i = 0; i < job->m_result.m_count; i++)
    messages[i] = safe_strdup(job->m_result.m_messages[i]->message);
  messages[job->m_result.m_count] = nullptr;

  quixcc_cc_dispose(job);
  return messages;
}
