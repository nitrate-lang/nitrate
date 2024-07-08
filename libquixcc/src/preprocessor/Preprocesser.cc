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
#include <preprocessor/Preprocessor.h>

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <regex>
#include <stdexcept>

#define QUIX_HEADER_EXTENSION ".qh"
#define QUIX_STATICS_FILE "~statics.qh"

using namespace libquixcc;

PrepEngine::PrepEngine(quixcc_job_t &_job) {
  /*============== INITIALIZE PREPROCESSOR ================*/

  job = &_job;
  m_file = nullptr;
  m_we_own_file = false;
  m_we_are_root = true;
  m_expansion_enabled = true;

  /* All n-th order metacode shall have access to the statics */
  if (m_we_are_root) {
    m_statics = std::make_shared<std::map<std::string, std::string>>();
  }
}

PrepEngine::~PrepEngine() {
  /*============== CLEANUP ================*/
  if (m_we_own_file) {
    fclose(m_file);
  }

  job->m_filename.pop();
}

void PrepEngine::setup() {}

void PrepEngine::addpath(const std::string &path) {
  /*============== ADD INCLUDE PATH ================*/
  m_include_dirs.insert(path);

  /*============== RECACULATE INCLUDE PATH ================*/
  include_path.clear();
  for (auto &dir : m_include_dirs) {
    include_path += std::filesystem::absolute(dir).string();

    if (dir != *m_include_dirs.rbegin()) {
      include_path += ":";
    }
  }
}

void libquixcc::PrepEngine::set_include_path(libquixcc::PrepEngine::rstr path) {
  /*============== SET INCLUDE PATH ================*/
  include_path = path;
  m_include_dirs.clear();

  std::string::size_type start = 0;
  std::string::size_type end = 0;

  while ((end = path.find(':', start)) != std::string::npos) {
    m_include_dirs.insert(path.substr(start, end - start));
    start = end + 1;
  }

  m_include_dirs.insert(path.substr(start));
}

PrepEngine::Entry PrepEngine::build_statics_decl() {
  /// TODO: detect actual platform and do this properly

  /*============== BUILD STATIC DECLARATIONS ================*/
  m_statics->insert({"CPU_WORD_SIZE", std::to_string(job->m_wordsize)});

  std::stringstream declcode;
  declcode << "@use \"v1.0\";\n";
  declcode << "/* BEGIN AUTOGENERATED STATIC DECLARATIONS */\n";
  // for (auto &[key, value] : *m_statics) {
  //   declcode << "@define " << key << " = " << value << ";\n";

  //   LOG(DEBUG) << log::raw << "Adding static declaration: " << key << " = "
  //              << value << std::endl;
  // }
  switch (job->m_wordsize) {
    case 8:
      declcode << "type usize = u8;\n";
      declcode << "type isize = i8;\n";
      break;
    case 16:
      declcode << "type usize = u16;\n";
      declcode << "type isize = i16;\n";
      break;
    case 32:
      declcode << "type usize = u32;\n";
      declcode << "type isize = i32;\n";
      break;
    case 64:
      declcode << "type usize = u64;\n";
      declcode << "type isize = i64;\n";
      break;
    case 128:
      declcode << "type usize = u128;\n";
      declcode << "type isize = i128;\n";
      break;
    default:
      throw std::runtime_error("Unsupported word size");
  }

  declcode << "/* END AUTOGENERATED STATIC DECLARATIONS */\n";
  declcode << "@use \"undef\";\n";

  m_content = declcode.str();

  /*============== CREATE STATIC DECLARATION "FILE" ================*/
  FILE *f = nullptr;

  f = fmemopen((void *)m_content.c_str(), m_content.size(), "r");
  if (!f) {
    throw std::runtime_error("Failed to create statics declaration file");
  }

  /*============== SET SOURCE TO STATIC DECLARATION "FILE" ================*/
  try {
    auto l = std::make_unique<StreamLexer>();
    if (!l->set_source(f, QUIX_STATICS_FILE)) {
      fclose(f);
      throw std::runtime_error("Failed to set source for statics declaration");
    }
    return Entry(std::move(l), QUIX_STATICS_FILE, f);
  } catch (std::exception &e) {
    fclose(f);
    throw e;
  }
}

