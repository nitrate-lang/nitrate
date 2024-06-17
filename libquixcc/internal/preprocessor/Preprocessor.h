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

#ifndef __QUIXCC_PREP_PREPROCESS_H__
#define __QUIXCC_PREP_PREPROCESS_H__

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <lexer/Lex.h>
#include <quixcc.h>
#include <stdio.h>

#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

namespace libquixcc {
class PrepEngine : public libquixcc::Scanner {
  using rstr = const std::string &;

 protected:
  struct Entry {
    std::unique_ptr<Scanner> scanner;
    std::string path;
    std::set<std::string> already_included;
    FILE *file;

    Entry(std::unique_ptr<Scanner> l, rstr p, FILE *f = nullptr)
        : scanner(std::move(l)), path(p), file(f) {}
    Entry() : path(), file(nullptr) {}
  };
  std::set<std::string> m_include_dirs;
  std::set<std::string> m_already_included;  
  std::vector<std::string> m_include_files;  
  std::stack<Entry> m_stack;
  quixcc_job_t *job;
  std::string m_content;
  std::string include_path;
  std::optional<Token> m_tok;
  std::queue<Token> m_pushed;
  std::shared_ptr<std::map<std::string, std::string>> m_statics;
  std::queue<Token> m_buffer;
  FILE *m_file;
  bool m_we_own_file;
  bool m_we_are_root;

  libquixcc::Token read_token();
  void emit(const Token &tok);
  bool handle_import(const Token &tok);
  Entry build_statics_decl();
  bool parse_macro(const libquixcc::Token &macro);
  std::unique_ptr<PrepEngine> clone() const;

 public:
  PrepEngine(quixcc_job_t &job);
  virtual ~PrepEngine();

  /*================== PREPROCESSOR CONFIGURATION ==================*/
  void setup();
  void addpath(rstr path);
  bool set_source(FILE *src, rstr filename) override;
  void set_source(rstr src, rstr filename);

  /*================== PREPROCESSOR INTERFACE ==================*/
  Token next() override;
  const Token &peek() override;
  void push(Token tok) override;

  /*================== PREPROCESSOR DEFINES ==================*/
  void set_static(rstr name, rstr value) ;
  bool get_static(rstr name, std::string &value) const;

  /*================== MACRO PROCESSING ==================*/
  bool ParseAuthor(const Token &tok, rstr der, rstr param);
  bool ParseDefine(const Token &tok, rstr der, rstr param);
  bool ParseDescription(const Token &tok, rstr der, rstr param);
  bool ParseEncoding(const Token &tok, rstr der, rstr param);
  bool ParseLang(const Token &tok, rstr der, rstr param);
  bool ParseLicense(const Token &tok, rstr der, rstr param);
  bool ParseMacroP(const Token &tok, rstr der, rstr param);
  bool ParsePragma(const Token &tok, rstr der, rstr param);
  bool ParsePrint(const Token &tok, rstr der, rstr param);
  bool ParseReadstdin(const Token &tok, rstr der, rstr param);
  bool ParseUse(const Token &tok, rstr der, rstr param);
  bool ParseInvariant(const Token &tok, rstr der, rstr param);
};
}  // namespace libquixcc

#endif  // __QUIXCC_PREP_PREPROCESS_H__