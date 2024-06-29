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

#include <IR/Q/Expr.h>
#include <IR/Q/QIR.h>
#include <IR/Q/Variable.h>
#include <core/Logger.h>
#include <lexer/Lex.h>
#include <parsetree/Parser.h>
#include <preprocessor/Preprocessor.h>

#include <vector>

using namespace libquixcc;

bool PrepEngine::ParseQSys(const Token &tok, const std::string &directive,
                           const std::string &parameter) {
  (void)job;
  (void)tok;
  (void)directive;

  /// TODO: Implement the QSys call registry / runtime modules

  /* --> Infomation about the QSys directive:
   * The QUIX System directive hereafter referred to as QSys, is a
   * transcendental directive that serves as a gateway into the
   * preprocessor. The QSys directive works as a "system call" into
   * the compiler, enabling code to interact meta-morphically with
   * the toolchain during compilation.
   *
   * !!! **WARNING:** !!! QSYS IS NOT SECURE WITH RESPECT TO LOCAL
   * SYSTEM INTEGRETY. QSYS ALLOWES DIRECT UNRESTRICTED ACCESS TO
   * ALL SYSTEM RESOURCES AVAILABLE TO THE ACTIVE PROCESS.
   * IT IS NOT POSSIBLE TO MAKE THE QSYS DIRECTIVE SECURE, AS BY
   * ITS VERY NATURE, IT IS A WAY TO EXECUTE ARBITRARY CODE AND
   * READ/WRITE TO THE COMPILER/SYSTEM STATE.
   *
   * For example, the QSys directive can be used to:
   * - Read/Write the target architecture
   * - Add/Remove link libraries programmatically
   * - Override or set all compiler flags
   * - Read/Append to the compiler log
   * - Read/Write to users terminal (if available)
   * - Read/Write to the local filesystem (if available)
   * - Read/Write to the network (if available)
   * - Compile and execute arbitrary low-level code
   **/

  uint32_t num = 0;

  /* Preserve lifetime of QSys arguments */
  std::vector<std::unique_ptr<ir::q::QModule>> args_ptrs;
  QSysCallRegistry::QSysArgs args;

  {
    std::string code = "qsys(" + parameter + ");";
    auto lexer = clone();
    lexer->set_source(code, "<qsys>");

    std::shared_ptr<ExprNode> expr;
    if (!parse_expr(*job, lexer.get(), {Token(TT::Punctor, Punctor::Semicolon)},
                    expr)) {
      LOG(FAILED) << "Failed to parse QSys directive." << tok << std::endl;
      return false;
    }

    if (!expr->is<CallExprNode>()) {
      LOG(FAILED) << "QSys directive must be a meta-function call." << tok
                  << std::endl;
      return false;
    }

    auto call = expr->as<CallExprNode>();

    if (call->m_positional_args.size() == 0) {
      LOG(ERROR) << "QSys directive must have at least one argument." << tok
                 << std::endl;
      return false;
    }

    if (!call->m_positional_args[0]->is<IntegerNode>()) {
      LOG(ERROR) << "QSys directive first argument must be an integer." << tok
                 << std::endl;
      return false;
    }

    num = std::stoi(call->m_positional_args[0]->as<IntegerNode>()->m_val);

    for (size_t i = 1; i < call->m_positional_args.size(); i++) {
      auto qir_arg = std::make_unique<ir::q::QModule>(job->m_filename.top());

      if (!qir_arg->from_ptree(job, call->m_positional_args[i])) {
        LOG(ERROR) << "Failed to parse QSys directive argument." << tok
                   << std::endl;

        return false;
      }

      args_ptrs.push_back(std::move(qir_arg));
      args.push_back((QSysCallRegistry::QSysArg)args_ptrs.back().get());
    }

    for (auto [name, arg] : call->m_named_args) {
      LOG(ERROR) << "QSys directive does not support named arguments." << tok
                 << std::endl;
      return false;
    }
  }

  if (!job->m_qsyscalls.IsRegistered(num)) {
    LOG(FAILED) << "QSys Call \"" << num
                << "\" was attempted, but no such call or implementation "
                   "exists in this toolchain."
                << tok << std::endl;
    LOG(INFO) << "Ensure that your requested service is actually available "
                 "in the your language version."
              << tok << std::endl;
    LOG(ERROR) << "QSys call \"" << num << "\" failed." << tok << std::endl;
    return false;
  }

  return job->m_qsyscalls.Call((quixcc_engine_t *)this, num, std::move(args));
}