std::unique_ptr<PrepEngine> PrepEngine::clone() const {
  /*============== CLONE PREPROCESSOR ================*/

  std::unique_ptr<PrepEngine> l(new PrepEngine(*job));
  l->m_include_dirs = m_include_dirs;
  l->m_already_included = m_already_included;
  l->m_include_files = m_include_files;
  l->include_path = include_path;
  l->m_statics = m_statics;
  l->m_we_are_root = false;

  return l;
}

void libquixcc::PrepEngine::disable_expansion() { m_expansion_enabled = false; }

void libquixcc::PrepEngine::enable_expansion() { m_expansion_enabled = true; }

bool PrepEngine::set_source(FILE *src, const std::string &filename) {
  /*============== SET SOURCE FILE ================*/
  auto l = std::make_unique<StreamLexer>();
  if (!l->set_source(src, filename)) {
    LOG(ERROR) << "Failed to set source" << std::endl;
    return false;
  }
  m_stack.push({std::move(l), filename, src});
  job->m_filename.push(filename);

  /*============== ADD STATIC DECLARATIONS FOR ROOT ================*/
  if (m_we_are_root) {
    m_stack.push(build_statics_decl());
    job->m_filename.push(QUIX_STATICS_FILE);
    m_include_files.push_back(QUIX_STATICS_FILE);
  }

  return true;
}

void PrepEngine::set_source(PrepEngine::rstr src, PrepEngine::rstr filename) {
  /*============== CREATE MEMORY FILE ================*/
  m_content = src;

  FILE *f = fmemopen((void *)m_content.c_str(), m_content.size(), "r");
  if (!f) {
    LOG(ERROR) << "Failed to create memory file" << std::endl;
    return;
  }

  /*============== SET SOURCE ================*/
  try {
    m_we_own_file = true;
    m_file = f;

    if (!set_source(f, filename)) {
      fclose(f);
      LOG(ERROR) << "Failed to set source" << std::endl;
    }
  } catch (std::exception &e) {
    fclose(f);
    throw e;
  }
}

quixcc_job_t *libquixcc::PrepEngine::get_job() const { return job; }

void PrepEngine::push(Token tok) {
  /*============== PUBLIC INTERFACE FOR PUSHING TOKEN ================*/
  m_pushed.push(tok);
  if (m_tok) m_pushed.push(m_tok.value());
  m_tok = std::nullopt;
}

void PrepEngine::emit(const Token &tok) {
  /*============== EMIT TOKEN FROM PREPROCESSOR ================*/
  m_buffer.push(tok);
}

Token PrepEngine::next() {
  /*============== PUBLIC INTERFACE FOR NEXT TOKEN ================*/
  Token tok = peek();
  m_tok = std::nullopt;
  return tok;
}

const Token &PrepEngine::peek() {
  /*============== PUBLIC INTERFACE FOR PEEKING TOKEN ================*/
  if (m_tok) return m_tok.value();

  if (!m_pushed.empty()) {
    m_tok = m_pushed.front();
    m_pushed.pop();
  } else {
    m_tok = read_token();
  }

  return m_tok.value();
}

void PrepEngine::set_static(PrepEngine::rstr name, PrepEngine::rstr value) {
  /*============== SET DEFINE VALUE ================*/
  (*m_statics)[name] = value;
}

bool PrepEngine::get_static(const std::string &name, std::string &value) const {
  /*============== GET DEFINE VALUE ================*/
  if (!m_statics->contains(name)) {
    return false;
  }
  value = m_statics->at(name);
  return true;
}

