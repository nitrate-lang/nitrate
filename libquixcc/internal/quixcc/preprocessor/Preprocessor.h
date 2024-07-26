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

#include <quixcc/Library.h>
#include <qast/Lexer.h>
#include <stdio.h>

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

namespace libquixcc {
  namespace ir {
    namespace q {
      class QModule;
    }
  } // namespace ir

  enum class MacroParamType {
    Int,
    UInt,
    Text,
    Bool,
    Real,
    Nil,
    List,
  };

  std::ostream &operator<<(std::ostream &os, MacroParamType type);

  struct MacroData {
    typedef std::tuple<std::string, MacroParamType, std::optional<std::string>> MacroArg;

    std::string luacode;
    std::vector<MacroArg> args;

    MacroData(const std::string &code = "", const std::vector<MacroArg> &args = {})
        : luacode(code), args(args) {}
  };

  /// @brief The Preprocessor Engine
  /// @warning Plugins must not interact with this class directly use the C Engine
  /// API instead.
  class PrepEngine : public libquixcc::Scanner {
    using rstr = const std::string &;
    typedef std::unordered_map<std::string, libquixcc::MacroData> MacroMap;

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
    quixcc_cc_job_t *job;
    std::string m_content;
    std::string include_path;
    std::optional<Token> m_tok;
    std::queue<Token> m_pushed;
    std::shared_ptr<std::map<std::string, std::string>> m_statics;
    std::queue<Token> m_buffer;
    std::shared_ptr<MacroMap> m_macros;

    FILE *m_file;
    bool m_we_own_file;
    bool m_we_are_root;
    bool m_expansion_enabled;
    bool m_emit_enabled;

    libquixcc::Token read_token();
    bool handle_import(const Token &tok);
    Entry build_statics_decl();
    bool parse_macro(const libquixcc::Token &macro);
    std::unique_ptr<PrepEngine> clone() const;

    bool expand_user_macro(const MacroData &fn_body,
                           const std::vector<std::pair<std::string, std::string>> &args);

public:
    PrepEngine(quixcc_cc_job_t &job);
    virtual ~PrepEngine();

    /*================== PREPROCESSOR CONFIGURATION ==================*/
    void setup();
    void addpath(rstr path);
    void set_include_path(rstr path);
    bool set_source(FILE *src, rstr filename) override;
    void set_source(rstr src, rstr filename);
    quixcc_cc_job_t *get_job() const;

    void set_expansion(bool enabled);
    void set_emit(bool enabled);

    /*================== NAMING IS HARD ==================*/
    std::shared_ptr<MacroMap> get_macros();

    /*================== PREPROCESSOR INTERFACE ==================*/
    Token next() override;
    const Token &peek() override;
    size_t offset() override;
    void push(Token tok) override;
    void emit(const Token &tok);
    void comments(bool ignore) override { throw std::runtime_error("Not implemented"); }

    /*================== PREPROCESSOR DEFINES ==================*/
    void set_static(rstr name, rstr value);
    bool get_static(rstr name, std::string &value) const;

    /*================== MACRO PROCESSING ==================*/
    bool ParseAuthor(const Token &tok, rstr der, rstr param);
    bool ParseDefine(const Token &tok, rstr der, rstr param);
    bool ParseDescription(const Token &tok, rstr der, rstr param);
    bool ParseEncoding(const Token &tok, rstr der, rstr param);
    bool ParseLang(const Token &tok, rstr der, rstr param);
    bool ParseLicense(const Token &tok, rstr der, rstr param);
    bool ParsePragma(const Token &tok, rstr der, rstr param);
    bool ParseUse(const Token &tok, rstr der, rstr param);
    bool ParseFn(const Token &tok, rstr der, rstr param);
  };

  class QSysCallRegistry {
    struct Impl;

    Impl *m_impl;

public:
    typedef const char *QSysArg;
    typedef std::vector<QSysArg> QSysArgs;
    typedef quixcc_qsys_impl_t QSysCall;

    QSysCallRegistry();
    ~QSysCallRegistry();

    void Register(uint32_t num, std::string name, QSysCall call);
    bool Unregister(uint32_t num);
    bool IsRegistered(uint32_t num) const;
    std::vector<uint32_t> GetRegistered() const;
    std::optional<QSysCall> Get(uint32_t num) const;
    std::string_view GetName(uint32_t num) const;

    std::string Call(quixcc_engine_t *handle, uint32_t num, QSysArgs args);
  };

} // namespace libquixcc

#endif // __QUIXCC_PREP_PREPROCESS_H__