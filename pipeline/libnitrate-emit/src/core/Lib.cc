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

#include <llvm-18/llvm/MC/TargetRegistry.h>
#include <llvm-18/llvm/Support/ManagedStatic.h>
#include <llvm-18/llvm/Support/TargetSelect.h>
#include <nitrate-emit/Lib.h>
#include <sys/resource.h>

#include <atomic>
#include <iostream>
#include <nitrate-core/Init.hh>
#include <nitrate-core/Macro.hh>

static std::atomic<size_t> qcode_lib_ref_count = 0;

static bool InitializeLLVM() {
#ifdef LLVM_SUUPORT_ALL_TARGETS
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  /* Check if LLVM is initialized */
  if (llvm::TargetRegistry::targets().empty()) {
    std::cerr << "error: LLVM initialization failed" << std::endl;
    return false;
  }
#else
#warning "Building LIBNITRATE without support for ANY LLVM targets!!"
#endif

  return true;
}

static void DoinitializeLLVM() { llvm::llvm_shutdown(); }

static bool do_init() {
  if (!InitializeLLVM()) {
    return false;
  }

  return true;
}

static void do_deinit() { DoinitializeLLVM(); }

C_EXPORT bool qcode_lib_init() {
  if (qcode_lib_ref_count++ > 1) {
    return true;
  }

  if (!ncc::core::Library::InitRC()) {
    return false;
  }

  return do_init();
}

C_EXPORT void qcode_lib_deinit() {
  if (--qcode_lib_ref_count > 0) {
    return;
  }

  do_deinit();

  ncc::core::Library::DeinitRC();
}

C_EXPORT const char* qcode_lib_version() {
  static const char* version_string =

      "[" __TARGET_VERSION
      "] ["

#if defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || \
    defined(_M_X64) || defined(_M_AMD64)
      "x86_64-"
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
      "x86-"
#elif defined(__aarch64__)
      "aarch64-"
#elif defined(__arm__)
      "arm-"
#else
      "unknown-"
#endif

#if defined(__linux__)
      "linux-"
#elif defined(__APPLE__)
      "macos-"
#elif defined(_WIN32)
      "win32-"
#else
      "unknown-"
#endif

#if defined(__clang__)
      "clang] "
#elif defined(__GNUC__)
      "gnu] "
#else
      "unknown] "
#endif

#if NDEBUG
      "[release]"
#else
      "[debug]"
#endif

      ;

  return version_string;
}

C_EXPORT const char* qcode_strerror() { return ""; }