bool PrepEngine::handle_import(const Token &input_tok) {
  /*============== HANDLE IMPORT DIRECTIVE ================*/
  auto lexer = clone();
  lexer->set_source(input_tok.as<std::string>().substr(6), "import");

  /*============== PARSE IMPORT DIRECTIVE ================*/
  Token tok = lexer->next();
  if (tok.type != TT::Identifier) {
    LOG(ERROR) << "Expected identifier after import" << tok << std::endl;
    return false;
  }

  /*============== PARSE FILENAME ================*/
  std::string filename =
      std::regex_replace(tok.as<std::string>(), std::regex("::"), "/");

  /*============== PARSE LIBRARY VERSION ================*/
  tok = lexer->peek();
  if (tok.is<Punctor>(Punctor::OpenParen)) {
    lexer->next();
    tok = lexer->next();
    if (tok.type == TT::Identifier) {
      filename = filename + "/" + tok.as<std::string>() + QUIX_HEADER_EXTENSION;
    } else if (tok.type == TT::Integer) {
      filename = filename + "/" + tok.as<std::string>() + QUIX_HEADER_EXTENSION;
    } else {
      LOG(ERROR) << "Expected identifier or integer literal after import" << tok
                 << std::endl;
      return false;
    }

    tok = lexer->next();
    if (!tok.is<Punctor>(Punctor::CloseParen)) {
      LOG(ERROR) << "Expected closing parenthesis after import" << tok
                 << std::endl;
      return false;
    }
  } else {
    filename += QUIX_HEADER_EXTENSION;
  }

  LOG(DEBUG) << log::raw << "Requested import of file: " << filename
             << std::endl;

  /*============== SKIP SEMICOLON ================*/
  tok = lexer->next();
  if (!tok.is<Punctor>(Punctor::Semicolon)) {
    LOG(ERROR) << "Expected semicolon after import" << tok << std::endl;
    return false;
  }

  /*============== SEARCH FOR FILE IN INCLUDE DIRECTORIES ================*/
  LOG(DEBUG) << "Searching for file: {} in include directories [{}]" << filename
             << include_path << tok << std::endl;

  std::string filepath;
  for (auto &dir : m_include_dirs) {
    std::string path = dir + "/" + filename;

    if (!std::filesystem::exists(path)) {
      continue;
    }

    filepath = path;
    break;
  }

  /*============== CHECK IF FILE EXISTS ================*/
  if (filepath.empty()) {
    LOG(ERROR) << "Could not find file: \"{}\" in include directories [{}]"
               << filename << include_path << tok << std::endl;
    return false;
  }

  filepath = std::filesystem::absolute(filepath).string();

  /*============== CHECK IF FILE IS ALREADY INCLUDED ================*/
  if (m_already_included.contains(filepath)) {
    LOG(WARN) << core::feedback[PREP_DUPLICATE_IMPORT] << m_stack.top().path
              << filepath << tok << std::endl;
    return true;
  }

  /*============== CHECK FOR CIRCULAR DEPENDENCIES ================*/
  if (std::find(m_include_files.begin(), m_include_files.end(), filepath) !=
      m_include_files.end()) {
    std::string msg;
    for (auto &file : m_include_files) {
      msg += "  -> " + file + "\n";
    }

    msg += "  -> " + filepath + "\n";

    LOG(FAILED) << "Circular include detected: \n" << msg << tok << std::endl;
    LOG(ERROR) << "Refusing to enter infinite loop. Try to split your "
                  "dependencies into smaller files."
               << tok << std::endl;
    __builtin_unreachable();
  }

  /*============== OPEN FILE ================*/
  FILE *f;
  if (!(f = fopen(filepath.c_str(), "r"))) {
    LOG(ERROR) << "Could not open file: \"{}\" in include directories [{}]"
               << filepath << include_path << tok << std::endl;
    __builtin_unreachable();
  }

  /*============== ADD FILE TO ALREADY INCLUDED ================*/
  m_stack.top().already_included.insert(filepath);
  m_include_files.push_back(filepath);
  m_stack.push({clone(), filepath, f});
  job->m_filename.push(filepath);
  m_already_included.insert(filepath);

  if (!m_stack.top().scanner->set_source(f, filepath)) {
    LOG(ERROR) << "Failed to set source for file: " << filepath << tok
               << std::endl;
    __builtin_unreachable();
  }

  return true;
}

