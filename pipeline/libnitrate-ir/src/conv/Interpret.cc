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

#include <nitrate-core/Error.h>
#include <nitrate-core/Macro.h>
#include <nitrate-ir/IR.h>
#include <nitrate-ir/TypeDecl.h>

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <nitrate-ir/IRGraph.hh>

using namespace nr;

namespace comptime {
  class Program {
    std::function<void(std::string_view)> m_eprintn_cb;
    std::optional<Expr *> m_result;

  public:
    Program(std::function<void(std::string_view)> cb) : m_eprintn_cb(cb) {}

    void eprintn(std::string_view message) { m_eprintn_cb(message); }

    void prepare(nr::Expr *E) {
      /// TODO: Verify and lower expression to LLVM-IR
      (void)E;
    }

    void emulate() {
      /// TODO: Emulate the LLVM-IR for comptime

      eprintn("Comptime evaluation is not supported yet");
    }

    std::optional<Expr *> get_result() { return m_result; }
  };
}  // namespace comptime

std::optional<nr::Expr *> nr::comptime_impl(
    nr::Expr *x,
    std::optional<std::function<void(std::string_view)>> eprintn) noexcept {
  comptime::Program P(eprintn.value_or([](std::string_view) {}));

  /**
   * 1. Treat the expression as seperate compilation unit
   * 2. Run semantic checks on it and its dependencies.
   * 3. Lower to LLVM-IR
   * 4. Emulate using LLVM
   * 5. Get result
   * 6. Convert the real data back into IRGraph node(s)
   */

  P.prepare(x);
  P.emulate();
  return P.get_result();
}
