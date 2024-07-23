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

#include <core/QuixJob.h>
#include <quixcc/core/Exception.h>
#include <quixcc/plugin/EngineAPI.h>
#include <quixcc/preprocessor/QSys.h>

QSYS_NOT_IMPLEMENTED(qsys_compile_and_execute_quix);
QSYS_NOT_IMPLEMENTED(qsys_get_type);
QSYS_NOT_IMPLEMENTED(qsys_undefine);
QSYS_NOT_IMPLEMENTED(qsys_get_source);

QSYS_NOT_IMPLEMENTED(qsys_set);

QSYS_DEFINE(qsys_get, "Get a value from the QSys") {
  QSYS_ARGASSERT(qsys_get, 1);

  QSYS_ARG_STRING(qsys_get, c_key, 0);

  std::string_view key_sv(c_key, c_key_len);

  PrepEngine *engine = (PrepEngine *)e;

  if (key_sv.empty()) {
    /* Return list of all defined macros */
    std::stringstream ss;
    for (auto it = engine->get_macros()->begin(); it != engine->get_macros()->end(); ++it) {
      ss << '@' << it->first;

      if (std::next(it) != engine->get_macros()->end()) {
        ss << ",";
      }
    }

    return strdup(ss.str().c_str());
  } else if (key_sv.starts_with("@")) {
    /* Get macro definition */
    auto it = std::find_if(engine->get_macros()->begin(), engine->get_macros()->end(),
                           [&key_sv](const auto &pair) { return pair.first == key_sv.substr(1); });

    if (it != engine->get_macros()->end()) {
      std::stringstream def;

      def << "fn " << it->first << "(";
      for (size_t i = 0; i < it->second.args.size(); i++) {
        def << std::get<0>(it->second.args[i]) << ": ";
        def << std::get<1>(it->second.args[i]);

        if (std::get<2>(it->second.args[i])) {
          def << " = " << std::get<2>(it->second.args[i]).value();
        }

        if (i < it->second.args.size() - 1) {
          def << ", ";
        }
      }

      def << ") {\n" << it->second.luacode << "\n}";

      return strdup(def.str().c_str());
    } else {
      quixcc_engine_message(e, QUIXCC_MESSAGE_ERROR, "Macro \"%s\" not found", c_key);
      return strdup("");
    }
  } else {
    quixcc_engine_message(e, QUIXCC_MESSAGE_ERROR, "Invalid QSys key \"%s\"", c_key);
    return strdup("");
  }
}

QSYS_DEFINE(qsys_abort, "Abort the compilation process") {
  QSYS_ARGASSERT(qsys_abort, 1);

  QSYS_ARG_STRING(qsys_abort, message, 0);
  (void)message_len;

  throw libquixcc::core::PreprocessorException(message);
}

QSYS_DEFINE(qsys_set_emit, "Toggle emitting of preprocessor tokens") {
  QSYS_ARGASSERT(qsys_set_emit, 1);
  QSYS_ARG_INT64(qsys_set_emit, enabled, 0);

  PrepEngine *engine = (PrepEngine *)e;

  engine->set_emit(enabled);

  return strdup("");
}
