////////////////////////////////////////////////////////////////////////////////
///                                                                          ///
///     .-----------------.    .----------------.     .----------------.     ///
///    | .--------------. |   | .--------------. |   | .--------------. |    ///
///    | | ____  _____  | |   | |     ____     | |   | |    ______    | |    ///
///    | ||_   _|_   _| | |   | |   .'    `.   | |   | |   / ____ `.  | |    ///
///    | |  |   \ | |   | |   | |  /  .--.  \  | |   | |   `'  __) |  | |    ///
///    | |  | |\ \| |   | |   | |  | |    | |  | |   | |   _  |__ '.  | |    ///
///    | | _| |_\   |_  | |   | |  \  `--'  /  | |   | |  | \____) |  | |    ///
///    | ||_____|\____| | |   | |   `.____.'   | |   | |   \______.'  | |    ///
///    | |              | |   | |              | |   | |              | |    ///
///    | '--------------' |   | '--------------' |   | '--------------' |    ///
///     '----------------'     '----------------'     '----------------'     ///
///                                                                          ///
///   * NITRATE TOOLCHAIN - The official toolchain for the Nitrate language. ///
///   * Copyright (C) 2024 Wesley C. Jones                                   ///
///                                                                          ///
///   The Nitrate Toolchain is free software; you can redistribute it or     ///
///   modify it under the terms of the GNU Lesser General Public             ///
///   License as published by the Free Software Foundation; either           ///
///   version 2.1 of the License, or (at your option) any later version.     ///
///                                                                          ///
///   The Nitrate Toolcain is distributed in the hope that it will be        ///
///   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of ///
///   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      ///
///   Lesser General Public License for more details.                        ///
///                                                                          ///
///   You should have received a copy of the GNU Lesser General Public       ///
///   License along with the Nitrate Toolchain; if not, see                  ///
///   <https://www.gnu.org/licenses/>.                                       ///
///                                                                          ///
////////////////////////////////////////////////////////////////////////////////

#ifndef __NITRATE_IR_CLASSES_H__
#define __NITRATE_IR_CLASSES_H__

#ifndef __cplusplus
#error "This header is for C++ only."
#endif

#include <nitrate-ir/Config.h>
#include <nitrate-ir/IR.h>

#include <nitrate-core/Logger.hh>
#include <nitrate-ir/IRGraph.hh>
#include <optional>
#include <string>

class nr_conf final {
  nr_conf_t *m_conf;

public:
  nr_conf(bool use_default = true) {
    if ((m_conf = nr_conf_new(use_default)) == nullptr) {
      qcore_panic("nr_conf_new failed");
    }
  }
  ~nr_conf() { nr_conf_free(m_conf); }

  nr_conf_t *get() const { return m_conf; }
};

class qmodule final {
  qmodule_t *m_module;

public:
  qmodule() { m_module = nullptr; }
  ~qmodule() {
    if (m_module) {
      nr_free(m_module);
    }
  }

  qmodule_t *&get() { return m_module; }
};

namespace nr {
  class SymbolEncoding final {
  public:
    SymbolEncoding() = default;

    std::optional<std::string> mangle_name(const nr::Expr *symbol,
                                           AbiTag abi) const;

    std::optional<std::string> demangle_name(std::string_view symbol) const;
  };
}  // namespace nr

#endif  // __NITRATE_IR_CLASSES_H__