Token PrepEngine::read_token() {
  /*============== DO PREPROCESSING ================*/
  Token tok;

  while (true) {
    /*============== HANDLE BUFFERED TOKENS ================*/
    while (!m_buffer.empty()) {
      tok = m_buffer.front();
      m_buffer.pop();
      goto end;
    }

    /*============== EOF ================*/
    if (m_stack.empty()) {
      tok = Token(TT::Eof, "");
      goto end;
    }

    /*============== GET NEXT TOKEN FROM LOWER ORDER SCANNER ================*/
    tok = m_stack.top().scanner->next();

    if (m_expansion_enabled) {
      /*============== HANDLE POSSIBLE MACRO EXPANSION ================*/
      if (tok.type == TT::Identifier) {
        /*============== HANDLE STATIC MACRO EXPANSION ================*/
        if (m_statics->contains(tok.as<std::string>())) {
          std::string value;
          if (!get_static(tok.as<std::string>(), value)) {
            LOG(ERROR) << "Failed to get static value" << tok << std::endl;
            __builtin_unreachable();
          }

          /*============== RECURSIVE EXPANSION ================*/
          auto lexer = clone();
          lexer->set_source(value, "static");
          Token t;
          while ((t = lexer->next()).type != TT::Eof) {
            m_buffer.push(t);
          }

          continue;
        }
      }

      /*============== HANDLE DIRECT EXPANSION ================*/
      if (tok.type == TT::MacroSingleLine || tok.type == TT::MacroBlock) {
        /*============== HANDLE HEADER IMPORT ================*/
        if (tok.as<std::string>().starts_with("import")) {
          if (!handle_import(tok)) {
            LOG(ERROR) << "Failed to process import" << tok << std::endl;
            __builtin_unreachable();
          }
          continue;
        }

        /*============== HANDLE OTHER MACRO EXPANSION ================*/
        if (!parse_macro(tok)) {
          LOG(ERROR) << "Failed to expand macro" << tok << std::endl;
          __builtin_unreachable();
        }

        continue;
      }
    }

    /*============== HANDLE END OF N-TH ORDER SCANNER ================*/
    if (tok.type == TT::Eof) {
      if (m_stack.size() == 1) {
        goto end;
      }

      fclose(m_stack.top().file);
      m_include_files.pop_back();
      m_stack.pop();
      job->m_filename.pop();
      continue;
    }

    break;
  }

end:
  /*============== VERIFY USE DIRECTIVE IS PRESENT ================*/
  if (m_we_are_root && !job->version) {
    if (tok.type != TT::MacroSingleLine ||
        !tok.as<std::string>().starts_with("use ")) {
      LOG(ERROR) << "Language version must be specified before any other "
                    "directives or code. Example: `@use \"v1.0\";`"
                 << tok << std::endl;
    }
  }
  return tok;
}

///=============================================================================

struct libquixcc::QSysCallRegistry::Impl {
  std::map<uint32_t, QSysCall> m_syscalls;
};

libquixcc::QSysCallRegistry::QSysCallRegistry() { m_impl = new Impl(); }

libquixcc::QSysCallRegistry::~QSysCallRegistry() { delete m_impl; }

void libquixcc::QSysCallRegistry::Register(
    uint32_t num, libquixcc::QSysCallRegistry::QSysCall call) {
  m_impl->m_syscalls[num] = call;
}

bool libquixcc::QSysCallRegistry::Unregister(uint32_t num) {
  if (m_impl->m_syscalls.contains(num)) {
    m_impl->m_syscalls.erase(num);
    return true;
  }
  return false;
}

bool libquixcc::QSysCallRegistry::IsRegistered(uint32_t num) const {
  return m_impl->m_syscalls.contains(num);
}

std::vector<uint32_t> libquixcc::QSysCallRegistry::GetRegistered() const {
  std::vector<uint32_t> nums;
  for (auto &pair : m_impl->m_syscalls) {
    nums.push_back(pair.first);
  }
  return nums;
}

std::optional<libquixcc::QSysCallRegistry::QSysCall>
libquixcc::QSysCallRegistry::Get(uint32_t num) const {
  if (m_impl->m_syscalls.contains(num)) {
    return m_impl->m_syscalls.at(num);
  }
  return std::nullopt;
}

bool libquixcc::QSysCallRegistry::Call(
    quixcc_engine_t *handle, uint32_t num,
    libquixcc::QSysCallRegistry::QSysArgs args) {
  if (!m_impl->m_syscalls.contains(num)) {
    return false;
  }

  assert(args.size() <= UINT32_MAX);

  return m_impl->m_syscalls.at(num)(handle, num, args.data(), args.size());
}