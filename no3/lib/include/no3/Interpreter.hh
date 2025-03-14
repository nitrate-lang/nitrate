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

#ifndef NO3_INTERPRETER_HH
#define NO3_INTERPRETER_HH

#include <functional>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

namespace no3 {
  namespace detail {
    using LibraryDeinitializationCallback = std::function<void()>;

    class RCInitializationContext final {
      friend class No3LibraryInitialization;

      LibraryDeinitializationCallback m_on_deinit;
      bool m_active{true};

      RCInitializationContext(LibraryDeinitializationCallback on_deinit) noexcept;

    public:
      RCInitializationContext(const RCInitializationContext& o) noexcept;
      RCInitializationContext(RCInitializationContext&& o) noexcept;
      auto operator=(const RCInitializationContext& o) noexcept -> RCInitializationContext&;
      auto operator=(RCInitializationContext&& o) noexcept -> RCInitializationContext&;
      ~RCInitializationContext() noexcept;
    };
  }  // namespace detail

  /**
   * @brief Acquire RC access to this library.
   *
   * @param on_deinit A callback to be called when the library is deinitialized.
   *
   * @note Initialization is reference counted, therefore this library
   * will be deinitialized automatically when the returned handle is dropped.
   *
   * @return A handle keeping the library initialized, or nullptr if initialization failed.
   */
  auto OpenLibrary(
      std::ostream& init_log = std::cerr, detail::LibraryDeinitializationCallback on_deinit = nullptr) noexcept -> std::unique_ptr<detail::RCInitializationContext>;

  class Interpreter {
    class PImpl;
    std::unique_ptr<PImpl> m_impl;

  public:
    using OutputHandler = std::function<void(std::string_view)>;

    Interpreter(OutputHandler output_handler = [](std::string_view buffer) {
      std::cout.write(buffer.data(), buffer.size());
    }) noexcept;
    Interpreter(const Interpreter&) = delete;
    Interpreter(Interpreter&& o) noexcept = default;
    auto operator=(const Interpreter&) -> Interpreter& = delete;
    auto operator=(Interpreter&& o) noexcept -> Interpreter& = default;
    ~Interpreter() noexcept;

    auto Execute(const std::vector<std::string>& command) noexcept -> bool;
  };
}  // namespace no3

#endif
